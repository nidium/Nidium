/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSWindow.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>

#include "Binding/JSFile.h"
#include "Binding/JSUtils.h"

#include "Interface/SystemInterface.h"
#include "Interface/UIInterface.h"

#include "Frontend/Context.h"
#include "Frontend/InputHandler.h"
#include "Graphics/Image.h"
#include "Graphics/SkiaContext.h"
#include "Binding/JSCanvas.h"
#include "Binding/JSImage.h"

using Nidium::Frontend::NML;
using Nidium::Frontend::NMLTag;
using Nidium::Frontend::Context;
using Nidium::Frontend::InputEvent;
using Nidium::Graphics::Image;
using Nidium::Graphics::SkiaContext;
using Nidium::Graphics::CanvasHandler;
using namespace Nidium::Interface;

namespace Nidium {
namespace Binding {

// {{{ Preamble
static JSClass MouseEvent_class = { "MouseEvent", 0};
static JSClass DragEvent_class = { "DragEvent", 0};
static JSClass TextEvent_class = { "TextInputEvent", 0};
static JSClass KeyEvent_class = { "KeyEvent", 0};
static JSClass NMLEvent_class = { "NMLEvent", 0};


// }}}

// {{{ Implementation
JSWindow::~JSWindow()
{
    if (m_Dragging) {
        /* cleanup drag files */
    }
};

void JSWindow::onReady(JS::HandleObject layout)
{
    JS::AutoValueArray<1> arg(m_Cx);

    if (layout) {
        JS::RootedObject rlayout(m_Cx, layout);
        arg[0].setObjectOrNull(rlayout);
    } else {
        JS::RootedObject lobj(m_Cx, JS_NewArrayObject(m_Cx, 0));
        arg[0].setObjectOrNull(lobj);
    }
    JS::RootedObject obj(m_Cx, m_Instance);
    JS::RootedValue onready(m_Cx);
    if (JS_GetProperty(m_Cx, obj, "_onready", &onready) && onready.isObject()
        && JS::IsCallable(&onready.toObject())) {
        JS::RootedValue rval(m_Cx);
        JS_CallFunctionValue(m_Cx, obj, onready, arg, &rval);
    }
}

bool JSWindow::onClose()
{
    JS::RootedValue onclose(m_Cx);
    JS::RootedObject obj(m_Cx, m_Instance);
    if (JS_GetProperty(m_Cx, obj, "_onbeforeclose", &onclose)
        && onclose.isObject()
        && JS::IsCallable(&onclose.toObject())) {
        JS::RootedValue rval(m_Cx);
        JS_CallFunctionValue(m_Cx, obj, onclose, JS::HandleValueArray::empty(),
                             &rval);

        return (rval.isUndefined() || rval.toBoolean());
    }

    return true;
}

void JSWindow::assetReady(const NMLTag &tag)
{
#define EVENT_PROP(name, val)                 \
    JS_DefineProperty(m_Cx, event, name, val, \
                      JSPROP_PERMANENT | JSPROP_ENUMERATE)

    JSContext *cx = m_Cx;
    JS::AutoValueArray<1> jevent(cx);

    JS::RootedObject event(cx, JS_NewObject(cx, &NMLEvent_class));
    jevent[0].set(JS::ObjectValue(*event));

    JS::RootedValue onassetready(cx);
    JS::RootedObject obj(cx, m_Instance);

    if (JS_GetProperty(cx, obj, "_onassetready", &onassetready)
        && onassetready.isObject()
        && JS::IsCallable(&onassetready.toObject())) {

        JS::RootedString tagStr(cx, JS_NewStringCopyZ(cx, (const char *)tag.tag));
        JS::RootedString idStr(cx, JS_NewStringCopyZ(cx, (const char *)tag.id));
        JS::RootedString dataStr(
            cx, JSUtils::NewStringWithEncoding(cx, (const char *)tag.content.data,
                                               tag.content.len, "utf8"));
        EVENT_PROP("tag", tagStr);
        EVENT_PROP("id", idStr);
        EVENT_PROP("data", dataStr);        

        JS::RootedValue rval(cx);
        JS_CallFunctionValue(cx, event, onassetready, jevent, &rval);
    }
#undef EVENT_PROP
}

void JSWindow::windowFocus()
{
    JS::RootedValue onfocus(m_Cx);
    JS::RootedObject obj(m_Cx, m_Instance);
    if (JS_GetProperty(m_Cx, obj, "_onfocus", &onfocus) && onfocus.isObject()
        && JS::IsCallable(&onfocus.toObject())) {
        JS::RootedValue rval(m_Cx);
        JS_CallFunctionValue(m_Cx, nullptr, onfocus,
                             JS::HandleValueArray::empty(), &rval);
    }
}

void JSWindow::windowBlur()
{
    JS::RootedValue onblur(m_Cx);
    JS::RootedObject obj(m_Cx, m_Instance);
    if (JS_GetProperty(m_Cx, obj, "_onblur", &onblur) && onblur.isObject()
        && JS::IsCallable(&onblur.toObject())) {
        JS::RootedValue rval(m_Cx);
        JS_CallFunctionValue(m_Cx, nullptr, onblur,
                             JS::HandleValueArray::empty(), &rval);
    }
}

void JSWindow::mouseWheel(int xrel, int yrel, int x, int y)
{
#define EVENT_PROP(name, val)                 \
    JS_DefineProperty(m_Cx, event, name, val, \
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY)

    JSAutoRequest ar(m_Cx);

    Context *nctx  = Context::GetObject<Frontend::Context>(m_Cx);
    InputEvent ev(InputEvent::kMouseWheel_Type, x, y);
    ev.setData(0, xrel);
    ev.setData(1, yrel);

    nctx->getInputHandler()->pushEvent(ev);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &MouseEvent_class));
    JS::RootedValue xrelv(m_Cx, JS::Int32Value(xrel));
    JS::RootedValue yrelv(m_Cx, JS::Int32Value(yrel));
    JS::RootedValue xv(m_Cx, JS::Int32Value(x));
    JS::RootedValue yv(m_Cx, JS::Int32Value(y));
    EVENT_PROP("xrel", xrelv);
    EVENT_PROP("yrel", yrelv);
    EVENT_PROP("x", xv);
    EVENT_PROP("y", yv);

