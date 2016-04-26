#include "Binding/JSDocument.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include <SkTypeface.h>
#include <SkStream.h>
#include <SkData.h>

#include <Binding/JSUtils.h>

#include "NML/NML.h"
#include "Graphics/CanvasHandler.h"
#include "Graphics/SkImage.h"
#include "Binding/JSCanvas2DContext.h"

namespace Nidium {
namespace Binding {

// {{{ Preamble
bool JSDocument::m_ShowFPS = false;

static bool nidium_document_run(JSContext *cx, unsigned argc, JS::Value *vp);
static void Document_Finalize(JSFreeOp *fop, JSObject *obj);

static JSPropertySpec document_props[] = {
    JS_PS_END
};

static bool nidium_document_showfps(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_document_setPasteBuffer(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_document_getPasteBuffer(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_document_loadFont(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_document_getElementById(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_document_getScreenData(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_document_toDataArray(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_document_parseNML(JSContext *cx, unsigned argc, JS::Value *vp);

static JSClass Document_class = {
    "NativeDocument", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Document_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

JSClass *JSDocument::jsclass = &Document_class;

template<>
JSClass *JSExposer<JSDocument>::jsclass = &Document_class;

static JSFunctionSpec document_funcs[] = {
    JS_FN("run", nidium_document_run, 1, NIDIUM_JS_FNPROPS),
    JS_FN("showFPS", nidium_document_showfps, 1, NIDIUM_JS_FNPROPS),
    JS_FN("setPasteBuffer", nidium_document_setPasteBuffer, 1, NIDIUM_JS_FNPROPS),
    JS_FN("getPasteBuffer", nidium_document_getPasteBuffer, 0, NIDIUM_JS_FNPROPS),
    JS_FN("loadFont", nidium_document_loadFont, 1, NIDIUM_JS_FNPROPS),
    JS_FN("getCanvasById", nidium_document_getElementById, 1, NIDIUM_JS_FNPROPS),
    JS_FN("getScreenData", nidium_document_getScreenData, 0, NIDIUM_JS_FNPROPS),
    JS_FN("toDataArray", nidium_document_toDataArray, 0, NIDIUM_JS_FNPROPS),
    JS_FN("parseNML", nidium_document_parseNML, 1, NIDIUM_JS_FNPROPS),
    JS_FS_END
};
// }}}

// {{{ Implementation
struct _nidium_document_restart_async
{
    Nidium::Interface::NativeUIInterface *ui;
    char *location;
};

static bool nidium_document_parseNML(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JS::RootedString str(cx);
    if (!JS_ConvertArguments(cx, args, "S", str.address())) {
        return false;
    }

    JSAutoByteString cstr;
    cstr.encodeUtf8(cx, str);

    JS::RootedObject retObj(cx, Nidium::NML::NML::BuildLST(cx, cstr.ptr()));
    args.rval().setObjectOrNull(retObj);

    return true;
}

static bool nidium_document_getElementById(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JS::RootedString str(cx);
    if (!JS_ConvertArguments(cx, args, "S", str.address())) {
        return false;
    }

    JSAutoByteString cid(cx, str);
    Graphics::CanvasHandler *elem = Nidium::NML::NativeContext::GetObject(cx)->getCanvasById(cid.ptr());
    if (elem) {
        args.rval().setObjectOrNull(elem->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

static bool nidium_document_getScreenData(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    Graphics::CanvasHandler *rootHandler = Nidium::NML::NativeContext::GetObject(cx)->getRootHandler();
    Canvas2DContext *context = static_cast<Canvas2DContext *>(rootHandler->getContext());

    int width, height;
    context->getSize(&width, &height);

    JS::RootedObject arrBuffer(cx, JS_NewUint8ClampedArray(cx, width * height * 4));
    uint8_t *pixels = JS_GetUint8ClampedArrayData(arrBuffer);

    Nidium::NML::NativeContext *nctx = Nidium::NML::NativeContext::GetObject(cx);

    uint8_t *fb = nctx->getUI()->readScreenPixel();

    memcpy(pixels, fb, width * height * 4);

    //Skia *skia = context->getSurface();
    //skia->readPixels(0, 0, width, height, pixels);
    //glReadPixels(0, 0, NUII->getWidth(), NUII->getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, NUII->getFrameBufferData());
    //glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    JS::RootedValue widthVal(cx, JS::Int32Value(width));
    JS::RootedValue heightVal(cx, JS::Int32Value(height));
    JS::RootedValue arVal(cx, JS::ObjectOrNullValue(arrBuffer));
    JS::RootedObject dataObject(cx, JS_NewObject(cx,  Canvas2DContext::jsclass, JS::NullPtr(), JS::NullPtr()));
    JS_DefineProperty(cx, dataObject, "width", widthVal, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "height", heightVal, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "data", arVal, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    args.rval().setObjectOrNull(dataObject);

    return true;
}

static bool nidium_document_toDataArray(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    Graphics::CanvasHandler *rootHandler = Nidium::NML::NativeContext::GetObject(cx)->getRootHandler();
    Canvas2DContext *context = static_cast<Canvas2DContext *>(rootHandler->getContext());

    int width, height;
    context->getSize(&width, &height);

    Nidium::NML::NativeContext *nctx = Nidium::NML::NativeContext::GetObject(cx);

    uint8_t *fb = nctx->getUI()->readScreenPixel();

    Graphics::Image *img = new Graphics::Image(fb, width, height);
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
    JS::RootedObject dataObject(cx, JS_NewObject(cx,  Canvas2DContext::jsclass, JS::NullPtr(), JS::NullPtr()));
    JS_DefineProperty(cx, dataObject, "width", widthVal, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "height", heightVal, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "data", arVal, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    args.rval().setObjectOrNull(dataObject);

    return true;
}

static bool nidium_document_setPasteBuffer(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JS::RootedString str(cx);
    if (!JS_ConvertArguments(cx, args, "S", str.address())) {
        return false;
    }

    char *text = JS_EncodeStringToUTF8(cx, str);

    Nidium::NML::NativeContext::GetObject(cx)->getUI()->setClipboardText(text);

    js_free(text);

    return true;
}

static bool nidium_document_getPasteBuffer(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    size_t outputlen;
    char16_t * jsc;
    char *text = Nidium::NML::NativeContext::GetObject(cx)->getUI()->getClipboardText();

    if (text == NULL) {
        args.rval().setNull();
        return true;
    }
    jsc = JSUtils::Utf8ToUtf16(cx, text, strlen(text), &outputlen);

    JS::RootedString jret(cx, JS_NewUCStringCopyN(cx, jsc, outputlen));
    args.rval().setString(jret);

    free(text);
    delete[] jsc; //@TODO: not shure

    return true;
}

static bool nidium_document_showfps(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    bool show = false;

    if (!JS_ConvertArguments(cx, args, "b", &show)) {
        return false;
    }

    JSDocument::m_ShowFPS = show;

    if (show) {
        Nidium::NML::NativeContext::GetObject(cx)->createDebugCanvas();
    }

    return true;
}


static int nidium_document_restart(void *param)
{
    struct _nidium_document_restart_async *ndra = (struct _nidium_document_restart_async *)param;

    ndra->ui->restartApplication(ndra->location);

    free(ndra->location);
    free(ndra);

    return 0;
}

static bool nidium_document_run(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JS::RootedString location(cx);
    if (!JS_ConvertArguments(cx, args, "S", location.address())) {
        return false;
    }

    Nidium::Interface::NativeUIInterface *NUI = Nidium::NML::NativeContext::GetObject(cx)->getUI();
    JSAutoByteString locationstr(cx, location);

    struct _nidium_document_restart_async *ndra = (struct _nidium_document_restart_async *)malloc(sizeof(*ndra));

    ndra->location = strdup(locationstr.ptr());
    ndra->ui = NUI;
    ape_global *ape = NidiumJS::GetObject(cx)->net;

    timer_dispatch_async(nidium_document_restart, ndra);

    return true;
}

static void Document_Finalize(JSFreeOp *fop, JSObject *obj)
{
    JSDocument *jdoc = JSDocument::GetObject(obj);

    if (jdoc != NULL) {
        delete jdoc;
    }
}

bool JSDocument::populateStyle(JSContext *cx, const char *data,
    size_t len, const char *filename)
{
    if (!m_Stylesheet) {
        return false;
    }

    JS::RootedValue ret(cx);
    if (!NidiumJS::LoadScriptReturn(cx, data, len, filename, &ret)) {
        return false;
    }

    JS::RootedObject style(cx, m_Stylesheet);
    JS::RootedObject jret(cx, ret.toObjectOrNull());
    NidiumJS::CopyProperties(cx, jret, &style);

    return true;
}

// todo destroy with Core::Hash cleaner
bool JSDocument::loadFont(const char *path, const char *name,
    int weight, NidiumFont::Style style)
{
    IO::Stream *stream = IO::Stream::Create(path);
    if (!stream) {
        return false;
    }

    Core::PtrAutoDelete<IO::Stream *> npad(stream);

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

    NidiumFont *oldfont = m_Fonts.get(name);

    NidiumFont *newfont = new NidiumFont();
    newfont->m_Weight = weight;
    newfont->m_Style = style;
    newfont->m_Typeface = tf;

    newfont->m_Next = oldfont;

    m_Fonts.set(name, newfont);

    return true;
}

static bool nidium_document_loadFont(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_INIT_OPT();
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject thisobj(cx, JSDocument::GetJSGlobalObject(cx));
    JSDocument *CppObj = static_cast<JSDocument *>(JS_GetPrivate(thisobj));

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

SkTypeface *JSDocument::getFont(char *name)
{
    char *pTmp = name;

    while (*pTmp != '\0') {
        *pTmp = tolower(*pTmp);
        pTmp++;
    }

    NidiumFont *font = m_Fonts.get(name);
    if (font) {
        return font->m_Typeface;
    }

    return NULL;
}
// }}}

// {{{ Registration
JSObject *JSDocument::RegisterObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedObject documentObj(cx, JS_DefineObject(cx, global,
        JSDocument::GetJSObjectName(), &Document_class , nullptr,
        JSPROP_PERMANENT | JSPROP_ENUMERATE));

    NidiumJS *njs = NidiumJS::GetObject(cx);

    JSDocument *jdoc = new JSDocument(documentObj, cx);
    JS_SetPrivate(documentObj, jdoc);

    /* We have to root it since the user can replace the document object */
    njs->rootObjectUntilShutdown(documentObj);

    njs->jsobjects.set(JSDocument::GetJSObjectName(), documentObj);

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
// }}}

} // namespace Nidium
} // namespace Binding

