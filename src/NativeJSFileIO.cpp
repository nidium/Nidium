#include "NativeJSFileIO.h"
#include <native_netlib.h>

static JSClass File_class = {
    "File", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSBool native_file_open(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_getContents(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_file_read(JSContext *cx, unsigned argc, jsval *vp);

static JSFunctionSpec File_funcs[] = {
    JS_FN("open", native_file_open, 1, 0),
    JS_FN("getContents", native_file_getContents, 1, 0),
    JS_FN("read", native_file_read, 2, 0),
    JS_FS_END
};

static JSBool native_File_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *url;
    NativeFileIO *NFIO;
    NativeJSFileIO *NJSFIO;

    JSObject *ret = JS_NewObjectForConstructor(cx, &File_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &url)) {
        return JS_TRUE;
    }

    JSAutoByteString curl(cx, url);

    NJSFIO = new NativeJSFileIO();
    NFIO = new NativeFileIO(curl.ptr(), NJSFIO,
        (ape_global *)JS_GetContextPrivate(cx));


    NJSFIO->jsobj = ret;
    NJSFIO->cx = cx;

    NJSFIO->setNFIO(NFIO);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));
    JS_DefineFunctions(cx, ret, File_funcs);

    JS_SetPrivate(ret, NJSFIO);

    return JS_TRUE;
}

static JSBool native_file_getContents(JSContext *cx, unsigned argc, jsval *vp)
{
    jsval callback;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    NativeJSFileIO *NJSFIO;
    NativeFileIO *NFIO;

    if (JS_InstanceOf(cx, caller, &File_class, JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[0], JSTYPE_FUNCTION, &callback)) {
        return JS_TRUE;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);
    NFIO = NJSFIO->getNFIO();

    if (NFIO->fd == NULL) {
        JS_ReportError(cx, "NativeFileIO : use File.open() first.");
        return JS_FALSE;
    }

    NJSFIO->callbacks.getContents = callback;

    JS_AddValueRoot(cx, &NJSFIO->callbacks.getContents);

    NFIO->getContents();    

    return JS_TRUE;
}

static JSBool native_file_read(JSContext *cx, unsigned argc, jsval *vp)
{
    jsval callback;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    NativeJSFileIO *NJSFIO;
    NativeFileIO *NFIO;
    double read_size;

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

    JS_AddValueRoot(cx, &NJSFIO->callbacks.read);

    NFIO->read((uint64_t)read_size);  

    return JS_TRUE;
}

static JSBool native_file_open(JSContext *cx, unsigned argc, jsval *vp)
{
    jsval callback;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    NativeJSFileIO *NJSFIO;
    NativeFileIO *NFIO;

    if (JS_InstanceOf(cx, caller, &File_class, JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[0], JSTYPE_FUNCTION, &callback)) {
        return JS_TRUE;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    NFIO = NJSFIO->getNFIO();

    NJSFIO->callbacks.open = callback;

    JS_AddValueRoot(cx, &NJSFIO->callbacks.open);

    NFIO->open();

    return JS_TRUE;
}

void NativeJSFileIO::onNFIOOpen(NativeFileIO *NSFIO)
{
    jsval rval;
    NativeJSFileIO *NJSFIO = static_cast<NativeJSFileIO *>(NSFIO->getDelegate());

    JS_CallFunctionValue(cx, NJSFIO->jsobj, NJSFIO->callbacks.open,
        0, NULL, &rval);

    JS_RemoveValueRoot(cx, &NJSFIO->callbacks.open);
}

void NativeJSFileIO::onNFIOError(NativeFileIO *NSFIO, int errno)
{
    jsval rval;
    NativeJSFileIO *NJSFIO = static_cast<NativeJSFileIO *>(NSFIO->getDelegate());

    /*JS_CallFunctionValue(cx, NJSFIO->jsobj, NJSFIO->callbacks.open,
        0, NULL, &rval);*/

    JS_RemoveValueRoot(cx, &NJSFIO->callbacks.open);
}

void NativeJSFileIO::onNFIORead(NativeFileIO *NSFIO, unsigned char *data, size_t len)
{
    jsval rval;
    jsval jdata;

    NativeJSFileIO *NJSFIO = static_cast<NativeJSFileIO *>(NSFIO->getDelegate());

    printf("build abuffer of %ld\n", len);

    JSObject *arrayBuffer = JS_NewArrayBuffer(cx, len);
    uint8_t *adata = JS_GetArrayBufferData(arrayBuffer);
    memcpy(adata, data, len);

    jdata = OBJECT_TO_JSVAL(arrayBuffer);

    JS_CallFunctionValue(cx, NJSFIO->jsobj, NJSFIO->callbacks.read,
        1, &jdata, &rval);

    JS_RemoveValueRoot(cx, &NJSFIO->callbacks.read);

}

void NativeJSFileIO::registerObject(JSContext *cx)
{
    JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, &File_class,
        native_File_constructor,
        0, NULL, NULL, NULL, NULL);
}