    JS::AutoValueArray<1> jevent(m_Cx);
    jevent[0].setObjectOrNull(event);

    JS::RootedObject obj(m_Cx, m_Instance);
    JS::RootedValue onwheel(m_Cx);
    if (JS_GetProperty(m_Cx, obj, "_onmousewheel", &onwheel)
        && onwheel.isObject()
        && JS::IsCallable(&onwheel.toObject())) {

        JS::RootedValue rval(m_Cx);
        JS_CallFunctionValue(m_Cx, event, onwheel, jevent, &rval);
    }

/*
    JS::RootedObject obje(cx, JSEvents::CreateEventObject(m_Cx));
    this->fireJSEvent("wheel", JS::ObjectValue(*obje));
    */

#undef EVENT_PROP
}

void JSWindow::keyupdown(
    int keycode, int mod, int state, int repeat, int location)
{
#define EVENT_PROP(name, val)                 \
    JS_DefineProperty(m_Cx, event, name, val, \
                      JSPROP_PERMANENT | JSPROP_ENUMERATE)

    JSAutoRequest ar(m_Cx);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &KeyEvent_class));
    JS::RootedValue keyV(m_Cx, JS::Int32Value(keycode));
    JS::RootedValue locationV(m_Cx, JS::Int32Value(location));
    JS::RootedValue alt(
        m_Cx, JS::BooleanValue(!!(mod & InputEvent::kKeyModifier_Alt)));
    JS::RootedValue ctl(
        m_Cx, JS::BooleanValue(!!(mod & InputEvent::kKeyModifier_Control)));
    JS::RootedValue shift(
        m_Cx, JS::BooleanValue(!!(mod & InputEvent::kKeyModifier_Shift)));
    JS::RootedValue meta(
        m_Cx, JS::BooleanValue(!!(mod & InputEvent::kKeyModifier_Meta)));
    JS::RootedValue space(m_Cx, JS::BooleanValue(keycode == 32));
    JS::RootedValue rep(m_Cx, JS::BooleanValue(!!(repeat)));
    EVENT_PROP("keyCode", keyV);
    EVENT_PROP("location", locationV);
    EVENT_PROP("altKey", alt);
    EVENT_PROP("ctrlKey", ctl);
    EVENT_PROP("shiftKey", shift);
    EVENT_PROP("metaKey", meta);
    EVENT_PROP("spaceKey", space);
    EVENT_PROP("repeat", rep);

    JS::AutoValueArray<1> jevent(m_Cx);
    jevent[0].setObjectOrNull(event);

    JS::RootedObject obj(m_Cx, m_Instance);
    JS::RootedValue onkeyupdown(m_Cx);
    if (JS_GetProperty(m_Cx, obj, (state ? "_onkeydown" : "_onkeyup"),
                       &onkeyupdown)
        && onkeyupdown.isObject()
        && JS::IsCallable(&onkeyupdown.toObject())) {

        JS::RootedValue rval(m_Cx);

        JS_CallFunctionValue(m_Cx, event, onkeyupdown, jevent, &rval);
    }
#undef EVENT_PROP
}

#define EVENT_PROP(name, val)                 \
    JS_DefineProperty(m_Cx, event, name, val, \
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)
void JSWindow::textEdit(const char *data)
{
    JSAutoRequest ar(m_Cx);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &TextEvent_class));
    JS::RootedString str(
        m_Cx, JSUtils::NewStringWithEncoding(m_Cx, data, strlen(data), "utf8"));
    EVENT_PROP("val", str);

    JS::AutoValueArray<1> jevent(m_Cx);
    jevent[0].setObjectOrNull(event);

    JS::RootedValue ontextedit(m_Cx);
    JS::RootedObject obj(m_Cx, m_Instance);
    if (JS_GetProperty(m_Cx, obj, "_ontextedit", &ontextedit)
        && ontextedit.isObject()
        && JS::IsCallable(&ontextedit.toObject())) {
        JS::RootedValue rval(m_Cx);
        JS_CallFunctionValue(m_Cx, event, ontextedit, jevent, &rval);
    }
}

void JSWindow::textInput(const char *data)
{
    JSAutoRequest ar(m_Cx);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &TextEvent_class));
    JS::RootedString str(
        m_Cx, JSUtils::NewStringWithEncoding(m_Cx, data, strlen(data), "utf8"));
    EVENT_PROP("val", str);

    JS::AutoValueArray<1> jevent(m_Cx);
    jevent[0].setObjectOrNull(event);

    JS::RootedValue ontextinput(m_Cx);
    JS::RootedObject obj(m_Cx, m_Instance);
    if (JS_GetProperty(m_Cx, obj, "_ontextinput", &ontextinput)
        && ontextinput.isObject()
        && JS::IsCallable(&ontextinput.toObject())) {
        JS::RootedValue rval(m_Cx);
        JS_CallFunctionValue(m_Cx, event, ontextinput, jevent, &rval);
    }
}
#undef EVENT_PROP

