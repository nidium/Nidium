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
#include "Binding/JSWebGL.h"
#include "Binding/JSEvents.h"
#include "Binding/JSUtils.h"

#include <js/TracingAPI.h>

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

static struct nidium_cursors
{
    const char *str;
    UIInterface::CURSOR_TYPE type;
} nidium_cursors_list[] = {
    { "default", UIInterface::ARROW },
    { "arrow", UIInterface::ARROW },
    { "beam", UIInterface::BEAM },
    { "text", UIInterface::BEAM },
    { "pointer", UIInterface::POINTING },
    { "grabbing", UIInterface::CLOSEDHAND },
    { "drag", UIInterface::CLOSEDHAND },
    { "hidden", UIInterface::HIDDEN },
    { "none", UIInterface::HIDDEN },
    { "col-resize", UIInterface::RESIZELEFTRIGHT },
    { NULL, UIInterface::ARROW },
};


// }}}

// {{{ Implementation


bool JSCanvas::JS_show(JSContext *cx, JS::CallArgs &args)
{
    m_CanvasHandler->setHidden(false);

    return true;
}

bool JSCanvas::JS_hide(JSContext *cx, JS::CallArgs &args)
{
    m_CanvasHandler->setHidden(true);

    return true;
}

bool JSCanvas::JS_clear(JSContext *cx, JS::CallArgs &args)
{
    if (m_CanvasHandler->m_Context) {
        m_CanvasHandler->m_Context->clear(0x00000000);
    }

    return true;
}

bool JSCanvas::JS_setZoom(JSContext *cx, JS::CallArgs &args)
{
    double zoom;

    if (!JS_ConvertArguments(cx, args, "d", &zoom)) {
        return false;
    }

    m_CanvasHandler->setZoom(zoom);

    return true;
}

bool JSCanvas::JS_setScale(JSContext *cx, JS::CallArgs &args)
{
    double x, y;

    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        return false;
    }

    m_CanvasHandler->setScale(x, y);

    return true;
}

bool JSCanvas::JS_setSize(JSContext *cx, JS::CallArgs &args)
{
    int width, height;

    if (!JS_ConvertArguments(cx, args, "ii", &width, &height)) {
        return false;
    }

    m_CanvasHandler->setSize(width, height);

    return true;
}

bool JSCanvas::JS_removeFromParent(JSContext *cx, JS::CallArgs &args)
{
    m_CanvasHandler->removeFromParent();

    return true;
}

bool JSCanvas::JS_bringToFront(JSContext *cx, JS::CallArgs &args)
{
    m_CanvasHandler->bringToFront();

    return true;
}

bool JSCanvas::JS_sendToBack(JSContext *cx, JS::CallArgs &args)
{
    m_CanvasHandler->sendToBack();

    return true;
}

