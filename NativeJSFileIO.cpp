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

// sync API should have a different constructor (new FileSync)

#include "NativeJSFileIO.h"
#include <native_netlib.h>
#include <NativePath.h>
#include <NativeStreamInterface.h>

#include "NativeJSUtils.h"

enum {
    FILE_PROP_FILESIZE,
    FILE_PROP_FILENAME,
    FILE_PROP_BINARY,
    FILE_PROP_ASYNC
};

class NativeJSFileAsyncReader : public NativeMessages
{
public:
    void onMessage(const NativeSharedMessages::Message &msg)
    {
        jsval params[2], rval;

        params[0].setNull();
        params[1].setUndefined();
        JSContext *cx = (JSContext *)m_Args[0].toPtr();
        NativeBaseStream *stream = (NativeBaseStream *)m_Args[2].toPtr();
        JSObject *callback = (JSObject *)m_Args[7].toPtr();

        char *encoding = (char *)m_Args[1].toPtr();

        switch (msg.event()) {
            case NATIVESTREAM_ERROR: {
                NativeBaseStream::StreamErrors err = 
                (NativeBaseStream::StreamErrors)msg.args[0].toInt();
                printf("Got an error :'( %d\n", err);
                params[0].setString(JS_NewStringCopyZ(cx, "Stream error"));
            }
                break;
            case NATIVESTREAM_READ_BUFFER:
            {
                jsval ret;
                buffer *buf = (buffer *)msg.args[0].toPtr();
                if (NativeJSUtils::strToJsval(cx, (const char *)buf->data,
                    buf->used, &ret, encoding)) {
                    
                    params[1] = ret;
                }
                break;
            }
            default:
                return;
        }

        if (JS_ObjectIsCallable(cx, callback)) {

            JSAutoRequest ar(cx); // TODO: Why do we need a request here?
            JS_CallFunctionValue(cx, NULL, OBJECT_TO_JSVAL(callback),
                2, params, &rval);
        }

        NativeJS::getNativeClass(cx)->unrootObject(callback);        

        stream->setListener(NULL);

        free(encoding);

        delete stream;
        delete this;
    }

    NativeArgs m_Args;
};

#define GET_OPT(name) if (opt != NULL && JS_GetProperty(cx, opt, name, &curopt) && curopt != JSVAL_VOID && curopt != JSVAL_NULL)

#define NJSFIO_GETTER(obj) ((class NativeJSFileIO *)JS_GetPrivate(obj))

static void File_Finalize(JSFreeOp *fop, JSObject *obj);
static JSBool native_file_prop_get(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSMutableHandleValue vp);
static JSBool native_file_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp);

