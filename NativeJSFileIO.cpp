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
#include "NativeStream.h"

enum {
    FILE_PROP_FILESIZE
};

#define NJSFIO_GETTER(obj) ((class NativeJSFileIO *)JS_GetPrivate(obj))

static void File_Finalize(JSFreeOp *fop, JSObject *obj);
static JSBool native_file_prop_get(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSMutableHandleValue vp);

static JSClass File_class = {
    "File", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, File_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSBool native_file_open(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_read(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_seek(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_rewind(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_close(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_write(JSContext *cx, unsigned argc, jsval *vp);

static JSPropertySpec File_props[] = {
    {"filesize", FILE_PROP_FILESIZE, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_file_prop_get),
        JSOP_NULLWRAPPER},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSFunctionSpec File_funcs[] = {
    JS_FN("open", native_file_open, 2, 0),
    JS_FN("read", native_file_read, 2, 0),
    JS_FN("seek", native_file_seek, 1, 0),
    JS_FN("rewind", native_file_rewind, 0, 0),
    JS_FN("close", native_file_close, 0, 0),
    JS_FN("write", native_file_write, 2, 0),
    JS_FS_END
};

static void File_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSFileIO *NJSFIO = (NativeJSFileIO *)JS_GetPrivate(obj);
    if (NJSFIO != NULL) {
        NativeFileIO *NFIO = NJSFIO->getNFIO();
        delete NFIO;
        delete NJSFIO;
    }
}

static JSBool native_file_prop_get(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSMutableHandleValue vp)
{
    NativeFileIO *NFIO = NJSFIO_GETTER(obj.get())->getNFIO();

    switch(JSID_TO_INT(id)) {
        case FILE_PROP_FILESIZE:
            vp.set(JS_NumberValue(NFIO->filesize));
            break;
        default:break;

    }

    return JS_TRUE;
}

static JSBool native_File_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *url;
    NativeFileIO *NFIO;
    NativeJSFileIO *NJSFIO;
    bool binary;

    JSObject *ret = JS_NewObjectForConstructor(cx, &File_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &url)) {
        return false;
    }

    // Passing "S/b" to JS_ConvertArguments crash (no crash in debug build)
    // Here is a quick workaround for that issue
    if (argc > 1) {
        JS::Value val = JS_ARGV(cx, vp)[1];
        if (val.isBoolean()) {
            binary = val.toBoolean();
        } else {
            binary = true;
        }
    }

    JSAutoByteString curl(cx, url);

    int flen = 0;

    if (NativeStream::typeInterface(curl.ptr(), &flen) != NativeStream::INTERFACE_UNKNOWN) {
        JS_ReportError(cx, "NativeFileIO : Invalid file path");
        return false;
    }

    NJSFIO = new NativeJSFileIO();
    NFIO = new NativeFileIO(curl.ptr(), NJSFIO,
        (ape_global *)JS_GetContextPrivate(cx), NativeJS::getNativeClass(cx)->getPath());

    NJSFIO->jsobj = ret;
    NJSFIO->cx = cx;
    NJSFIO->m_Binary = binary;

    NJSFIO->setNFIO(NFIO);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));
    JS_DefineFunctions(cx, ret, File_funcs);
    JS_DefineProperties(cx, ret, File_props);   

    JS_SetPrivate(ret, NJSFIO);

    return JS_TRUE;
}

static JSBool native_file_write(JSContext *cx, unsigned argc, jsval *vp)
{
    jsval callback;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    NativeJSFileIO *NJSFIO;
    NativeFileIO *NFIO;

    NATIVE_CHECK_ARGS("write", 2);

    if (JS_InstanceOf(cx, caller, &File_class, JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[1], JSTYPE_FUNCTION, &callback)) {
        return JS_TRUE;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);
    NFIO = NJSFIO->getNFIO();

    if (NFIO->fd == NULL) {
        JS_ReportError(cx, "NativeFileIO : use File.open() first.");
        return JS_FALSE;
    }

    if (JSVAL_IS_STRING(JS_ARGV(cx, vp)[0])) {
        //printf("got a string to write\n");
        JSString *str = JS_ValueToString(cx, JS_ARGV(cx, vp)[0]);
        JSAutoByteString cstr(cx, str);
        size_t len = strlen(cstr.ptr());
        char *dupstr = strndup(cstr.ptr(), len);

        NJSFIO->callbacks.write = callback;

        NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(JSVAL_TO_OBJECT(callback));

        NFIO->write((unsigned char *)dupstr, len);

    } else if (!JSVAL_IS_PRIMITIVE(JS_ARGV(cx, vp)[0])) {
        JSObject *jsobj = JSVAL_TO_OBJECT(JS_ARGV(cx, vp)[0]);

        if (!JS_IsArrayBufferObject(jsobj)) {
            JS_ReportError(cx, "NATIVE_INVALID_VALUE : only accept string or ArrayBuffer");
            return JS_FALSE;
        }
        uint32_t len = JS_GetArrayBufferByteLength(jsobj);
        uint8_t *data = JS_GetArrayBufferData(jsobj);
        uint8_t *cdata = (uint8_t *)malloc(sizeof(char) * len);

        //printf("Got an arraybuffer of size : %d\n", len);

        memcpy(cdata, data, len);

        NJSFIO->callbacks.write = callback;

        NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(JSVAL_TO_OBJECT(callback));

        NFIO->write(cdata, len);

    } else {
        JS_ReportError(cx, "NATIVE_INVALID_VALUE : only accept string or ArrayBuffer");
        return JS_FALSE;        
    }

    return JS_TRUE;
}

static JSBool native_file_read(JSContext *cx, unsigned argc, jsval *vp)
{
    jsval callback;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    NativeJSFileIO *NJSFIO;
    NativeFileIO *NFIO;
    double read_size;

    NATIVE_CHECK_ARGS("read", 2);

    if (JS_InstanceOf(cx, caller, &File_class, JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }

    if (!JS_ConvertArguments(cx, 1, JS_ARGV(cx, vp), "d", &read_size)) {
        return JS_TRUE;
    }

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[1], JSTYPE_FUNCTION, &callback)) {
        return JS_TRUE;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);
    NFIO = NJSFIO->getNFIO();

    if (NFIO->fd == NULL) {
        JS_ReportError(cx, "NativeFileIO : use File.open() first.");
        return JS_FALSE;
    }

    NJSFIO->callbacks.read = callback;

    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(JSVAL_TO_OBJECT(callback));

    NFIO->read((uint64_t)read_size);  

    return JS_TRUE;
}

