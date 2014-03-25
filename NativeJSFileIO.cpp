/*
    NativeJS Core Library
    Copyright (C) 2013 Anthony Catel <paraboul@gmail.com>
    Copyright (C) 2013 Nicolas Trani <n.trani@weelya.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "NativeJSFileIO.h"
#include <native_netlib.h>
#include <NativePath.h>
#include <NativeStreamInterface.h>

#include <string>
#include <jsstr.h>

enum {
    FILE_PROP_FILESIZE,
    FILE_PROP_FILENAME,
    FILE_PROP_BINARY,
    FILE_PROP_ASYNC
};

#define NJSFIO_GETTER(obj) ((class NativeJSFileIO *)JS_GetPrivate(obj))

static void File_Finalize(JSFreeOp *fop, JSObject *obj);
static JSBool native_file_prop_get(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSMutableHandleValue vp);
static JSBool native_file_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp);

static JSClass File_class = {
    "File", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, File_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSBool native_file_open(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_openSync(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_read(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_seek(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_rewind(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_close(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_write(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_file_readFileSync(JSContext *cx, unsigned argc, jsval *vp);

static JSPropertySpec File_props[] = {
    {"filesize", FILE_PROP_FILESIZE, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_file_prop_get),
        JSOP_NULLWRAPPER},
    {"filename", FILE_PROP_FILENAME, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_file_prop_get),
        JSOP_NULLWRAPPER},
    {"binary", FILE_PROP_BINARY, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_file_prop_get),
        JSOP_WRAPPER(native_file_prop_set)},
    {"async", FILE_PROP_ASYNC, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_file_prop_get),
        JSOP_WRAPPER(native_file_prop_set)},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSFunctionSpec File_funcs[] = {
    JS_FN("open", native_file_open, 1, 0),
    JS_FN("openSync", native_file_openSync, 1, 0),
    JS_FN("read", native_file_read, 1, 0),
    JS_FN("seek", native_file_seek, 1, 0),
    JS_FN("rewind", native_file_rewind, 0, 0),
    JS_FN("close", native_file_close, 0, 0),
    JS_FN("write", native_file_write, 1, 0),
    JS_FS_END
};

static JSFunctionSpec File_static_funcs[] = {
    JS_FN("readFileSync", native_file_readFileSync, 1, 0),
    JS_FS_END
};


static void File_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSFileIO *NJSFIO = (NativeJSFileIO *)JS_GetPrivate(obj);
    if (NJSFIO != NULL) {
        NativeFile *file = NJSFIO->getFile();
        delete file;
        delete NJSFIO;
    }
}

static JSBool native_file_prop_get(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSMutableHandleValue vp)
{
    NativeFile *file = NJSFIO_GETTER(obj.get())->getFile();

    switch(JSID_TO_INT(id)) {
        case FILE_PROP_FILESIZE:
            vp.set(JS_NumberValue(file->getFileSize()));
            break;
        case FILE_PROP_FILENAME:
            vp.set(STRING_TO_JSVAL(JS_NewStringCopyZ(cx, file->getFullPath())));
            break;
        case FILE_PROP_BINARY:
            vp.set(BOOLEAN_TO_JSVAL(NJSFIO_GETTER(obj.get())->m_Binary));
            break;
        case FILE_PROP_ASYNC:
            vp.set(BOOLEAN_TO_JSVAL(NJSFIO_GETTER(obj.get())->m_Async));
            break;
        default:break;

    }

    return JS_TRUE;
}

static JSBool native_file_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
    NativeJSFileIO *nfio = NJSFIO_GETTER(obj.get());

    if (nfio == NULL) {
        return true;
    }

    switch(JSID_TO_INT(id)) {
        case FILE_PROP_BINARY:
        {
            if (JSVAL_IS_BOOLEAN(vp)) {
                nfio->m_Binary = JSVAL_TO_BOOLEAN(vp);
            }        
        }
        break;
        case FILE_PROP_ASYNC:
        {
            if (JSVAL_IS_BOOLEAN(vp)) {
                nfio->m_Async = JSVAL_TO_BOOLEAN(vp);
            }        
        }
        break;    
        default:
            break;
    }
    return true;    
}

static JSBool native_File_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *url;
    NativeFile *file;
    NativeJSFileIO *NJSFIO;
    JSBool binary = JS_TRUE;

    JSObject *ret = JS_NewObjectForConstructor(cx, &File_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S/b", &url, &binary)) {
        return false;
    }

    JSAutoByteString curl(cx, url);
    NativePath path(curl.ptr());

    if (!path.path()) {
        JS_ReportError(cx, "NativeFileIO : Invalid file path");
        return false;
    }

    NJSFIO = new NativeJSFileIO();
    file = new NativeFile(path.path());
    file->setListener(NJSFIO);

    NJSFIO->jsobj = ret;
    NJSFIO->cx = cx;
    NJSFIO->m_Binary = binary;

    NJSFIO->setFile(file);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));
    JS_DefineFunctions(cx, ret, File_funcs);
    JS_DefineProperties(cx, ret, File_props);   

    JS_SetPrivate(ret, NJSFIO);

    return JS_TRUE;
}

static JSBool native_file_write(JSContext *cx, unsigned argc, jsval *vp)
{
    jsval callback;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    NativeJSFileIO *NJSFIO;
    NativeFile *file;

    if (JS_InstanceOf(cx, caller, &File_class, args.array()) == JS_FALSE) {
        return false;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    NATIVE_CHECK_ARGS("write", (NJSFIO->m_Async ? 2 : 1));

    if (NJSFIO->m_Async &&
        !JS_ConvertValue(cx, args[1], JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "write() bad callback");
        return false;
    }

    file = NJSFIO->getFile();

    if (args[0].isString()) {
        //printf("got a string to write\n");
        JSString *str = args[0].toString();
        JSAutoByteString cstr(cx, str);
        size_t len = strlen(cstr.ptr());

        if (NJSFIO->m_Async) {
            char *dupstr = strndup(cstr.ptr(), len);

            NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

            file->write(dupstr, len, callback.toObjectOrNull());
        } else {
            int err;
            int ret = file->writeSync(cstr.ptr(), len, &err);

            if (err != 0 || ret == -1) {
                JS_ReportError(cx, "write() error %d : %s", err, strerror(err));
                return false;
            }

            args.rval().setInt32(ret);
        }

    } else if (args[0].isObject()) {
        JSObject *jsobj = args[0].toObjectOrNull();

        if (jsobj == NULL || !JS_IsArrayBufferObject(jsobj)) {
            JS_ReportError(cx, "NATIVE_INVALID_VALUE : only accept string or ArrayBuffer");
            return false;
        }
        uint32_t len = JS_GetArrayBufferByteLength(jsobj);
        uint8_t *data = JS_GetArrayBufferData(jsobj);

        if (NJSFIO->m_Async) {
            char *cdata = (char *)malloc(sizeof(char) * len);

            //printf("Got an arraybuffer of size : %d\n", len);

            memcpy(cdata, data, len);

            NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

            file->write(cdata, len, callback.toObjectOrNull());
        } else {
            int err;
            int ret = file->writeSync((char *)data, len, &err);

            if (err != 0 || ret == -1) {
                JS_ReportError(cx, "write() error %d : %s", err, strerror(err));
                return false;
            }

            args.rval().setInt32(ret);            
        }
    } else {
        JS_ReportError(cx, "NATIVE_INVALID_VALUE : only accept string or ArrayBuffer");
        return false;        
    }

    return true;
}

static JSBool native_file_read(JSContext *cx, unsigned argc, jsval *vp)
{
    jsval callback;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    NativeJSFileIO *NJSFIO;
    NativeFile *file;
    double read_size;

    if (JS_InstanceOf(cx, caller, &File_class, args.array()) == JS_FALSE) {
        return false;
    }

    if (!JS_ConvertArguments(cx, 1, args.array(), "d", &read_size)) {
        return false;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    NATIVE_CHECK_ARGS("read", (NJSFIO->m_Async ? 2 : 1));

    if (NJSFIO->m_Async && !JS_ConvertValue(cx, args[1], JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "read() bad callback");
        return false;
    }

    file = NJSFIO->getFile();

    if (NJSFIO->m_Async) {

        NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

        file->read((uint64_t)read_size, callback.toObjectOrNull());
    } else {
        char *data;
        int err;

        ssize_t ret = file->readSync((uint64_t)read_size, &data, &err);

        if (ret > 0) {
            if (NJSFIO->m_Binary) {
                jsval jdata;

                JSObject *arrayBuffer = JS_NewArrayBuffer(cx, ret);
                if (arrayBuffer == NULL) {
                    args.rval().setNull();
                } else {
                    uint8_t *adata = JS_GetArrayBufferData(arrayBuffer);
                    memcpy(adata, data, ret);

                    args.rval().setObjectOrNull(arrayBuffer);
                }
            } else {
                args.rval().setString(JS_NewStringCopyN(cx, (char *)data, ret));
            }
            free(data);
        } else if (ret < 0) {
            JS_ReportError(cx, "read() error %d : %s", err, strerror(err));
            return false;
        } else {
            args.rval().setNull();
        }
    }
    return true;
}

static JSBool native_file_seek(JSContext *cx, unsigned argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    NativeJSFileIO *NJSFIO;
    NativeFile *file;
    double seek_pos;

    if (JS_InstanceOf(cx, caller, &File_class, args.array()) == JS_FALSE) {
        return false;
    }

    if (!JS_ConvertArguments(cx, 1, args.array(), "d", &seek_pos)) {
        return false;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    file = NJSFIO->getFile();

    file->seek(seek_pos);    

    return true;
}

static JSBool native_file_rewind(JSContext *cx, unsigned argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    NativeJSFileIO *NJSFIO;
    NativeFile *file;

    if (JS_InstanceOf(cx, caller, &File_class, args.array()) == JS_FALSE) {
        return false;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    file = NJSFIO->getFile();

    file->seek(0);    

    return true;
}

static JSBool native_file_close(JSContext *cx, unsigned argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    NativeJSFileIO *NJSFIO;
    NativeFile *file;

    if (JS_InstanceOf(cx, caller, &File_class, args.array()) == JS_FALSE) {
        return false;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    file = NJSFIO->getFile();

    file->close();    

    return JS_TRUE;
}

static JSBool native_file_open(JSContext *cx, unsigned argc, jsval *vp)
{
    jsval callback;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    NativeJSFileIO *NJSFIO;
    NativeFile *file;
    JSString *modes;

    NATIVE_CHECK_ARGS("open", 2);

    if (JS_InstanceOf(cx, caller, &File_class, args.array()) == JS_FALSE) {
        return false;
    }

    if (!JS_ConvertArguments(cx, 1, args.array(), "S", &modes)) {
        return false;
    }

    if (!JS_ConvertValue(cx, args[1], JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "open() invalid callback");
        return false;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    file = NJSFIO->getFile();

    JSAutoByteString cmodes(cx, modes);

    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());
    file->open(cmodes.ptr(), callback.toObjectOrNull());

    return true;
}

static JSBool native_file_openSync(JSContext *cx, unsigned argc, jsval *vp)
{
    return true;
}

static JSBool native_file_readSync(JSContext *cx, unsigned argc, jsval *vp)
{
    return true;
}

static JSBool native_file_writeSync(JSContext *cx, unsigned argc, jsval *vp)
{
    return true;
}

static JSBool native_file_readFileSync(JSContext *cx, unsigned argc, jsval *vp)
{
#define GET_OPT(name) if (opt != NULL && JS_GetProperty(cx, opt, name, &curopt) && curopt != JSVAL_VOID && curopt != JSVAL_NULL)
    JSString *filename;
    JSObject *opt = NULL;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    jsval curopt;
    char *buf;
    size_t len;
    bool isRawBuffer = true;

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "S/o", &filename, &opt)) {
        return false;
    }

    JSAutoByteString cfilename(cx, filename);
    NativePath path(cfilename.ptr());

    if (!path.getScheme()->allowSyncStream()) {
        JS_ReportError(cx, "can't open this file for sync read");
        return false;
    }

    NativePtrAutoDelete<NativeBaseStream *> stream(path.createStream());

    if (!stream.ptr() || !stream.ptr()->getContentSync(&buf, &len)) {
        args.rval().setNull();
        printf("couldnt read the file\n");
        return true;
    }

    NativePtrAutoDelete<char *> cbuf(buf, free);
    char *cencoding = NULL;

    GET_OPT("encoding") {
        JSAutoByteString encoding(cx, curopt.toString());
        cencoding = encoding.ptr();
    }

    jsval ret;

    if (!NativeJSFileIO::dataOutput(cx, buf, len, &ret, cencoding)) {
        return false;
    }

    args.rval().set(ret);

    return true;
#undef GET_OPT
}

void NativeJSFileIO::onMessage(const NativeSharedMessages::Message &msg)
{
    JS::Value rval, jdata[8], params[9];

    int jcount = 0;

    params[0].setNull();

    switch (msg.event()) {
        case NATIVEFILE_OPEN_SUCCESS:
            /* do nothing */
            break;
        case NATIVEFILE_OPEN_ERROR:
            // todo : replace by error object
            params[0].setString(JS_NewStringCopyZ(cx, "Open error"));
            break;
        case NATIVEFILE_SEEK_ERROR:
            params[0].setString(JS_NewStringCopyZ(cx, "Seek error"));
            break;
        case NATIVEFILE_SEEK_SUCCESS:
            break;
        case NATIVEFILE_READ_ERROR:
            params[0].setString(JS_NewStringCopyZ(cx, "read error"));
            break;
        case NATIVEFILE_READ_SUCCESS:
        {
            buffer *buf = (buffer *)msg.args[0].toPtr();
            this->onRead(buf, jdata, &jcount);
            break;
        }
        case NATIVEFILE_WRITE_ERROR:
            params[0].setString(JS_NewStringCopyZ(cx, "write error"));
            break;
        case NATIVEFILE_WRITE_SUCCESS:
            break;
        case NATIVEFILE_CLOSE_SUCCESS:
            // TODO: dont do anything if finalizing...
            break;
    }

    JSObject *callback = (JSObject *)msg.args[7].toPtr();

    if (!callback) {
        return;
    }

    if (JS_ObjectIsCallable(cx, callback)) {

        for (int i = 0; i < jcount; i++) {
            params[i+1] = jdata[i];
        }

        JSAutoRequest ar(cx); // TODO: Why do we need a request here?
        JS_CallFunctionValue(cx, jsobj, OBJECT_TO_JSVAL(callback),
            jcount+1, params, &rval);
    }

    NativeJS::getNativeClass(cx)->unrootObject(callback);
}