bool JSCanvas::JS_getParent(JSContext *cx, JS::CallArgs &args)
{
    CanvasHandler *parent = m_CanvasHandler->getParent();
    if (parent) {
        args.rval().setObjectOrNull(parent->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

bool JSCanvas::JS_getFirstChild(JSContext *cx, JS::CallArgs &args)
{
    CanvasHandler *val = m_CanvasHandler->getFirstChild();
    if (val) {
        args.rval().setObjectOrNull(val->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

bool JSCanvas::JS_getLastChild(JSContext *cx, JS::CallArgs &args)
{
    CanvasHandler *val = m_CanvasHandler->getLastChild();
    if (val) {
        args.rval().setObjectOrNull(val->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

bool JSCanvas::JS_getNextSibling(JSContext *cx, JS::CallArgs &args)
{
    CanvasHandler *val = m_CanvasHandler->getNextSibling();
    if (val) {
        args.rval().setObjectOrNull(val->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

bool JSCanvas::JS_getPrevSibling(JSContext *cx, JS::CallArgs &args)
{
    CanvasHandler *val = m_CanvasHandler->getPrevSibling();
    if (val) {
        args.rval().setObjectOrNull(val->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

bool JSCanvas::JS_getChildren(JSContext *cx, JS::CallArgs &args)
{
    uint32_t i;
    int32_t count = m_CanvasHandler->countChildren();

    if (!count) {
        JS::RootedObject retObj(cx, JS_NewArrayObject(cx, 0));
        args.rval().setObjectOrNull(retObj);
        return true;
    }

    CanvasHandler *list[count];

    m_CanvasHandler->getChildren(list);
    JS::RootedObject jlist(cx, JS_NewArrayObject(cx, count));
    for (i = 0; i < count; i++) {
        JS::RootedValue objVal(cx, JS::ObjectValue(*list[i]->m_JsObj));
        JS_SetElement(cx, jlist, i, objVal);
    }
    JS::RootedValue val(cx, JS::ObjectValue(*jlist));

    args.rval().set(val);

    return true;
}

bool JSCanvas::JS_getVisibleRect(JSContext *cx, JS::CallArgs &args)
{
    Rect rect = m_CanvasHandler->getVisibleRect();
    JS::RootedObject ret(cx, JS_NewPlainObject(cx));

#define SET_PROP(where, name, val)                        \
    JS_DefineProperty(cx, where, (const char *)name, val, \
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    SET_PROP(ret, "left", rect.m_fLeft);
    SET_PROP(ret, "top", rect.m_fTop);
    SET_PROP(ret, "width", nidium_max(0, rect.m_fRight - rect.m_fLeft));
    SET_PROP(ret, "height", nidium_max(0, rect.m_fBottom - rect.m_fTop));
#undef SET_PROP

    args.rval().setObjectOrNull(ret);

    return true;
}

bool JSCanvas::JS_setCoordinates(JSContext *cx, JS::CallArgs &args)
{
    double left, top;

    if (!JS_ConvertArguments(cx, args, "dd", &left, &top)) {
        return false;
    }

    m_CanvasHandler->m_Left = left;
    m_CanvasHandler->m_Top  = top;

    return true;
}

bool JSCanvas::JS_translate(JSContext *cx, JS::CallArgs &args)
{
    double left, top;

    if (!JS_ConvertArguments(cx, args, "dd", &left, &top)) {
        return false;
    }

    m_CanvasHandler->translate(left, top);

    return true;
}

bool JSCanvas::JS_addSubCanvas(JSContext *cx, JS::CallArgs &args)
{
    CanvasHandler *handler = NULL;
    JSCanvas *csub;

    JS::RootedObject sub(cx);
    if (!JS_ConvertArguments(cx, args, "o", sub.address())) {
        return false;
    }

    if ((csub = JSCanvas::GetInstance(sub)) == NULL) {
        JS_ReportError(cx, "add() First parameter is not a Canvas Object");
        return false;
    }

    if ((handler = csub->getHandler()) == NULL) {
        return true;
    }

    if (m_CanvasHandler == handler) {
        JS_ReportError(cx, "Canvas: can't add to itself");
        return false;
    }

    m_CanvasHandler->addChild(handler);

    return true;
}

bool JSCanvas::JS_insertBefore(JSContext *cx, JS::CallArgs &args)
{
    CanvasHandler *handler_insert = NULL, *handler_ref = NULL;

    JS::RootedObject ref(cx);
    JS::RootedObject insert(cx);

    if (!JS_ConvertArguments(cx, args, "oo", insert.address(), ref.address())) {
        return false;
    }

    if (!insert || !JSCanvas::InstanceOf(insert)) {
        JS_ReportError(cx, "add() First parameter is not a Canvas Object");
        return false;
    }

    if ((handler_insert = JSCanvas::GetInstanceUnsafe(insert)->getHandler())
        == NULL) {

        return true;
    }

    if (ref && JSCanvas::InstanceOf(ref)) {
        handler_ref = JSCanvas::GetInstanceUnsafe(ref)->getHandler();
    }

    if (m_CanvasHandler == handler_insert) {
        JS_ReportError(cx, "Canvas: can't add to itself");
        return false;
    }

    m_CanvasHandler->insertBefore(handler_insert, handler_ref);

    return true;
}

bool JSCanvas::JS_insertAfter(JSContext *cx, JS::CallArgs &args)
{
    CanvasHandler *handler_insert = NULL, *handler_ref = NULL;

    JS::RootedObject insert(cx);
    JS::RootedObject ref(cx);

    if (!JS_ConvertArguments(cx, args, "oo", insert.address(), ref.address())) {
        return false;
    }

    if (!insert || !JSCanvas::InstanceOf(insert)) {
        JS_ReportError(cx, "add() First parameter is not a Canvas Object");
        return false;
    }

    if ((handler_insert = JSCanvas::GetInstanceUnsafe(insert)->getHandler())
        == NULL) {

        return true;
    }

    if (ref && JSCanvas::InstanceOf(ref)) {
        handler_ref = JSCanvas::GetInstanceUnsafe(ref)->getHandler();
    }

    if (m_CanvasHandler == handler_insert) {
        JS_ReportError(cx, "Canvas: can't add to itself");
        return false;
    }

    m_CanvasHandler->insertAfter(handler_insert, handler_ref);

    return true;
}

bool JSCanvas::JS_requestPaint(JSContext *cx, JS::CallArgs &args)
{
    m_CanvasHandler->invalidate();

    args.rval().setBoolean(true);

    return true;
}

bool JSCanvas::JS_getContext(JSContext *cx, JS::CallArgs &args)
{

    Context *nctx   = Context::GetObject<Frontend::Context>(cx);
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

    CanvasContext *canvasctx = m_CanvasHandler->getContext();
    JS::RootedObject JSCanvasCtx(cx);

    /* The context is lazy-created */
    if (canvasctx == NULL) {
        switch (ctxmode) {
            case CanvasContext::CONTEXT_2D: {
                Canvas2DContext *ctx2d = new Canvas2DContext(
                    m_CanvasHandler, cx,
                    m_CanvasHandler->getWidth()
                        + (m_CanvasHandler->m_Padding.global * 2),
                    m_CanvasHandler->getHeight()
                        + (m_CanvasHandler->m_Padding.global * 2),
                    ui);

                if (ctx2d->getSkiaContext() == NULL) {
                    delete ctx2d;
                    JS_ReportError(
                        cx, "Could not create 2D context for this canvas");
                    return false;
                }
                m_CanvasHandler->setContext(ctx2d);

                /* Inherit from the Context glstate */
                ctx2d->setGLState(nctx->getGLState());

                JSCanvasCtx = Canvas2DContext::CreateObject(cx, ctx2d);

                break;
            }
            case CanvasContext::CONTEXT_WEBGL:
                JSWebGLRenderingContext *ctxWebGL = new JSWebGLRenderingContext(
                    m_CanvasHandler, cx,
                    m_CanvasHandler->getWidth()
                        + (m_CanvasHandler->m_Padding.global * 2),
                    m_CanvasHandler->getHeight()
                        + (m_CanvasHandler->m_Padding.global * 2),
                    ui);

                m_CanvasHandler->setContext(static_cast<Canvas3DContext *>(ctxWebGL));

                JSCanvasCtx = JSWebGLRenderingContext::CreateObject(cx, ctxWebGL);

                break;
        }

        canvasctx = m_CanvasHandler->getContext();

        assert(JSCanvasCtx != nullptr);

        /*  Protect against GC
            Canvas.slot[0] = context
        */
        JS_SetReservedSlot(m_Instance, 0, JS::ObjectValue(*JSCanvasCtx));

    } else if (canvasctx->m_Mode != ctxmode) {
        JS_ReportWarning(cx, "Bad context requested");
        /* A mode is requested but another one was already created */
        args.rval().setNull();
        return true;
    } else {
        JSCanvasCtx = canvasctx->getJSInstance();
    }

    args.rval().setObjectOrNull(JSCanvasCtx);

    return true;
}

bool JSCanvas::JS_setContext(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedObject obj(cx, args[0].toObjectOrNull());
    if (!obj.get()) {
        return true;
    }

    CanvasContext *context = Canvas2DContext::GetInstance(obj);

    if (!context) {
        JS_ReportError(
            cx, "setContext() argument must a CanvasRenderingContext2D object");
        return false;
    }

    m_CanvasHandler->setContext(context);

    /*
        If a context was already attached, it's going to be GC'd
        since it's not longer reachable from slot 0.
    */
    JS::RootedValue slot(cx, JS::ObjectValue(*obj));
    JS_SetReservedSlot(m_CanvasHandler->m_JsObj, 0, slot);

    return true;
}

bool JSCanvas::JSSetter_scrollLeft(JSContext *cx, JS::MutableHandleValue vp)
{
    int32_t dval;

    if (!JS::ToInt32(cx, vp, &dval)) {
        return true;
    }

    m_CanvasHandler->setScrollLeft(dval);

    return true;
}

bool JSCanvas::JSSetter_scrollTop(JSContext *cx, JS::MutableHandleValue vp)
{
    int32_t dval;

    if (!JS::ToInt32(cx, vp, &dval)) {
        return true;
    }

    m_CanvasHandler->setScrollTop(dval);

    return true;
}

bool JSCanvas::JSSetter_allowNegativeScroll(JSContext *cx, JS::MutableHandleValue vp)
{
    if (!vp.isBoolean()) {

        return true;
    }

    m_CanvasHandler->setAllowNegativeScroll(vp.toBoolean());

    return true;
}

bool JSCanvas::JSSetter_width(JSContext *cx, JS::MutableHandleValue vp)
{
    uint32_t dval;

    if (!JS::ToUint32(cx, vp, &dval)) {
        return true;
    }

    if (!m_CanvasHandler->setWidth(dval)) {
        // JS_ReportError(cx, "Can't set canvas width (this canvas has a
        // dynamic width)");

        return true;
    }

    return true;
}

bool JSCanvas::JSSetter_height(JSContext *cx, JS::MutableHandleValue vp)
{
    uint32_t dval;

    if (!JS::ToUint32(cx, vp, &dval)) {
        return true;
    }

    if (!m_CanvasHandler->setHeight(dval)) {
        return true;
    }

    return true;
}

bool JSCanvas::JSSetter_left(JSContext *cx, JS::MutableHandleValue vp)
{
    double dval;

    if (!m_CanvasHandler->hasStaticLeft()) {
        return true;
    }

    if (vp.isNullOrUndefined()) {
        m_CanvasHandler->unsetLeft();

        return true;
    }

    if (!JS::ToNumber(cx, vp, &dval)) {
        return true;
    }

    m_CanvasHandler->setLeft(dval);

    return true;
}

bool JSCanvas::JSSetter_right(JSContext *cx, JS::MutableHandleValue vp)
{
    double dval;

    if (!m_CanvasHandler->hasStaticRight()) {
        return true;
    }

    if (vp.isNullOrUndefined()) {
        m_CanvasHandler->unsetRight();

        return true;
    }

    if (!JS::ToNumber(cx, vp, &dval)) {
        return true;
    }

    m_CanvasHandler->setRight(dval);

    return true;
}

bool JSCanvas::JSSetter_top(JSContext *cx, JS::MutableHandleValue vp)
{
    double dval;

    if (!m_CanvasHandler->hasStaticTop()) {
        return true;
    }

    if (vp.isNullOrUndefined()) {
        m_CanvasHandler->unsetTop();

        return true;
    }

    if (!JS::ToNumber(cx, vp, &dval)) {
        return true;
    }

    m_CanvasHandler->setTop(dval);

    return true;
}

bool JSCanvas::JSSetter_bottom(JSContext *cx, JS::MutableHandleValue vp)
{
    double dval;

    if (!m_CanvasHandler->hasStaticBottom()) {
        return true;
    }

    if (vp.isNullOrUndefined()) {
        m_CanvasHandler->unsetBottom();

        return true;
    }

    if (!JS::ToNumber(cx, vp, &dval)) {
        return true;
    }

    m_CanvasHandler->setBottom(dval);

    return true;
}

bool JSCanvas::JSSetter_minWidth(JSContext *cx, JS::MutableHandleValue vp)
{
    uint32_t dval;

    if (!JS::ToUint32(cx, vp, &dval)) {
        return true;
    }

    if (!m_CanvasHandler->setMinWidth(dval)) {
        return true;
    }

    return true;
}

bool JSCanvas::JSSetter_minHeight(JSContext *cx, JS::MutableHandleValue vp)
{
    uint32_t dval;

    if (!JS::ToUint32(cx, vp, &dval)) {
        return true;
    }

    if (!m_CanvasHandler->setMinHeight(dval)) {
        return true;
    }

    return true;
}

bool JSCanvas::JSSetter_maxWidth(JSContext *cx, JS::MutableHandleValue vp)
{
    uint32_t dval;

    if (!JS::ToUint32(cx, vp, &dval)) {
        return true;
    }

    if (!m_CanvasHandler->setMaxWidth(dval)) {
        return true;
    }

    return true;
}

bool JSCanvas::JSSetter_maxHeight(JSContext *cx, JS::MutableHandleValue vp)
{
    uint32_t dval;

    if (!JS::ToUint32(cx, vp, &dval)) {
        return true;
    }

    if (!m_CanvasHandler->setMaxHeight(dval)) {
        return true;
    }

    return true;
}

bool JSCanvas::JSSetter_visible(JSContext *cx, JS::MutableHandleValue vp)
{
    if (!vp.isBoolean()) {

        return true;
    }

    m_CanvasHandler->setHidden(!vp.toBoolean());

    return true;
}

bool JSCanvas::JSSetter_coating(JSContext *cx, JS::MutableHandleValue vp)
{
    int32_t dval;

    if (!JS::ToInt32(cx, vp, &dval)) {
        return true;
    }

    m_CanvasHandler->setPadding(dval);

    return true;
}

bool JSCanvas::JSSetter_staticLeft(JSContext *cx, JS::MutableHandleValue vp)
{
    if (!vp.isBoolean()) {

        return true;
    }

    if (vp.toBoolean()) {
        m_CanvasHandler->setLeft(m_CanvasHandler->m_Left);
    } else {
        m_CanvasHandler->unsetLeft();
    }

    return true;
}

bool JSCanvas::JSSetter_staticTop(JSContext *cx, JS::MutableHandleValue vp)
{
    if (!vp.isBoolean()) {

        return true;
    }

    if (vp.toBoolean()) {
        m_CanvasHandler->setTop(m_CanvasHandler->m_Top);
    } else {
        m_CanvasHandler->unsetTop();
    }

    return true;
}

bool JSCanvas::JSSetter_staticRight(JSContext *cx, JS::MutableHandleValue vp)
{
    if (!vp.isBoolean()) {

        return true;
    }

    if (vp.toBoolean()) {
        m_CanvasHandler->setRight(m_CanvasHandler->m_Right);
    } else {
        m_CanvasHandler->unsetRight();
    }

    return true;
}

bool JSCanvas::JSSetter_staticBottom(JSContext *cx, JS::MutableHandleValue vp)
{
    if (!vp.isBoolean()) {

        return true;
    }
    if (vp.toBoolean()) {
        m_CanvasHandler->setBottom(m_CanvasHandler->m_Bottom);
    } else {
        m_CanvasHandler->unsetBottom();
    }

    return true;
}

bool JSCanvas::JSSetter_marginLeft(JSContext *cx, JS::MutableHandleValue vp)
{
    double dval;

    if (!JS::ToNumber(cx, vp, &dval)) {
        return true;
    }

    m_CanvasHandler->setMargin(
                m_CanvasHandler->m_Margin.top,
                m_CanvasHandler->m_Margin.right,
                m_CanvasHandler->m_Margin.bottom,
                dval);

    return true;
}

bool JSCanvas::JSSetter_marginTop(JSContext *cx, JS::MutableHandleValue vp)
{
    double dval;

    if (!JS::ToNumber(cx, vp, &dval)) {
        return true;
    }

    m_CanvasHandler->setMargin(
                dval,
                m_CanvasHandler->m_Margin.right,
                m_CanvasHandler->m_Margin.bottom,
                m_CanvasHandler->m_Margin.left);

    return true;
}

bool JSCanvas::JSSetter_marginRight(JSContext *cx, JS::MutableHandleValue vp)
{
    double dval;

    if (!JS::ToNumber(cx, vp, &dval)) {
        return true;
    }

    m_CanvasHandler->setMargin(
                m_CanvasHandler->m_Margin.top,
                dval,
                m_CanvasHandler->m_Margin.bottom,
                m_CanvasHandler->m_Margin.left);

    return true;
}

bool JSCanvas::JSSetter_marginBottom(JSContext *cx, JS::MutableHandleValue vp)
{
    double dval;

    if (!JS::ToNumber(cx, vp, &dval)) {
        return true;
    }

    m_CanvasHandler->setMargin(
                m_CanvasHandler->m_Margin.top,
                m_CanvasHandler->m_Margin.right,
                dval,
                m_CanvasHandler->m_Margin.left);

    return true;
}

bool JSCanvas::JSSetter_position(JSContext *cx, JS::MutableHandleValue vp)
{
    if (!vp.isString()) {
        vp.setNull();
        return true;
    }
    JS::RootedString vpStr(cx, JS::ToString(cx, vp));
    JSAutoByteString mode(cx, vpStr);

    if (strcasecmp(mode.ptr(), "absolute") == 0) {
        m_CanvasHandler->setPositioning(CanvasHandler::COORD_ABSOLUTE);
    } else if (strcasecmp(mode.ptr(), "fixed") == 0) {
        m_CanvasHandler->setPositioning(CanvasHandler::COORD_FIXED);
    } else if (strcasecmp(mode.ptr(), "inline") == 0) {
        m_CanvasHandler->setPositioning(CanvasHandler::COORD_INLINE);
    } else if (strcasecmp(mode.ptr(), "inline-break") == 0) {
        m_CanvasHandler->setPositioning(CanvasHandler::COORD_INLINEBREAK);
    } else {
        m_CanvasHandler->setPositioning(CanvasHandler::COORD_RELATIVE);
    }

    return true;
}

bool JSCanvas::JSSetter_id(JSContext *cx, JS::MutableHandleValue vp)
{
    JS::RootedString sid(cx, JS::ToString(cx, vp));

    if (!sid.get()) {
        return true;
    }

    JSAutoByteString cid(cx, sid);

    m_CanvasHandler->setId(cid.ptr());

    return true;
}

bool JSCanvas::JSSetter_fluidWidth(JSContext *cx, JS::MutableHandleValue vp)
{
    if (!vp.isBoolean()) {
        return true;
    }

    m_CanvasHandler->setFluidWidth(vp.toBoolean());

    return true;
}

bool JSCanvas::JSSetter_fluidHeight(JSContext *cx, JS::MutableHandleValue vp)
{
    if (!vp.isBoolean()) {
        return true;
    }

    m_CanvasHandler->setFluidHeight(vp.toBoolean());

    return true;
}

bool JSCanvas::JSSetter_cursor(JSContext *cx, JS::MutableHandleValue vp)
{
    if (!vp.isString()) {

        return true;
    }

    JS::RootedString vpStr(cx, vp.toString());
    JSAutoByteString type(cx, vpStr);
    for (int i = 0; nidium_cursors_list[i].str != NULL; i++) {
        if (strncasecmp(nidium_cursors_list[i].str, type.ptr(),
                        strlen(nidium_cursors_list[i].str)) == 0) {

            m_CanvasHandler->setCursor(nidium_cursors_list[i].type);
            break;
        }
    }

    return true;
}


bool JSCanvas::JSGetter_cursor(JSContext *cx, JS::MutableHandleValue vp)
{
    int cursor = m_CanvasHandler->getCursor();

    for (int i = 0; nidium_cursors_list[i].str != NULL; i++) {
        if (nidium_cursors_list[i].type == cursor) {
            vp.setString(
                JS_NewStringCopyZ(cx, nidium_cursors_list[i].str));

            break;
        }
    }
    return true;
}

bool JSCanvas::JSGetter_clientWidth(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->getWidth() +
        (m_CanvasHandler->m_Padding.global * 2));

    return true;
}

bool JSCanvas::JSGetter_clientHeight(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->getHeight() +
        (m_CanvasHandler->m_Padding.global * 2));

    return true;
}

bool JSCanvas::JSGetter_clientTop(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->getTop() - m_CanvasHandler->m_Padding.global);

    return true;
}

bool JSCanvas::JSGetter_clientLeft(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->getLeft() - m_CanvasHandler->m_Padding.global);

    return true;
}

bool JSCanvas::JSGetter_contentWidth(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->getContentWidth());

    return true;
}

bool JSCanvas::JSGetter_contentHeight(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->getContentHeight());

    return true;
}

bool JSCanvas::JSGetter_innerWidth(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->getContentWidth(true));

    return true;
}

bool JSCanvas::JSGetter_innerHeight(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->getContentHeight(true));

    return true;
}

bool JSCanvas::JSGetter___visible(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setBoolean(m_CanvasHandler->isDisplayed());

    return true;
}

bool JSCanvas::JSGetter___top(JSContext *cx, JS::MutableHandleValue vp)
{
    m_CanvasHandler->computeAbsolutePosition();
    vp.setDouble(m_CanvasHandler->getTop(true));

    return true;
}

bool JSCanvas::JSGetter___left(JSContext *cx, JS::MutableHandleValue vp)
{
    m_CanvasHandler->computeAbsolutePosition();
    vp.setDouble(m_CanvasHandler->getLeft(true));

    return true;
}

bool JSCanvas::JSGetter___fixed(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setBoolean(m_CanvasHandler->hasAFixedAncestor());

    return true;
}

bool JSCanvas::JSGetter___outofbound(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setBoolean(m_CanvasHandler->isOutOfBound());

    return true;
}

bool JSCanvas::JSGetter_ctx(JSContext *cx, JS::MutableHandleValue vp)
{
    if (m_CanvasHandler->m_Context == NULL) {
        vp.setNull();

        return true;
    }
    JS::RootedObject retObj(cx, m_CanvasHandler->m_Context->getJSInstance());
    vp.setObjectOrNull(retObj);

    return true;
}

bool JSCanvas::JSGetter_opacity(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setDouble(m_CanvasHandler->getOpacity());

    return true;
}

bool JSCanvas::JSSetter_opacity(JSContext *cx, JS::MutableHandleValue vp)
{
    double dval;

    if (!JS::ToNumber(cx, vp, &dval)) {

        return true;
    }
    
    m_CanvasHandler->setOpacity(dval);

    return true;
}

bool JSCanvas::JSGetter_overflow(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setBoolean(m_CanvasHandler->m_Overflow);

    return true;
}

bool JSCanvas::JSSetter_overflow(JSContext *cx, JS::MutableHandleValue vp)
{
    if (!vp.isBoolean()) {

        return true;
    }

    m_CanvasHandler->m_Overflow = vp.toBoolean();

    return true;
}

bool JSCanvas::JSGetter_scrollLeft(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->m_Content.scrollLeft);

    return true;
}

bool JSCanvas::JSGetter_scrollTop(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->m_Content.scrollTop);

    return true;
}

bool JSCanvas::JSGetter_allowNegativeScroll(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setBoolean(m_CanvasHandler->getAllowNegativeScroll());

    return true;
}

bool JSCanvas::JSGetter_width(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->getWidth());

    return true;
}

bool JSCanvas::JSGetter_height(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->getHeight());

    return true;
}

bool JSCanvas::JSGetter_maxWidth(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->getMaxWidth());

    return true;
}

bool JSCanvas::JSGetter_maxHeight(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->getMaxHeight());

    return true;
}

bool JSCanvas::JSGetter_minWidth(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->getMinWidth());

    return true;
}

bool JSCanvas::JSGetter_minHeight(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->getMinHeight());
    
    return true;
}

