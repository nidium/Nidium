#include "NativeJSDocument.h"
#include "NativeJS.h"
#include "NativeContext.h"
#include "NativeUIInterface.h"
#include <native_netlib.h>
#include <jsapi.h>
#include <jsstr.h>
#include <NativeStreamInterface.h>
#include <SkTypeface.h>
#include <SkStream.h>
#include <NativeCanvasHandler.h>
#include <NativeNML.h>
#include "NativeSkia.h"

#include "NativeCanvas2DContext.h"

bool NativeJSdocument::showFPS = false;

static JSBool native_document_run(JSContext *cx, unsigned argc, jsval *vp);
static void Document_Finalize(JSFreeOp *fop, JSObject *obj);

static JSPropertySpec document_props[] = {
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSBool native_document_showfps(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_document_setPasteBuffer(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_document_getPasteBuffer(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_document_loadFont(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_document_getElementById(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_document_getScreenData(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_document_parseNML(JSContext *cx, unsigned argc, jsval *vp);

static JSClass document_class = {
    "NativeDocument", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Document_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

JSClass *NativeJSdocument::jsclass = &document_class;

template<>
JSClass *NativeJSExposer<NativeJSdocument>::jsclass = &document_class;

static JSFunctionSpec document_funcs[] = {
    JS_FN("run", native_document_run, 1, 0),
    JS_FN("showFPS", native_document_showfps, 1, 0),
    JS_FN("setPasteBuffer", native_document_setPasteBuffer, 1, 0),
    JS_FN("getPasteBuffer", native_document_getPasteBuffer, 0, 0),
    JS_FN("loadFont", native_document_loadFont, 1, JSPROP_ENUMERATE),
    JS_FN("getCanvasById", native_document_getElementById, 1, JSPROP_ENUMERATE),
    JS_FN("getScreenData", native_document_getScreenData, 0, 0),
    JS_FN("parseNML", native_document_parseNML, 1, 0),
    JS_FS_END
};

struct _native_document_restart_async
{
    NativeUIInterface *ui;
    char *location;
};

static JSBool native_document_parseNML(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *str;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "S",
        &str)) {
        return false;
    }

    JSAutoByteString cstr;
    cstr.encodeUtf8(cx, str);

    args.rval().setObjectOrNull(NativeNML::BuildLST(cx, cstr.ptr()));

    return true;
}

static JSBool native_document_getElementById(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *str;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "S",
        &str)) {
        return false;
    }

    JSAutoByteString cid(cx, str);

    NativeCanvasHandler *elem = NativeContext::getNativeClass(cx)->getCanvasById(cid.ptr());

    args.rval().set(elem ? OBJECT_TO_JSVAL(elem->jsobj) : JSVAL_NULL);

    return true;
}

static JSBool native_document_getScreenData(JSContext *cx, unsigned argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    NativeCanvasHandler *rootHandler = NativeContext::getNativeClass(cx)->getRootHandler();
    NativeCanvas2DContext *context = (NativeCanvas2DContext *)rootHandler->getContext();

    int width, height;
    context->getSize(&width, &height);

    JSObject *arrBuffer = JS_NewUint8ClampedArray(cx, width * height * 4);
    uint8_t *pixels = JS_GetUint8ClampedArrayData(arrBuffer);


    NativeContext *nctx = NativeContext::getNativeClass(cx);

    uint8_t *fb = nctx->getUI()->readScreenPixel();

    memcpy(pixels, fb, width * height * 4);
    
    //NativeSkia *skia = context->getSurface();
    //skia->readPixels(0, 0, width, height, pixels);
    //glReadPixels(0, 0, NUII->getWidth(), NUII->getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, NUII->getFrameBufferData());
    //glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    JSObject *dataObject = JS_NewObject(cx,  NativeCanvas2DContext::ImageData_jsclass, NULL, NULL);

    JS_DefineProperty(cx, dataObject, "width", UINT_TO_JSVAL(width), NULL, NULL,
        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS_DefineProperty(cx, dataObject, "height", UINT_TO_JSVAL(height), NULL, NULL,
        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS_DefineProperty(cx, dataObject, "data", OBJECT_TO_JSVAL(arrBuffer), NULL,
        NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    args.rval().setObject(*dataObject);

    return true;
}

static JSBool native_document_setPasteBuffer(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *str;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "S",
        &str)) {
        return false;
    }

    char *text = JS_EncodeStringToUTF8(cx, str);

    NativeContext::getNativeClass(cx)->getUI()->setClipboardText(text);

    js_free(text);

    return true;
}

static JSBool native_document_getPasteBuffer(JSContext *cx, unsigned argc, jsval *vp)
{
    using namespace js;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    char *text = NativeContext::getNativeClass(cx)->getUI()->getClipboardText();

    if (text == NULL) {
        args.rval().setNull();
        return true;
    }

    size_t len = strlen(text)*2;
    jschar *jsc = new jschar[len];
    js::InflateUTF8StringToBufferReplaceInvalid(cx, text, strlen(text), jsc, &len);

    JSString *jret = JS_NewUCStringCopyN(cx, jsc, len);

    args.rval().set(STRING_TO_JSVAL(jret));

    free(text);
    delete[] jsc;

    return true;
}

static JSBool native_document_showfps(JSContext *cx, unsigned argc, jsval *vp)
{
    JSBool show = false;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "b", &show)) {
        return false;
    }

    NativeJSdocument::showFPS = show;

    if (show) {
        NativeContext::getNativeClass(cx)->createDebugCanvas();
    }

    return true;
}


static int native_document_restart(void *param)
{
    struct _native_document_restart_async *ndra = (struct _native_document_restart_async *)param;

    ndra->ui->restartApplication(ndra->location);

    free(ndra->location);
    free(ndra);

    return 0;
}

static JSBool native_document_run(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *location;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &location)) {
        return false;
    }

    NativeUIInterface *NUI = NativeContext::getNativeClass(cx)->getUI();
    JSAutoByteString locationstr(cx, location);

    struct _native_document_restart_async *ndra = (struct _native_document_restart_async *)malloc(sizeof(*ndra));

    ndra->location = strdup(locationstr.ptr());
    ndra->ui = NUI;
    ape_global *ape = NativeJS::getNativeClass(cx)->net;

    timer_dispatch_async(native_document_restart, ndra);

    return true;
}

static void Document_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSdocument *jdoc = NativeJSdocument::getNativeClass(obj);

    if (jdoc != NULL) {
        delete jdoc;
    }
}