void JSWindow::onKeyUpDown(int keyCode, int location, int mod, bool repeat, bool isUpKey)
{
    JS::RootedObject obje(m_Cx, JSEvents::CreateEventObject(m_Cx));
    JSObjectBuilder obj(m_Cx, obje);

    JS::RootedValue space(m_Cx, JS::BooleanValue(keyCode == 32));
    JS::RootedValue repeatValue(m_Cx, JS::BooleanValue(!!(repeat)));
    JS::RootedValue locationValue(m_Cx, JS::Int32Value(location));

    JS::RootedValue alt(
        m_Cx, JS::BooleanValue(!!(mod & InputEvent::kKeyModifier_Alt)));
    JS::RootedValue ctl(
        m_Cx, JS::BooleanValue(!!(mod & InputEvent::kKeyModifier_Control)));
    JS::RootedValue shift(
        m_Cx, JS::BooleanValue(!!(mod & InputEvent::kKeyModifier_Shift)));
    JS::RootedValue meta(
        m_Cx, JS::BooleanValue(!!(mod & InputEvent::kKeyModifier_Meta)));

    obj.set("keyCode", keyCode);
    obj.set("location", locationValue);
    obj.set("repeat", repeatValue);
    obj.set("altKey", alt);
    obj.set("ctrlKey", ctl);
    obj.set("shiftKey", shift);
    obj.set("metaKey", meta);
    obj.set("spaceKey", space);

    JS::RootedValue evVal(m_Cx, obj.jsval());

    if (isUpKey) {
        this->fireJSEvent("keyup", &evVal);
    } else {
        this->fireJSEvent("keydown", &evVal);
    }
}

void JSWindow::onKeyPress(const char *c)
{
    JS::RootedObject obje(m_Cx, JSEvents::CreateEventObject(m_Cx));
    JSObjectBuilder obj(m_Cx, obje);

    obj.set("char", c);

    JS::RootedValue evVal(m_Cx, obj.jsval());
    this->fireJSEvent("keypress", &evVal);
}


void JSWindow::onCompositionStart()
{
    JS::RootedObject obje(m_Cx, JSEvents::CreateEventObject(m_Cx));
    JSObjectBuilder obj(m_Cx, obje);

    obj.set("data", "");

    JS::RootedValue evVal(m_Cx, obj.jsval());
    this->fireJSEvent("compositionstart", &evVal);
}

void JSWindow::onCompositionUpdate(const char *data)
{
    JS::RootedObject obje(m_Cx, JSEvents::CreateEventObject(m_Cx));
    JSObjectBuilder obj(m_Cx, obje);

    JS::RootedString dataStr(m_Cx, JSUtils::NewStringWithEncoding(
                                   m_Cx, data, strlen(data), "utf8"));

    obj.set("data", (JS::HandleString)(&dataStr));

    JS::RootedValue evVal(m_Cx, obj.jsval());
    this->fireJSEvent("compositionupdate", &evVal);
}

void JSWindow::onCompositionEnd()
{
    JS::RootedObject obje(m_Cx, JSEvents::CreateEventObject(m_Cx));
    JSObjectBuilder obj(m_Cx, obje);

    obj.set("data", "");

    JS::RootedValue evVal(m_Cx, obj.jsval());
    this->fireJSEvent("compositionend", &evVal);
}

void JSWindow::systemMenuClicked(const char *id)
{
    JSContext *cx = m_Cx;
    JS::RootedObject event(cx, JS_NewPlainObject(m_Cx));

    NIDIUM_JSOBJ_SET_PROP_CSTR(event, "id", id);
    JS::AutoValueArray<1> ev(cx);
    ev[0].setObjectOrNull(event);
    JS::RootedObject obj(cx, m_Instance);
    JSOBJ_CALLFUNCNAME(obj, "_onsystemtrayclick", ev);
}

bool JSWindow::onHardwareKey(InputEvent::Type evType)
{
    JS::RootedObject evObj(m_Cx, JSEvents::CreateEventObject(m_Cx));
    JS::RootedValue evValue(m_Cx);

    evValue.setObjectOrNull(evObj);

    const char *evName = InputEvent::GetName(evType);

    this->fireJSEvent(evName, &evValue);

    JS::RootedValue defaultPrevented(m_Cx);
    if (JS_GetProperty(m_Cx, evObj, "defaultPrevented", &defaultPrevented)) {
        if (defaultPrevented.isBoolean() && defaultPrevented.toBoolean()) {
            return true;
        }
    }

    return false;
}

void JSWindow::mouseClick(int x, int y, int state, int button, int clicks)
{
#define EVENT_PROP(name, val)                 \
    JS_DefineProperty(m_Cx, event, name, val, \
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    JSAutoRequest ar(m_Cx);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &MouseEvent_class));

    Context *nctx  = Context::GetObject<Frontend::Context>(m_Cx);
    InputEvent ev(state ? InputEvent::kMouseClick_Type
                        : InputEvent::kMouseClickRelease_Type,
                  x, y);

    ev.setData(0, button);

    nctx->getInputHandler()->pushEvent(ev);

    /*
        Handle double click.
        clicks receiv the number of successive clicks.
        Only trigger for even number on release.
    */
    if (clicks % 2 == 0 && !state) {
        InputEvent dcEv(InputEvent::kMouseDoubleClick_Type, x, y);

        dcEv.setData(0, button);
        nctx->getInputHandler()->pushEvent(dcEv);
    }
    JS::RootedValue xv(m_Cx, JS::Int32Value(x));
    JS::RootedValue yv(m_Cx, JS::Int32Value(y));
    JS::RootedValue bv(m_Cx, JS::Int32Value(button));
    EVENT_PROP("x", xv);
    EVENT_PROP("y", yv);
    EVENT_PROP("clientX", xv);
    EVENT_PROP("clientY", yv);
    EVENT_PROP("which", bv);

    JS::AutoValueArray<1> jevent(m_Cx);
    jevent[0].setObjectOrNull(event);
    JS::RootedValue onclick(m_Cx);
    JS::RootedObject obj(m_Cx, m_Instance);
    if (JS_GetProperty(m_Cx, obj, (state ? "_onmousedown" : "_onmouseup"),
                       &onclick)
        && onclick.isObject()
        && JS::IsCallable(&onclick.toObject())) {

        JS::RootedValue rval(m_Cx);
        JS_CallFunctionValue(m_Cx, event, onclick, jevent, &rval);
    }
