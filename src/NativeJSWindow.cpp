#include "NativeJSWindow.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>

#include <NativeJSFileIO.h>

#include "NativeDB.h"
#include "NativeNML.h"
#include "NativeSkia.h"
#include "NativeContext.h"
#include "NativeJSCanvas.h"
#include "NativeJSUtils.h"
#include "NativeJSImage.h"
#include "NativeSkImage.h"
#include "NativeSystemInterface.h"

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


static void Window_Finalize(JSFreeOp *fop, JSObject *obj);
static void Storage_Finalize(JSFreeOp *fop, JSObject *obj);

enum {
    WINDOW_PROP_LEFT,
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

static JSClass window_class = {
    "Window", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Window_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

enum {
    NAVIGATOR_PROP_LANGUAGE,
    NAVIGATOR_PROP_VIBRATE,
    NAVIGATOR_PROP_APPNAME,
    NAVIGATOR_PROP_APPVERSION,
    NAVIGATOR_PROP_PLATFORM,
    NAVIGATOR_PROP_USERAGENT
};

static JSClass navigator_class = {
    "Navigator", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass storage_class = {
    "NidiumStorage", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Storage_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

JSClass *NativeJSwindow::jsclass = &window_class;
template<>
JSClass *NativeJSExposer<NativeJSwindow>::jsclass = &window_class;


static JSClass mouseEvent_class = {
    "MouseEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass dragEvent_class = {
    "DragEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

#if 0
static JSClass windowEvent_class = {
    "WindowEvent", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};
#endif

static JSClass textEvent_class = {
    "TextInputEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass keyEvent_class = {
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
    JS_FN("set", native_storage_set, 2, 0),
    JS_FN("get", native_storage_get, 1, 0),
    JS_FS_END
};

static JSFunctionSpec window_funcs[] = {
    JS_FN("openFileDialog", native_window_openFileDialog, 2, 0),
    JS_FN("openDirDialog", native_window_openDirDialog, 1, 0),
    JS_FN("setSize", native_window_setSize, 2, 0),
    JS_FN("requestAnimationFrame", native_window_requestAnimationFrame, 1, 0),
    JS_FN("center", native_window_center, 0, 0),
    JS_FN("setPosition", native_window_setPosition, 2, 0),
    JS_FN("setFrame", native_window_setFrame, 4, 0),
    JS_FN("notify", native_window_notify, 2, 0),
    JS_FN("quit", native_window_quit, 0, 0),
    JS_FN("close", native_window_close, 0, 0),
    JS_FN("open", native_window_open, 0, 0),
    JS_FN("setSystemTray", native_window_setSystemTray, 1, 0),
    JS_FN("openURL", native_window_openURLInBrowser, 1, 0),
    JS_FN("exec", native_window_exec, 1, 0),
    JS_FS_END
};

static struct native_cursors {
    const char *str;
    NativeUIInterface::CURSOR_TYPE type;
} native_cursors_list[] = {
    {"default",             NativeUIInterface::ARROW},
    {"arrow",               NativeUIInterface::ARROW},
    {"beam",                NativeUIInterface::BEAM},
    {"text",                NativeUIInterface::BEAM},
    {"pointer",             NativeUIInterface::POINTING},
    {"grabbing",            NativeUIInterface::CLOSEDHAND},
    {"drag",                NativeUIInterface::CLOSEDHAND},
    {"hidden",              NativeUIInterface::HIDDEN},
    {"none",                NativeUIInterface::HIDDEN},
    {"col-resize",          NativeUIInterface::RESIZELEFTRIGHT},
    {NULL,                  NativeUIInterface::NOCHANGE},
};

static JSPropertySpec window_props[] = {
    {"devicePixelRatio", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS | JSPROP_READONLY,
        NATIVE_JS_GETTER(WINDOW_PROP_DEVICE_PIXELRATIO, native_window_prop_get),
        JSOP_NULLWRAPPER},
    {"left", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,
        NATIVE_JS_GETTER(WINDOW_PROP_LEFT, native_window_prop_get),
        NATIVE_JS_SETTER(WINDOW_PROP_LEFT, native_window_prop_set)},
    {"top", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,
        NATIVE_JS_GETTER(WINDOW_PROP_TOP, native_window_prop_get),
        NATIVE_JS_SETTER(WINDOW_PROP_TOP, native_window_prop_set)},
    {"innerWidth", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,
        NATIVE_JS_GETTER(WINDOW_PROP_WIDTH, native_window_prop_get),
        NATIVE_JS_SETTER(WINDOW_PROP_WIDTH, native_window_prop_set)},
    {"outerWidth", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,
        NATIVE_JS_GETTER(WINDOW_PROP_WIDTH, native_window_prop_get),
        NATIVE_JS_SETTER(WINDOW_PROP_WIDTH, native_window_prop_set)},
    {"innerHeight", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,
        NATIVE_JS_GETTER(WINDOW_PROP_HEIGHT, native_window_prop_get),
        NATIVE_JS_SETTER(WINDOW_PROP_HEIGHT, native_window_prop_set)},
    {"outerHeight", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,
        NATIVE_JS_GETTER(WINDOW_PROP_HEIGHT, native_window_prop_get),
        NATIVE_JS_SETTER(WINDOW_PROP_HEIGHT, native_window_prop_set)},
    {"title", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,
        NATIVE_JS_GETTER(WINDOW_PROP_TITLE, native_window_prop_get),
        NATIVE_JS_SETTER(WINDOW_PROP_TITLE, native_window_prop_set)},
    {"cursor", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,
        NATIVE_JS_STUBGETTER(),
        NATIVE_JS_SETTER(WINDOW_PROP_CURSOR, native_window_prop_set)},
    {"titleBarColor", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,
        NATIVE_JS_STUBGETTER(),
        NATIVE_JS_SETTER(WINDOW_PROP_TITLEBAR_COLOR, native_window_prop_set)},
    {"titleBarControlsOffsetX", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,
        NATIVE_JS_STUBGETTER(),
        NATIVE_JS_SETTER(WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETX, native_window_prop_set)},
    {"titleBarControlsOffsetY", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,
        NATIVE_JS_STUBGETTER(),
        NATIVE_JS_SETTER(WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETY, native_window_prop_set)},
    JS_PS_END
};

static JSPropertySpec navigator_props[] = {
    {"language", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS | JSPROP_READONLY,
        NATIVE_JS_GETTER(NAVIGATOR_PROP_LANGUAGE, native_navigator_prop_get),
        JSOP_NULLWRAPPER},
    {"vibrate", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS | JSPROP_READONLY,
        NATIVE_JS_GETTER(NAVIGATOR_PROP_VIBRATE, native_navigator_prop_get),
        JSOP_NULLWRAPPER},
    {"appName", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS | JSPROP_READONLY,
        NATIVE_JS_GETTER(NAVIGATOR_PROP_APPNAME, native_navigator_prop_get),
        JSOP_NULLWRAPPER},
    {"appVersion", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS | JSPROP_READONLY,
        NATIVE_JS_GETTER(NAVIGATOR_PROP_APPVERSION, native_navigator_prop_get),
        JSOP_NULLWRAPPER},
    {"platform", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS | JSPROP_READONLY,
        NATIVE_JS_GETTER(NAVIGATOR_PROP_PLATFORM, native_navigator_prop_get),
        JSOP_NULLWRAPPER},
    {"userAgent", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS | JSPROP_READONLY,
        NATIVE_JS_GETTER(NAVIGATOR_PROP_USERAGENT, native_navigator_prop_get),
        JSOP_NULLWRAPPER},
    JS_PS_END
};

NativeJSwindow::~NativeJSwindow()
{
    if (m_Dragging) {
        /* cleanup drag files */
    }
    if (m_Db) {
        delete m_Db;
    }
};

void NativeJSwindow::onReady(JS::HandleObject layout)
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

bool NativeJSwindow::onClose()
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

void NativeJSwindow::assetReady(const NMLTag &tag)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
        val, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS)

    JSContext *cx = m_Cx;
    JS::AutoValueArray<1> jevent(cx);

    JS::RootedObject event(cx, JS_NewObject(cx, &NMLEvent_class, JS::NullPtr(), JS::NullPtr()));
    jevent[0].set(OBJECT_TO_JSVAL(event));
    JS::RootedString tagStr(cx, JS_NewStringCopyZ(cx, (const char *)tag.tag));
    JS::RootedString idStr(cx, JS_NewStringCopyZ(cx, (const char *)tag.id));
    JS::RootedString dataStr(cx, NativeJSUtils::newStringWithEncoding(cx,
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

void NativeJSwindow::windowFocus()
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

void NativeJSwindow::windowBlur()
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

void NativeJSwindow::mouseWheel(int xrel, int yrel, int x, int y)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS | JSPROP_READONLY)

    JSAutoRequest ar(m_Cx);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &mouseEvent_class, JS::NullPtr(), JS::NullPtr()));
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
    JS::RootedObject obje(cx, NativeJSEvents::CreateEventObject(m_Cx));
    this->fireJSEvent("wheel", OBJECT_TO_JSVAL(obje));
    */

#undef EVENT_PROP
}

void NativeJSwindow::keyupdown(int keycode, int mod, int state, int repeat, int location)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, JSPROP_PERMANENT | JSPROP_ENUMERATE)

