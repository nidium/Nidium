#include "Binding/JSWindow.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>

#include <Binding/JSFileIO.h>
#include <Binding/JSUtils.h>

#include <SystemInterface.h>

#include "Frontend/NML.h"
#include "Frontend/Context.h"
#include "Graphics/Image.h"
#include "Graphics/SkiaContext.h"
#include "Binding/JSCanvas.h"
#include "Binding/JSImage.h"

namespace Nidium {
namespace Binding {

// {{{ Preamble
static bool native_window_prop_set(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp);
static bool native_window_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp);
static bool native_navigator_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp);
static bool native_window_openFileDialog(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_window_openDirDialog(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_window_setSize(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_window_requestAnimationFrame(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_window_center(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_window_setPosition(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_window_setFrame(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_window_notify(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_window_quit(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_window_close(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_window_open(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_window_setSystemTray(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_window_openURLInBrowser(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_window_exec(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_storage_set(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_storage_get(JSContext *cx, unsigned argc, JS::Value *vp);

static void Storage_Finalize(JSFreeOp *fop, JSObject *obj);

enum {
    WINDOW_PROP_LEFT = JSCLASS_GLOBAL_SLOT_COUNT,
    WINDOW_PROP_TOP,
    WINDOW_PROP_WIDTH,
    WINDOW_PROP_HEIGHT,
    WINDOW_PROP_TITLE,
    WINDOW_PROP_CURSOR,
    WINDOW_PROP_TITLEBAR_COLOR,
    WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETX,
    WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETY,
    WINDOW_PROP_DEVICE_PIXELRATIO
};


enum {
    NAVIGATOR_PROP_LANGUAGE,
    NAVIGATOR_PROP_VIBRATE,
    NAVIGATOR_PROP_APPNAME,
    NAVIGATOR_PROP_APPVERSION,
    NAVIGATOR_PROP_PLATFORM,
    NAVIGATOR_PROP_USERAGENT
};

static JSClass Navigator_class = {
    "Navigator", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass Storage_class = {
    "NidiumStorage", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Storage_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

extern JSClass global_class;

template<>
JSClass *JSExposer<JSWindow>::jsclass = &global_class;

static JSClass MouseEvent_class = {
    "MouseEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass DragEvent_class = {
    "DragEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

#if 0
static JSClass WindowEvent_class = {
    "WindowEvent", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};
#endif

static JSClass TextEvent_class = {
    "TextInputEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass KeyEvent_class = {
    "keyEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass NMLEvent_class = {
    "NMLEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSFunctionSpec storage_funcs[] = {
    JS_FN("set", native_storage_set, 2, NIDIUM_JS_FNPROPS),
    JS_FN("get", native_storage_get, 1, NIDIUM_JS_FNPROPS),
    JS_FS_END
};

static JSFunctionSpec window_funcs[] = {
    JS_FN("openFileDialog", native_window_openFileDialog, 2, NIDIUM_JS_FNPROPS),
    JS_FN("openDirDialog", native_window_openDirDialog, 1, NIDIUM_JS_FNPROPS),
    JS_FN("setSize", native_window_setSize, 2, NIDIUM_JS_FNPROPS),
    JS_FN("requestAnimationFrame", native_window_requestAnimationFrame, 1, NIDIUM_JS_FNPROPS),
    JS_FN("center", native_window_center, 0, NIDIUM_JS_FNPROPS),
    JS_FN("setPosition", native_window_setPosition, 2, NIDIUM_JS_FNPROPS),
    JS_FN("setFrame", native_window_setFrame, 4, NIDIUM_JS_FNPROPS),
    JS_FN("notify", native_window_notify, 2, NIDIUM_JS_FNPROPS),
    JS_FN("quit", native_window_quit, 0, NIDIUM_JS_FNPROPS),
    JS_FN("close", native_window_close, 0, NIDIUM_JS_FNPROPS),
    JS_FN("open", native_window_open, 0, NIDIUM_JS_FNPROPS),
    JS_FN("setSystemTray", native_window_setSystemTray, 1, NIDIUM_JS_FNPROPS),
    JS_FN("openURL", native_window_openURLInBrowser, 1, NIDIUM_JS_FNPROPS),
    JS_FN("exec", native_window_exec, 1, NIDIUM_JS_FNPROPS),
    JS_FS_END
};

static struct native_cursors {
    const char *str;
    Nidium::Interface::NativeUIInterface::CURSOR_TYPE type;
} native_cursors_list[] = {
    {"default",             Nidium::Interface::NativeUIInterface::ARROW},
    {"arrow",               Nidium::Interface::NativeUIInterface::ARROW},
    {"beam",                Nidium::Interface::NativeUIInterface::BEAM},
    {"text",                Nidium::Interface::NativeUIInterface::BEAM},
    {"pointer",             Nidium::Interface::NativeUIInterface::POINTING},
    {"grabbing",            Nidium::Interface::NativeUIInterface::CLOSEDHAND},
    {"drag",                Nidium::Interface::NativeUIInterface::CLOSEDHAND},
    {"hidden",              Nidium::Interface::NativeUIInterface::HIDDEN},
    {"none",                Nidium::Interface::NativeUIInterface::HIDDEN},
    {"col-resize",          Nidium::Interface::NativeUIInterface::RESIZELEFTRIGHT},
    {NULL,                  Nidium::Interface::NativeUIInterface::NOCHANGE},
};

static JSPropertySpec window_props[] = {

    NIDIUM_JS_PSG("devicePixelRatio", WINDOW_PROP_DEVICE_PIXELRATIO, native_window_prop_get),

    NIDIUM_JS_PSGS("left", WINDOW_PROP_LEFT, native_window_prop_get, native_window_prop_set),
    NIDIUM_JS_PSGS("top", WINDOW_PROP_TOP, native_window_prop_get, native_window_prop_set),
    NIDIUM_JS_PSGS("innerWidth", WINDOW_PROP_WIDTH, native_window_prop_get, native_window_prop_set),
    NIDIUM_JS_PSGS("outerWidth", WINDOW_PROP_WIDTH, native_window_prop_get, native_window_prop_set),
    NIDIUM_JS_PSGS("innerHeight", WINDOW_PROP_HEIGHT, native_window_prop_get, native_window_prop_set),
    NIDIUM_JS_PSGS("outerHeight", WINDOW_PROP_HEIGHT, native_window_prop_get, native_window_prop_set),
    NIDIUM_JS_PSGS("title", WINDOW_PROP_TITLE, native_window_prop_get, native_window_prop_set),
    NIDIUM_JS_PSGS("cursor", WINDOW_PROP_CURSOR, native_window_prop_get, native_window_prop_set),
    NIDIUM_JS_PSS("titleBarColor", WINDOW_PROP_TITLEBAR_COLOR, native_window_prop_set),
    NIDIUM_JS_PSS("titleBarControlsOffsetX", WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETX, native_window_prop_set),
    NIDIUM_JS_PSS("titleBarControlsOffsetY", WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETY, native_window_prop_set),

    JS_PS_END
};

static JSPropertySpec navigator_props[] = {

    NIDIUM_JS_PSG("language", NAVIGATOR_PROP_LANGUAGE, native_navigator_prop_get),
    NIDIUM_JS_PSG("vibrate", NAVIGATOR_PROP_VIBRATE, native_navigator_prop_get),
    NIDIUM_JS_PSG("appName", NAVIGATOR_PROP_APPNAME, native_navigator_prop_get),
    NIDIUM_JS_PSG("appVersion", NAVIGATOR_PROP_APPVERSION, native_navigator_prop_get),
    NIDIUM_JS_PSG("platform", NAVIGATOR_PROP_PLATFORM, native_navigator_prop_get),
    NIDIUM_JS_PSG("userAgent", NAVIGATOR_PROP_USERAGENT, native_navigator_prop_get),
    JS_PS_END
};
// }}}

// {{{ Implementation
JSWindow::~JSWindow()
{
    if (m_Dragging) {
        /* cleanup drag files */
    }
    if (m_Db) {
        delete m_Db;
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
    JS::RootedObject obj(m_Cx, m_JSObject);
    JS::RootedValue onready(m_Cx);
    if (JS_GetProperty(m_Cx, obj, "_onready", &onready) &&
            onready.isObject() &&
            JS_ObjectIsCallable(m_Cx, &onready.toObject())) {
        JS::RootedValue rval(m_Cx);
        JS_CallFunctionValue(m_Cx, obj, onready, arg, &rval);
    }
}

bool JSWindow::onClose()
{
    JS::RootedValue onclose(m_Cx);
    JS::RootedObject obj(m_Cx, m_JSObject);
    if (JS_GetProperty(m_Cx, obj, "_onbeforeclose", &onclose) &&
            onclose.isObject() &&
            JS_ObjectIsCallable(m_Cx, &onclose.toObject())) {
        JS::RootedValue rval(m_Cx);
        JS_CallFunctionValue(m_Cx, obj, onclose, JS::HandleValueArray::empty(), &rval);

        return (rval.isUndefined() || rval.toBoolean());
    }

    return true;
}

void JSWindow::assetReady(const Nidium::Frontend::NMLTag &tag)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
        val, JSPROP_PERMANENT | JSPROP_ENUMERATE)

    JSContext *cx = m_Cx;
    JS::AutoValueArray<1> jevent(cx);

    JS::RootedObject event(cx, JS_NewObject(cx, &NMLEvent_class, JS::NullPtr(), JS::NullPtr()));
    jevent[0].set(OBJECT_TO_JSVAL(event));
    JS::RootedString tagStr(cx, JS_NewStringCopyZ(cx, (const char *)tag.tag));
    JS::RootedString idStr(cx, JS_NewStringCopyZ(cx, (const char *)tag.id));
    JS::RootedString dataStr(cx, JSUtils::NewStringWithEncoding(cx,
        (const char *)tag.content.data, tag.content.len, "utf8"));
    EVENT_PROP("tag", tagStr);
    EVENT_PROP("id", idStr);
    EVENT_PROP("data", dataStr);

    JS::RootedValue onassetready(cx);
    JS::RootedObject obj(cx, m_JSObject);
    if (JS_GetProperty(cx, obj, "_onassetready", &onassetready) &&
        onassetready.isObject() &&
        JS_ObjectIsCallable(cx, &onassetready.toObject())) {

        JS::RootedValue rval(cx);
        JS_CallFunctionValue(cx, event, onassetready, jevent, &rval);
    }
#undef EVENT_PROP
}

void JSWindow::windowFocus()
{
    JS::RootedValue onfocus(m_Cx);
    JS::RootedObject obj(m_Cx, m_JSObject);
    if (JS_GetProperty(m_Cx, obj, "_onfocus", &onfocus) &&
        onfocus.isObject() &&
        JS_ObjectIsCallable(m_Cx, &onfocus.toObject())) {
            JS::RootedValue rval(m_Cx);
            JS_CallFunctionValue(m_Cx, JS::NullPtr(), onfocus, JS::HandleValueArray::empty(), &rval);
    }
}

void JSWindow::windowBlur()
{
    JS::RootedValue onblur(m_Cx);
    JS::RootedObject obj(m_Cx, m_JSObject);
    if (JS_GetProperty(m_Cx, obj, "_onblur", &onblur) &&
        onblur.isObject() &&
        JS_ObjectIsCallable(m_Cx, &onblur.toObject())) {
            JS::RootedValue rval(m_Cx);
            JS_CallFunctionValue(m_Cx, JS::NullPtr(), onblur, JS::HandleValueArray::empty(), &rval);
    }
}

void JSWindow::mouseWheel(int xrel, int yrel, int x, int y)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY)

    JSAutoRequest ar(m_Cx);

    Nidium::Frontend::Context *nctx = Nidium::Frontend::Context::GetObject(m_Cx);
    Nidium::Frontend::InputEvent *ev = new Nidium::Frontend::InputEvent(Nidium::Frontend::InputEvent::kMouseWheel_Type, x, y);

    ev->setData(0, xrel);
    ev->setData(1, yrel);

    nctx->addInputEvent(ev);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &MouseEvent_class, JS::NullPtr(), JS::NullPtr()));
    JS::RootedValue xrelv(m_Cx, INT_TO_JSVAL(xrel));
    JS::RootedValue yrelv(m_Cx, INT_TO_JSVAL(yrel));
    JS::RootedValue xv(m_Cx, INT_TO_JSVAL(x));
    JS::RootedValue yv(m_Cx, INT_TO_JSVAL(y));
    EVENT_PROP("xrel", xrelv);
    EVENT_PROP("yrel", yrelv);
    EVENT_PROP("x", xv);
    EVENT_PROP("y", yv);

    JS::AutoValueArray<1> jevent(m_Cx);
    jevent[0].setObjectOrNull(event);

    JS::RootedObject obj(m_Cx, m_JSObject);
    JS::RootedValue onwheel(m_Cx);
    if (JS_GetProperty(m_Cx, obj, "_onmousewheel", &onwheel) &&
            onwheel.isObject() &&
            JS_ObjectIsCallable(m_Cx, &onwheel.toObject())) {

        JS::RootedValue rval(m_Cx);
        JS_CallFunctionValue(m_Cx, event, onwheel, jevent, &rval);
    }

    /*
    JS::RootedObject obje(cx, JSEvents::CreateEventObject(m_Cx));
    this->fireJSEvent("wheel", OBJECT_TO_JSVAL(obje));
    */

#undef EVENT_PROP
}

void JSWindow::keyupdown(int keycode, int mod, int state, int repeat, int location)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, JSPROP_PERMANENT | JSPROP_ENUMERATE)

    JSAutoRequest ar(m_Cx);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &KeyEvent_class, JS::NullPtr(), JS::NullPtr()));
    JS::RootedValue keyV(m_Cx, INT_TO_JSVAL(keycode));
    JS::RootedValue locationV(m_Cx, INT_TO_JSVAL(location));
    JS::RootedValue alt(m_Cx, BOOLEAN_TO_JSVAL(!!(mod & NIDIUM_KEY_ALT)));
    JS::RootedValue ctl(m_Cx, BOOLEAN_TO_JSVAL(!!(mod & NIDIUM_KEY_CTRL)));
    JS::RootedValue shift(m_Cx, BOOLEAN_TO_JSVAL(!!(mod & NIDIUM_KEY_SHIFT)));
    JS::RootedValue meta(m_Cx, BOOLEAN_TO_JSVAL(!!(mod & NIDIUM_KEY_META)));
    JS::RootedValue space(m_Cx, BOOLEAN_TO_JSVAL(keycode == 32));
    JS::RootedValue rep(m_Cx, BOOLEAN_TO_JSVAL(!!(repeat)));
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

    JS::RootedObject obj(m_Cx, m_JSObject);
    JS::RootedValue onkeyupdown(m_Cx);
    if (JS_GetProperty(m_Cx, obj,
        (state ? "_onkeydown" : "_onkeyup"), &onkeyupdown) &&
        onkeyupdown.isObject() &&
        JS_ObjectIsCallable(m_Cx, &onkeyupdown.toObject())) {

        JS::RootedValue rval(m_Cx);

        JS_CallFunctionValue(m_Cx, event, onkeyupdown, jevent, &rval);
    }
#undef EVENT_PROP
}

void JSWindow::textInput(const char *data)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    JSAutoRequest ar(m_Cx);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &TextEvent_class, JS::NullPtr(), JS::NullPtr()));
    JS::RootedString str(m_Cx, JSUtils::NewStringWithEncoding(m_Cx, data, strlen(data), "utf8"));
    EVENT_PROP("val", str);

    JS::AutoValueArray<1> jevent(m_Cx);
    jevent[0].setObjectOrNull(event);

    JS::RootedValue ontextinput(m_Cx);
    JS::RootedObject obj(m_Cx, m_JSObject);
    if (JS_GetProperty(m_Cx, obj, "_ontextinput", &ontextinput) &&
            ontextinput.isObject() &&
            JS_ObjectIsCallable(m_Cx, &ontextinput.toObject())) {
        JS::RootedValue rval(m_Cx);
        JS_CallFunctionValue(m_Cx, event, ontextinput, jevent, &rval);
    }
#undef EVENT_PROP
}

void JSWindow::systemMenuClicked(const char *id)
{
    JSContext *cx = m_Cx;
    JS::RootedObject event(cx, JS_NewObject(m_Cx, nullptr, JS::NullPtr(), JS::NullPtr()));

    NIDIUM_JSOBJ_SET_PROP_CSTR(event, "id", id);
    JS::AutoValueArray<1> ev(cx);
    ev[0].setObjectOrNull(event);
    JS::RootedObject obj(cx, m_JSObject);
    JSOBJ_CALLFUNCNAME(obj, "_onsystemtrayclick", ev);
}

void JSWindow::mouseClick(int x, int y, int state, int button, int clicks)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    JSAutoRequest ar(m_Cx);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &MouseEvent_class, JS::NullPtr(), JS::NullPtr()));

    Nidium::Frontend::Context *nctx = Nidium::Frontend::Context::GetObject(m_Cx);
    Nidium::Frontend::InputEvent *ev = new Nidium::Frontend::InputEvent(state ?
        Nidium::Frontend::InputEvent::kMouseClick_Type :
        Nidium::Frontend::InputEvent::kMouseClickRelease_Type, x, y);

    ev->setData(0, button);

    nctx->addInputEvent(ev);

    /*
        Handle double click.
        clicks receiv the number of successive clicks.
        Only trigger for even number on release.
    */
    if (clicks % 2 == 0 && !state) {
        Nidium::Frontend::InputEvent *ev = new Nidium::Frontend::InputEvent(Nidium::Frontend::InputEvent::kMouseDoubleClick_Type, x, y);

        ev->setData(0, button);
        nctx->addInputEvent(ev);
    }
    JS::RootedValue xv(m_Cx, INT_TO_JSVAL(x));
    JS::RootedValue yv(m_Cx, INT_TO_JSVAL(y));
    JS::RootedValue bv(m_Cx, INT_TO_JSVAL(button));
    EVENT_PROP("x", xv);
    EVENT_PROP("y", yv);
    EVENT_PROP("clientX", xv);
    EVENT_PROP("clientY", yv);
    EVENT_PROP("which", bv);

    JS::AutoValueArray<1> jevent(m_Cx);
    jevent[0].setObjectOrNull(event);
    JS::RootedValue onclick(m_Cx);
    JS::RootedObject obj(m_Cx, m_JSObject);
    if (JS_GetProperty(m_Cx, obj,
        (state ? "_onmousedown" : "_onmouseup"), &onclick) &&
        onclick.isObject() &&
        JS_ObjectIsCallable(m_Cx, &onclick.toObject())) {

        JS::RootedValue rval(m_Cx);
        JS_CallFunctionValue(m_Cx, event, onclick, jevent, &rval);
    }
#undef EVENT_PROP
}

bool JSWindow::dragEvent(const char *name, int x, int y)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)
    JSAutoRequest ar(m_Cx);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &DragEvent_class, JS::NullPtr(), JS::NullPtr()));
    JS::RootedValue xv(m_Cx, INT_TO_JSVAL(x));
    JS::RootedValue yv(m_Cx, INT_TO_JSVAL(y));
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
    JS::RootedObject obj(m_Cx, m_JSObject);
    if (JS_GetProperty(m_Cx, obj, name, &ondragevent) &&
            ondragevent.isObject() &&
            JS_ObjectIsCallable(m_Cx, &ondragevent.toObject())) {
        JS::RootedValue rval(m_Cx);

        if (!JS_CallFunctionValue(m_Cx, event, ondragevent, jevent, &rval)) {
            fprintf(stderr, "Failed to exec func\n");
            return false;
        }

        return rval.toBoolean();
    }

    return false;