#undef EVENT_PROP
}

bool JSWindow::dragEvent(const char *name, int x, int y)
{
#define EVENT_PROP(name, val)                 \
    JS_DefineProperty(m_Cx, event, name, val, \
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)
    JSAutoRequest ar(m_Cx);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &DragEvent_class));
    JS::RootedValue xv(m_Cx, JS::Int32Value(x));
    JS::RootedValue yv(m_Cx, JS::Int32Value(y));
    EVENT_PROP("x", xv);
    EVENT_PROP("y", yv);
    EVENT_PROP("clientX", xv);
    EVENT_PROP("clientY", yv);

    if (m_DraggedFiles) {
        JS::RootedObject drag(m_Cx, m_DraggedFiles);
        EVENT_PROP("files", drag);
    }

    JS::AutoValueArray<1> jevent(m_Cx);
    jevent[0].setObjectOrNull(event);

    JS::RootedValue ondragevent(m_Cx);
    JS::RootedObject obj(m_Cx, m_Instance);
    if (JS_GetProperty(m_Cx, obj, name, &ondragevent) && ondragevent.isObject()
        && JS::IsCallable(&ondragevent.toObject())) {
        JS::RootedValue rval(m_Cx);

        if (!JS_CallFunctionValue(m_Cx, event, ondragevent, jevent, &rval)) {
            ndm_log(NDM_LOG_ERROR, "Window", "Failed to exec func");
            return false;
        }

        return rval.toBoolean();
    }

    return false;
#undef EVENT_PROP
}

bool JSWindow::dragBegin(int x, int y, const char *const *files, size_t nfiles)
{
#define EVENT_PROP(name, val)                 \
    JS_DefineProperty(m_Cx, event, name, val, \
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    if (m_Dragging) {
        return false;
    }
    m_Dragging = true; // Duh..

    m_DraggedFiles = JS_NewArrayObject(m_Cx, (int)nfiles);
    NidiumLocalContext::RootObjectUntilShutdown(m_DraggedFiles);

    JS::RootedObject dragged(m_Cx, m_DraggedFiles);

    for (int i = 0; i < nfiles; i++) {
        JS::RootedValue val(
            m_Cx, JS::ObjectValue(*JSFile::GenerateJSObject(m_Cx, files[i])));
        JS_SetElement(m_Cx, dragged, i, val);
    }

    return this->dragEvent("_onFileDragEnter", x, y);
#undef EVENT_PROP
}

void JSWindow::dragLeave()
{
    if (!m_Dragging) {
        return;
    }
    this->dragEvent("_onFileDragLeave", 0, 0);
    this->dragEnd();
}

bool JSWindow::dragUpdate(int x, int y)
{
    if (!m_Dragging) {
        return false;
    }
    bool drag = this->dragEvent("_onFileDrag", x, y);

    return drag;
}

bool JSWindow::dragDroped(int x, int y)
{
    if (!m_Dragging) {
        return false;
    }

    return this->dragEvent("_onFileDrop", x, y);
}

void JSWindow::dragEnd()
{
    if (!m_Dragging) {
        return;
    }

    NidiumLocalContext::UnrootObject(m_DraggedFiles);

    m_DraggedFiles = NULL;
    m_Dragging     = false;
}

void JSWindow::resized(int width, int height)
{
    Context::GetObject<Frontend::Context>(m_Cx)->sizeChanged(width, height);
}

void JSWindow::mouseMove(int x, int y, int xrel, int yrel)
{
#define EVENT_PROP(name, val)                 \
    JS_DefineProperty(m_Cx, event, name, val, \
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    Context *nctx = Context::GetObject<Frontend::Context>(m_Cx);

    CanvasHandler *rootHandler = nctx->getRootHandler();

    rootHandler->m_MousePosition.x = x;
    rootHandler->m_MousePosition.y = y;
    rootHandler->m_MousePosition.xrel += xrel;
    rootHandler->m_MousePosition.yrel += yrel;
    rootHandler->m_MousePosition.consumed = false;

    InputEvent ev(InputEvent::kMouseMove_Type, x, y);

    ev.setData(0, xrel);
    ev.setData(1, yrel);

    nctx->getInputHandler()->pushEvent(ev);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &MouseEvent_class));
    JS::RootedValue xv(m_Cx, JS::Int32Value(x));
    JS::RootedValue yv(m_Cx, JS::Int32Value(y));
    JS::RootedValue xrelv(m_Cx, JS::Int32Value(xrel));
    JS::RootedValue yrelv(m_Cx, JS::Int32Value(yrel));
    EVENT_PROP("x", xv);
    EVENT_PROP("y", yv);
    EVENT_PROP("xrel", xrelv);
    EVENT_PROP("yrel", yrelv);
    EVENT_PROP("clientX", xv);
    EVENT_PROP("clientY", yv);

    JS::AutoValueArray<1> jevent(m_Cx);
    jevent[0].setObjectOrNull(event);

    JS::RootedValue onmove(m_Cx);
    JS::RootedObject obj(m_Cx, m_Instance);
    if (JS_GetProperty(m_Cx, obj, "_onmousemove", &onmove) && onmove.isObject()
        && JS::IsCallable(&onmove.toObject())) {

        JS::RootedValue rval(m_Cx);
        JS_CallFunctionValue(m_Cx, event, onmove, jevent, &rval);
    }
#undef EVENT_PROP
}


