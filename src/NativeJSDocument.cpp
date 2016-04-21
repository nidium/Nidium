#include "NativeJSDocument.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include <SkTypeface.h>
#include <SkStream.h>
#include <SkData.h>

#include "NativeNML.h"
#include "NativeCanvasHandler.h"
#include "NativeCanvas2DContext.h"
#include "NativeSkImage.h"

#include "Binding/JSUtils.h"


bool NativeJSdocument::m_ShowFPS = false;

static bool native_document_run(JSContext *cx, unsigned argc, JS::Value *vp);
static void Document_Finalize(JSFreeOp *fop, JSObject *obj);

static JSPropertySpec document_props[] = {
    JS_PS_END
};

static bool native_document_showfps(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_document_setPasteBuffer(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_document_getPasteBuffer(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_document_loadFont(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_document_getElementById(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_document_getScreenData(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_document_toDataArray(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_document_parseNML(JSContext *cx, unsigned argc, JS::Value *vp);

static JSClass document_class = {
    "NativeDocument", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Document_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

JSClass *NativeJSdocument::jsclass = &document_class;

template<>
JSClass *Nidium::Binding::JSExposer<NativeJSdocument>::jsclass = &document_class;

static JSFunctionSpec document_funcs[] = {
    JS_FN("run", native_document_run, 1, NATIVE_JS_FNPROPS),
    JS_FN("showFPS", native_document_showfps, 1, NATIVE_JS_FNPROPS),
    JS_FN("setPasteBuffer", native_document_setPasteBuffer, 1, NATIVE_JS_FNPROPS),
    JS_FN("getPasteBuffer", native_document_getPasteBuffer, 0, NATIVE_JS_FNPROPS),
    JS_FN("loadFont", native_document_loadFont, 1, NATIVE_JS_FNPROPS),
    JS_FN("getCanvasById", native_document_getElementById, 1, NATIVE_JS_FNPROPS),
    JS_FN("getScreenData", native_document_getScreenData, 0, NATIVE_JS_FNPROPS),
    JS_FN("toDataArray", native_document_toDataArray, 0, NATIVE_JS_FNPROPS),
    JS_FN("parseNML", native_document_parseNML, 1, NATIVE_JS_FNPROPS),
    JS_FS_END
};

struct _native_document_restart_async
{
    NativeUIInterface *ui;
    char *location;
};

static bool native_document_parseNML(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JS::RootedString str(cx);
    if (!JS_ConvertArguments(cx, args, "S", str.address())) {
        return false;
    }

    JSAutoByteString cstr;
    cstr.encodeUtf8(cx, str);

    JS::RootedObject retObj(cx, NativeNML::BuildLST(cx, cstr.ptr()));
    args.rval().setObjectOrNull(retObj);

    return true;
}

static bool native_document_getElementById(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JS::RootedString str(cx);
    if (!JS_ConvertArguments(cx, args, "S", str.address())) {
        return false;
    }

    JSAutoByteString cid(cx, str);
    NativeCanvasHandler *elem = NativeContext::GetObject(cx)->getCanvasById(cid.ptr());
    if (elem) {
        args.rval().setObjectOrNull(elem->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

static bool native_document_getScreenData(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    NativeCanvasHandler *rootHandler = NativeContext::GetObject(cx)->getRootHandler();
    NativeCanvas2DContext *context = static_cast<NativeCanvas2DContext *>(rootHandler->getContext());

    int width, height;
    context->getSize(&width, &height);

    JS::RootedObject arrBuffer(cx, JS_NewUint8ClampedArray(cx, width * height * 4));
    uint8_t *pixels = JS_GetUint8ClampedArrayData(arrBuffer);

    NativeContext *nctx = NativeContext::GetObject(cx);

    uint8_t *fb = nctx->getUI()->readScreenPixel();

    memcpy(pixels, fb, width * height * 4);

    //NativeSkia *skia = context->getSurface();
    //skia->readPixels(0, 0, width, height, pixels);
    //glReadPixels(0, 0, NUII->getWidth(), NUII->getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, NUII->getFrameBufferData());
    //glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    JS::RootedValue widthVal(cx, JS::Int32Value(width));
    JS::RootedValue heightVal(cx, JS::Int32Value(height));
    JS::RootedValue arVal(cx, JS::ObjectOrNullValue(arrBuffer));
    JS::RootedObject dataObject(cx, JS_NewObject(cx,  NativeCanvas2DContext::jsclass, JS::NullPtr(), JS::NullPtr()));
    JS_DefineProperty(cx, dataObject, "width", widthVal, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "height", heightVal, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "data", arVal, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    args.rval().setObjectOrNull(dataObject);

    return true;
}

static bool native_document_toDataArray(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    NativeCanvasHandler *rootHandler = NativeContext::GetObject(cx)->getRootHandler();
    NativeCanvas2DContext *context = static_cast<NativeCanvas2DContext *>(rootHandler->getContext());

    int width, height;
    context->getSize(&width, &height);

    NativeContext *nctx = NativeContext::GetObject(cx);

    uint8_t *fb = nctx->getUI()->readScreenPixel();
    
    NativeSkImage *img = new NativeSkImage(fb, width, height);
    SkData *data;

    data = img->getPNG();

    if (!data) {
        args.rval().setNull();

        return true;
    }

    JS::RootedObject arrBuffer(cx, JS_NewUint8ClampedArray(cx, data->size()));
    uint8_t *pixels = JS_GetUint8ClampedArrayData(arrBuffer);

    memcpy(pixels, data->data(), data->size());

    SkSafeUnref(data);

    JS::RootedValue widthVal(cx, JS::Int32Value(width));
    JS::RootedValue heightVal(cx, JS::Int32Value(height));
    JS::RootedValue arVal(cx, JS::ObjectOrNullValue(arrBuffer));
    JS::RootedObject dataObject(cx, JS_NewObject(cx,  NativeCanvas2DContext::jsclass, JS::NullPtr(), JS::NullPtr()));
    JS_DefineProperty(cx, dataObject, "width", widthVal, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "height", heightVal, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "data", arVal, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    args.rval().setObjectOrNull(dataObject);

    return true;
}

static bool native_document_setPasteBuffer(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JS::RootedString str(cx);
    if (!JS_ConvertArguments(cx, args, "S", str.address())) {
        return false;
    }

    char *text = JS_EncodeStringToUTF8(cx, str);

    NativeContext::GetObject(cx)->getUI()->setClipboardText(text);

    js_free(text);

    return true;
}

static bool native_document_getPasteBuffer(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    size_t outputlen;
    char16_t * jsc;
    char *text = NativeContext::GetObject(cx)->getUI()->getClipboardText();

    if (text == NULL) {
        args.rval().setNull();
        return true;
    }
    jsc = Nidium::Binding::JSUtils::Utf8ToUtf16(cx, text, strlen(text), &outputlen);

    JS::RootedString jret(cx, JS_NewUCStringCopyN(cx, jsc, outputlen));
    args.rval().setString(jret);

    free(text);
    delete[] jsc; //@TODO: not shure

    return true;
}

static bool native_document_showfps(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    bool show = false;

    if (!JS_ConvertArguments(cx, args, "b", &show)) {
        return false;
    }

    NativeJSdocument::m_ShowFPS = show;

    if (show) {
        NativeContext::GetObject(cx)->createDebugCanvas();
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
    if (!JS_ConvertArguments(cx, args, "S", location.address())) {
        return false;
    }

    NativeUIInterface *NUI = NativeContext::GetObject(cx)->getUI();
    JSAutoByteString locationstr(cx, location);

    struct _native_document_restart_async *ndra = (struct _native_document_restart_async *)malloc(sizeof(*ndra));

    ndra->location = strdup(locationstr.ptr());
    ndra->ui = NUI;
    ape_global *ape = Nidium::Binding::NidiumJS::GetObject(cx)->net;

    timer_dispatch_async(native_document_restart, ndra);

    return true;
}

static void Document_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSdocument *jdoc = NativeJSdocument::GetObject(obj);

    if (jdoc != NULL) {
        delete jdoc;
    }
}

bool NativeJSdocument::populateStyle(JSContext *cx, const char *data,
    size_t len, const char *filename)
{
    if (!m_Stylesheet) {
        return false;
    }

    JS::RootedValue ret(cx);
    if (!Nidium::Binding::NidiumJS::LoadScriptReturn(cx, data, len, filename, &ret)) {
        return false;
    }

    JS::RootedObject style(cx, m_Stylesheet);
    JS::RootedObject jret(cx, ret.toObjectOrNull());
    Nidium::Binding::NidiumJS::copyProperties(cx, jret, &style);

    return true;
}

JSObject *NativeJSdocument::registerObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedObject documentObj(cx, JS_DefineObject(cx, global,
        NativeJSdocument::getJSObjectName(), &document_class , nullptr,
        JSPROP_PERMANENT | JSPROP_ENUMERATE));

    Nidium::Binding::NidiumJS *njs = Nidium::Binding::NidiumJS::GetObject(cx);

    NativeJSdocument *jdoc = new NativeJSdocument(documentObj, cx);
    JS_SetPrivate(documentObj, jdoc);

    /* We have to root it since the user can replace the document object */
    njs->rootObjectUntilShutdown(documentObj);

    njs->jsobjects.set(NativeJSdocument::getJSObjectName(), documentObj);

    JS::RootedObject styleObj(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));
    jdoc->m_Stylesheet = styleObj;

    JS::RootedValue objV(cx, JS::ObjectOrNullValue(jdoc->m_Stylesheet));

    /* implicitly root m_Stylesheet */
    JS_SetReservedSlot(documentObj, 0, objV);

    JS_SetProperty(cx, documentObj, "stylesheet", objV);
    JS_DefineFunctions(cx, documentObj, document_funcs);
    JS_DefineProperties(cx, documentObj, document_props);

    return documentObj;
}

// todo destroy with Nidium::Core::Hash cleaner
bool NativeJSdocument::loadFont(const char *path, const char *name,
    int weight, nativefont::Style style)
{
    Nidium::IO::Stream *stream = Nidium::IO::Stream::create(path);
    if (!stream) {
        return false;
    }

    Nidium::Core::PtrAutoDelete<Nidium::IO::Stream *> npad(stream);

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
    newfont->m_Weight = weight;
    newfont->m_Style = style;
    newfont->m_Typeface = tf;

    newfont->m_Next = oldfont;

    m_Fonts.set(name, newfont);

    return true;
}

static bool native_document_loadFont(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_INIT_OPT();
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject thisobj(cx, NativeJSdocument::getJSGlobalObject(cx));
    NativeJSdocument *CppObj = static_cast<NativeJSdocument *>(JS_GetPrivate(thisobj));

    JS::RootedObject fontdef(cx);
    if (!JS_ConvertArguments(cx, args, "o", fontdef.address())) {
        return false;
    }

    JSAutoByteString cfile, cname;

    NIDIUM_JS_GET_OPT_TYPE(fontdef, "file", String) {
        JS::RootedString file(cx, __curopt.toString());
        cfile.encodeUtf8(cx, file);
    } else {
        JS_ReportError(cx, "Missing 'file' (string) value");
        return false;
    }

    NIDIUM_JS_GET_OPT_TYPE(fontdef, "name", String) {
        JS::RootedString name(cx, __curopt.toString());
        cname.encodeLatin1(cx, name);
    } else {
        JS_ReportError(cx, "Missing 'name' (string) value");
        return false;
    }

    char *pTmp = cname.ptr();

    while (*pTmp != '\0') {
        *pTmp = tolower(*pTmp);
        pTmp++;
    }

    Nidium::Core::Path fpath(cfile.ptr());

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
        return font->m_Typeface;
    }

    return NULL;
}

