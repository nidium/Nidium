/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSDocument.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include <SkTypeface.h>
#include <SkStream.h>
#include <SkData.h>

#include "Binding/JSUtils.h"

#include "Frontend/NML.h"
#include "Graphics/CanvasHandler.h"
#include "Graphics/Image.h"
#include "Graphics/SkiaContext.h"
#include "Binding/JSCanvas2DContext.h"
#include "Binding/JSImageData.h"

using Nidium::Core::Path;
using Nidium::Core::Hash;
using Nidium::Core::PtrAutoDelete;
using Nidium::IO::Stream;
using Nidium::Graphics::Image;
using Nidium::Graphics::CanvasHandler;
using Nidium::Graphics::SkiaContext;
using Nidium::Frontend::Context;
using Nidium::Frontend::NML;
using Nidium::Interface::UIInterface;


namespace Nidium {
namespace Binding {


bool JSDocument::m_ShowFPS = false;


struct _nidium_document_restart_async
{
    UIInterface *ui;
    char *location;
};


static int nidium_document_restart(void *param)
{
    struct _nidium_document_restart_async *ndra
        = (struct _nidium_document_restart_async *)param;

    ndra->ui->restartApplication(ndra->location);

    free(ndra->location);
    free(ndra);

    return 0;
}


bool JSDocument::JS_run(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString location(cx);
    if (!JS_ConvertArguments(cx, args, "S", location.address())) {
        return false;
    }

    UIInterface *NUI = Context::GetObject<Frontend::Context>(cx)->getUI();
    JSAutoByteString locationstr(cx, location);

    struct _nidium_document_restart_async *ndra
        = (struct _nidium_document_restart_async *)malloc(sizeof(*ndra));

    ndra->location = strdup(locationstr.ptr());
    ndra->ui       = NUI;

    ape_global *ape = NidiumJS::GetObject(cx)->m_Net;
    timer_dispatch_async(nidium_document_restart, ndra);

    return true;
}

bool JSDocument::JS_parseNML(JSContext *cx, JS::CallArgs &args)
{

    JS::RootedString str(cx);
    if (!JS_ConvertArguments(cx, args, "S", str.address())) {
        return false;
    }

    JSAutoByteString cstr;
    cstr.encodeUtf8(cx, str);

    JS::RootedObject retObj(cx, NML::BuildLST(cx, cstr.ptr()));

    args.rval().setObjectOrNull(retObj);

    return true;
}

bool JSDocument::JS_getCanvasById(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString str(cx);
    if (!JS_ConvertArguments(cx, args, "S", str.address())) {
        return false;
    }

    JSAutoByteString cid(cx, str);
    CanvasHandler *elem
        = Context::GetObject<Frontend::Context>(cx)->getCanvasById(cid.ptr());
    if (elem) {
        args.rval().setObjectOrNull(elem->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

bool JSDocument::JS_getCanvasByIdx(JSContext *cx, JS::CallArgs &args)
{
    uint64_t idx;
    if (!JS::ToUint64(cx, args.get(0), &idx)) {
        args.rval().setNull();

        return true;
    }

    CanvasHandler *elem =
        Context::GetObject<Frontend::Context>(cx)->getCanvasByIdx(idx);

    if (elem) {
        args.rval().setObjectOrNull(elem->m_JsObj);
    } else {
        args.rval().setNull();
    }


    return true;
}

bool JSDocument::JS_getScreenData(JSContext *cx, JS::CallArgs &args)
{
    CanvasHandler *rootHandler
        = Context::GetObject<Frontend::Context>(cx)->getRootHandler();
    Canvas2DContext *context
        = static_cast<Canvas2DContext *>(rootHandler->getContext());

    int width, height;
    context->getSize(&width, &height);


    Context *nctx = Context::GetObject<Frontend::Context>(cx);

    uint8_t *fb = nctx->getUI()->readScreenPixel();


    JS::RootedObject arrBuffer(cx,
                               JS_NewUint8ClampedArray(cx, width * height * 4));

    // Skia *skia = context->getSurface();
    // skia->readPixels(0, 0, width, height, pixels);
    // glReadPixels(0, 0, NUII->getWidth(), NUII->getHeight(), GL_RGBA,
    // GL_UNSIGNED_BYTE,
    // NUII->getFrameBufferData());
    // glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    JS::RootedValue widthVal(cx, JS::Int32Value(width));
    JS::RootedValue heightVal(cx, JS::Int32Value(height));

    bool shared;

    {
        JS::AutoCheckCannotGC nogc;
        uint8_t * pixels = JS_GetUint8ClampedArrayData(arrBuffer, &shared, nogc);
        memcpy(pixels, fb, width * height * 4);
    }

    JS::RootedValue arVal(cx, JS::ObjectOrNullValue(arrBuffer));

    JS::RootedObject dataObject(cx,
      JSImageData::CreateObject(cx, new JSImageData()));

    JS_DefineProperty(cx, dataObject, "width", widthVal,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "height", heightVal,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "data", arVal,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    args.rval().setObjectOrNull(dataObject);

    return true;
}

bool JSDocument::JS_toDataArray(JSContext *cx, JS::CallArgs &args)
{
    SkData *data;
    int width, height;

    CanvasHandler *rootHandler = Context::GetObject<Frontend::Context>(cx)->getRootHandler();
    Canvas2DContext *context = static_cast<Canvas2DContext *>(rootHandler->getContext());

    Image *simg = Image::CreateFromSkImage(context->getSkiaContext()->getSurface()->makeImageSnapshot());
    if (!simg) {
        JS_ReportWarning(cx, "Couldnt readback snapshot surface");
        args.rval().setNull();
        return true;            
    }

    data = simg->getPNG();

    if (!data) {
        JS_ReportWarning(cx, "Couldnt read PNG data from surface");
        args.rval().setNull();
        return true;
    }

    context->getSize(&width, &height);

    JS::RootedObject arrBuffer(cx, JS_NewUint8ClampedArray(cx, data->size()));
    JS::RootedValue  widthVal (cx, JS::Int32Value(width));
    JS::RootedValue  heightVal(cx, JS::Int32Value(height));

    bool shared;
    {
        JS::AutoCheckCannotGC nogc;

        uint8_t * pixels = JS_GetUint8ClampedArrayData(arrBuffer, &shared, nogc);
        memcpy(pixels, data->data(), data->size());
    }

    JS::RootedValue arVal(cx, JS::ObjectOrNullValue(arrBuffer));

    JS::RootedObject dataObject(cx,
      JSImageData::CreateObject(cx, new JSImageData()));

    JS_DefineProperty(cx, dataObject, "width", widthVal,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "height", heightVal,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "data", arVal,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    args.rval().setObjectOrNull(dataObject);

    SkSafeUnref(data);
    delete simg;

    return true;
}

bool JSDocument::JS_setPasteBuffer(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString str(cx);
    if (!JS_ConvertArguments(cx, args, "S", str.address())) {
        return false;
    }

    char *text = JS_EncodeStringToUTF8(cx, str);

    Context::GetObject<Frontend::Context>(cx)->getUI()->setClipboardText(text);

    js_free(text);

    return true;
}

bool JSDocument::JS_getPasteBuffer(JSContext *cx, JS::CallArgs &args)
{
    size_t outputlen;
    char16_t *jsc;
    char *text = Context::GetObject<Frontend::Context>(cx)
                     ->getUI()
                     ->getClipboardText();

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

bool JSDocument::JS_showFPS(JSContext *cx, JS::CallArgs &args)
{
    bool show         = false;

    if (!JS_ConvertArguments(cx, args, "b", &show)) {
        return false;
    }

    JSDocument::m_ShowFPS = show;

    if (show) {
        Context::GetObject<Frontend::Context>(cx)->createDebugCanvas();
    }

    return true;
}


bool JSDocument::populateStyle(JSContext *cx,
                               const char *data,
                               size_t len,
                               const char *filename)
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

// todo destroy with Hash cleaner
bool JSDocument::loadFont(const char *path,
                          const char *name,
                          int weight,
                          NidiumFont::Style style)
{
    Stream *stream = Stream::Create(path);
    if (!stream) {
        return false;
    }

    PtrAutoDelete<Stream *> npad(stream);

    char *data;
    size_t len;

    if (!stream->getContentSync(&data, &len)) {
        return false;
    }

    SkMemoryStream *skmemory = new SkMemoryStream(data, len, true);
    free(data);

    sk_sp<SkTypeface> tf = SkTypeface::MakeFromStream(skmemory);
    if (tf == NULL) {
        delete skmemory;
        return false;
    }

    NidiumFont *oldfont = m_Fonts.get(name);

    NidiumFont *newfont = new NidiumFont();
    newfont->m_Weight   = weight;
    newfont->m_Style    = style;
    newfont->m_Typeface = tf;

    newfont->m_Next = oldfont;

    m_Fonts.set(name, newfont);

    return true;
}

bool JSDocument::JS_loadFont(JSContext *cx, JS::CallArgs &args)
{
    NIDIUM_JS_INIT_OPT();


    JS::RootedObject fontdef(cx);
    if (!JS_ConvertArguments(cx, args, "o", fontdef.address())) {
        return false;
    }

    JSAutoByteString cfile, cname;

    NIDIUM_JS_GET_OPT_TYPE(fontdef, "file", String)
    {
        JS::RootedString file(cx, __curopt.toString());
        cfile.encodeUtf8(cx, file);
    }
    else
    {
        JS_ReportErrorUTF8(cx, "Missing 'file' (string) value");
        return false;
    }

    NIDIUM_JS_GET_OPT_TYPE(fontdef, "name", String)
    {
        JS::RootedString name(cx, __curopt.toString());
        cname.encodeLatin1(cx, name);
    }
    else
    {
        JS_ReportErrorUTF8(cx, "Missing 'name' (string) value");
        return false;
    }

    char *pTmp = cname.ptr();

    while (*pTmp != '\0') {
        *pTmp = tolower(*pTmp);
        pTmp++;
    }

    Path fpath(cfile.ptr());

    args.rval().setBoolean(this->loadFont(fpath.path(), cname.ptr()));

    return true;
}

sk_sp<SkTypeface> JSDocument::getFont(const char *name)
{
    char *pTmp = strdup(name);

    for (int i = 0; name[i] != '\0'; i++) {
        pTmp[i] = tolower(name[i]);
    }

    NidiumFont *font = m_Fonts.get(pTmp);

    free(pTmp);

    if (font) {
        return font->m_Typeface;
    }

    return sk_sp<SkTypeface>(nullptr);
}
// }}}

JSFunctionSpec *JSDocument::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSDocument, run, 1),
        CLASSMAPPER_FN(JSDocument, showFPS, 1),
        CLASSMAPPER_FN(JSDocument, setPasteBuffer, 1),
        CLASSMAPPER_FN(JSDocument, getPasteBuffer, 0),
        CLASSMAPPER_FN(JSDocument, loadFont, 1),
        CLASSMAPPER_FN(JSDocument, getCanvasById, 1),
        CLASSMAPPER_FN(JSDocument, getCanvasByIdx, 1),
        CLASSMAPPER_FN(JSDocument, getScreenData, 0),
        CLASSMAPPER_FN(JSDocument, toDataArray, 0),
        CLASSMAPPER_FN(JSDocument, parseNML, 1),
        JS_FS_END
    };

    return funcs;
}


// {{{ Registration
JSObject *JSDocument::RegisterObject(JSContext *cx)
{
    JSDocument::ExposeClass(cx, "NidiumDocument",
        JSCLASS_HAS_RESERVED_SLOTS(1));

    JSDocument *jdoc = new JSDocument();
    JS::RootedObject documentObj(cx,
        JSDocument::CreateUniqueInstance(cx, jdoc, "document"));

    JS::RootedObject styleObj(
        cx, JS_NewPlainObject(cx));

    jdoc->m_Stylesheet = styleObj;

    JS::RootedValue objV(cx, JS::ObjectOrNullValue(jdoc->m_Stylesheet));

    /* implicitly root m_Stylesheet */
    JS_SetReservedSlot(documentObj, 0, objV);

    JS_SetProperty(cx, documentObj, "stylesheet", objV);

    return documentObj;
}
// }}}

} // namespace Binding
} // namespace Nidium