bool JSWindow::JSGetter_devicePixelRatio(JSContext *cx,
    JS::MutableHandleValue vp)
{
    vp.setInt32(1);

    return true;
}

bool JSWindow::JSGetter_left(JSContext *cx,
    JS::MutableHandleValue vp)
{
    int x;
    UIInterface *NUI = Context::GetObject<Frontend::Context>(m_Cx)->getUI();
    NUI->getWindowPosition(&x, NULL);

    vp.setInt32(x);

    return true;
}

bool JSWindow::JSSetter_left(JSContext *cx,
    JS::MutableHandleValue vp)
{
    UIInterface *NUI = Context::GetObject<Frontend::Context>(m_Cx)->getUI();

    int32_t dval;

    if (!JS::ToInt32(cx, vp, &dval)) {
        return true;
    }

    NUI->setWindowPosition((int)dval, NIDIUM_WINDOWPOS_UNDEFINED_MASK);

    return true;
}

bool JSWindow::JSGetter_top(JSContext *cx,
    JS::MutableHandleValue vp)
{
    UIInterface *NUI = Context::GetObject<Frontend::Context>(m_Cx)->getUI();

    int y;
    NUI->getWindowPosition(NULL, &y);
    vp.setInt32(y);

    return true;
}

bool JSWindow::JSSetter_top(JSContext *cx,
    JS::MutableHandleValue vp)
{
    UIInterface *NUI = Context::GetObject<Frontend::Context>(m_Cx)->getUI();

    int32_t dval;

    if (!JS::ToInt32(cx, vp, &dval)) {
        return true;
    }

    NUI->setWindowPosition(NIDIUM_WINDOWPOS_UNDEFINED_MASK, (int)dval);

    return true;
}

bool JSWindow::JSGetter_innerWidth(JSContext *cx,
    JS::MutableHandleValue vp)
{
    UIInterface *NUI = Context::GetObject<Frontend::Context>(m_Cx)->getUI();

    vp.setInt32(NUI->getWidth());

    return true;
}

bool JSWindow::JSSetter_innerWidth(JSContext *cx,
    JS::MutableHandleValue vp)
{
    UIInterface *NUI = Context::GetObject<Frontend::Context>(m_Cx)->getUI();

    int32_t dval;

    if (!JS::ToInt32(cx, vp, &dval)) {
        return true;
    }

    dval = ape_max(dval, 1);

    Context::GetObject<Frontend::Context>(cx)
        ->setWindowSize((int)dval, NUI->getHeight());

    return true;
}

bool JSWindow::JSGetter_innerHeight(JSContext *cx,
    JS::MutableHandleValue vp)
{
    UIInterface *NUI = Context::GetObject<Frontend::Context>(m_Cx)->getUI();

    vp.setInt32(NUI->getHeight());

    return true;
}

bool JSWindow::JSSetter_innerHeight(JSContext *cx,
    JS::MutableHandleValue vp)
{
    UIInterface *NUI = Context::GetObject<Frontend::Context>(m_Cx)->getUI();

    int32_t dval;

    if (!JS::ToInt32(cx, vp, &dval)) {
        return true;
    }

    dval = ape_max(dval, 1);

    Context::GetObject<Frontend::Context>(cx)
        ->setWindowSize((int)NUI->getWidth(), (int)dval);

    return true;
}

bool JSWindow::JSGetter_title(JSContext *cx,
    JS::MutableHandleValue vp)
{
    UIInterface *NUI = Context::GetObject<Frontend::Context>(m_Cx)->getUI();

    const char *title = NUI->getWindowTitle();
    JS::RootedString str(m_Cx, JSUtils::NewStringWithEncoding(
                                   m_Cx, title, strlen(title), "utf8"));
    vp.setString(str);

    return true;
}

bool JSWindow::JSSetter_title(JSContext *cx,
    JS::MutableHandleValue vp)
{
    UIInterface *NUI = Context::GetObject<Frontend::Context>(m_Cx)->getUI();

    JS::RootedString vpStr(cx, JS::ToString(cx, vp));

    JSAutoByteString title(cx, vpStr);
    NUI->setWindowTitle(title.ptr());

    return true;
}

struct _nidiumopenfile
{
    JSContext *m_Cx;
    JS::PersistentRootedValue m_Cb;
};

static void
nidium_window_openfilecb(void *_nof, const char *lst[], uint32_t len)
{
    struct _nidiumopenfile *nof = (struct _nidiumopenfile *)_nof;
    JS::RootedObject arr(nof->m_Cx, JS_NewArrayObject(nof->m_Cx, len));
    for (int i = 0; i < len; i++) {
        JS::RootedValue val(nof->m_Cx, JS::ObjectValue(*JSFile::GenerateJSObject(
                                           nof->m_Cx, lst[i])));
        JS_SetElement(nof->m_Cx, arr, i, val);
    }

    JS::AutoValueArray<1> jarr(nof->m_Cx);
    jarr[0].setObjectOrNull(arr);
    JS::RootedValue rval(nof->m_Cx);
    JS::RootedValue cb(nof->m_Cx, nof->m_Cb);
    JS::RootedObject global(nof->m_Cx, JS::CurrentGlobalOrNull(nof->m_Cx));
    JS_CallFunctionValue(nof->m_Cx, global, cb, jarr, &rval);

    nof->m_Cx = NULL;
    free(nof);
}