#undef EVENT_PROP
}

bool JSWindow::dragBegin(int x, int y, const char * const *files, size_t nfiles)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    if (m_Dragging) {
        return false;
    }
    m_Dragging = true; //Duh..

    m_DraggedFiles = JS_NewArrayObject(m_Cx, (int)nfiles);
    NidiumJS::GetObject(m_Cx)->rootObjectUntilShutdown(m_DraggedFiles);

    JS::RootedObject dragged(m_Cx, m_DraggedFiles);

    for (int i = 0; i < nfiles; i++) {
        JS::RootedValue val(m_Cx, OBJECT_TO_JSVAL(JSFileIO::GenerateJSObject(m_Cx, files[i])));
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

    NidiumJS::GetObject(m_Cx)->unrootObject(m_DraggedFiles);

    m_DraggedFiles = NULL;
    m_Dragging = false;
}

void JSWindow::resized(int width, int height)
{
    Nidium::Frontend::Context::GetObject(m_Cx)->sizeChanged(width, height);
}

void JSWindow::mouseMove(int x, int y, int xrel, int yrel)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    Nidium::Frontend::Context *nctx = Nidium::Frontend::Context::GetObject(m_Cx);

    Graphics::CanvasHandler *rootHandler = nctx->getRootHandler();

    rootHandler->m_MousePosition.x = x;
    rootHandler->m_MousePosition.y = y;
    rootHandler->m_MousePosition.xrel += xrel;
    rootHandler->m_MousePosition.yrel += yrel;
    rootHandler->m_MousePosition.consumed = false;

    Nidium::Frontend::InputEvent *ev = new Nidium::Frontend::InputEvent(Nidium::Frontend::InputEvent::kMouseMove_Type, x, y);

    ev->setData(0, xrel);
    ev->setData(1, yrel);

    nctx->addInputEvent(ev);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &MouseEvent_class, JS::NullPtr(), JS::NullPtr()));
    JS::RootedValue xv(m_Cx, INT_TO_JSVAL(x));
    JS::RootedValue yv(m_Cx, INT_TO_JSVAL(y));
    JS::RootedValue xrelv(m_Cx, INT_TO_JSVAL(xrel));
    JS::RootedValue yrelv(m_Cx, INT_TO_JSVAL(yrel));
    EVENT_PROP("x", xv);
    EVENT_PROP("y", yv);
    EVENT_PROP("xrel", xrelv);
    EVENT_PROP("yrel", yrelv);
    EVENT_PROP("clientX", xv);
    EVENT_PROP("clientY", yv);

    JS::AutoValueArray<1> jevent(m_Cx);
    jevent[0].setObjectOrNull(event);

    JS::RootedValue onmove(m_Cx);
    JS::RootedObject obj(m_Cx, m_JSObject);
    if (JS_GetProperty(m_Cx, obj, "_onmousemove", &onmove) &&
        onmove.isObject() &&
        JS_ObjectIsCallable(m_Cx, &onmove.toObject())) {

        JS::RootedValue rval(m_Cx);
        JS_CallFunctionValue(m_Cx, event, onmove, jevent, &rval);
    }