void NativeJSFileIO::onRead(buffer *buf, jsval *vals, int *nvals)
{
    *nvals = 1;

    if (m_Binary) {
        JSObject *arrayBuffer = JS_NewArrayBuffer(cx, buf->used);
        if (arrayBuffer == NULL) {
            vals[0] = JSVAL_NULL;
        } else {
            uint8_t *adata = JS_GetArrayBufferData(arrayBuffer);
            memcpy(adata, buf->data, buf->used);

            vals[0] = OBJECT_TO_JSVAL(arrayBuffer);
        }
    } else {
        JSString *str = JS_NewStringCopyN(cx, (const char*)buf->data, buf->used);
        if (str == NULL) {
            vals[0] = JSVAL_NULL;
        } else {
            vals[0] = STRING_TO_JSVAL(str);
        }
    }    
}

#if 0
void NativeJSFileIO::onNFIOOpen(NativeFileIO *NSFIO)
{
    jsval rval;
    NativeJSFileIO *NJSFIO = static_cast<NativeJSFileIO *>(NSFIO->getDelegate());

    JSAutoRequest ar(cx);

    JS_CallFunctionValue(cx, NJSFIO->jsobj, NJSFIO->callbacks.open,
        0, NULL, &rval);

    NativeJS::getNativeClass(cx)->unrootObject(JSVAL_TO_OBJECT(NJSFIO->callbacks.open));
}