    JSAutoRequest ar(m_Cx);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &keyEvent_class, JS::NullPtr(), JS::NullPtr()));
    JS::RootedValue keyV(m_Cx, INT_TO_JSVAL(keycode));
    JS::RootedValue locationV(m_Cx, INT_TO_JSVAL(location));
    JS::RootedValue alt(m_Cx, BOOLEAN_TO_JSVAL(!!(mod & NATIVE_KEY_ALT)));
    JS::RootedValue ctl(m_Cx, BOOLEAN_TO_JSVAL(!!(mod & NATIVE_KEY_CTRL)));
    JS::RootedValue shift(m_Cx, BOOLEAN_TO_JSVAL(!!(mod & NATIVE_KEY_SHIFT)));
    JS::RootedValue meta(m_Cx, BOOLEAN_TO_JSVAL(!!(mod & NATIVE_KEY_META)));
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

void NativeJSwindow::textInput(const char *data)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    JSAutoRequest ar(m_Cx);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &textEvent_class, JS::NullPtr(), JS::NullPtr()));
    JS::RootedString str(m_Cx, NativeJSUtils::newStringWithEncoding(m_Cx, data, strlen(data), "utf8"));
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

void NativeJSwindow::systemMenuClicked(const char *id)
{
    JSContext *cx = m_Cx;
    JS::RootedObject event(cx, JS_NewObject(m_Cx, nullptr, JS::NullPtr(), JS::NullPtr()));

    JSOBJ_SET_PROP_CSTR(event, "id", id);
    JS::AutoValueArray<1> ev(cx);
    ev[0].setObjectOrNull(event);
    JS::RootedObject obj(cx, m_JSObject);
    JSOBJ_CALLFUNCNAME(obj, "_onsystemtrayclick", ev);
}