#undef EVENT_PROP
}


static void Storage_Finalize(JSFreeOp *fop, JSObject *obj)
{

}

static bool native_window_prop_get(JSContext *m_Cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp)
{
    Nidium::Interface::NativeUIInterface *NUI = Nidium::Frontend::Context::GetObject(m_Cx)->getUI();

    switch (id) {
        case WINDOW_PROP_DEVICE_PIXELRATIO:
            /* TODO: Actual value */
            vp.setInt32(1);
            break;
        case WINDOW_PROP_LEFT:
        {
            int x;
            NUI->getWindowPosition(&x, NULL);
            vp.setInt32(x);
            break;
        }
        case WINDOW_PROP_TOP:
        {
            int y;
            NUI->getWindowPosition(NULL, &y);
            vp.setInt32(y);
            break;
        }
        case WINDOW_PROP_WIDTH:
            vp.setInt32(NUI->getWidth());
            break;
        case WINDOW_PROP_HEIGHT:
            vp.setInt32(NUI->getHeight());
            break;
        case WINDOW_PROP_TITLE:
        {
            const char *title =  NUI->getWindowTitle();
            JS::RootedString str(m_Cx, JSUtils::NewStringWithEncoding(m_Cx, title,
                strlen(title), "utf8"));
            vp.setString(str);
        }
        break;
        case WINDOW_PROP_CURSOR:
        {
            const char * cCursor;

            cCursor = native_cursors_list[1].str;
            for (size_t i = 0; native_cursors_list[i].str != NULL; i++) {
                if (native_cursors_list[i].type == NUI->m_CurrentCursor) {
                    cCursor = native_cursors_list[i].str;
                    break;
                }
            }

            vp.setString(JS_NewStringCopyZ(m_Cx, cCursor));
        }
        break;
        default: break;
    }

    return true;
}