void NativeJSFileIO::onNFIOError(NativeFileIO *NSFIO, int errno)
{
    //jsval rval;
    NativeJSFileIO *NJSFIO = static_cast<NativeJSFileIO *>(NSFIO->getDelegate());

    JSAutoRequest ar(cx);

    /*JS_CallFunctionValue(cx, NJSFIO->jsobj, NJSFIO->callbacks.open,
        0, NULL, &rval);*/

    NativeJS::getNativeClass(cx)->unrootObject(JSVAL_TO_OBJECT(NJSFIO->callbacks.open));
}

void NativeJSFileIO::onNFIOWrite(NativeFileIO *NSFIO, size_t written)
{
    jsval rval;
    jsval jdata;
    NativeJSFileIO *NJSFIO = static_cast<NativeJSFileIO *>(NSFIO->getDelegate());

    JSAutoRequest ar(cx);

    jdata = DOUBLE_TO_JSVAL((double)written);

    JS_CallFunctionValue(cx, NJSFIO->jsobj, NJSFIO->callbacks.write,
        1, &jdata, &rval);

    NativeJS::getNativeClass(cx)->unrootObject(JSVAL_TO_OBJECT(NJSFIO->callbacks.write));
}

void NativeJSFileIO::onNFIORead(NativeFileIO *NSFIO, unsigned char *data, size_t len)
{
    jsval rval;
    jsval jdata;

    NativeJSFileIO *NJSFIO = static_cast<NativeJSFileIO *>(NSFIO->getDelegate());

    JSAutoRequest ar(cx);

    if (NJSFIO->m_Binary) {
        JSObject *arrayBuffer = JS_NewArrayBuffer(cx, len);
        if (arrayBuffer == NULL) {
            jdata = JSVAL_NULL;
        } else {
            uint8_t *adata = JS_GetArrayBufferData(arrayBuffer);
            memcpy(adata, data, len);

            jdata = OBJECT_TO_JSVAL(arrayBuffer);
        }
    } else {
        JSString *str = JS_NewStringCopyN(cx, (const char*)data, len);
        if (str == NULL) {
            jdata = JSVAL_NULL;
        } else {
            jdata = STRING_TO_JSVAL(str);
        }
    }

    JS_CallFunctionValue(cx, NJSFIO->jsobj, NJSFIO->callbacks.read,
        1, &jdata, &rval);

    NativeJS::getNativeClass(cx)->unrootObject(JSVAL_TO_OBJECT(NJSFIO->callbacks.read));
}
#endif