void NativeJSwindow::mouseClick(int x, int y, int state, int button, int clicks)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    JSAutoRequest ar(m_Cx);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &mouseEvent_class, JS::NullPtr(), JS::NullPtr()));

    NativeContext *nctx = NativeContext::getNativeClass(m_Cx);
    NativeInputEvent *ev = new NativeInputEvent(state ?
        NativeInputEvent::kMouseClick_Type :
        NativeInputEvent::kMouseClickRelease_Type, x, y);

    ev->m_data[0] = button;

    nctx->addInputEvent(ev);

    /*
        Handle double click.
        clicks receiv the number of successive clicks.
        Only trigger for even number on release.
    */
    if (clicks % 2 == 0 && !state) {
        NativeInputEvent *ev = new NativeInputEvent(NativeInputEvent::kMouseDoubleClick_Type, x, y);

        ev->m_data[0] = button;
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

bool NativeJSwindow::dragEvent(const char *name, int x, int y)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)
    JSAutoRequest ar(m_Cx);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &dragEvent_class, JS::NullPtr(), JS::NullPtr()));
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

bool NativeJSwindow::dragBegin(int x, int y, const char * const *files, size_t nfiles)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    if (m_Dragging) {
        return false;
    }
    m_Dragging = true; //Duh..

    m_DraggedFiles = JS_NewArrayObject(m_Cx, (int)nfiles);
    JS::RootedObject dragged(m_Cx, m_DraggedFiles);

    for (int i = 0; i < nfiles; i++) {
        JS::RootedValue val(m_Cx, OBJECT_TO_JSVAL(NativeJSFileIO::generateJSObject(m_Cx, files[i])));
        JS_SetElement(m_Cx, dragged, i, val);
    }

    NativeJS::getNativeClass(m_Cx)->rootObjectUntilShutdown(m_DraggedFiles);

    return this->dragEvent("_onFileDragEnter", x, y);