static bool native_window_prop_set(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp)
{
    Nidium::Interface::NativeUIInterface *NUI = Nidium::Frontend::Context::GetObject(cx)->getUI();
    switch(id) {
        case WINDOW_PROP_LEFT:
        {
            int32_t dval;
            if (!vp.isNumber()) {

                return true;
            }

            JS::ToInt32(cx, vp, &dval);
            NUI->setWindowPosition((int)dval, NATIVE_WINDOWPOS_UNDEFINED_MASK);

            break;
        }

        case WINDOW_PROP_TOP:
        {
            int32_t dval;
            if (!vp.isNumber()) {

                return true;
            }

            JS::ToInt32(cx, vp, &dval);
            NUI->setWindowPosition(NATIVE_WINDOWPOS_UNDEFINED_MASK, (int)dval);

            break;
        }
        case WINDOW_PROP_WIDTH:
        {
            int32_t dval;
            if (!vp.isNumber()) {

                return true;
            }

            JS::ToInt32(cx, vp, &dval);

            dval = ape_max(dval, 1);

            Nidium::Frontend::Context::GetObject(cx)->setWindowSize((int)dval, NUI->getHeight());

            break;
        }
        case WINDOW_PROP_HEIGHT:
        {
            int32_t dval;
            if (!vp.isNumber()) {

                return true;
            }

            JS::ToInt32(cx, vp, &dval);

            dval = ape_max(dval, 1);

            Nidium::Frontend::Context::GetObject(cx)->setWindowSize((int)NUI->getWidth(), (int)dval);

            break;
        }
        case WINDOW_PROP_TITLE:
        {
            if (!vp.isString()) {

                return true;
            }

            JS::RootedString vpStr(cx, JS::ToString(cx, vp));
            JSAutoByteString title(cx, vpStr);
            NUI->setWindowTitle(title.ptr());
            break;
        }
        case WINDOW_PROP_CURSOR:
        {
            if (!vp.isString()) {

                return true;
            }

            JS::RootedString vpStr(cx, JS::ToString(cx, vp));
            JSAutoByteString type(cx, vpStr);
            for (size_t i = 0; native_cursors_list[i].str != NULL; i++) {
                if (strncasecmp(native_cursors_list[i].str, type.ptr(),
                    strlen(native_cursors_list[i].str)) == 0) {
                    NUI->setCursor(native_cursors_list[i].type);
                    break;
                }
            }

            break;
        }
        case WINDOW_PROP_TITLEBAR_COLOR:
        {
            if (!vp.isString()) {

                return true;
            }
            JS::RootedString vpStr(cx, JS::ToString(cx, vp));
            JSAutoByteString color(cx, vpStr);
            uint32_t icolor = Graphics::SkiaContext::ParseColor(color.ptr());

            NUI->setTitleBarRGBAColor(
                (icolor & 0x00FF0000) >> 16,
                (icolor & 0x0000FF00) >> 8,
                 icolor & 0x000000FF,
                 icolor >> 24);

            break;
        }
        case WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETX:
        {
            double dval, oval;
            if (!vp.isNumber()) {

                return true;
            }

            dval = vp.toNumber();
            JS::RootedValue offsety(cx);

            if (JS_GetProperty(cx, obj, "titleBarControlsOffsetY", &offsety) == false) {
                offsety = DOUBLE_TO_JSVAL(0);
            }
            if (!offsety.isNumber()) {
                offsety = DOUBLE_TO_JSVAL(0);
            }
            oval = offsety.toNumber();
            NUI->setWindowControlsOffset(dval, oval);
            break;
        }
        case WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETY:
        {
            double dval, oval;
            if (!vp.isNumber()) {

                return true;
            }

            dval = vp.toNumber();
            JS::RootedValue offsetx(cx);
            if (JS_GetProperty(cx, obj, "titleBarControlsOffsetX", &offsetx) == false) {
                offsetx = DOUBLE_TO_JSVAL(0);
            }
            if (!offsetx.isNumber()) {
                offsetx = DOUBLE_TO_JSVAL(0);
            }
            oval = offsetx.toNumber();
            NUI->setWindowControlsOffset(oval, dval);
            break;
        }
        default:
            break;
    }
    return true;
}