bool JSCanvas::JSGetter_position(JSContext *cx, JS::MutableHandleValue vp)
{
    JS::RootedString jstr(cx);

    switch (m_CanvasHandler->getPositioning()) {
        case CanvasHandler::COORD_RELATIVE:
            if (m_CanvasHandler->getFlowMode()
                & CanvasHandler::kFlowBreakPreviousSibling) {
                jstr = JS_NewStringCopyZ(cx, "inline-break");
                vp.setString(jstr);
            } else if (m_CanvasHandler->getFlowMode()
                       & CanvasHandler::kFlowInlinePreviousSibling) {
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
    
    return true;
}

bool JSCanvas::JSGetter_top(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setDouble(m_CanvasHandler->getTop());
    
    return true;
}

bool JSCanvas::JSGetter_left(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setDouble(m_CanvasHandler->getLeft());
    
    return true;
}

bool JSCanvas::JSGetter_right(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setDouble(m_CanvasHandler->getRight());
    
    return true;
}

bool JSCanvas::JSGetter_bottom(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setDouble(m_CanvasHandler->getBottom());
    
    return true;
}

bool JSCanvas::JSGetter_visible(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setBoolean(!m_CanvasHandler->isHidden());
    
    return true;
}

bool JSCanvas::JSGetter_staticLeft(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setBoolean(m_CanvasHandler->hasStaticLeft());
    
    return true;
}

bool JSCanvas::JSGetter_staticTop(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setBoolean(m_CanvasHandler->hasStaticTop());
    
    return true;
}

bool JSCanvas::JSGetter_staticRight(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setBoolean(m_CanvasHandler->hasStaticRight());
    
    return true;
}

bool JSCanvas::JSGetter_staticBottom(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setBoolean(m_CanvasHandler->hasStaticBottom());
    
    return true;
}

bool JSCanvas::JSGetter_fluidWidth(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setBoolean(m_CanvasHandler->isWidthFluid());
    
    return true;
}

bool JSCanvas::JSGetter_fluidHeight(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setBoolean(m_CanvasHandler->isHeightFluid());
    
    return true;
}

bool JSCanvas::JSGetter_marginLeft(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setDouble(m_CanvasHandler->m_Margin.left);
    
    return true;
}

bool JSCanvas::JSGetter_marginRight(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setDouble(m_CanvasHandler->m_Margin.right);
    
    return true;
}

bool JSCanvas::JSGetter_marginTop(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setDouble(m_CanvasHandler->m_Margin.top);
    
    return true;
}

bool JSCanvas::JSGetter_marginBottom(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setDouble(m_CanvasHandler->m_Margin.bottom);
    
    return true;
}

bool JSCanvas::JSGetter_coating(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_CanvasHandler->m_Padding.global);
    
    return true;
}

bool JSCanvas::JSGetter_id(JSContext *cx, JS::MutableHandleValue vp)
{
    char *cid;
    m_CanvasHandler->getIdentifier(&cid);

    if (cid == nullptr) {
        vp.setNull();

        return true;
    }

    JS::RootedString jstr(cx, JS_NewStringCopyZ(cx, cid));
    vp.setString(jstr);
    
    return true;
}

bool JSCanvas::JSGetter_idx(JSContext *cx, JS::MutableHandleValue vp)
{
    uint64_t idx = m_CanvasHandler->getIdentifier();

    vp.setNumber((double)idx);
    
    return true;
}

JSCanvas *JSCanvas::Constructor(JSContext *cx, JS::CallArgs &args,
    JS::HandleObject obj)
{
    int width, height;
    bool lazyLoad;
    CanvasHandler *handler;

    JS::RootedObject opt(cx);
    if (!JS_ConvertArguments(cx, args, "ii/o", &width, &height,
                             opt.address())) {
        return nullptr;
    }

    NIDIUM_JS_INIT_OPT();
    NIDIUM_JS_GET_OPT_TYPE(opt, "lazy", Boolean)
    {
        lazyLoad = __curopt.toBoolean();
    }
    lazyLoad = false; /* Always lazy load for now.  */

    handler = new CanvasHandler(width, height,
        Context::GetObject<Frontend::Context>(cx), true);

    handler->m_Context = NULL;
    handler->m_JsCx    = cx;
    handler->m_JsObj   = obj;

    return new JSCanvas(handler);
}

JSFunctionSpec *JSCanvas::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSCanvas, requestPaint, 0),
        CLASSMAPPER_FN(JSCanvas, getContext, 1),
        CLASSMAPPER_FN(JSCanvas, setContext, 1),
        CLASSMAPPER_FN(JSCanvas, addSubCanvas, 1),
        CLASSMAPPER_FN_ALIAS(JSCanvas, add, 1, addSubCanvas),
        CLASSMAPPER_FN(JSCanvas, insertBefore, 2),
        CLASSMAPPER_FN(JSCanvas, insertAfter, 2),
        CLASSMAPPER_FN(JSCanvas, removeFromParent, 0),
        CLASSMAPPER_FN(JSCanvas, show, 0),
        CLASSMAPPER_FN(JSCanvas, hide, 0),
        CLASSMAPPER_FN(JSCanvas, bringToFront, 0),
        CLASSMAPPER_FN(JSCanvas, sendToBack, 0),
        CLASSMAPPER_FN(JSCanvas, getParent, 0),
        CLASSMAPPER_FN(JSCanvas, getFirstChild, 0),
        CLASSMAPPER_FN(JSCanvas, getLastChild, 0),
        CLASSMAPPER_FN(JSCanvas, getNextSibling, 0),
        CLASSMAPPER_FN(JSCanvas, getPrevSibling, 0),
        CLASSMAPPER_FN(JSCanvas, getChildren, 0),
        CLASSMAPPER_FN(JSCanvas, setCoordinates, 2),
        CLASSMAPPER_FN(JSCanvas, translate, 2),
        CLASSMAPPER_FN(JSCanvas, getVisibleRect, 0),
        CLASSMAPPER_FN(JSCanvas, setSize, 2),
        CLASSMAPPER_FN(JSCanvas, clear, 0),
        CLASSMAPPER_FN(JSCanvas, setZoom, 1),
        CLASSMAPPER_FN(JSCanvas, setScale, 2),
        JS_FS_END
    };

    return funcs;
}