JSClass File_class = {
    "File", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, File_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSBool native_file_open(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_openSync(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_read(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_seek(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_close(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_closeSync(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_write(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_isDir(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_listFiles(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_rmrf(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_file_readFileSync(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_readFile(JSContext *cx, unsigned argc, jsval *vp);

static JSPropertySpec File_props[] = {
    {"filesize", FILE_PROP_FILESIZE, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_file_prop_get),
        JSOP_NULLWRAPPER},
    {"filename", FILE_PROP_FILENAME, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_file_prop_get),
        JSOP_NULLWRAPPER},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSFunctionSpec File_funcs[] = {
    JS_FN("open", native_file_open, 1, 0),
    JS_FN("openSync", native_file_openSync, 1, 0),
    JS_FN("read", native_file_read, 2, 0),
    JS_FN("seek", native_file_seek, 2, 0),
    JS_FN("close", native_file_close, 0, 0),
    JS_FN("closeSync", native_file_closeSync, 0, 0),
    JS_FN("write", native_file_write, 1, 0),
    JS_FN("isDir", native_file_isDir, 0, 0),
    JS_FN("listFiles", native_file_listFiles, 1, 0),
    JS_FN("rmrf", native_file_rmrf, 0, 0),
    JS_FS_END
};

static JSFunctionSpec File_static_funcs[] = {
    JS_FN("readSync", native_file_readFileSync, 1, 0),
    JS_FN("read",     native_file_readFile, 2, 0),
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

    // currently no props
    return true;    
}

static JSBool native_File_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *url;
    NativeFile *file;
    NativeJSFileIO *NJSFIO;
    JSObject *opt = NULL;
    jsval curopt;

    if (!JS_IsConstructing(cx, vp)) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }    

    JSObject *ret = JS_NewObjectForConstructor(cx, &File_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S/o", &url, &opt)) {
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

    GET_OPT("encoding") {
        JSAutoByteString encoding(cx, curopt.toString());
        NJSFIO->m_Encoding = strdup(encoding.ptr());
    }

    NJSFIO->jsobj = ret;
    NJSFIO->cx = cx;

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

    NATIVE_CHECK_ARGS("write", 2);

    if (!JS_ConvertValue(cx, args[1], JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "write() bad callback");
        return false;
    }

    file = NJSFIO->getFile();
    
    if (!file->isOpen()) {
        file->open("a+");
    }

    if (args[0].isString()) {
        //printf("got a string to write\n");
        JSString *str = args[0].toString();
        JSAutoByteString cstr(cx, str);
        size_t len = strlen(cstr.ptr());

        NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

        file->write(cstr.ptr(), len, callback.toObjectOrNull());

    } else if (args[0].isObject()) {
        JSObject *jsobj = args[0].toObjectOrNull();

        if (jsobj == NULL || !JS_IsArrayBufferObject(jsobj)) {
            JS_ReportError(cx, "NATIVE_INVALID_VALUE : only accept string or ArrayBuffer");
            return false;
        }
        uint32_t len = JS_GetArrayBufferByteLength(jsobj);
        uint8_t *data = JS_GetArrayBufferData(jsobj);

        NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

        file->write((char *)data, len, callback.toObjectOrNull());

    } else {
        JS_ReportError(cx, "NATIVE_INVALID_VALUE : only accept string or ArrayBuffer");
        return false;        
    }

    return true;
}

static JSBool native_file_isDir(JSContext *cx, unsigned argc, jsval *vp)
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

    args.rval().setBoolean(file->isDir());
    printf("Catching %s %d\n", file->getFullPath(), file->isDir());

    return true;
}

static JSBool native_file_rmrf(JSContext *cx, unsigned argc, jsval *vp)
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

    file->rmrf();

    return true;
}

static JSBool native_file_listFiles(JSContext *cx, unsigned argc, jsval *vp)
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

    NATIVE_CHECK_ARGS("listFiles", 1);

    if (!JS_ConvertValue(cx, args[0], JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "listFiles() bad callback");
        return false;
    }

    file = NJSFIO->getFile();

    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

    /*
        If the directory is not open, open it anyway
    */
    if (!file->isOpen()) {
        file->open("r");
    }
    file->listFiles(callback.toObjectOrNull());

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

    NATIVE_CHECK_ARGS("read", 2);

    if (!JS_ConvertValue(cx, args[1], JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "read() bad callback");
        return false;
    }

    file = NJSFIO->getFile();

    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

    if (!file->isOpen()) {
        file->open("r");
    }
    file->read((uint64_t)read_size, callback.toObjectOrNull());

    return true;
}

static JSBool native_file_seek(JSContext *cx, unsigned argc, jsval *vp)
{
    jsval callback;
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

    NATIVE_CHECK_ARGS("seek", 2);

    if (!JS_ConvertValue(cx, args[1], JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "seek() bad callback");
        return false;
    }

    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    file = NJSFIO->getFile();

    file->seek(seek_pos, callback.toObjectOrNull());

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

static JSBool native_file_closeSync(JSContext *cx, unsigned argc, jsval *vp)
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

    file->closeSync();

    return true;
}

static JSBool native_file_readFileSync(JSContext *cx, unsigned argc, jsval *vp)
{
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
    JSAutoByteString encoding;

    GET_OPT("encoding") {
        encoding.encodeLatin1(cx, curopt.toString());
        cencoding = encoding.ptr();
    }
    
    jsval ret;

    if (!NativeJSUtils::strToJsval(cx, buf, len, &ret, cencoding)) {
        return false;
    }

    args.rval().set(ret);

    return true;
}

static JSBool native_file_readFile(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *opt = NULL;
    JSObject *secondarg;
    JSString *filename;
    jsval argcallback, callback;
    jsval curopt;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    char *cencoding = NULL;

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "So", &filename, &secondarg)) {
        return false;
    }

    if (JS_TypeOfValue(cx, args[1]) != JSTYPE_FUNCTION) {
        NATIVE_CHECK_ARGS("readFile", 3);
        opt = args[1].toObjectOrNull();
        argcallback = args[2];
    } else {
        argcallback = args[1];
    }

    if (!JS_ConvertValue(cx, argcallback, JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "readFile() invalid callback");
        return false;
    }

    JSAutoByteString cfilename(cx, filename);
    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

    NativeBaseStream *stream = NativeBaseStream::create(NativePath(cfilename.ptr()));

    if (!stream) {
        JS_ReportError(cx, "couldn't open stream");
        return false;
    }

    GET_OPT("encoding") {
        JSAutoByteString encoding(cx, curopt.toString());
        cencoding = strdup(encoding.ptr());
    }

    NativeJSFileAsyncReader *async = new NativeJSFileAsyncReader();
    async->m_Args[0].set(cx);
    async->m_Args[1].set(cencoding);
    async->m_Args[2].set(stream);
    async->m_Args[7].set(callback.toObjectOrNull());

    stream->setListener(async);
    stream->getContent();
    
    return true;
}

bool NativeJSFileIO::handleError(JSContext *cx, const NativeSharedMessages::Message &msg,
    jsval &vals)
{
    switch (msg.event()) {
        case NATIVEFILE_OPEN_ERROR:
            // todo : replace by error object
            vals.setString(JS_NewStringCopyZ(cx, "Open error"));
            break;
        case NATIVEFILE_SEEK_ERROR:
            vals.setString(JS_NewStringCopyZ(cx, "Seek error"));
            break;
        case NATIVEFILE_READ_ERROR:
            vals.setString(JS_NewStringCopyZ(cx, "read error"));
            break;
        case NATIVEFILE_WRITE_ERROR:
            vals.setString(JS_NewStringCopyZ(cx, "write error"));
            break;
        default:
            vals.setNull(); // no error
            return false;
    }

    return true;
}

static const char *NativeJSFileIO_dirtype_to_str(const dirent *entry)
{
    switch (entry->d_type) {
        case DT_DIR:
            return "dir";
        case DT_REG:
            return "file";
        case DT_LNK:
            return "link";
        case DT_SOCK:
            return "socket";
        default:
            return "unknown";
    }
}

bool NativeJSFileIO::callbackForMessage(JSContext *cx,
    const NativeSharedMessages::Message &msg, JSObject *thisobj,
    const char *encoding)
{
    JS::Value rval, params[2];

    params[1].setUndefined();

    if (!NativeJSFileIO::handleError(cx, msg, params[0])) {
        switch (msg.event()) {
            case NATIVEFILE_READ_SUCCESS:
            {
                buffer *buf = (buffer *)msg.args[0].toPtr();
                NativeJSUtils::strToJsval(cx, (const char *)buf->data,
                    buf->used, &params[1], encoding);
                break;
            }
            case NATIVEFILE_WRITE_SUCCESS:
                params[1] = DOUBLE_TO_JSVAL(msg.args[0].toInt64());
                break;
            case NATIVEFILE_LISTFILES_ENTRIES:
            {
                NativeFile::DirEntries *entries = (NativeFile::DirEntries *)msg.args[0].toPtr();
                JSObject *arr = JS_NewArrayObject(cx, entries->size, NULL);

                for (int i = 0; i < entries->size; i++) {
                    JSObject *entry = JS_NewObject(cx, NULL, NULL, NULL);
                    JSOBJ_SET_PROP_STR(entry, "name",
                        JS_NewStringCopyZ(cx, entries->lst[i].d_name));

                    JSOBJ_SET_PROP_CSTR(entry, "type",
                        NativeJSFileIO_dirtype_to_str(&entries->lst[i]));

                    jsval val = OBJECT_TO_JSVAL(entry);
                    JS_SetElement(cx, arr, i, &val);

                }

                params[1] = OBJECT_TO_JSVAL(arr);
            }
        }
    }
    JSObject *callback = (JSObject *)msg.args[7].toPtr();

    if (!callback) {
        return false;
    }

    if (JS_ObjectIsCallable(cx, callback)) {

        JSAutoRequest ar(cx); // TODO: Why do we need a request here?
        JS_CallFunctionValue(cx, thisobj, OBJECT_TO_JSVAL(callback),
            2, params, &rval);
    }

    NativeJS::getNativeClass(cx)->unrootObject(callback);

    return true;
}

void NativeJSFileIO::onMessage(const NativeSharedMessages::Message &msg)
{
    NativeJSFileIO::callbackForMessage(cx, msg, jsobj, m_Encoding);
}

#if 0

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

    NJSFIO->setFile(file);

    JS_DefineFunctions(cx, ret, File_funcs);
    JS_DefineProperties(cx, ret, File_props);   

    JS_SetPrivate(ret, NJSFIO);

    return ret;
}

NativeFile *NativeJSFileIO::GetFileFromJSObject(JSContext *cx, JSObject *jsobj)
{
    NativeJSFileIO *nfio = (NativeJSFileIO *)JS_GetInstancePrivate(cx, jsobj,
        &File_class, NULL);

    return nfio ? nfio->getFile() : NULL;
}