static bool native_navigator_prop_get(JSContext *m_Cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp)
{
#define APP_NAME "nidium"
#define APP_LANGUAGE "en"
#define APP_LOCALE APP_LANGUAGE "-US"
    switch(id) {
       case NAVIGATOR_PROP_LANGUAGE:
            {
            JS::RootedString jStr(m_Cx, JS_NewStringCopyZ(m_Cx, APP_LANGUAGE));
            vp.setString(jStr);
            }
            break;
        case NAVIGATOR_PROP_VIBRATE:
            {
            vp.setBoolean(false);
            }
            break;
           break;
        case NAVIGATOR_PROP_APPVERSION:
            {
            JS::RootedString jStr(m_Cx, JS_NewStringCopyZ(m_Cx, NIDIUM_VERSION_STR));
            vp.setString(jStr);
            }
            break;
        case NAVIGATOR_PROP_APPNAME:
            {
            JS::RootedString jStr(m_Cx, JS_NewStringCopyZ(m_Cx, APP_NAME));
            vp.setString(jStr);
            }
            break;
        case NAVIGATOR_PROP_USERAGENT:
            {
            JS::RootedString jStr(m_Cx, JS_NewStringCopyZ(m_Cx, APP_NAME "/"
                NIDIUM_VERSION_STR "(" APP_LOCALE "; rv:" NIDIUM_BUILD ") "
                NIDIUM_FRAMEWORK_STR));
            vp.setString(jStr);
            }
            break;
        case NAVIGATOR_PROP_PLATFORM:
            {
            const char *platform;
            // http://stackoverflow.com/questions/19877924/what-is-the-list-of-possible-values-for-navigator-platform-as-of-today
#if defined(_WIN32) || defined(_WIN64)
            platform = "Win32";
#elif defined(__APPLE) || defined(_WIN64)
            platform = "MacOSX";
#elif defined(__FreeBSD)
            platform = "FreeBSD";
#elif defined(__DragonFly)
            platform = "DragonFly";
#elif __linux
            platform = "Linux";
#elif __unix
            platform = "Unix";
#elif __posix
            platform = "Posix";
#else
            platform = "Unknown";
#endif
            JS::RootedString jStr(m_Cx, JS_NewStringCopyZ(m_Cx, platform));
            vp.setString(jStr);
            }
            break;
    }
#undef APP_NAME
#undef APP_LANGUAGE
#undef APP_LOCALE
    return true;
}