#undef EVENT_PROP
}

void NativeJSwindow::dragLeave()
{
    if (!m_Dragging) {
        return;
    }
    this->dragEvent("_onFileDragLeave", 0, 0);
    this->dragEnd();
}

bool NativeJSwindow::dragUpdate(int x, int y)
{
    if (!m_Dragging) {
        return false;
    }
    bool drag = this->dragEvent("_onFileDrag", x, y);

    return drag;
}

bool NativeJSwindow::dragDroped(int x, int y)
{
    if (!m_Dragging) {
        return false;
    }

    return this->dragEvent("_onFileDrop", x, y);
}

void NativeJSwindow::dragEnd()
{
    if (!m_Dragging) {
        return;
    }

    NativeJS::getNativeClass(m_Cx)->unrootObject(m_DraggedFiles);

    m_DraggedFiles = NULL;
    m_Dragging = false;
}

void NativeJSwindow::resized(int width, int height)
{
    NativeContext::getNativeClass(m_Cx)->sizeChanged(width, height);
}

void NativeJSwindow::mouseMove(int x, int y, int xrel, int yrel)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    NativeContext *nctx = NativeContext::getNativeClass(m_Cx);

    NativeCanvasHandler *rootHandler = nctx->getRootHandler();

    rootHandler->m_MousePosition.x = x;
    rootHandler->m_MousePosition.y = y;
    rootHandler->m_MousePosition.xrel += xrel;
    rootHandler->m_MousePosition.yrel += yrel;
    rootHandler->m_MousePosition.consumed = false;

    NativeInputEvent *ev = new NativeInputEvent(NativeInputEvent::kMouseMove_Type, x, y);
    ev->m_data[0] = xrel;
    ev->m_data[1] = yrel;

    nctx->addInputEvent(ev);

    JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &mouseEvent_class, JS::NullPtr(), JS::NullPtr()));
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

static void Window_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSwindow *jwin = NativeContext::getNativeClass()->getJSWindow();

    if (jwin != NULL) {
        delete jwin;
    }
}

static void Storage_Finalize(JSFreeOp *fop, JSObject *obj)
{

}

static bool native_window_prop_get(JSContext *m_Cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp)
{
    NativeUIInterface *NUI = NativeContext::getNativeClass(m_Cx)->getUI();

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
            JS::RootedString str(m_Cx, NativeJSUtils::newStringWithEncoding(m_Cx, title,
                strlen(title), "utf8"));
            vp.setString(str);
        }
        break;
    }

    return true;
}

static bool native_window_prop_set(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp)
{
    NativeUIInterface *NUI = NativeContext::getNativeClass(cx)->getUI();
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

            NativeContext::getNativeClass(cx)->setWindowSize((int)dval, NUI->getHeight());

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

            NativeContext::getNativeClass(cx)->setWindowSize((int)NUI->getWidth(), (int)dval);

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
            for (int i = 0; native_cursors_list[i].str != NULL; i++) {
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
            uint32_t icolor = NativeSkia::parseColor(color.ptr());

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
            JS::RootedString jStr(m_Cx, JS_NewStringCopyZ(m_Cx, NATIVE_VERSION_STR));
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
                NATIVE_VERSION_STR "(" APP_LOCALE "; rv:" NATIVE_BUILD ") "
                NATIVE_FRAMEWORK_STR));
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
        JS::RootedValue val(nof->cx, OBJECT_TO_JSVAL(NativeJSFileIO::generateJSObject(nof->cx, lst[i])));
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

    NativeContext::getNativeClass(cx)->setWindowSize(w, h);

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

    NativeSystemInterface::getInstance()->openURLInBrowser(curl.ptr());

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
    const char *ret = NativeSystemInterface::getInstance()->execute(curl.ptr());

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

    NativeContext::getNativeClass(cx)->getUI()->openFileDialog(
        NULL,
        native_window_openfilecb, nof,
        NativeUIInterface::kOpenFile_CanChooseDir);

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

    NativeContext::getNativeClass(cx)->getUI()->openFileDialog(
        (const char **)ctypes,
        native_window_openfilecb, nof,
        NativeUIInterface::kOpenFile_CanChooseFile | NativeUIInterface::kOpenFile_AlloMultipleSelection);

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
    NATIVE_CHECK_ARGS("requestAnimationFrame", 1);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedValue cb(cx);
    if (!JS_ConvertValue(cx, args[0], JSTYPE_FUNCTION, &cb)) {
        return true;
    }
    NativeJSwindow::getNativeClass(cx)->addFrameCallback(&cb);

    return true;
}