bool JSWindow::JS_setSize(JSContext *cx, JS::CallArgs &args)
{
    double w, h;

    if (!JS_ConvertArguments(cx, args, "dd", &w, &h)) {
        return false;
    }

    Context::GetObject<Frontend::Context>(cx)->setWindowSize(w, h);

    return true;
}



bool JSWindow::JS_openURL(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString url(cx);
    if (!JS_ConvertArguments(cx, args, "S", url.address())) {
        return false;
    }

    JSAutoByteString curl(cx, url);

    SystemInterface::GetInstance()->openURLInBrowser(curl.ptr());

    return true;
}

bool JSWindow::JS_exec(JSContext *cx, JS::CallArgs &args)
{

    JS::RootedString url(cx);
    if (!JS_ConvertArguments(cx, args, "S", url.address())) {
        return false;
    }

    JSAutoByteString curl(cx, url);
    const char *ret = SystemInterface::GetInstance()->execute(curl.ptr());

    JS::RootedString retStr(cx, JS_NewStringCopyZ(cx, ret));
    args.rval().setString(retStr);

    return true;
}

bool JSWindow::JS_alert(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString msg(cx);
    if (!JS_ConvertArguments(cx, args, "S", msg.address())) {
        return false;
    }

    JSAutoByteString cmsg(cx, msg);
    SystemInterface::GetInstance()->alert(cmsg.ptr());

    return true;
}

bool JSWindow::JS_bridge(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString data(cx);
    if (!JS_ConvertArguments(cx, args, "S", data.address())) {
        return false;
    }

    JSAutoByteString cdata(cx, data);
    
    Context::GetObject<Frontend::Context>(cx)->getUI()->bridge(cdata.ptr());

    return true;
}


bool JSWindow::JS_openDirDialog(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedValue callback(cx);
    if (!JS_ConvertArguments(cx, args, "v", callback.address())) {
        return false;
    }

    struct _nidiumopenfile *nof
        = (struct _nidiumopenfile *)malloc(sizeof(*nof));
    nof->m_Cb = callback;
    nof->m_Cx = cx;

    Context::GetObject<Frontend::Context>(cx)->getUI()->openFileDialog(
        NULL, nidium_window_openfilecb, nof,
        UIInterface::kOpenFile_CanChooseDir);

    return true;
}

/* TODO: leak if the user click "cancel" */
bool JSWindow::JS_openFileDialog(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedObject types(cx);
    JS::RootedValue callback(cx);
    if (!JS_ConvertArguments(cx, args, "ov", types.address(),
                             callback.address())) {
        return false;
    }

    bool isarray = false;

    if (!JS::ObjectValue(*types).isNull()
        && (!JS_IsArrayObject(cx, types, &isarray) || !isarray)) {
        JS_ReportError(cx, "First parameter must be an array or null");
        return false;
    }
    uint32_t len = 0;

    char **ctypes = NULL;

    if (!JS::ObjectValue(*types).isNull()) {
        JS_GetArrayLength(cx, types, &len);

        ctypes = (char **)malloc(sizeof(char *) * (len + 1));
        memset(ctypes, 0, sizeof(char *) * (len + 1));
        int j = 0;
        for (int i = 0; i < len; i++) {
            JS::RootedValue val(cx);
            JS_GetElement(cx, types, i, &val);
            if (val.isString()) {
                JS::RootedString valStr(cx, JS::ToString(cx, val));
                JS::RootedString str(cx, valStr);
                ctypes[j] = JS_EncodeString(cx, str);
                j++;
            }
        }
        ctypes[j] = NULL;
    }

    struct _nidiumopenfile *nof
        = (struct _nidiumopenfile *)malloc(sizeof(*nof));
    nof->m_Cb = callback;
    nof->m_Cx = cx;

    Context::GetObject<Frontend::Context>(cx)->getUI()->openFileDialog(
        (const char **)ctypes, nidium_window_openfilecb, nof,
        UIInterface::kOpenFile_CanChooseFile
            | UIInterface::kOpenFile_AlloMultipleSelection);

    if (ctypes) {
        for (int i = 0; i < len; i++) {
            JS_free(cx, ctypes[i]);
        }
        free(ctypes);
    }
    return true;
}

bool JSWindow::JS_requestAnimationFrame(JSContext *cx, JS::CallArgs &args)
{
    if (!JSUtils::ReportIfNotFunction(cx, args[0])) {

        return false;
    }

    this->addFrameCallback(args[0]);

    return true;
}

bool JSWindow::JS_center(JSContext *cx, JS::CallArgs &args)
{
    Context::GetObject<Frontend::Context>(cx)->getUI()->centerWindow();

    return true;
}

bool JSWindow::JS_setPosition(JSContext *cx, JS::CallArgs &args)
{
    int x = (args[0].isUndefined() || args[0].isNull())
                ? NIDIUM_WINDOWPOS_UNDEFINED_MASK
                : args[0].toInt32();

    int y = (args[1].isUndefined() || args[1].isNull())
                ? NIDIUM_WINDOWPOS_UNDEFINED_MASK
                : args[1].toInt32();

    Context::GetObject<Frontend::Context>(cx)->getUI()->setWindowPosition(x, y);

    return true;
}

bool JSWindow::JS_notify(JSContext *cx, JS::CallArgs &args)
{
    bool sound = false;

    JS::RootedString title(cx);
    JS::RootedString body(cx);

    if (!JS_ConvertArguments(cx, args, "SS/b", title.address(), body.address(),
                             &sound)) {
        return false;
    }

    JSAutoByteString ctitle;
    ctitle.encodeUtf8(cx, title);

    JSAutoByteString cbody;
    cbody.encodeUtf8(cx, body);

    SystemInterface::GetInstance()->sendNotification(ctitle.ptr(), cbody.ptr(),
                                                     sound);

    return true;
}