struct _nativeopenfile
{
    JSContext *cx;
    JS::PersistentRootedValue cb;
};

static void native_window_openfilecb(void *_nof, const char *lst[], uint32_t len)
{
    struct _nativeopenfile *nof = (struct _nativeopenfile *)_nof;
    JS::RootedObject arr(nof->cx, JS_NewArrayObject(nof->cx, len));
    for (int i = 0; i < len; i++) {
        JS::RootedValue val(nof->cx, OBJECT_TO_JSVAL(JSFileIO::GenerateJSObject(nof->cx, lst[i])));
        JS_SetElement(nof->cx, arr, i, val);
    }

    JS::AutoValueArray<1> jarr(nof->cx);
    jarr[0].setObjectOrNull(arr);
    JS::RootedValue rval(nof->cx);
    JS::RootedValue cb(nof->cx, nof->cb);
    JS::RootedObject global(nof->cx, JS::CurrentGlobalOrNull(nof->cx));
    JS_CallFunctionValue(nof->cx, global, cb, jarr, &rval);

    nof->cx = NULL;
    free(nof);
}

static bool native_window_setSize(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    double w, h;

    if (!JS_ConvertArguments(cx, args, "dd", &w, &h)) {
        return false;
    }

    Nidium::Frontend::Context::GetObject(cx)->setWindowSize(w, h);

    return true;
}

static bool native_window_openURLInBrowser(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JS::RootedString url(cx);
    if (!JS_ConvertArguments(cx, args, "S", url.address())) {
        return false;
    }

    JSAutoByteString curl(cx, url);

    Nidium::Interface::NativeSystemInterface::GetInstance()->openURLInBrowser(curl.ptr());

    return true;
}

static bool native_window_exec(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JS::RootedString url(cx);
    if (!JS_ConvertArguments(cx, args, "S", url.address())) {
        return false;
    }

    JSAutoByteString curl(cx, url);
    const char *ret = Nidium::Interface::NativeSystemInterface::GetInstance()->execute(curl.ptr());

    JS::RootedString retStr(cx, JS_NewStringCopyZ(cx, ret));
    args.rval().setString(retStr);

    return true;
}

static bool native_window_openDirDialog(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JS::RootedValue callback(cx);
    if (!JS_ConvertArguments(cx, args, "v", callback.address())) {
        return false;
    }

    struct _nativeopenfile *nof = (struct _nativeopenfile *)malloc(sizeof(*nof));
    nof->cb = callback;
    nof->cx = cx;

    Nidium::Frontend::Context::GetObject(cx)->getUI()->openFileDialog(
        NULL,
        native_window_openfilecb, nof,
        Nidium::Interface::NativeUIInterface::kOpenFile_CanChooseDir);

    return true;
}