bool NativeJSdocument::populateStyle(JSContext *cx, const char *data,
    size_t len, const char *filename)
{
    JS::Value ret;
    if (!NativeJS::LoadScriptReturn(cx, data, len, filename, &ret)) {
        return false;
    }
    JSObject *jret = ret.toObjectOrNull();


    NativeJS::copyProperties(cx, jret, this->stylesheet);

    return true;
}

JSObject *NativeJSdocument::registerObject(JSContext *cx)
{
    JSObject *documentObj;
    
    NativeJS *njs = NativeJS::getNativeClass(cx);

    documentObj = JS_DefineObject(cx, JS_GetGlobalObject(cx),
        NativeJSdocument::getJSObjectName(),
        &document_class , NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE);

    NativeJSdocument *jdoc = new NativeJSdocument(documentObj, cx);

    JS_SetPrivate(documentObj, jdoc);

    /* We have to root it since the user can replace the document object */
    njs->rootObjectUntilShutdown(documentObj);
    njs->jsobjects.set(NativeJSdocument::getJSObjectName(), documentObj);

    jdoc->stylesheet = JS_NewObject(cx, NULL, NULL, NULL);
    jsval obj = OBJECT_TO_JSVAL(jdoc->stylesheet);
    JS_SetProperty(cx, documentObj, "stylesheet", &obj);
    JS_DefineFunctions(cx, documentObj, document_funcs);
    JS_DefineProperties(cx, documentObj, document_props);

    return documentObj;
}

// todo destroy with NativeHash cleaner
bool NativeJSdocument::loadFont(const char *path, const char *name,
    int weight, nativefont::Style style)
{
    NativeBaseStream *stream = NativeBaseStream::create(path);
    if (!stream) {
        return false;
    }

    NativePtrAutoDelete<NativeBaseStream *> npad(stream);

    char *data;
    size_t len;

    if (!stream->getContentSync(&data, &len)) {
        return false;
    }

    SkMemoryStream *skmemory = new SkMemoryStream(data, len, true);
    free(data);

    SkTypeface *tf = SkTypeface::CreateFromStream(skmemory);
    if (tf == NULL) {
        delete skmemory;
        return false;
    }

    nativefont *oldfont = m_Fonts.get(name);

    nativefont *newfont = new nativefont();
    newfont->weight = weight;
    newfont->style = style;
    newfont->typeface = tf;

    newfont->next = oldfont;

    m_Fonts.set(name, newfont);

    return true;
}

static JSBool native_document_loadFont(JSContext *cx, unsigned argc, jsval *vp)
{
    JS_INITOPT();
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject *thisobj = NativeJSdocument::getJSGlobalObject(cx);
    NativeJSdocument *CppObj = (NativeJSdocument *)JS_GetPrivate(thisobj);

    JSObject *fontdef;


    if (!JS_ConvertArguments(cx, args.length(), args.array(), "o", &fontdef)) {
        return false;
    }

    JSAutoByteString cfile, cname;

    JSGET_OPT_TYPE(fontdef, "file", String) {
        cfile.encodeUtf8(cx, __curopt.toString());
    } else {
        JS_ReportError(cx, "Missing 'file' (string) value");
        return false;
    }

    JSGET_OPT_TYPE(fontdef, "name", String) {
        cname.encodeLatin1(cx, __curopt.toString());
    } else {
        JS_ReportError(cx, "Missing 'name' (string) value");
        return false;
    }

    char *pTmp = cname.ptr();

    while (*pTmp != '\0') {
        *pTmp = tolower(*pTmp);
        pTmp++;
    }

    NativePath fpath(cfile.ptr());

    args.rval().setBoolean(CppObj->loadFont(fpath.path(), cname.ptr()));

    return true;
}

SkTypeface *NativeJSdocument::getFont(char *name)
{
    char *pTmp = name;

    while (*pTmp != '\0') {
        *pTmp = tolower(*pTmp);
        pTmp++;
    }

    nativefont *font = m_Fonts.get(name);
    if (font) {
        return font->typeface;
    }

    return NULL;
}
