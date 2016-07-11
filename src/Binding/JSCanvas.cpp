/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSCanvas.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>

#include "Graphics/Canvas3DContext.h"
#include "Graphics/CanvasHandler.h"
#include "Binding/JSCanvas2DContext.h"
#include "JSCanvasProperties.h"

using Nidium::Core::SharedMessages;
using Nidium::Interface::UIInterface;
using Nidium::Graphics::CanvasContext;
using Nidium::Graphics::Canvas3DContext;
using Nidium::Graphics::CanvasHandler;
using Nidium::Graphics::Rect;
using Nidium::Frontend::Context;
using Nidium::Frontend::InputEvent;

namespace Nidium {
namespace Binding {

// {{{ Preamble
extern JSClass Canvas2DContext_class;

#define NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET() \
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(JSCanvas, &Canvas_class)\
    CanvasHandler *canvasHandler = CppObj->getHandler();

static struct nidium_cursors {
    const char *str;
    UIInterface::CURSOR_TYPE type;
} nidium_cursors_list[] = {
    {"default",             UIInterface::ARROW},
    {"arrow",               UIInterface::ARROW},
    {"beam",                UIInterface::BEAM},
    {"text",                UIInterface::BEAM},
    {"pointer",             UIInterface::POINTING},
    {"grabbing",            UIInterface::CLOSEDHAND},
    {"drag",                UIInterface::CLOSEDHAND},
    {"hidden",              UIInterface::HIDDEN},
    {"none",                UIInterface::HIDDEN},
    {"col-resize",          UIInterface::RESIZELEFTRIGHT},
    {NULL,                  UIInterface::NOCHANGE},
};

static void Canvas_Finalize(JSFreeOp *fop, JSObject *obj);
static void Canvas_Trace(JSTracer *trc, JSObject *obj);

static bool CanvasInherit_get(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp);

JSClass Canvas_class = {
    "Canvas", JSCLASS_HAS_PRIVATE | JSCLASS_IMPLEMENTS_BARRIERS | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Canvas_Finalize,
    nullptr, nullptr, nullptr, Canvas_Trace, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *JSExposer<JSCanvas>::jsclass = &Canvas_class;

static JSClass Canvas_Inherit_class = {
    "CanvasInherit", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, CanvasInherit_get, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

bool nidium_canvas_prop_set(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp);
static bool nidium_canvas_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp);

static bool nidium_canvas_getContext(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_setContext(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_addSubCanvas(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_insertBefore(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_insertAfter(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_removeFromParent(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_bringToFront(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_sendToBack(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_getParent(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_getFirstChild(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_getLastChild(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_getNextSibling(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_getPrevSibling(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_getChildren(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_getVisibleRect(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_setCoordinates(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_translate(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas_show(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas_hide(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas_setSize(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas_clear(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas_setZoom(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas_setScale(JSContext *cx, unsigned argc, JS::Value *vp);

static JSPropertySpec canvas_props[] = {
    NIDIUM_JS_PSGS("opacity", CANVAS_PROP_OPACITY, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("overflow", CANVAS_PROP_OVERFLOW, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("scrollLeft", CANVAS_PROP_SCROLLLEFT, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("scrollTop", CANVAS_PROP_SCROLLTOP, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("allowNegativeScroll", CANVAS_PROP_ALLOWNEGATIVESCROLL, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("width", CANVAS_PROP_WIDTH, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("coating", CANVAS_PROP_COATING, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("height", CANVAS_PROP_HEIGHT, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("maxwidth", CANVAS_PROP_MAXWIDTH, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("maxHeight", CANVAS_PROP_MAXHEIGHT, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("minWidth", CANVAS_PROP_MINWIDTH, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("minHeight", CANVAS_PROP_MINHEIGHT, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("position", CANVAS_PROP_POSITION, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("top", CANVAS_PROP_TOP, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("left", CANVAS_PROP_LEFT, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("right", CANVAS_PROP_RIGHT, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("bottom", CANVAS_PROP_BOTTOM, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("visible", CANVAS_PROP_VISIBLE, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("staticLeft", CANVAS_PROP_STATICLEFT, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("staticRight", CANVAS_PROP_STATICRIGHT, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("staticTop", CANVAS_PROP_STATICTOP, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("staticBottom", CANVAS_PROP_STATICBOTTOM, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("fluidHeight", CANVAS_PROP_FLUIDHEIGHT, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("fluidWidth", CANVAS_PROP_FLUIDWIDTH, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("id", CANVAS_PROP_ID, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("marginLeft", CANVAS_PROP_MARGINLEFT, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("marginRight", CANVAS_PROP_MARGINRIGHT, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("marginTop", CANVAS_PROP_MARGINTOP, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("marginBottom", CANVAS_PROP_MARGINBOTTOM, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSGS("cursor", CANVAS_PROP_CURSOR, nidium_canvas_prop_get, nidium_canvas_prop_set),
    NIDIUM_JS_PSG("clientWidth", CANVAS_PROP_CLIENTWIDTH, nidium_canvas_prop_get),
    NIDIUM_JS_PSG("clientHeight", CANVAS_PROP_CLIENTHEIGHT, nidium_canvas_prop_get),
    NIDIUM_JS_PSG("clientTop", CANVAS_PROP_CLIENTTOP, nidium_canvas_prop_get),
    NIDIUM_JS_PSG("clientLeft", CANVAS_PROP_CLIENTLEFT, nidium_canvas_prop_get),
    NIDIUM_JS_PSG("contentWidth", CANVAS_PROP_CONTENTWIDTH, nidium_canvas_prop_get),
    NIDIUM_JS_PSG("contentHeight", CANVAS_PROP_CONTENTHEIGHT, nidium_canvas_prop_get),
    NIDIUM_JS_PSG("innerWidth", CANVAS_PROP_INNERWIDTH, nidium_canvas_prop_get),
    NIDIUM_JS_PSG("innerHeight", CANVAS_PROP_INNERHEIGHT, nidium_canvas_prop_get),
    NIDIUM_JS_PSG("__visible", CANVAS_PROP___VISIBLE, nidium_canvas_prop_get),
    NIDIUM_JS_PSG("__top", CANVAS_PROP___TOP, nidium_canvas_prop_get),
    NIDIUM_JS_PSG("__left", CANVAS_PROP___LEFT, nidium_canvas_prop_get),
    NIDIUM_JS_PSG("__fixed", CANVAS_PROP___FIXED, nidium_canvas_prop_get),
    NIDIUM_JS_PSG("__outofbound", CANVAS_PROP___OUTOFBOUND, nidium_canvas_prop_get),
    NIDIUM_JS_PSG("ctx", CANVAS_PROP_CTX, nidium_canvas_prop_get),
    JS_PS_END
};

static JSFunctionSpec canvas_funcs[] = {
    JS_FN("getContext", nidium_canvas_getContext, 1, NIDIUM_JS_FNPROPS),
    JS_FN("setContext", nidium_canvas_setContext, 1, NIDIUM_JS_FNPROPS),
    JS_FN("add", nidium_canvas_addSubCanvas, 1, NIDIUM_JS_FNPROPS),
    JS_FN("insertBefore", nidium_canvas_insertBefore, 2, NIDIUM_JS_FNPROPS),
    JS_FN("insertAfter", nidium_canvas_insertAfter, 2, NIDIUM_JS_FNPROPS),
    JS_FN("removeFromParent", nidium_canvas_removeFromParent, 0, NIDIUM_JS_FNPROPS),
    JS_FN("show", nidium_canvas_show, 0, NIDIUM_JS_FNPROPS),
    JS_FN("hide", nidium_canvas_hide, 0, NIDIUM_JS_FNPROPS),
    JS_FN("bringToFront", nidium_canvas_bringToFront, 0, NIDIUM_JS_FNPROPS),
    JS_FN("sendToBack", nidium_canvas_sendToBack, 0, NIDIUM_JS_FNPROPS),
    JS_FN("getParent", nidium_canvas_getParent, 0, NIDIUM_JS_FNPROPS),
    JS_FN("getFirstChild", nidium_canvas_getFirstChild, 0, NIDIUM_JS_FNPROPS),
    JS_FN("getLastChild", nidium_canvas_getLastChild, 0, NIDIUM_JS_FNPROPS),
    JS_FN("getNextSibling", nidium_canvas_getNextSibling, 0, NIDIUM_JS_FNPROPS),
    JS_FN("getPrevSibling", nidium_canvas_getPrevSibling, 0, NIDIUM_JS_FNPROPS),
    JS_FN("getChildren", nidium_canvas_getChildren, 0, NIDIUM_JS_FNPROPS),
    JS_FN("setCoordinates", nidium_canvas_setCoordinates, 2, NIDIUM_JS_FNPROPS),
    JS_FN("translate", nidium_canvas_translate, 2, NIDIUM_JS_FNPROPS),
    JS_FN("getVisibleRect", nidium_canvas_getVisibleRect, 0, NIDIUM_JS_FNPROPS),
    JS_FN("setSize", nidium_canvas_setSize, 2, NIDIUM_JS_FNPROPS),
    JS_FN("clear", nidium_canvas_clear, 0, NIDIUM_JS_FNPROPS),
    JS_FN("setZoom", nidium_canvas_setZoom, 1, NIDIUM_JS_FNPROPS),
    JS_FN("setScale", nidium_canvas_setScale, 2, NIDIUM_JS_FNPROPS),
    JS_FS_END
};
// }}}

// {{{ Implementation
static bool CanvasInherit_get(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
{
    JSCanvas *jscanvas = (class JSCanvas *)JS_GetPrivate(obj);
    if (!jscanvas) {
        return true;
    }

    CanvasHandler *handler = jscanvas->getHandler();

    if (vp.isNull()) {
        CanvasHandler *parent;

        if ((parent = handler->getParent()) == NULL || !parent->m_JsObj) {
            return true;
        }

        JSCanvas *jscanvas_parent =
            (class JSCanvas *)JS_GetPrivate(parent->m_JsObj);

        JS::RootedValue ret(cx);
        JS::RootedObject parentObj(cx, jscanvas_parent->getInherit());
        JS_GetPropertyById(cx, parentObj, id, &ret);
        vp.set(ret);
    }

    return true;
}

CanvasHandler *HANDLER_GETTER(JSObject *obj)
{
    JSCanvas *jscanvas = (class JSCanvas *)JS_GetPrivate(obj);
    if (!jscanvas) {
        return NULL;
    }

    return jscanvas->getHandler();
}

static CanvasHandler *HANDLER_GETTER_SAFE(JSContext *cx, JS::HandleObject obj)
{
    JSCanvas *jscanvas = (class JSCanvas *)JS_GetPrivate(obj);

    const JSClass *cl = JS_GetClass(obj);
    if (!jscanvas || cl != &Canvas_class) {
        printf("Missmatch class %s\n", cl->name); //TODO error reporting
        return NULL;
    }

    return jscanvas->getHandler();
}

static bool nidium_canvas_show(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    canvasHandler->setHidden(false);

    return true;
}

static bool nidium_canvas_hide(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    canvasHandler->setHidden(true);

    return true;
}

static bool nidium_canvas_clear(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    if (canvasHandler->m_Context) {
        canvasHandler->m_Context->clear(0x00000000);
    }

    return true;
}

static bool nidium_canvas_setZoom(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    double zoom;

    if (!JS_ConvertArguments(cx, args, "d", &zoom)) {
        return false;
    }

    canvasHandler->setZoom(zoom);

    return true;
}

static bool nidium_canvas_setScale(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    double x, y;

    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        return false;
    }

    canvasHandler->setScale(x, y);

    return true;
}

static bool nidium_canvas_setSize(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    int width, height;

    if (!JS_ConvertArguments(cx, args, "ii", &width, &height)) {
        return false;
    }

    canvasHandler->setSize(width, height);

    return true;
}

static bool nidium_canvas_removeFromParent(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    canvasHandler->removeFromParent();

    return true;
}

static bool nidium_canvas_bringToFront(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    canvasHandler->bringToFront();

    return true;
}

static bool nidium_canvas_sendToBack(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    canvasHandler->sendToBack();

    return true;
}

static bool nidium_canvas_getParent(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    CanvasHandler *parent = canvasHandler->getParent();
    if (parent) {
        args.rval().setObjectOrNull(parent->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

static bool nidium_canvas_getFirstChild(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    CanvasHandler *val = canvasHandler->getFirstChild();
    if (val) {
        args.rval().setObjectOrNull(val->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

static bool nidium_canvas_getLastChild(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    CanvasHandler *val = canvasHandler->getLastChild();
    if (val) {
        args.rval().setObjectOrNull(val->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

static bool nidium_canvas_getNextSibling(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    CanvasHandler *val = canvasHandler->getNextSibling();
    if (val) {
        args.rval().setObjectOrNull(val->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

static bool nidium_canvas_getPrevSibling(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    CanvasHandler *val = canvasHandler->getPrevSibling();
    if (val) {
        args.rval().setObjectOrNull(val->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

static bool nidium_canvas_getChildren(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    uint32_t i;
    int32_t count = canvasHandler->countChildren();

    if (!count) {
        JS::RootedObject retObj(cx, JS_NewArrayObject(cx, 0));
        args.rval().setObjectOrNull(retObj);
        return true;
    }

    CanvasHandler *list[count];

    canvasHandler->getChildren(list);
    JS::RootedObject jlist(cx, JS_NewArrayObject(cx, count));
    for (i = 0; i < count; i++) {
        JS::RootedValue objVal(cx, OBJECT_TO_JSVAL(list[i]->m_JsObj));
        JS_SetElement(cx, jlist, i, objVal);
    }
    JS::RootedValue val(cx, OBJECT_TO_JSVAL(jlist));

    args.rval().set(val);

    return true;
}

static bool nidium_canvas_getVisibleRect(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    Rect rect = canvasHandler->getVisibleRect();
    JS::RootedObject ret(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));

#define SET_PROP(where, name, val) JS_DefineProperty(cx, where, \
    (const char *)name, val, JSPROP_PERMANENT | JSPROP_READONLY | \
        JSPROP_ENUMERATE)

    SET_PROP(ret, "left", rect.m_fLeft);
    SET_PROP(ret, "top", rect.m_fTop);
    SET_PROP(ret, "width", nidium_max(0, rect.m_fRight - rect.m_fLeft));
    SET_PROP(ret, "height", nidium_max(0, rect.m_fBottom-rect.m_fTop));
#undef SET_PROP

    args.rval().setObjectOrNull(ret);

    return true;
}

static bool nidium_canvas_setCoordinates(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    double left, top;

    if (!JS_ConvertArguments(cx, args, "dd", &left, &top)) {
        return false;
    }

    canvasHandler->m_Left = left;
    canvasHandler->m_Top = top;

    return true;
}

static bool nidium_canvas_translate(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    double left, top;

    if (!JS_ConvertArguments(cx, args, "dd", &left, &top)) {
        return false;
    }

    canvasHandler->translate(left, top);

    return true;
}

static bool nidium_canvas_addSubCanvas(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    CanvasHandler *handler = NULL;

    JS::RootedObject sub(cx);
    if (!JS_ConvertArguments(cx, args, "o", sub.address())) {
        return false;
    }

    if (sub && JS_GetClass(sub) != &Canvas_class) {
        JS_ReportError(cx, "add() First parameter is not a Canvas Object");
        return false;
    }

    handler = (static_cast<JSCanvas *>(JS_GetPrivate(sub)))->getHandler();

    if (handler == NULL) {
        return true;
    }

    if (canvasHandler == handler) {
        JS_ReportError(cx, "Canvas: can't add to itself");
        return false;
    }

    canvasHandler->addChild(handler);

    return true;
}

static bool nidium_canvas_insertBefore(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    CanvasHandler *handler_insert = NULL, *handler_ref = NULL;

    JS::RootedObject ref(cx);
    JS::RootedObject insert(cx);
    if (!JS_ConvertArguments(cx, args, "oo", insert.address(), ref.address())) {
        return false;
    }

    if (!insert || JS_GetClass(insert) != &Canvas_class) {
        JS_ReportError(cx, "add() First parameter is not a Canvas Object");
        return false;
    }

    handler_insert = (static_cast<JSCanvas *>(JS_GetPrivate(insert)))->getHandler();
    if (handler_insert == NULL) {
        return true;
    }

    if (ref && JS_GetClass(ref) == &Canvas_class) {
        handler_ref= (static_cast<JSCanvas *>(JS_GetPrivate(ref)))->getHandler();
    }

    if (canvasHandler == handler_insert) {
        JS_ReportError(cx, "Canvas: can't add to itself");
        return false;
    }

    canvasHandler->insertBefore(handler_insert, handler_ref);

    return true;
}

static bool nidium_canvas_insertAfter(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();

    CanvasHandler *handler_insert = NULL, *handler_ref = NULL;

    JS::RootedObject insert(cx);
    JS::RootedObject ref(cx);
    if (!JS_ConvertArguments(cx, args, "oo", insert.address(), ref.address())) {
        return false;
    }

    if (!insert || JS_GetClass(insert) != &Canvas_class) {
        JS_ReportError(cx, "add() First parameter is not a Canvas Object");
        return false;
    }

    handler_insert= (static_cast<JSCanvas *>(JS_GetPrivate(insert)))->getHandler();
    if (handler_insert == NULL) {
        return true;
    }

    if (ref && JS_GetClass(ref) == &Canvas_class) {
        handler_ref= (static_cast<JSCanvas *>(JS_GetPrivate(ref)))->getHandler();
    }

    if (canvasHandler == handler_insert) {
        JS_ReportError(cx, "Canvas: can't add to itself");
        return false;
    }

    canvasHandler->insertAfter(handler_insert, handler_ref);

    return true;
}

static bool nidium_canvas_getContext(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();
    NIDIUM_JS_CHECK_ARGS("getContext", 1);

    Context *nctx = Context::GetObject<Frontend::Context>(cx);
    UIInterface *ui = nctx->getUI();

    JS::RootedString mode(cx, args[0].toString());
    JSAutoByteString cmode(cx, mode);
    CanvasContext::mode ctxmode = CanvasContext::CONTEXT_2D;
    if (strncmp(cmode.ptr(), "2d", 2) == 0) {
        ctxmode = CanvasContext::CONTEXT_2D;
    } else if (strncmp(cmode.ptr(), "webgl", 5) == 0) {
        ctxmode = CanvasContext::CONTEXT_WEBGL;
    } else {
        args.rval().setNull();
        return true;
    }

    CanvasContext *canvasctx = canvasHandler->getContext();

    /* The context is lazy-created */
    if (canvasctx == NULL) {
        switch(ctxmode) {
            case CanvasContext::CONTEXT_2D:
            {
                Canvas2DContext *ctx2d = new Canvas2DContext(canvasHandler, cx,
                        canvasHandler->getWidth() + (canvasHandler->m_Padding.global * 2),
                        canvasHandler->getHeight() + (canvasHandler->m_Padding.global * 2), ui);

                if (ctx2d->getSurface() == NULL) {
                    delete ctx2d;
                    JS_ReportError(cx, "Could not create 2D context for this canvas");
                    return false;
                }
                canvasHandler->setContext(ctx2d);

                /* Inherit from the Context glstate */
                ctx2d->setGLState(nctx->getGLState());

                break;
            }
            case CanvasContext::CONTEXT_WEBGL:
                /*
                    TODO :
                    canvasHandler->setContext(new CanvasWebGLContext(...))
                */
                Canvas3DContext *ctx3d = new Canvas3DContext(canvasHandler, cx,
                        canvasHandler->getWidth() + (canvasHandler->m_Padding.global * 2),
                        canvasHandler->getHeight() + (canvasHandler->m_Padding.global * 2), ui);

                canvasHandler->setContext(ctx3d);
                break;
        }

        canvasctx = canvasHandler->getContext();

        /*  Protect against GC
            Canvas.slot[0] = context
        */
        JS::RootedValue slot(cx, OBJECT_TO_JSVAL(canvasHandler->getContext()->m_JsObj));
        JS_SetReservedSlot(canvasHandler->m_JsObj, 0, slot);
    } else if (canvasctx->m_Mode != ctxmode) {
        JS_ReportWarning(cx, "Bad context requested");
        /* A mode is requested but another one was already created */
        args.rval().setNull();
        return true;
    }
    JS::RootedObject ret(cx, canvasctx->m_JsObj);

    args.rval().setObjectOrNull(ret);

    return true;
}

static bool nidium_canvas_setContext(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CANVASCLASS_NO_RET();
    NIDIUM_JS_CHECK_ARGS("setContext", 1);

    JS::RootedObject obj(cx, args[0].toObjectOrNull());
    if (!obj.get()) {
        return true;
    }

    CanvasContext *context;

    if (!(context = static_cast<CanvasContext *>(JS_GetInstancePrivate(cx,
            obj, &Canvas2DContext_class, &args)))) {
        JS_ReportError(cx, "setContext() argument must a CanvasRenderingContext2D object");
        return false;
    }

    canvasHandler->setContext(context);

    /*
        If a context was already attached, it's going to be GC'd
        since it's not longer reachable from slot 0.
    */
    JS::RootedValue slot(cx, OBJECT_TO_JSVAL(context->m_JsObj));
    JS_SetReservedSlot(canvasHandler->m_JsObj, 0, slot);

    return true;
}


/* TODO: do not change the value when a wrong type is set */
bool nidium_canvas_prop_set(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp)
{
    CanvasHandler *handler = HANDLER_GETTER_SAFE(cx, obj);
    if (!handler) {
        return true;
    }

    switch (id) {
        case CANVAS_PROP_POSITION:
        {
            if (!vp.isString()) {
                vp.setNull();

                return true;
            }
            JS::RootedString vpStr(cx, JS::ToString(cx, vp));
            JSAutoByteString mode(cx, vpStr);
            if (strcasecmp(mode.ptr(), "absolute") == 0) {
                handler->setPositioning(CanvasHandler::COORD_ABSOLUTE);
            } else if(strcasecmp(mode.ptr(), "fixed") == 0) {
                handler->setPositioning(CanvasHandler::COORD_FIXED);
            } else if (strcasecmp(mode.ptr(), "inline") == 0) {
                handler->setPositioning(CanvasHandler::COORD_INLINE);
            } else if (strcasecmp(mode.ptr(), "inline-break") == 0) {
                handler->setPositioning(CanvasHandler::COORD_INLINEBREAK);
            } else {
                handler->setPositioning(CanvasHandler::COORD_RELATIVE);
            }
        }
        break;
        case CANVAS_PROP_WIDTH:
        {
            uint32_t dval;
            if (!vp.isNumber()) {

                return true;
            }

            JS::ToUint32(cx, vp, &dval);

            if (!handler->setWidth(dval)) {
                //JS_ReportError(cx, "Can't set canvas width (this canvas has a dynamic width)");

                return true;
            }
        }
        break;
        case CANVAS_PROP_MINWIDTH:
        {
            uint32_t dval;
            if (!vp.isNumber()) {

                return true;
            }

            JS::ToUint32(cx, vp, &dval);

            if (!handler->setMinWidth(dval)) {
                return true;
            }
        }
        break;
        case CANVAS_PROP_MAXWIDTH:
        {
            uint32_t dval;
            if (!vp.isNumber()) {
                return true;
            }

            JS::ToUint32(cx, vp, &dval);

            if (!handler->setMaxWidth(dval)) {
                return true;
            }
        }
        break;
        case CANVAS_PROP_HEIGHT:
        {
            uint32_t dval;
            if (!vp.isNumber()) {
                return true;
            }

            JS::ToUint32(cx, vp, &dval);

            if (!handler->setHeight(dval)) {
                //JS_ReportError(cx, "Can't set canvas height (this canvas has a dynamic height)");
                return true;
            }
        }
        break;
        case CANVAS_PROP_MINHEIGHT:
        {
            uint32_t dval;
            if (!vp.isNumber()) {
                return true;
            }

            JS::ToUint32(cx, vp, &dval);

            if (!handler->setMinHeight(dval)) {
                return true;
            }
        }
        break;
        case CANVAS_PROP_MAXHEIGHT:
        {
            uint32_t dval;
            if (!vp.isNumber()) {
                return true;
            }

            JS::ToUint32(cx, vp, &dval);

            if (!handler->setMaxHeight(dval)) {
                return true;
            }
        }
        break;
        case CANVAS_PROP_LEFT:
        {
            double dval;

            if (!handler->hasStaticLeft()) {
                //JS_ReportError(cx, "Can't set .left property if .staticLeft == false");
                return true;
            }

            if (vp.isNullOrUndefined()) {
                handler->unsetLeft();

                return true;
            }
            if (!vp.isNumber()) {

                return true;
            }

            dval = vp.toNumber();

            handler->setLeft(dval);
        }
        break;
        case CANVAS_PROP_RIGHT:
        {
            double dval;

            if (!handler->hasStaticRight()) {
                //JS_ReportError(cx, "Can't set .right property if .staticRight == false");
                return true;
            }

            if (vp.isNullOrUndefined()) {
                handler->unsetRight();

                return true;
            }
            if (!vp.isNumber()) {

                return true;
            }

            dval = vp.toNumber();
            handler->setRight(dval);
        }
        break;
        case CANVAS_PROP_TOP:
        {
            double dval;
            if (!handler->hasStaticTop()) {
                return true;
            }
            if (vp.isNullOrUndefined()) {
                handler->unsetTop();

                return true;
            }
            if (!vp.isNumber()) {

                return true;
            }

            dval = vp.toNumber();
            handler->setTop(dval);
        }
        break;
        case CANVAS_PROP_BOTTOM:
        {
            double dval;
            if (!handler->hasStaticBottom()) {

                return true;
            }
            if (vp.isNullOrUndefined()) {
                handler->unsetBottom();

                return true;
            }
            if (!vp.isNumber()) {

                return true;
            }

            dval = vp.toNumber();
            handler->setBottom(dval);
        }
        break;
        case CANVAS_PROP_SCROLLLEFT:
        {
            int32_t dval;
            if (!vp.isNumber()) {

                return true;
            }

            JS::ToInt32(cx, vp, &dval);

            handler->setScrollLeft(dval);
        }
        break;
        case CANVAS_PROP_SCROLLTOP:
        {
            int32_t dval;
            if (!vp.isNumber()) {

                return true;
            }

            JS::ToInt32(cx, vp, &dval);

            handler->setScrollTop(dval);
        }
        break;
        case CANVAS_PROP_ALLOWNEGATIVESCROLL:
        {
            if (!vp.isBoolean()) {

                return true;
            }

            handler->setAllowNegativeScroll(vp.toBoolean());
        }
        break;
        case CANVAS_PROP_VISIBLE:
        {
            if (!vp.isBoolean()) {

                return true;
            }

            handler->setHidden(!vp.toBoolean());
        }
        break;
        case CANVAS_PROP_OVERFLOW:
        {
            if (!vp.isBoolean()) {

                return true;
            }

            handler->m_Overflow = vp.toBoolean();
        }
        break;
        case CANVAS_PROP_COATING:
        {
            int32_t dval;
            if (!vp.isNumber()) {

                return true;
            }

            JS::ToInt32(cx, vp, &dval);

            handler->setPadding(dval);
        }
        break;
        case CANVAS_PROP_OPACITY:
        {
            double dval;
            if (!vp.isNumber()) {

                return true;
            }

            dval = vp.toNumber();

            handler->setOpacity(dval);
        }
        break;
        case CANVAS_PROP_STATICLEFT:
        {
            if (!vp.isBoolean()) {

                return true;
            }

            if (vp.toBoolean()) {
                handler->setLeft(handler->m_Left);
            } else {
                handler->unsetLeft();
            }
        }
        break;
        case CANVAS_PROP_STATICRIGHT:
        {
            if (!vp.isBoolean()) {

                return true;
            }

            if (vp.toBoolean()) {
                handler->setRight(handler->m_Right);
            } else {
                handler->unsetRight();
            }
        }
        break;
        case CANVAS_PROP_STATICTOP:
        {
            if (!vp.isBoolean()) {

                return true;
            }

            if (vp.toBoolean()) {
                handler->setTop(handler->m_Top);
            } else {
                handler->unsetTop();
            }
        }
        break;
        case CANVAS_PROP_STATICBOTTOM:
        {
            if (!vp.isBoolean()) {

                return true;
            }
            if (vp.toBoolean()) {
                handler->setBottom(handler->m_Bottom);
            } else {
                handler->unsetBottom();
            }
        }
        break;
        case CANVAS_PROP_FLUIDHEIGHT:
        {
            if (!vp.isBoolean()) {

                return true;
            }
            handler->setFluidHeight(vp.toBoolean());
        }
        break;
        case CANVAS_PROP_FLUIDWIDTH:
        {
            if (!vp.isBoolean()) {

                return true;
            }
            handler->setFluidWidth(vp.toBoolean());
        }
        break;
        case CANVAS_PROP_ID:
        {
            JS::RootedString sid(cx, JS::ToString(cx, vp));
            if (!sid.get()) {

                return true;
            }
            JSAutoByteString cid(cx, sid);
            handler->setId(cid.ptr());
        }
        break;
        case CANVAS_PROP_MARGINLEFT:
        {
            double dval;
            if (!vp.isNumber()) {

                return true;
            }

            dval = vp.toNumber();

            handler->setMargin(handler->m_Margin.top, handler->m_Margin.right,
                handler->m_Margin.bottom, dval);
        }
        break;
        case CANVAS_PROP_MARGINRIGHT:
        {
            double dval;
            if (!vp.isNumber()) {

                return true;
            }

            dval = vp.toNumber();
            handler->setMargin(handler->m_Margin.top, dval,
                handler->m_Margin.bottom, handler->m_Margin.left);
        }
        break;
        case CANVAS_PROP_MARGINTOP:
        {
            double dval;
            if (!vp.isNumber()) {

                return true;
            }

            dval = vp.toNumber();
            handler->setMargin(dval, handler->m_Margin.right,
                handler->m_Margin.bottom, handler->m_Margin.left);
        }
        break;
        case CANVAS_PROP_MARGINBOTTOM:
        {
            double dval;
            if (!vp.isNumber()) {

                return true;
            }

            dval = vp.toNumber();
            handler->setMargin(handler->m_Margin.top, handler->m_Margin.right,
                dval, handler->m_Margin.left);
        }
        break;
        case CANVAS_PROP_CURSOR:
        {
            if (!vp.isString()) {

                return true;
            }

            JS::RootedString vpStr(cx, vp.toString());
            JSAutoByteString type(cx, vpStr);
            for (int i = 0; nidium_cursors_list[i].str != NULL; i++) {
                if (strncasecmp(nidium_cursors_list[i].str, type.ptr(),
                    strlen(nidium_cursors_list[i].str)) == 0) {
                    handler->setCursor(nidium_cursors_list[i].type);
                    break;
                }
            }
        }
        break;
        default:
            break;
    }

    return true;
}

static bool nidium_canvas_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp)
{
    CanvasHandler *handler = HANDLER_GETTER_SAFE(cx, obj);
    if (!handler) {
        return true;
    }

    switch (id) {
        case CANVAS_PROP_OPACITY:
            vp.setDouble(handler->getOpacity());
            break;
        case CANVAS_PROP_WIDTH:
            vp.setInt32(handler->getWidth());
            break;
        case CANVAS_PROP_MINWIDTH:
            vp.setInt32(handler->getMinWidth());
            break;
        case CANVAS_PROP_MAXWIDTH:
            vp.setInt32(handler->getMaxWidth());
            break;
        case CANVAS_PROP_CLIENTWIDTH:
            vp.setInt32(handler->getWidth() + (handler->m_Padding.global * 2));
            break;
        case CANVAS_PROP_HEIGHT:
            vp.setInt32(handler->getHeight());
            break;
        case CANVAS_PROP_MINHEIGHT:
            vp.setInt32(handler->getMinHeight());
            break;
        case CANVAS_PROP_MAXHEIGHT:
            vp.setInt32(handler->getMaxHeight());
            break;
        case CANVAS_PROP_CLIENTHEIGHT:
            vp.setInt32(handler->getHeight() + (handler->m_Padding.global * 2));
            break;
        case CANVAS_PROP_COATING:
            vp.setInt32(handler->m_Padding.global);
            break;
        case CANVAS_PROP_LEFT:
            vp.setDouble(handler->getLeft());
            break;
        case CANVAS_PROP_RIGHT:
            vp.setDouble(handler->getRight());
            break;
        case CANVAS_PROP_BOTTOM:
            vp.setDouble(handler->getBottom());
            break;
        case CANVAS_PROP_CLIENTLEFT:
            vp.setInt32(handler->getLeft() - handler->m_Padding.global);
            break;
        case CANVAS_PROP_TOP:
            vp.setDouble(handler->getTop());
            break;
        case CANVAS_PROP_CLIENTTOP:
            vp.setInt32(handler->getTop() - handler->m_Padding.global);
            break;
        case CANVAS_PROP_STATICLEFT:
            vp.setBoolean(handler->hasStaticLeft());
            break;
        case CANVAS_PROP_STATICRIGHT:
            vp.setBoolean(handler->hasStaticRight());
            break;
        case CANVAS_PROP_STATICTOP:
            vp.setBoolean(handler->hasStaticTop());
            break;
        case CANVAS_PROP_STATICBOTTOM:
            vp.setBoolean(handler->hasStaticBottom());
            break;
        case CANVAS_PROP_FLUIDHEIGHT:
            vp.setBoolean(handler->isHeightFluid());
            break;
        case CANVAS_PROP_FLUIDWIDTH:
            vp.setBoolean(handler->isWidthFluid());
            break;
        case CANVAS_PROP_VISIBLE:
            vp.setBoolean(!handler->isHidden());
            break;
        case CANVAS_PROP_OVERFLOW:
            vp.setBoolean(handler->m_Overflow);
            break;
        case CANVAS_PROP___VISIBLE:
            vp.setBoolean(handler->isDisplayed());
            break;
        case CANVAS_PROP_CONTENTWIDTH:
            vp.setInt32(handler->getContentWidth());
            break;
        case CANVAS_PROP_CONTENTHEIGHT:
            vp.setInt32(handler->getContentHeight());
            break;
        case CANVAS_PROP_INNERWIDTH:
            vp.setInt32(handler->getContentWidth(true));
            break;
        case CANVAS_PROP_INNERHEIGHT:
            vp.setInt32(handler->getContentHeight(true));
            break;
        case CANVAS_PROP_SCROLLLEFT:
            vp.setInt32(handler->m_Content.scrollLeft);
            break;
        case CANVAS_PROP_SCROLLTOP:
            vp.setInt32(handler->m_Content.scrollTop);
            break;
        case CANVAS_PROP_ALLOWNEGATIVESCROLL:
            vp.setBoolean(handler->getAllowNegativeScroll());
            break;
        case CANVAS_PROP___FIXED:
            vp.setBoolean(handler->hasAFixedAncestor());
            break;
        case CANVAS_PROP___TOP:
        {
            handler->computeAbsolutePosition();
            vp.setDouble(handler->getTop(true));
            break;
        }
        case CANVAS_PROP___LEFT:
        {
            handler->computeAbsolutePosition();
            vp.setDouble(handler->getLeft(true));
            break;
        }
        case CANVAS_PROP_CTX:
        {
            if (handler->m_Context == NULL) {
                vp.setNull();
                break;
            }
            JS::RootedObject retObj(cx, handler->m_Context->m_JsObj);
            vp.setObjectOrNull(retObj);
            break;
        }
        case CANVAS_PROP___OUTOFBOUND:
        {
            vp.setBoolean(handler->isOutOfBound());
            break;
        }
        case CANVAS_PROP_POSITION:
        {
            JS::RootedString jstr(cx);
            switch (handler->getPositioning()) {
                case CanvasHandler::COORD_RELATIVE:
                    if (handler->getFlowMode() & CanvasHandler::kFlowBreakPreviousSibling) {
                        jstr = JS_NewStringCopyZ(cx, "inline-break");
                        vp.setString(jstr);
                    } else if (handler->getFlowMode() & CanvasHandler::kFlowInlinePreviousSibling) {
                        jstr = JS_NewStringCopyZ(cx, "inline");
                        vp.setString(jstr);
                    } else {
                        jstr = JS_NewStringCopyZ(cx, "relative");
                        vp.setString(jstr);
                    }
                    break;
                case CanvasHandler::COORD_ABSOLUTE:
                    jstr = JS_NewStringCopyZ(cx, "absolute");
                    vp.setString(jstr);
                    break;
                case CanvasHandler::COORD_FIXED:
                    jstr = JS_NewStringCopyZ(cx, "fixed");
                    vp.setString(jstr);
                    break;
                case CanvasHandler::COORD_INLINE:
                    jstr = JS_NewStringCopyZ(cx, "inline");
                    vp.setString(jstr);
                    break;
                case CanvasHandler::COORD_INLINEBREAK:
                    jstr = JS_NewStringCopyZ(cx, "inline-break");
                    vp.setString(jstr);
                    break;
            }
            break;
        }
        case CANVAS_PROP_ID:
        {
            char *cid;
            handler->getIdentifier(&cid);
            JS::RootedString jstr(cx, JS_NewStringCopyZ(cx, cid));
            vp.setString(jstr);

            break;
        }
        case CANVAS_PROP_MARGINLEFT:
        {
            vp.setDouble(handler->m_Margin.left);
            break;
        }
        case CANVAS_PROP_MARGINRIGHT:
        {
            vp.setDouble(handler->m_Margin.right);
            break;
        }
        case CANVAS_PROP_MARGINTOP:
        {
            vp.setDouble(handler->m_Margin.top);
            break;
        }
        case CANVAS_PROP_MARGINBOTTOM:
        {
            vp.setDouble(handler->m_Margin.bottom);
            break;
        }
        case CANVAS_PROP_CURSOR:
        {

            int cursor = handler->getCursor();

            for (int i = 0; nidium_cursors_list[i].str != NULL; i++) {
                if (nidium_cursors_list[i].type == cursor) {
                    vp.setString(JS_NewStringCopyZ(cx, nidium_cursors_list[i].str));

                    break;
                }
            }
        }
        break;
        default:
            break;
    }

    return true;
}

static bool nidium_Canvas_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    CanvasHandler *handler;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    int width, height;
    bool lazyLoad;

    if (!args.isConstructing()) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    JS::RootedObject opt(cx);
    if (!JS_ConvertArguments(cx, args, "ii/o", &width, &height, opt.address())) {
        return false;
    }

    NIDIUM_JS_INIT_OPT();
    NIDIUM_JS_GET_OPT_TYPE(opt, "lazy", Boolean) {
        lazyLoad = __curopt.toBoolean();
    }
    lazyLoad = false; /* Always lazy load for now.  */

    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &Canvas_class, args));
    handler = new CanvasHandler(width, height, Context::GetObject<Frontend::Context>(cx), true);
    handler->m_Context = NULL;
    handler->m_JsObj = ret;
    handler->m_JsCx = cx;

    JSCanvas *jscanvas = new JSCanvas(ret, cx, handler);
    JS::RootedObject inherit(cx, JS_DefineObject(cx, ret, "inherit", &Canvas_Inherit_class, nullptr,
        JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT));
    jscanvas->setInherit(inherit);

    JS_SetPrivate(ret, jscanvas);
    JS_SetPrivate(inherit, jscanvas);

    args.rval().setObjectOrNull(ret);

    return true;
}

void Canvas_Finalize(JSFreeOp *fop, JSObject *obj)
{
    JSCanvas *jscanvas = ((class JSCanvas *)JS_GetPrivate(obj));

    if (jscanvas != NULL) {
        delete jscanvas;
    }
}

#if 0 && defined(DEBUG)
void PrintGetTraceName(JSTracer* trc, char *buf, size_t bufsize)
{
    snprintf(buf, bufsize, "[0x%p].mJSVal", trc->debugPrintArg());
}
#endif

static void Canvas_Trace(JSTracer *trc, JSObject *obj)
{
    CanvasHandler *handler = HANDLER_GETTER(obj);
    if (handler != NULL) {
        CanvasHandler *cur;

        for (cur = handler->getFirstChild(); cur != NULL; cur = cur->m_Next) {
            if (cur->m_JsObj) {

                JS_CallHeapObjectTracer(trc, &cur->m_JsObj, "nidiumcanvasroot");
            }
        }
    }
}

JSObject *JSCanvas::GenerateJSObject(JSContext *cx, int width,
    int height, CanvasHandler **out)
{
    CanvasHandler *handler;
    Context *nctx = Context::GetObject<Frontend::Context>(cx);
    UIInterface *ui = nctx->getUI();

    JS::RootedObject ret(cx, JS_NewObject(cx, &Canvas_class, JS::NullPtr(), JS::NullPtr()));
    JS::RootedObject inherit(cx, JS_DefineObject(cx, ret, "inherit",
        &Canvas_Inherit_class, nullptr,
        JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT));

    handler = new CanvasHandler(width, height, nctx);
    handler->setContext(new Canvas2DContext(handler, cx, width, height, ui));
    handler->getContext()->setGLState(nctx->getGLState());

    /* window.canvas.overflow default to false */
    handler->m_Overflow = false;
    handler->m_JsObj = ret;
    handler->m_JsCx = cx;
    JS::RootedValue val(cx, OBJECT_TO_JSVAL(handler->m_Context->m_JsObj));
    JS_SetReservedSlot(ret, 0, val);

    JSCanvas *jscanvas = new JSCanvas(ret, cx, handler);

    jscanvas->setInherit(inherit);

    JS_SetPrivate(ret, jscanvas);
    JS_SetPrivate(inherit, jscanvas);

    *out = handler;

    return ret;
}

void JSCanvas::onMessage(const SharedMessages::Message &msg)
{
    JSContext *cx = m_Cx;
    JS::RootedObject ro(cx, m_JSObject);
    switch (msg.event()) {
        case NIDIUM_EVENT(CanvasHandler, RESIZE_EVENT):
        {
            JS::RootedObject eventObj(m_Cx, JSEvents::CreateEventObject(m_Cx));
            JS::RootedValue eventValue(m_Cx);

            eventValue.setObjectOrNull(eventObj);
            this->fireJSEvent("resize", &eventValue);
            break;
        }
        case NIDIUM_EVENT(CanvasHandler, LOADED_EVENT):
        {
            JS::RootedObject eventObj(m_Cx, JSEvents::CreateEventObject(m_Cx));
            JS::RootedValue eventValue(m_Cx);

            eventValue.setObjectOrNull(eventObj);
            this->fireJSEvent("load", &eventValue);
            break;
        }
        case NIDIUM_EVENT(CanvasHandler, CHANGE_EVENT):
        {
            const char *name = NULL;
            JS::RootedValue value(cx);

            switch (msg.m_Args[1].toInt()) {
                case CanvasHandler::kContentWidth_Changed:
                    name = "contentWidth";
                    value.setInt32(msg.m_Args[2].toInt());
                    break;
                case CanvasHandler::kContentHeight_Changed:
                    name = "contentHeight";
                    value.setInt32(msg.m_Args[2].toInt());
                    break;
            }

            JS::RootedObject eventObj(m_Cx, JSEvents::CreateEventObject(m_Cx));
            JSObjectBuilder obj(m_Cx, eventObj);
            JS::RootedValue eventValue(m_Cx, obj.jsval());
            obj.set("property", name);
            obj.set("value", value);

            this->fireJSEvent("change", &eventValue);

            break;
        }
        case NIDIUM_EVENT(CanvasHandler, DRAG_EVENT):
        {
            printf("Drag event detected\n");
        }
        case NIDIUM_EVENT(CanvasHandler, MOUSE_EVENT):
        {
            JS::RootedObject eventObj(m_Cx, JSEvents::CreateEventObject(m_Cx));
            CanvasHandler *target = static_cast<CanvasHandler *>(msg.m_Args[8].toPtr());
            JSObjectBuilder obj(m_Cx, eventObj);
            obj.set("x", msg.m_Args[2].toInt());
            obj.set("y", msg.m_Args[3].toInt());
            obj.set("clientX", msg.m_Args[2].toInt());
            obj.set("clientY", msg.m_Args[3].toInt());
            obj.set("layerX", msg.m_Args[6].toInt());
            obj.set("layerY", msg.m_Args[7].toInt());
            obj.set("target", target->m_JsObj);

            int evtype = (InputEvent::Type)msg.m_Args[1].toInt();

            switch (evtype) {
                case InputEvent::kMouseMove_Type:
                case InputEvent::kMouseDrag_Type:
                case InputEvent::kMouseDragOver_Type:
                case InputEvent::kMouseWheel_Type:
                    obj.set("xrel", msg.m_Args[4].toInt());
                    obj.set("yrel", msg.m_Args[5].toInt());
                    break;
                default:
                    break;
            }

            switch (evtype) {
                case InputEvent::kMouseClick_Type:
                case InputEvent::kMouseDoubleClick_Type:
                case InputEvent::kMouseClickRelease_Type:
                    obj.set("which", msg.m_Args[4].toInt());
                    break;
                case InputEvent::kMouseDrag_Type:
                case InputEvent::kMouseDragStart_Type:
                case InputEvent::kMouseDragEnd_Type:
                    obj.set("source", this->getJSObject());
                    break;
                case InputEvent::kMouseDragOver_Type:
                case InputEvent::kMouseDrop_Type:
                    obj.set("source", target->m_JsObj);
                    obj.set("target", this->getJSObject());
                    break;
                default:
                    break;
            }

            JS::RootedValue evVal(cx, obj.jsval());
            if (!this->fireJSEvent(InputEvent::GetName(msg.m_Args[1].toInt()), &evVal)) {
                break;
            }

            JS::RootedValue cancelBubble(cx);
            JS::RootedObject robj(cx, obj.obj());
            if (JS_GetProperty(cx, robj, "cancelBubble", &cancelBubble)) {
                if (cancelBubble.isBoolean() && cancelBubble.toBoolean()) {
                    /* TODO: sort out this dirty hack */
                    SharedMessages::Message *nonconstmsg = (SharedMessages::Message *)&msg;
                    nonconstmsg->m_Priv = 1;
                }
            }
        }
            break;
        default:
            break;
    }
}

void JSCanvas::onMessageLost(const SharedMessages::Message &msg)
{

}

JSCanvas::JSCanvas(JS::HandleObject obj, JSContext *cx,
    CanvasHandler *handler) :
    JSExposer<JSCanvas>(obj, cx),
    m_CanvasHandler(handler)
{
    m_CanvasHandler->addListener(this);

    /*
        Trigger "loaded" event if not lazy loaded
    */
    m_CanvasHandler->checkLoaded();
}

JSCanvas::~JSCanvas()
{
    delete m_CanvasHandler;
}
// }}}

// {{{ Registration
void JSCanvas::RegisterObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS_InitClass(cx, global, JS::NullPtr(), &Canvas_class, nidium_Canvas_constructor, 2, canvas_props, canvas_funcs,
        nullptr, nullptr);
}
// }}}

} // namespace Binding
} // namespace Nidium