JSPropertySpec *JSCanvas::ListProperties()
{
    static JSPropertySpec props[] = {
        CLASSMAPPER_PROP_GS(JSCanvas, opacity),
        CLASSMAPPER_PROP_GS(JSCanvas, overflow),
        CLASSMAPPER_PROP_GS(JSCanvas, scrollLeft),
        CLASSMAPPER_PROP_GS(JSCanvas, scrollTop),
        CLASSMAPPER_PROP_GS(JSCanvas, allowNegativeScroll),
        CLASSMAPPER_PROP_GS(JSCanvas, width),
        CLASSMAPPER_PROP_GS(JSCanvas, coating),
        CLASSMAPPER_PROP_GS(JSCanvas, height),
        CLASSMAPPER_PROP_GS(JSCanvas, maxWidth),
        CLASSMAPPER_PROP_GS(JSCanvas, maxHeight),
        CLASSMAPPER_PROP_GS(JSCanvas, minWidth),
        CLASSMAPPER_PROP_GS(JSCanvas, minHeight),
        CLASSMAPPER_PROP_GS(JSCanvas, position),
        CLASSMAPPER_PROP_GS(JSCanvas, top),
        CLASSMAPPER_PROP_GS(JSCanvas, left),
        CLASSMAPPER_PROP_GS(JSCanvas, right),
        CLASSMAPPER_PROP_GS(JSCanvas, bottom),
        CLASSMAPPER_PROP_GS(JSCanvas, visible),
        CLASSMAPPER_PROP_GS(JSCanvas, staticLeft),
        CLASSMAPPER_PROP_GS(JSCanvas, staticRight),
        CLASSMAPPER_PROP_GS(JSCanvas, staticTop),
        CLASSMAPPER_PROP_GS(JSCanvas, staticBottom),
        CLASSMAPPER_PROP_GS(JSCanvas, fluidHeight),
        CLASSMAPPER_PROP_GS(JSCanvas, fluidWidth),
        CLASSMAPPER_PROP_GS(JSCanvas, id),
        CLASSMAPPER_PROP_GS(JSCanvas, marginLeft),
        CLASSMAPPER_PROP_GS(JSCanvas, marginRight),
        CLASSMAPPER_PROP_GS(JSCanvas, marginTop),
        CLASSMAPPER_PROP_GS(JSCanvas, marginBottom),
        CLASSMAPPER_PROP_GS(JSCanvas, cursor),

        CLASSMAPPER_PROP_G(JSCanvas, idx),
        CLASSMAPPER_PROP_G(JSCanvas, clientWidth),
        CLASSMAPPER_PROP_G(JSCanvas, clientHeight),
        CLASSMAPPER_PROP_G(JSCanvas, clientTop),
        CLASSMAPPER_PROP_G(JSCanvas, clientLeft),
        CLASSMAPPER_PROP_G(JSCanvas, contentWidth),
        CLASSMAPPER_PROP_G(JSCanvas, contentHeight),
        CLASSMAPPER_PROP_G(JSCanvas, innerWidth),
        CLASSMAPPER_PROP_G(JSCanvas, innerHeight),
        CLASSMAPPER_PROP_G(JSCanvas, __visible),
        CLASSMAPPER_PROP_G(JSCanvas, __top),
        CLASSMAPPER_PROP_G(JSCanvas, __left),
        CLASSMAPPER_PROP_G(JSCanvas, __fixed),
        CLASSMAPPER_PROP_G(JSCanvas, __outofbound),
        CLASSMAPPER_PROP_G(JSCanvas, ctx),

        JS_PS_END
    };

    return props;
}