static bool native_window_center(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativeContext::getNativeClass(cx)->getUI()->centerWindow();

    return true;
}

static bool native_window_setPosition(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NATIVE_CHECK_ARGS("setPosition", 2);

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    int x = (args[0].isUndefined() || args[0].isNull()) ?
        NATIVE_WINDOWPOS_UNDEFINED_MASK : args[0].toInt32();

    int y = (args[1].isUndefined() || args[1].isNull()) ?
        NATIVE_WINDOWPOS_UNDEFINED_MASK : args[1].toInt32();

    NativeContext::getNativeClass(cx)->getUI()->setWindowPosition(x, y);

    return true;
}

static bool native_window_notify(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NATIVE_CHECK_ARGS("notify", 2);
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

    NativeSystemInterface::getInstance()->sendNotification(ctitle.ptr(), cbody.ptr(), sound);

    return true;
}

static bool native_window_quit(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativeUIInterface *NUI = NativeContext::getNativeClass(cx)->getUI();

    NUI->quit();

    return true;
}

static bool native_window_close(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativeUIInterface *NUI = NativeContext::getNativeClass(cx)->getUI();

    NUI->hideWindow();

    return true;
}

static bool native_window_open(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativeUIInterface *NUI = NativeContext::getNativeClass(cx)->getUI();

    NUI->showWindow();

    return true;
}