bool JSWindow::JS_quit(JSContext *cx, JS::CallArgs &args)
{
    UIInterface *NUI = Context::GetObject<Frontend::Context>(cx)->getUI();

    NUI->quit();

    return true;
}

bool JSWindow::JS_close(JSContext *cx, JS::CallArgs &args)
{
    UIInterface *NUI = Context::GetObject<Frontend::Context>(cx)->getUI();

    NUI->hideWindow();

    return true;
}

bool JSWindow::JS_open(JSContext *cx, JS::CallArgs &args)
{
    UIInterface *NUI = Context::GetObject<Frontend::Context>(cx)->getUI();

    NUI->showWindow();

    return true;
}

bool JSWindow::JS_setSystemTray(JSContext *cx, JS::CallArgs &args)
{
    UIInterface *NUI = Context::GetObject<Frontend::Context>(cx)->getUI();
    JS::RootedObject jobj(cx, &args[0].toObject());
    if (!jobj.get()) {
        NUI->disableSysTray();
        return true;
    }

    NIDIUM_JS_INIT_OPT();

    SystemMenu &menu = NUI->getSystemMenu();
    menu.deleteItems();

    NIDIUM_JS_GET_OPT_TYPE(jobj, "icon", Object)
    {
        JS::RootedObject jsimg(cx, __curopt.toObjectOrNull());
        Image *skimage;
        if (JSImage::InstanceOf(jsimg)
            && (skimage = JSImage::JSObjectToImage(jsimg))) {

            const uint8_t *pixels = skimage->getPixels(NULL);
            menu.setIcon(pixels, skimage->getWidth(), skimage->getHeight());
        }
    }

    NIDIUM_JS_GET_OPT_TYPE(jobj, "menu", Object)
    {
        JS::RootedObject arr(cx, __curopt.toObjectOrNull());
        bool isarray;
        if (JS_IsArrayObject(cx, arr, &isarray) && isarray) {
            uint32_t len;
            JS_GetArrayLength(cx, arr, &len);

            /*
                The list is a FIFO
                Walk in inverse order to keep the same Array definition order
            */
            for (int i = len - 1; i >= 0; i--) {
                JS::RootedValue val(cx);
                JS_GetElement(cx, arr, i, &val);
                if (val.isObject()) {
                    JS::RootedObject valObj(cx, &val.toObject());
                    SystemMenuItem *menuItem = new SystemMenuItem();
                    NIDIUM_JS_GET_OPT_TYPE(valObj, "title", String)
                    {

                        JSAutoByteString ctitle;
                        JS::RootedString str(cx, __curopt.toString());
                        ctitle.encodeUtf8(cx, str);
                        menuItem->title(ctitle.ptr());
                    }
                    else
                    {
                        menuItem->title("");
                    }
                    NIDIUM_JS_GET_OPT_TYPE(valObj, "id", String)
                    {
                        JSAutoByteString cid;
                        JS::RootedString str(cx, __curopt.toString());
                        cid.encodeUtf8(cx, str);
                        menuItem->id(cid.ptr());
                    }
                    else
                    {
                        menuItem->id("");
                    }
                    menu.addItem(menuItem);
                }
            }
        }
    }

    NUI->enableSysTray();

    return true;
}

bool JSWindow::JS_setFrame(JSContext *cx, JS::CallArgs &args)
{
    int32_t x = 0, y = 0;
    int32_t w, h;

    if (args[0].isString()) {
        JS::RootedString xstr(cx, args[0].toString());
        JSAutoByteString cxstr(cx, xstr);

        if (strcmp(cxstr.ptr(), "center") == 0) {
            x = NIDIUM_WINDOWPOS_CENTER_MASK;
        } else if (!JS::ToInt32(cx, args[0], &x)) {
            x = 0;
        }

    } else if (args[0].isNumber()) {
        JS::ToInt32(cx, args[0], &x);
    } else {
        JS_ReportError(cx, "setFrame() invalid position");

        return false;
    }
    if (args[1].isString()) {
        JS::RootedString ystr(cx, args[1].toString());
        JSAutoByteString cystr(cx, ystr);

        if (strcmp(cystr.ptr(), "center") == 0) {
            y = NIDIUM_WINDOWPOS_CENTER_MASK;
        } else if (!JS::ToInt32(cx, args[1], &y)) {
            y = 0;
        }

    } else if (args[1].isNumber()) {
        JS::ToInt32(cx, args[1], &y);
    } else {
        JS_ReportError(cx, "setFrame() invalid position");

        return false;
    }
    if (JS::ToInt32(cx, args[2], &w)) {
        w = ape_max(w, 1);
    } else {
        JS_ReportError(cx, "setFrame() invalid width");

        return false;
    }

    if (JS::ToInt32(cx, args[3], &h)) {
        h = ape_max(h, 1);

    } else {
        JS_ReportError(cx, "setFrame() invalid height");

        return false;
    }


    Context::GetObject<Frontend::Context>(cx)
        ->setWindowFrame((int)x, (int)y, (int)w, (int)h);

    return true;
}

void JSWindow::addFrameCallback(JS::MutableHandleValue cb)
{
    struct _requestedFrame *frame = new struct _requestedFrame(m_Cx);
    frame->m_Next                 = m_RequestedFrame;
    frame->m_Cb                   = cb;

    m_RequestedFrame = frame;
}