void JSCanvas::jsTrace(class JSTracer *trc)
{
    CanvasHandler *handler = this->getHandler();

    if (handler != NULL) {
        CanvasHandler *cur;

        for (cur = handler->getFirstChild(); cur != NULL; cur = cur->m_Next) {
            if (cur->m_JsObj) {
                JS_CallTenuredObjectTracer(trc, &cur->m_JsObj, "nidiumcanvasroot");
            }
        }
    }
}

JSObject *JSCanvas::GenerateJSObject(JSContext *cx,
                                     int width,
                                     int height,
                                     CanvasHandler **out)
{
    CanvasHandler *handler;
    Context *nctx   = Context::GetObject<Frontend::Context>(cx);
    UIInterface *ui = nctx->getUI();
    handler = new CanvasHandler(width, height, nctx);
    
    Canvas2DContext *ctx2d = new Canvas2DContext(handler, cx, width, height, ui);

    JS::RootedObject ctxjsobj(cx, Canvas2DContext::CreateObject(cx, ctx2d));

    
    handler->setContext(ctx2d);
    handler->getContext()->setGLState(nctx->getGLState());

    /* window.canvas.overflow default to false */
    handler->m_Overflow = false;

    JSCanvas *jscanvas = new JSCanvas(handler);

    JS::RootedObject ret(cx, JSCanvas::CreateObject(cx, jscanvas));

    handler->m_JsCx  = cx;
    handler->m_JsObj = ret;

    JS_SetReservedSlot(ret, 0, JS::ObjectValue(*ctxjsobj));

    *out = handler;

    return ret;
}