static bool native_window_setSystemTray(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NATIVE_CHECK_ARGS("setSystemTray", 1);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    NativeUIInterface *NUI = NativeContext::getNativeClass(cx)->getUI();
    JS::RootedObject jobj(cx, &args[0].toObject());
    if (!jobj.get()) {
        NUI->disableSysTray();
        return true;
    }

    JS_INITOPT();

    NativeSystemMenu &menu = NUI->getSystemMenu();
    menu.deleteItems();

    JSGET_OPT_TYPE(jobj, "icon", Object) {
        JS::RootedObject jsimg(cx, __curopt.toObjectOrNull());
        NativeSkImage *skimage;
        if (NativeJSImage::JSObjectIs(cx, jsimg) &&
            (skimage = NativeJSImage::JSObjectToNativeSkImage(jsimg))) {

            const uint8_t *pixels = skimage->getPixels(NULL);
            menu.setIcon(pixels, skimage->getWidth(), skimage->getHeight());
        }
    }

    JSGET_OPT_TYPE(jobj, "menu", Object) {
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
                    NativeSystemMenuItem *menuItem = new NativeSystemMenuItem();
                    JSGET_OPT_TYPE(valObj, "title", String) {

                        JSAutoByteString ctitle;
                        JS::RootedString str(cx, __curopt.toString());
                        ctitle.encodeUtf8(cx, str);
                        menuItem->title(ctitle.ptr());
                    } else {
                        menuItem->title("");
                    }
                    JSGET_OPT_TYPE(valObj, "id", String) {
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
    NATIVE_CHECK_ARGS("setFrame", 4);

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
    } else {
        JS_ReportError(cx, "setFrame() invalid width");

        return false;
    }

    if (args[3].isNumber()) {
        JS::ToInt32(cx, args[3], &h);
    } else {
        JS_ReportError(cx, "setFrame() invalid height");

        return false;
    }


    NativeContext::getNativeClass(cx)->setWindowFrame((int)x, (int)y, (int) w, (int) h );

    return true;
}

void NativeJSwindow::addFrameCallback(JS::MutableHandleValue cb)
{
    struct _requestedFrame *frame = new struct _requestedFrame(m_Cx);
    frame->next = m_RequestedFrame;
    frame->cb = cb;

    m_RequestedFrame = frame;
}

void NativeJSwindow::callFrameCallbacks(double ts, bool garbage)
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

void NativeJSwindow::initDataBase()
{
    NativeNML *nml = NativeContext::getNativeClass(m_Cx)->getNML();
    if (!nml) {
        NLOG("[Notice] Unable to create window.storage (no NML provided)");
        return;
    }

    m_Db = new NativeDB(nml->getIdentifier());

    if (m_Db->ok()) {
        this->createStorage();
    } else {
        NLOG("[Notice] Unable to create window.storage (empty identifier tag in NML?)");
    }
}

void NativeJSwindow::createMainCanvas(int width, int height, JS::HandleObject docObj)
{
    JS::RootedObject canvas(m_Cx, NativeJSCanvas::generateJSObject(m_Cx, width, height, &m_Handler));
    NativeContext::getNativeClass(m_Cx)->getRootHandler()->addChild(m_Handler);
    JS::RootedValue canval(m_Cx, OBJECT_TO_JSVAL(canvas));
    JS_DefineProperty(m_Cx, docObj, "canvas", canval, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
}

void NativeJSwindow::createStorage()
{
    JS::RootedObject storage(m_Cx, JS_NewObject(m_Cx, &storage_class, JS::NullPtr(), JS::NullPtr()));
    JS_DefineFunctions(m_Cx, storage, storage_funcs);
    JS::RootedValue jsstorage(m_Cx, OBJECT_TO_JSVAL(storage));
    JS::RootedObject obj(m_Cx, m_JSObject);
    JS_SetProperty(m_Cx, obj, "storage", jsstorage);
}

bool native_storage_set(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    NATIVE_CHECK_ARGS("set", 2);
    if (!args[0].isString()) {
        JS_ReportError(cx, "set() : key must be a string");
        return false;
    }

    JSAutoByteString key(cx, args[0].toString());
    if (!NativeJSwindow::getNativeClass(cx)->getDataBase()->
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

    NATIVE_CHECK_ARGS("get", 1);
    if (!args[0].isString()) {
        JS_ReportError(cx, "get() : key must be a string");
        return false;
    }

    NativeDB *db = NativeJSwindow::getNativeClass(cx)->getDataBase();


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

NativeJSwindow *NativeJSwindow::registerObject(JSContext *cx, int width,
    int height, JS::HandleObject docObj)
{
    JS::RootedObject globalObj(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedObject windowObj(cx, globalObj);
    NativeJSwindow *jwin = new NativeJSwindow(globalObj, cx);
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

    JS::RootedString jVersion(cx, JS_NewStringCopyZ(cx, NATIVE_VERSION_STR));
    JS_DefineProperty(cx, nidiumObj, "version", jVersion, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS::RootedString jFramework(cx, JS_NewStringCopyZ(cx, NATIVE_FRAMEWORK_STR));
    JS_DefineProperty(cx, nidiumObj, "build", jFramework, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS::RootedString jRevision(cx, JS_NewStringCopyZ(cx, NATIVE_BUILD));
    JS_DefineProperty(cx, nidiumObj, "revision", jRevision, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    val = OBJECT_TO_JSVAL(nidiumObj);
    JS_SetProperty(cx, windowObj, "__nidium__", val);

    //  Set the navigator properties
    JS::RootedObject navigatorObj(cx, JS_NewObject(cx, &navigator_class, JS::NullPtr(), JS::NullPtr()));
    JS_DefineProperties(cx, navigatorObj, navigator_props);

    val = OBJECT_TO_JSVAL(navigatorObj);
    JS_SetProperty(cx, windowObj, "navigator", val);

    return jwin;
}

NativeJSwindow* NativeJSwindow::getNativeClass(JSContext *cx)
{
    return NativeContext::getNativeClass(cx)->getJSWindow();
}

NativeJSwindow* NativeJSwindow::getNativeClass(NativeJS *njs)
{
    return NativeContext::getNativeClass(njs)->getJSWindow();
}