static JSBool native_file_seek(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    NativeJSFileIO *NJSFIO;
    NativeFileIO *NFIO;
    double seek_pos;

    if (JS_InstanceOf(cx, caller, &File_class, JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }

    if (!JS_ConvertArguments(cx, 1, JS_ARGV(cx, vp), "d", &seek_pos)) {
        return JS_TRUE;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    NFIO = NJSFIO->getNFIO();

    NFIO->seek(seek_pos);    

    return JS_TRUE;
}

static JSBool native_file_rewind(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    NativeJSFileIO *NJSFIO;
    NativeFileIO *NFIO;

    if (JS_InstanceOf(cx, caller, &File_class, JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    NFIO = NJSFIO->getNFIO();

    NFIO->seek(0);    

    return JS_TRUE;
}

static JSBool native_file_close(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    NativeJSFileIO *NJSFIO;
    NativeFileIO *NFIO;

    if (JS_InstanceOf(cx, caller, &File_class, JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    NFIO = NJSFIO->getNFIO();

    NFIO->close();    

    return JS_TRUE;
}

static JSBool native_file_open(JSContext *cx, unsigned argc, jsval *vp)
{
    jsval callback;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    NativeJSFileIO *NJSFIO;
    NativeFileIO *NFIO;
    JSString *modes;

    NATIVE_CHECK_ARGS("open", 2);

    if (JS_InstanceOf(cx, caller, &File_class, JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }

    if (!JS_ConvertArguments(cx, 1, JS_ARGV(cx, vp), "S", &modes)) {
        return JS_TRUE;
    }

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[1], JSTYPE_FUNCTION, &callback)) {
        return JS_TRUE;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    NFIO = NJSFIO->getNFIO();

    NJSFIO->callbacks.open = callback;

    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(JSVAL_TO_OBJECT(callback));

    JSAutoByteString cmodes(cx, modes);

    NFIO->open(cmodes.ptr());

    return JS_TRUE;
}

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
        uint8_t *adata = JS_GetArrayBufferData(arrayBuffer);
        memcpy(adata, data, len);

        jdata = OBJECT_TO_JSVAL(arrayBuffer);
    } else {
        JSString *str = JS_NewStringCopyN(cx, (const char*)data, len);
        jdata = STRING_TO_JSVAL(str);
    }

    JS_CallFunctionValue(cx, NJSFIO->jsobj, NJSFIO->callbacks.read,
        1, &jdata, &rval);

    NativeJS::getNativeClass(cx)->unrootObject(JSVAL_TO_OBJECT(NJSFIO->callbacks.read));
}

void NativeJSFileIO::registerObject(JSContext *cx)
{
    JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, &File_class,
        native_File_constructor,
        0, NULL, NULL, NULL, NULL);
}