static JS::Value TouchToJSVal(JSContext *cx, Frontend::InputTouch *touch)
{
    JSObjectBuilder obj(cx);

    JS::RootedObject targetObj(cx, touch->getTarget()->m_JsObj);

    obj.set("screenX", touch->x);
    obj.set("screenY", touch->y);
    obj.set("clientX", touch->x);
    obj.set("clientY", touch->y);
    obj.set("pageX", touch->x);
    obj.set("pageY", touch->y);

    obj.set("target", (JS::HandleObject)targetObj);
    obj.set("identifier", touch->getIdentifier());

    return obj;
}

void JSCanvas::onMessage(const SharedMessages::Message &msg)
{
    /*
        Don't process any Javascript when shutting down
    */
    if (NidiumLocalContext::Get()->isShuttingDown()) {
        return;
    }

    JSContext *cx = m_Cx;
    JS::RootedObject ro(cx, m_Instance);

    switch (msg.event()) {
        case NIDIUM_EVENT(CanvasHandler, RESIZE_EVENT): {
            JS::RootedObject eventObj(m_Cx, JSEvents::CreateEventObject(m_Cx));
            JS::RootedValue eventValue(m_Cx);

            eventValue.setObjectOrNull(eventObj);
            this->fireJSEvent("resize", &eventValue);
            break;
        }
        case NIDIUM_EVENT(CanvasHandler, PAINT_EVENT): {
            JS::RootedObject eventObj(m_Cx, JSEvents::CreateEventObject(m_Cx));
            JS::RootedValue eventValue(m_Cx);

            eventValue.setObjectOrNull(eventObj);
            this->fireJSEvent("paint", &eventValue);
            break;
        }        
        case NIDIUM_EVENT(CanvasHandler, LOADED_EVENT): {
            JS::RootedObject eventObj(m_Cx, JSEvents::CreateEventObject(m_Cx));
            JS::RootedValue eventValue(m_Cx);

            eventValue.setObjectOrNull(eventObj);
            this->fireJSEvent("load", &eventValue);
            break;
        }
        case NIDIUM_EVENT(CanvasHandler, MOUNT_EVENT): {
            JS::RootedObject eventObj(m_Cx, JSEvents::CreateEventObject(m_Cx));
            JS::RootedValue eventValue(m_Cx);

            eventValue.setObjectOrNull(eventObj);
            this->fireJSEvent("mount", &eventValue);
            break;
        }
        case NIDIUM_EVENT(CanvasHandler, UNMOUNT_EVENT): {
            JS::RootedObject eventObj(m_Cx, JSEvents::CreateEventObject(m_Cx));
            JS::RootedValue eventValue(m_Cx);

            eventValue.setObjectOrNull(eventObj);
            this->fireJSEvent("unmount", &eventValue);
            break;
        }
        case NIDIUM_EVENT(CanvasHandler, CHANGE_EVENT): {
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
        case NIDIUM_EVENT(CanvasHandler, DRAG_EVENT): {
            printf("Drag event detected\n");
        }
        case NIDIUM_EVENT(CanvasHandler, MOUSE_EVENT): {
            JS::RootedObject eventObj(m_Cx, JSEvents::CreateEventObject(m_Cx));
            CanvasHandler *target
                = static_cast<CanvasHandler *>(msg.m_Args[8].toPtr());
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

            if (!this->fireInputEvent(msg.m_Args[1].toInt(), eventObj, msg)) {
                break;
            }
        } break;
        case NIDIUM_EVENT(CanvasHandler, TOUCH_EVENT): {
            Frontend::InputHandler *inputHandler
                = Context::GetObject<Frontend::Context>(m_Cx)->getInputHandler();
            std::vector<std::shared_ptr<Frontend::InputTouch>> touches
                = inputHandler->getTouches();
            std::set<std::shared_ptr<Frontend::InputTouch>> changedTouches
                = inputHandler->getChangedTouches();
            InputEvent::Type evType = static_cast<InputEvent::Type>(msg.m_Args[1].toInt());

            JS::AutoValueVector jsTouchesVector(m_Cx);
            JS::AutoValueVector jsTargetTouchesVector(m_Cx);
            for(auto const& touch : touches) {
                if (!touch) continue;

                jsTouchesVector.append(TouchToJSVal(m_Cx, touch.get()));

                if (touch->hasOrigin(this->getHandler())) {
                    jsTargetTouchesVector.append(TouchToJSVal(m_Cx, touch.get()));
                }
            }

            JS::AutoValueVector jsChangedTouchesVector(m_Cx);
            if (evType == InputEvent::kTouchStart_Type || evType == InputEvent::kTouchEnd_Type) {
                Frontend::InputTouch *touch
                    = static_cast<Frontend::InputTouch *>(msg.m_Args[2].toPtr());
                if (touch) {
                    jsChangedTouchesVector.append(TouchToJSVal(m_Cx, touch));
                }
            } else {
                for(auto const& touch : changedTouches) {
                    if (!touch) continue;
                    jsChangedTouchesVector.append(TouchToJSVal(m_Cx, touch.get()));
                }
            }

            JS::RootedObject eventObj(m_Cx, JSEvents::CreateEventObject(m_Cx));
            JSObjectBuilder obj(m_Cx, eventObj);

            JS::RootedObject jsTouchesObj(m_Cx, JS_NewArrayObject(m_Cx, jsTouchesVector));
            JS::RootedObject jsChangedTouchesObj(m_Cx, JS_NewArrayObject(m_Cx, jsChangedTouchesVector));
            JS::RootedObject jsTargetTouchesObj(m_Cx, JS_NewArrayObject(m_Cx, jsTargetTouchesVector));

            obj.set("touches", (JS::HandleObject)jsTouchesObj);
            obj.set("changedTouches", (JS::HandleObject)jsChangedTouchesObj);
            obj.set("targetTouches", (JS::HandleObject)jsTargetTouchesObj);

            if (!this->fireInputEvent(msg.m_Args[1].toInt(), eventObj, msg)) {
                break;
            }
        } break;
        default:
            break;
    }
}

void JSCanvas::onMessageLost(const SharedMessages::Message &msg)
{
}

JSCanvas::JSCanvas(CanvasHandler *handler)
    : m_CanvasHandler(handler)
{
    m_CanvasHandler->addListener(this);

    /*
        Trigger "loaded" event if not lazy loaded
    */
    m_CanvasHandler->checkLoaded(true);
}

bool JSCanvas::fireInputEvent(int ev, JS::HandleObject evObj, const Core::SharedMessages::Message &msg)
{
    JS::RootedValue evVal(m_Cx);
    evVal.setObjectOrNull(evObj);
    if (!this->fireJSEvent(InputEvent::GetName(ev), &evVal)) {
        return false;
    }

    JS::RootedValue cancelBubble(m_Cx);
    if (JS_GetProperty(m_Cx, evObj, "cancelBubble", &cancelBubble)) {
        if (cancelBubble.isBoolean() && cancelBubble.toBoolean()) {
            /* TODO: sort out this dirty hack */
            SharedMessages::Message *nonconstmsg
                = (SharedMessages::Message *)&msg;
            nonconstmsg->m_Priv = 1;
        }
    }

    return true;
}

JSCanvas::~JSCanvas()
{
    delete m_CanvasHandler;
}
// }}}

void JSCanvas::RegisterObject(JSContext *cx)
{
    JSCanvas::ExposeClass(cx, "Canvas",
        JSCLASS_HAS_RESERVED_SLOTS(1), kJSTracer_ExposeFlag);

}

} // namespace Binding
} // namespace Nidium
