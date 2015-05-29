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

static bool native_document_run(JSContext *cx, unsigned argc, JS::Value *vp);
static void Document_Finalize(JSFreeOp *fop, JSObject *obj);

static JSPropertySpec document_props[] = {
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static bool native_document_showfps(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_document_setPasteBuffer(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_document_getPasteBuffer(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_document_loadFont(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_document_getElementById(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_document_getScreenData(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_document_parseNML(JSContext *cx, unsigned argc, JS::Value *vp);

static JSClass document_class = {
    "NativeDocument", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
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

static bool native_document_parseNML(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString str(cx);
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

static bool native_document_getElementById(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString str(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "S",
        &str)) {
        return false;
    }

    JSAutoByteString cid(cx, str.get());

    NativeCanvasHandler *elem = NativeContext::getNativeClass(cx)->getCanvasById(cid.ptr());

    args.rval().set(elem ? OBJECT_TO_JSVAL(elem->jsobj) : JSVAL_NULL);

    return true;
}

static bool native_document_getScreenData(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    NativeCanvasHandler *rootHandler = NativeContext::getNativeClass(cx)->getRootHandler();
    NativeCanvas2DContext *context = (NativeCanvas2DContext *)rootHandler->getContext();

    int width, height;
    context->getSize(&width, &height);

    JS::RootedObject arrBuffer(cx, JS_NewUint8ClampedArray(cx, width * height * 4));
    uint8_t *pixels = JS_GetUint8ClampedArrayData(arrBuffer.get());


    NativeContext *nctx = NativeContext::getNativeClass(cx);

    uint8_t *fb = nctx->getUI()->readScreenPixel();

    memcpy(pixels, fb, width * height * 4);

    //NativeSkia *skia = context->getSurface();
    //skia->readPixels(0, 0, width, height, pixels);
    //glReadPixels(0, 0, NUII->getWidth(), NUII->getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, NUII->getFrameBufferData());
    //glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    JS::RootedObject dataObject(cx, JS_NewObject(cx,  NativeCanvas2DContext::ImageData_jsclass, nullptr, nullptr));

    JS_DefineProperty(cx, dataObject.get(), "width", UINT_TO_JSVAL(width), nullptr, nullptr,
        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS_DefineProperty(cx, dataObject.get(), "height", UINT_TO_JSVAL(height), nullptr, nullptr,
        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS_DefineProperty(cx, dataObject.get(), "data", OBJECT_TO_JSVAL(arrBuffer.get()), nullptr,
        nullptr, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    args.rval().setObject(dataObject.get());

    return true;
}

static bool native_document_setPasteBuffer(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString str(cx);
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

static bool native_document_getPasteBuffer(JSContext *cx, unsigned argc, JS::Value *vp)
{
    using namespace js;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    char *text = NativeContext::getNativeClass(cx)->getUI()->getClipboardText();

    if (text == NULL) {
        args.rval().set(JSVAL_NULL);
        return true;
    }

    size_t len = strlen(text)*2;
    jschar *jsc = new jschar[len];
    //@FIXME: js:: is private -> use JS::
    js::InflateUTF8StringToBufferReplaceInvalid(cx, text, strlen(text), jsc, &len);

    JS::RootedString jret(cx, JS_NewUCStringCopyN(cx, jsc, len));
    args.rval().set(STRING_TO_JSVAL(jret.get()));

    free(text);
    delete[] jsc;

    return true;
}

static bool native_document_showfps(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    bool show = false;

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "b", &show)) {
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

static bool native_document_run(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedString location(cx);

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "S", &location)) {
        return false;
    }

    NativeUIInterface *NUI = NativeContext::getNativeClass(cx)->getUI();
    JSAutoByteString locationstr(cx, location.get());

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
    JS::RootedValue ret(cx);
    if (!NativeJS::LoadScriptReturn(cx, data, len, filename, &ret.get())) {
        return false;
    }
    JS::RootedObject jret(cx, ret.toObjectOrNull());

    NativeJS::copyProperties(cx, jret.get(), this->stylesheet);

    return true;
}

JSObject *NativeJSdocument::registerObject(JSContext *cx)
{
    JS::RootedObject documentObj(cx, JS_DefineObject(cx, JS_GetGlobalObject(cx),
        NativeJSdocument::getJSObjectName(), &document_class , nullptr,
        JSPROP_PERMANENT | JSPROP_ENUMERATE));

    NativeJS *njs = NativeJS::getNativeClass(cx);

    NativeJSdocument *jdoc = new NativeJSdocument(documentObj.get(), cx);

    JS_SetPrivate(documentObj.get(), jdoc);

    /* We have to root it since the user can replace the document object */
    njs->rootObjectUntilShutdown(documentObj.get());
    njs->jsobjects.set(NativeJSdocument::getJSObjectName(), documentObj.get());

    jdoc->stylesheet = JS_NewObject(cx, nullptr, nullptr, nullptr);
    JS::RootedValue objV(cx, OBJECT_TO_JSVAL(jdoc->stylesheet));
    JS_SetProperty(cx, documentObj.get(), "stylesheet", &objV.get());
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

static bool native_document_loadFont(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS_INITOPT();
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject thisobj(cx, NativeJSdocument::getJSGlobalObject(cx));
    NativeJSdocument *CppObj = (NativeJSdocument *)JS_GetPrivate(thisobj.get());

    JS::RootedObject fontdef(cx);

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