/* TODO: leak if the user click "cancel" */
static bool native_window_openFileDialog(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JS::RootedObject types(cx);
    JS::RootedValue callback(cx);
    if (!JS_ConvertArguments(cx, args, "ov", types.address(), callback.address())) {
        return false;
    }

    if (!JSVAL_IS_NULL(OBJECT_TO_JSVAL(types)) && !JS_IsArrayObject(cx, types)) {
        JS_ReportError(cx, "First parameter must be an array or null");
        return false;
    }
    uint32_t len = 0;

    char **ctypes = NULL;

    if (!JSVAL_IS_NULL(OBJECT_TO_JSVAL(types))) {
        JS_GetArrayLength(cx, types, &len);

        ctypes = (char **)malloc(sizeof(char *) * (len+1));
        memset(ctypes, 0, sizeof(char *) * (len+1));
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

    struct _nativeopenfile *nof = (struct _nativeopenfile *)malloc(sizeof(*nof));
    nof->cb = callback;
    nof->cx = cx;

    Nidium::Frontend::Context::GetObject(cx)->getUI()->openFileDialog(
        (const char **)ctypes,
        native_window_openfilecb, nof,
        Nidium::Interface::NativeUIInterface::kOpenFile_CanChooseFile | Nidium::Interface::NativeUIInterface::kOpenFile_AlloMultipleSelection);

    if (ctypes) {
        for (int i = 0; i < len; i++) {
            JS_free(cx, ctypes[i]);
        }
        free(ctypes);
    }
    return true;
}

static bool native_window_requestAnimationFrame(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_CHECK_ARGS("requestAnimationFrame", 1);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedValue cb(cx);
    if (!JS_ConvertValue(cx, args[0], JSTYPE_FUNCTION, &cb)) {
        return true;
    }
    JSWindow::GetObject(cx)->addFrameCallback(&cb);

    return true;
}

static bool native_window_center(JSContext *cx, unsigned argc, JS::Value *vp)
{
    Nidium::Frontend::Context::GetObject(cx)->getUI()->centerWindow();

    return true;
}

static bool native_window_setPosition(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_CHECK_ARGS("setPosition", 2);

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    int x = (args[0].isUndefined() || args[0].isNull()) ?
        NATIVE_WINDOWPOS_UNDEFINED_MASK : args[0].toInt32();

    int y = (args[1].isUndefined() || args[1].isNull()) ?
        NATIVE_WINDOWPOS_UNDEFINED_MASK : args[1].toInt32();

    Nidium::Frontend::Context::GetObject(cx)->getUI()->setWindowPosition(x, y);

    return true;
}

static bool native_window_notify(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_CHECK_ARGS("notify", 2);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    bool sound = false;

    JS::RootedString title(cx);
    JS::RootedString body(cx);
    if (!JS_ConvertArguments(cx, args, "SS/b", title.address(), body.address(), &sound)) {
        return false;
    }

    JSAutoByteString ctitle;
    ctitle.encodeUtf8(cx, title);

    JSAutoByteString cbody;
    cbody.encodeUtf8(cx, body);

    Nidium::Interface::NativeSystemInterface::GetInstance()->sendNotification(ctitle.ptr(), cbody.ptr(), sound);

    return true;
}

static bool native_window_quit(JSContext *cx, unsigned argc, JS::Value *vp)
{
    Nidium::Interface::NativeUIInterface *NUI = Nidium::Frontend::Context::GetObject(cx)->getUI();

    NUI->quit();

    return true;
}

static bool native_window_close(JSContext *cx, unsigned argc, JS::Value *vp)
{
    Nidium::Interface::NativeUIInterface *NUI = Nidium::Frontend::Context::GetObject(cx)->getUI();

    NUI->hideWindow();

    return true;
}

static bool native_window_open(JSContext *cx, unsigned argc, JS::Value *vp)
{
    Nidium::Interface::NativeUIInterface *NUI = Nidium::Frontend::Context::GetObject(cx)->getUI();

    NUI->showWindow();

    return true;
}

static bool native_window_setSystemTray(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_CHECK_ARGS("setSystemTray", 1);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    Nidium::Interface::NativeUIInterface *NUI = Nidium::Frontend::Context::GetObject(cx)->getUI();
    JS::RootedObject jobj(cx, &args[0].toObject());
    if (!jobj.get()) {
        NUI->disableSysTray();
        return true;
    }

    NIDIUM_JS_INIT_OPT();

    Nidium::Interface::NativeSystemMenu &menu = NUI->getSystemMenu();
    menu.deleteItems();

    NIDIUM_JS_GET_OPT_TYPE(jobj, "icon", Object) {
        JS::RootedObject jsimg(cx, __curopt.toObjectOrNull());
        Graphics::Image *skimage;
        if (JSImage::JSObjectIs(cx, jsimg) &&
            (skimage = JSImage::JSObjectToImage(jsimg))) {

            const uint8_t *pixels = skimage->getPixels(NULL);
            menu.setIcon(pixels, skimage->getWidth(), skimage->getHeight());
        }
    }

    NIDIUM_JS_GET_OPT_TYPE(jobj, "menu", Object) {
        JS::RootedObject arr(cx,  __curopt.toObjectOrNull());
        if (JS_IsArrayObject(cx, arr)) {
            uint32_t len;
            JS_GetArrayLength(cx, arr, &len);

            /*
                The list is a FIFO
                Walk in inverse order to keep the same Array definition order
            */
            for (int i = len-1; i >= 0; i--) {
                JS::RootedValue val(cx);
                JS_GetElement(cx, arr, i, &val);
                if (val.isObject()) {
                    JS::RootedObject valObj(cx, &val.toObject());
                    Nidium::Interface::NativeSystemMenuItem *menuItem = new Nidium::Interface::NativeSystemMenuItem();
                    NIDIUM_JS_GET_OPT_TYPE(valObj, "title", String) {

                        JSAutoByteString ctitle;
                        JS::RootedString str(cx, __curopt.toString());
                        ctitle.encodeUtf8(cx, str);
                        menuItem->title(ctitle.ptr());
                    } else {
                        menuItem->title("");
                    }
                    NIDIUM_JS_GET_OPT_TYPE(valObj, "id", String) {
                        JSAutoByteString cid;
                        JS::RootedString str(cx, __curopt.toString());
                        cid.encodeUtf8(cx, str);
                        menuItem->id(cid.ptr());
                    } else {
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

static bool native_window_setFrame(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_CHECK_ARGS("setFrame", 4);

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    int32_t x = 0, y = 0;
    int32_t w, h;

    if (args[0].isString()) {
        JS::RootedString xstr(cx, args[0].toString());
        JSAutoByteString cxstr(cx, xstr);

        if (strcmp(cxstr.ptr(), "center") == 0) {
            x = NATIVE_WINDOWPOS_CENTER_MASK;
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
            y = NATIVE_WINDOWPOS_CENTER_MASK;
        }

    } else if (args[1].isNumber()) {
        JS::ToInt32(cx, args[1], &y);
    } else {
        JS_ReportError(cx, "setFrame() invalid position");

        return false;
    }
    if (args[2].isNumber()) {
        JS::ToInt32(cx, args[2], &w);
        w = ape_max(w, 1);
    } else {
        JS_ReportError(cx, "setFrame() invalid width");

        return false;
    }

    if (args[3].isNumber()) {
        JS::ToInt32(cx, args[3], &h);

        h = ape_max(h, 1);

    } else {
        JS_ReportError(cx, "setFrame() invalid height");

        return false;
    }


    Nidium::Frontend::Context::GetObject(cx)->setWindowFrame((int)x, (int)y, (int) w, (int) h);

    return true;
}

void JSWindow::addFrameCallback(JS::MutableHandleValue cb)
{
    struct _requestedFrame *frame = new struct _requestedFrame(m_Cx);
    frame->next = m_RequestedFrame;
    frame->cb = cb;

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
            JS::RootedValue cb(m_Cx, frame->cb);
            JS::RootedValue rval(m_Cx);
            JS_CallFunctionValue(m_Cx, global, cb, arg, &rval);
        }
        struct _requestedFrame *tmp = frame->next;
        delete frame;
        frame = tmp;
    }
}

void JSWindow::initDataBase()
{
    Nidium::Frontend::NML *nml = Nidium::Frontend::Context::GetObject(m_Cx)->getNML();
    if (!nml) {
        NLOG("[Notice] Unable to create window.storage (no NML provided)");
        return;
    }

    m_Db = new JSDB(nml->getIdentifier());

    if (m_Db->ok()) {
        this->createStorage();
    } else {
        NLOG("[Notice] Unable to create window.storage (empty identifier tag in NML?)");
    }
}

void JSWindow::createMainCanvas(int width, int height, JS::HandleObject docObj)
{
    JS::RootedObject canvas(m_Cx, JSCanvas::GenerateJSObject(m_Cx, width, height, &m_Handler));
    Nidium::Frontend::Context::GetObject(m_Cx)->getRootHandler()->addChild(m_Handler);
    JS::RootedValue canval(m_Cx, OBJECT_TO_JSVAL(canvas));
    JS_DefineProperty(m_Cx, docObj, "canvas", canval, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
}

void JSWindow::createStorage()
{
    JS::RootedObject storage(m_Cx, JS_NewObject(m_Cx, &Storage_class, JS::NullPtr(), JS::NullPtr()));
    JS_DefineFunctions(m_Cx, storage, storage_funcs);
    JS::RootedValue jsstorage(m_Cx, OBJECT_TO_JSVAL(storage));
    JS::RootedObject obj(m_Cx, m_JSObject);
    JS_SetProperty(m_Cx, obj, "storage", jsstorage);
}

bool native_storage_set(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    NIDIUM_JS_CHECK_ARGS("set", 2);
    if (!args[0].isString()) {
        JS_ReportError(cx, "set() : key must be a string");
        return false;
    }

    JSAutoByteString key(cx, args[0].toString());
    if (!JSWindow::GetObject(cx)->getDataBase()->
        insert(key.ptr(), cx, args[1])) {

        JS_ReportError(cx, "Cant insert data in storage");
        return false;
    }

    args.rval().setBoolean(true);

    return true;
}

bool native_storage_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    std::string data;

    NIDIUM_JS_CHECK_ARGS("get", 1);
    if (!args[0].isString()) {
        JS_ReportError(cx, "get() : key must be a string");
        return false;
    }

    JSDB *db = JSWindow::GetObject(cx)->getDataBase();


    JSAutoByteString key(cx, args[0].toString());
    if (!db->get(key.ptr(), data)) {
        args.rval().setUndefined();
        return true;
    }
    JS::RootedValue ret(cx);

    uint64_t *aligned_data;

    /*
        ReadStructuredClone requires 8-bytes aligned memory
    */
    if (((uintptr_t)data.data() & 7) == 0) {
        aligned_data = (uint64_t *)data.data();
    } else {
        if (posix_memalign((void **)&aligned_data, 8, data.length()) != 0) {
            return false;
        }

        memcpy(aligned_data, data.data(), data.length());
    }

    if (!JS_ReadStructuredClone(cx, aligned_data, data.length(),
        JS_STRUCTURED_CLONE_VERSION, &ret, nullptr, NULL)) {

        JS_ReportError(cx, "Unable to read internal data");
        return false;
    }

    args.rval().set(ret);

    if ((void *)aligned_data != data.data()) {
        free(aligned_data);
    }

    return true;
}

JSWindow* JSWindow::GetObject(JSContext *cx)
{
    return Nidium::Frontend::Context::GetObject(cx)->getJSWindow();
}

JSWindow* JSWindow::GetObject(NidiumJS *njs)
{
    return Nidium::Frontend::Context::GetObject(njs)->getJSWindow();
}
// }}}

// {{{ Registration
JSWindow *JSWindow::RegisterObject(JSContext *cx, int width,
    int height, JS::HandleObject docObj)
{
    JS::RootedObject globalObj(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedObject windowObj(cx, globalObj);
    JSWindow *jwin = new JSWindow(globalObj, cx);

    JS_SetPrivate(globalObj, jwin);

    jwin->initDataBase();
    jwin->createMainCanvas(width, height, docObj);
    JS_DefineFunctions(cx, windowObj, window_funcs);
    JS_DefineProperties(cx, windowObj, window_props);

    JS::RootedValue val(cx, DOUBLE_TO_JSVAL(0));
    JS_SetProperty(cx, windowObj, "titleBarControlsOffsetX", val);

    val = DOUBLE_TO_JSVAL(0);
    JS_SetProperty(cx, windowObj, "titleBarControlsOffsetY", val);

    // Set the __nidium__ properties
    JS::RootedObject nidiumObj(cx, JS_NewObject(cx,  nullptr, JS::NullPtr(), JS::NullPtr()));

    JS::RootedString jVersion(cx, JS_NewStringCopyZ(cx, NIDIUM_VERSION_STR));
    JS_DefineProperty(cx, nidiumObj, "version", jVersion, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS::RootedString jFramework(cx, JS_NewStringCopyZ(cx, NIDIUM_FRAMEWORK_STR));
    JS_DefineProperty(cx, nidiumObj, "build", jFramework, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS::RootedString jRevision(cx, JS_NewStringCopyZ(cx, NIDIUM_BUILD));
    JS_DefineProperty(cx, nidiumObj, "revision", jRevision, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    val = OBJECT_TO_JSVAL(nidiumObj);
    JS_SetProperty(cx, windowObj, "__nidium__", val);

    //  Set the navigator properties
    JS::RootedObject navigatorObj(cx, JS_NewObject(cx, &Navigator_class, JS::NullPtr(), JS::NullPtr()));
    JS_DefineProperties(cx, navigatorObj, navigator_props);

    val = OBJECT_TO_JSVAL(navigatorObj);
    JS_SetProperty(cx, windowObj, "navigator", val);

    return jwin;
}
// }}}

} // namespace Nidium
} // namespace Binding