void NativeJSFileIO::registerObject(JSContext *cx)
{
    JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, &File_class,
        native_File_constructor,
        0, NULL, NULL, NULL, File_static_funcs);
}

JSObject *NativeJSFileIO::generateJSObject(JSContext *cx, const char *path)
{
    JSObject *ret;

    ret = JS_NewObject(cx, &File_class, NULL, NULL);
    NativeFile *file;
    NativeJSFileIO *NJSFIO;

    NJSFIO = new NativeJSFileIO();
    file = new NativeFile(path);
    file->setListener(NJSFIO);

    NJSFIO->jsobj = ret;
    NJSFIO->cx = cx;
    NJSFIO->m_Binary = true;

    NJSFIO->setFile(file);

    JS_DefineFunctions(cx, ret, File_funcs);
    JS_DefineProperties(cx, ret, File_props);   

    JS_SetPrivate(ret, NJSFIO);

    return ret;
}

bool NativeJSFileIO::dataOutput(JSContext *cx, const char *buf, size_t len, jsval *ret,
    const char *encoding)
{
    *ret = JSVAL_NULL;

    if (encoding) {
        if (strcasecmp(encoding, "utf8") == 0) {
            size_t jlen = 0;

            NativePtrAutoDelete<jschar *> content(NativeUtils::Utf8ToUtf16(buf, len, &jlen));

            if (content.ptr() == NULL) {
                JS_ReportError(cx, "Could not decode string to utf8");
                return false;
            }

            JSString *str = JS_NewUCString(cx, content.ptr(), jlen);
            if (!str) {
                return false;
            }

            content.disable(); /* JS_NewUCString took ownership */
            *ret = STRING_TO_JSVAL(str);

        } else {
            *ret = STRING_TO_JSVAL(JS_NewStringCopyN(cx, buf, len));
        }
    } else {
        JSObject *arrayBuffer = JS_NewArrayBuffer(cx, len);

        if (arrayBuffer == NULL) {
            JS_ReportOutOfMemory(cx);
            return false;
        } else {
            uint8_t *adata = JS_GetArrayBufferData(arrayBuffer);
            memcpy(adata, buf, len);

            *ret = OBJECT_TO_JSVAL(arrayBuffer);
        }        
    }

    return true;
}