void JSWindow::callFrameCallbacks(double ts, bool garbage)
{
    struct _requestedFrame *frame = m_RequestedFrame;

    /* Set to NULL so that callbacks can "fork" the new chain */
    m_RequestedFrame = NULL;

    JS::AutoValueArray<1> arg(m_Cx);
    arg[0].setNumber(ts / 1000000L);

    while (frame != NULL) {
        if (!garbage) {
            JS::RootedObject global(m_Cx, JS::CurrentGlobalOrNull(m_Cx));
            JS::RootedValue cb(m_Cx, frame->m_Cb);
            JS::RootedValue rval(m_Cx);
            JS_CallFunctionValue(m_Cx, global, cb, arg, &rval);
        }
        struct _requestedFrame *tmp = frame->m_Next;
        delete frame;
        frame = tmp;
    }
}

JSWindow *JSWindow::GetObject(JSContext *cx)
{
    return Context::GetObject<Frontend::Context>(cx)->getJSWindow();
}

JSWindow *JSWindow::GetObject(NidiumJS *njs)
{
    return Context::GetObject<Frontend::Context>(njs)->getJSWindow();
}
// }}}

// {{{ Registration

JSClass *JSWindow::GetJSClass()
{
    static JSClass global_class = {
        "Window",
        JSCLASS_GLOBAL_FLAGS_WITH_SLOTS(16) | JSCLASS_HAS_PRIVATE,
        nullptr,  nullptr,
        nullptr,  nullptr,
        nullptr,  nullptr,
        nullptr,  nullptr,
        nullptr,  nullptr,
        nullptr,  JS_GlobalObjectTraceHook
    };

    return &global_class;
}


JSPropertySpec *JSWindow::ListProperties()
{
    static JSPropertySpec props[] = {
        CLASSMAPPER_PROP_G(JSWindow, devicePixelRatio),
        CLASSMAPPER_PROP_GS(JSWindow, top),
        CLASSMAPPER_PROP_GS(JSWindow, left),
        CLASSMAPPER_PROP_GS(JSWindow, innerWidth),
        CLASSMAPPER_PROP_GS_ALIAS(JSWindow, outerWidth, innerWidth),
        CLASSMAPPER_PROP_GS(JSWindow, innerHeight),
        CLASSMAPPER_PROP_GS_ALIAS(JSWindow, outerHeight, innerHeight),
        CLASSMAPPER_PROP_GS(JSWindow, title),
        JS_PS_END
    };

    return props;
}

JSFunctionSpec *JSWindow::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSWindow, requestAnimationFrame, 1),
        CLASSMAPPER_FN(JSWindow, openFileDialog, 2),
        CLASSMAPPER_FN(JSWindow, openDirDialog, 1),
        CLASSMAPPER_FN(JSWindow, setSize, 2),
        CLASSMAPPER_FN(JSWindow, center, 0),
        CLASSMAPPER_FN(JSWindow, setPosition, 2),
        CLASSMAPPER_FN(JSWindow, setFrame, 4),
        CLASSMAPPER_FN(JSWindow, notify, 2),
        CLASSMAPPER_FN(JSWindow, quit, 0),
        CLASSMAPPER_FN(JSWindow, close, 0),
        CLASSMAPPER_FN(JSWindow, open, 0),
        CLASSMAPPER_FN(JSWindow, setSystemTray, 1),
        CLASSMAPPER_FN(JSWindow, openURL, 1),
        CLASSMAPPER_FN(JSWindow, exec, 1),
        CLASSMAPPER_FN(JSWindow, alert, 1),
        CLASSMAPPER_FN(JSWindow, bridge, 1),
        JS_FS_END
    };

    return funcs;
}

JSWindow *JSWindow::RegisterObject(JSContext *cx,
                                   int width,
                                   int height,
                                   JS::HandleObject docObj)
{
    JS::RootedObject globalObj(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedObject windowObj(cx, globalObj);

    JSWindow *jwin = new JSWindow(NidiumJS::GetObject(cx));

    JSWindow::AssociateObject(cx, jwin, globalObj, true);
    jwin->setUniqueInstance();

    // Set the __nidium__ properties
    JS::RootedObject nidiumObj(cx, JS_NewPlainObject(cx));

    JS::RootedValue val(cx);

    JS::RootedString jVersion(cx, JS_NewStringCopyZ(cx, NIDIUM_VERSION_STR));
    JS_DefineProperty(cx, nidiumObj, "version", jVersion,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS::RootedString jBuild(cx, JS_NewStringCopyZ(cx, NIDIUM_BUILD));
    JS_DefineProperty(cx, nidiumObj, "build", jBuild,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS::RootedString jRevision(cx, JS_NewStringCopyZ(cx, NIDIUM_BUILD));
    JS_DefineProperty(cx, nidiumObj, "revision", jRevision,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    val = JS::ObjectValue(*nidiumObj);
    JS_SetProperty(cx, windowObj, "__nidium__", val);

#if 0
    //@TODO: intented to be used in a future
    JS::RootedObject titleBar(cx, JSCanvas::GenerateJSObject(cx, width, 35));
    static_cast<CanvasHandler *>(JS_GetPrivate(canvas))->translate(0, 35);

    /* Set the newly generated CanvasHandler as first child of rootHandler */
    NJS->rootHandler->addChild((CanvasHandler *)JS_GetPrivate(titleBar));

    JS::RootedValue titleVal(cx, JS::ObjectValue(*titleBar));
    JS_DefineProperty(cx, NidiumObj, "titleBar", titleVal, JSPROP_READONLY | JSPROP_PERMANENT | JSPROP_PERMANENT | JSPROP_ENUMERATE);
#endif

    return jwin;
}
// }}}

} // namespace Binding
} // namespace Nidium
