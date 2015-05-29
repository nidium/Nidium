#include "NativeJSWindow.h"
#include "NativeJS.h"
#include "NativeUIInterface.h"
#include "NativeSkia.h"
#include "NativeUtils.h"
#include "NativeContext.h"
#include "NativeJSNative.h"
#include "NativeJSCanvas.h"
#include "NativeDB.h"
#include "NativeNML.h"
#include "NativeMacros.h"
#include "NativeJSUtils.h"
#include "NativeJSImage.h"
#include "NativeSkImage.h"

#include <NativeSystemInterface.h>

#include <NativeJSFileIO.h>

static bool native_window_prop_set(JSContext *cx, JS::HandleObject obj,
    JS::HandleId id, bool strict, JS::MutableHandleValue vp);
static bool native_window_prop_get(JSContext *cx, JS::HandleObject obj,
    JS::HandleId id, JS::MutableHandleValue vp);

static bool native_navigator_prop_get(JSContext *cx, JS::HandleObject obj,
    JS::HandleId id, JS::MutableHandleValue vp);

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
	JSCLASS_NO_OPTIONAL_MEMBERS
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
	JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass storage_class = {
    "NidiumStorage", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Storage_Finalize,
	JSCLASS_NO_OPTIONAL_MEMBERS
};

JSClass *NativeJSwindow::jsclass = &window_class;
template<>
JSClass *NativeJSExposer<NativeJSwindow>::jsclass = &window_class;


static JSClass mouseEvent_class = {
    "MouseEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass dragEvent_class = {
    "DragEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
	JSCLASS_NO_OPTIONAL_MEMBERS
};

#if 0
static JSClass windowEvent_class = {
    "WindowEvent", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
	JSCLASS_NO_OPTIONAL_MEMBERS
};
#endif

static JSClass textEvent_class = {
    "TextInputEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass keyEvent_class = {
    "keyEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass NMLEvent_class = {
    "NMLEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    JSCLASS_NO_OPTIONAL_MEMBERS
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
    {"devicePixelRatio", WINDOW_PROP_DEVICE_PIXELRATIO, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY,
        JSOP_WRAPPER(native_window_prop_get),
        JSOP_NULLWRAPPER},
    {"left", WINDOW_PROP_LEFT, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_window_prop_get),
        JSOP_WRAPPER(native_window_prop_set)},
    {"top", WINDOW_PROP_TOP, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_window_prop_get),
        JSOP_WRAPPER(native_window_prop_set)},
    {"innerWidth", WINDOW_PROP_WIDTH, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_window_prop_get),
        JSOP_WRAPPER(native_window_prop_set)},
    {"outerWidth", WINDOW_PROP_WIDTH, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_window_prop_get),
        JSOP_WRAPPER(native_window_prop_set)},
    {"innerHeight", WINDOW_PROP_HEIGHT, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_window_prop_get),
        JSOP_WRAPPER(native_window_prop_set)},
    {"outerHeight", WINDOW_PROP_HEIGHT, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_window_prop_get),
        JSOP_WRAPPER(native_window_prop_set)},
    {"title", WINDOW_PROP_TITLE, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_window_prop_get),
        JSOP_WRAPPER(native_window_prop_set)},
    {"cursor", WINDOW_PROP_CURSOR, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_NULLWRAPPER,
        JSOP_WRAPPER(native_window_prop_set)},
    {"titleBarColor", WINDOW_PROP_TITLEBAR_COLOR, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_NULLWRAPPER,
        JSOP_WRAPPER(native_window_prop_set)},
    {"titleBarControlsOffsetX", WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETX, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_NULLWRAPPER,
        JSOP_WRAPPER(native_window_prop_set)},
    {"titleBarControlsOffsetY", WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETY, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_NULLWRAPPER,
        JSOP_WRAPPER(native_window_prop_set)},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSPropertySpec navigator_props[] = {
    {"language", NAVIGATOR_PROP_LANGUAGE, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY,
        JSOP_WRAPPER(native_navigator_prop_get),
        JSOP_NULLWRAPPER},
    {"vibrate", NAVIGATOR_PROP_VIBRATE, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY,
        JSOP_WRAPPER(native_navigator_prop_get),
        JSOP_NULLWRAPPER},
    {"appName", NAVIGATOR_PROP_APPNAME, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY,
        JSOP_WRAPPER(native_navigator_prop_get),
        JSOP_NULLWRAPPER},
    {"appVersion", NAVIGATOR_PROP_APPVERSION, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY,
        JSOP_WRAPPER(native_navigator_prop_get),
        JSOP_NULLWRAPPER},
    {"platform", NAVIGATOR_PROP_PLATFORM, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY,
        JSOP_WRAPPER(native_navigator_prop_get),
        JSOP_NULLWRAPPER},
    {"userAgent", NAVIGATOR_PROP_USERAGENT, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY,
        JSOP_WRAPPER(native_navigator_prop_get),
        JSOP_NULLWRAPPER},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
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

void NativeJSwindow::onReady(JSObject *layout)
{
    JS::Value arg[1];
    JS::RootedValue onready(m_Cx);
    JS::RootedValue rval(m_Cx);
    JS::RootedObject rlayout(m_Cx);

    if (layout) {
        rlayout = layout;
        arg[0] = OBJECT_TO_JSVAL(rlayout);
    } else {
        arg[0] = OBJECT_TO_JSVAL(JS_NewArrayObject(m_Cx, 0));
    }

    if (JS_GetProperty(m_Cx, m_JSObject, "_onready", &onready) &&
        !JSVAL_IS_PRIMITIVE(onready) &&
        JS_ObjectIsCallable(m_Cx, &onready.toObject())) {

        JS_CallFunctionValue(m_Cx, m_JSObject, onready, 1, arg, &rval);
    }
}

bool NativeJSwindow::onClose()
{
    JS::RootedValue onclose(m_Cx);
    JS::RootedValue rval(m_Cx);

    if (JS_GetProperty(m_Cx, m_JSObject, "_onbeforeclose", &onclose) &&
        !JSVAL_IS_PRIMITIVE(onclose) &&
        JS_ObjectIsCallable(m_Cx, &onclose.toObject())) {

        JS_CallFunctionValue(m_Cx, m_JSObject, onclose, 0, nullptr, &rval);

        return (rval.isUndefined() || rval.toBoolean());
    }

    return true;
}

void NativeJSwindow::assetReady(const NMLTag &tag)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
        val, nullptr, nullptr, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    JSContext *cx = m_Cx;
    JS::RootedValue onassetready(cx);
    JS::RootedValue rval(cx);
    JS::RootedValue jevent(cx);
    JS::RootedObject event(cx);

    event = JS_NewObject(m_Cx, &NMLEvent_class, JS::NullPtr(), JS::NullPtr());
    jevent = OBJECT_TO_JSVAL(event);

    EVENT_PROP("data", STRING_TO_JSVAL(NativeJSUtils::newStringWithEncoding(cx,
        (const char *)tag.content.data, tag.content.len, "utf8")));
    EVENT_PROP("tag", STRING_TO_JSVAL(JS_NewStringCopyZ(cx, (const char *)tag.tag)));
    EVENT_PROP("id", STRING_TO_JSVAL(JS_NewStringCopyZ(cx, (const char *)tag.id)));

    if (JS_GetProperty(cx, m_JSObject, "_onassetready", &onassetready) &&
        !JSVAL_IS_PRIMITIVE(onassetready) &&
        JS_ObjectIsCallable(cx, &onassetready.toObject())) {

        JS_CallFunctionValue(cx, event, onassetready, 1, &jevent, &rval);
    }
}

void NativeJSwindow::windowFocus()
{
    JS::RootedValue rval(m_Cx);
    JS::RootedValue onfocus(m_Cx);

    if (JS_GetProperty(m_Cx, m_JSObject, "_onfocus", &onfocus) &&
        !JSVAL_IS_PRIMITIVE(onfocus) &&
        JS_ObjectIsCallable(m_Cx, &onfocus.toObject())) {

        JS_CallFunctionValue(m_Cx, nullptr, onfocus, 0, nullptr, &rval);
    }
}

void NativeJSwindow::windowBlur()
{
    JS::RootedValue rval(m_Cx);
    JS::RootedValue onblur(m_Cx);

    if (JS_GetProperty(m_Cx, m_JSObject, "_onblur", &onblur) &&
        !JSVAL_IS_PRIMITIVE(onblur) &&
        JS_ObjectIsCallable(m_Cx, &onblur.toObject())) {

        JS_CallFunctionValue(m_Cx, nullptr, onblur, 0, nullptr, &rval);
    }
}

void NativeJSwindow::mouseWheel(int xrel, int yrel, int x, int y)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, nullptr, nullptr, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    JS::RootedValue rval(m_Cx);
    JS::RootedValue jevent(m_Cx);
    JS::RootedValue onwheel(m_Cx);
    JS::RootedObject event(m_Cx);

    JSAutoRequest ar(m_Cx);

    event = JS_NewObject(m_Cx, &mouseEvent_class, JS::NullPtr(), JS::NullPtr());

    EVENT_PROP("xrel", INT_TO_JSVAL(xrel));
    EVENT_PROP("yrel", INT_TO_JSVAL(yrel));
    EVENT_PROP("x", INT_TO_JSVAL(x));
    EVENT_PROP("y", INT_TO_JSVAL(y));

    jevent = OBJECT_TO_JSVAL(event);

    if (JS_GetProperty(m_Cx, m_JSObject, "_onmousewheel", &onwheel) &&
        !JSVAL_IS_PRIMITIVE(onwheel) &&
        JS_ObjectIsCallable(m_Cx, &onwheel.toObject())) {

        JS_CallFunctionValue(m_Cx, event, onwheel, 1, &jevent, &rval);
    }

    /*JS::RootedObject obj(cx, NativeJSEvents::CreateEventObject(m_Cx));

    this->fireJSEvent("wheel", OBJECT_TO_JSVAL(obj));*/

#undef EVENT_PROP
}

void NativeJSwindow::keyupdown(int keycode, int mod, int state, int repeat, int location)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE)

    JS::RootedObject event(m_Cx);
    JS::RootedValue jevent(m_Cx);
    JS::RootedValue onkeyupdown(m_Cx);
    JS::RootedValue rval(m_Cx);

    JSAutoRequest ar(m_Cx);

    event = JS_NewObject(m_Cx, &keyEvent_class, JS::NullPtr(), JS::NullPtr());

    EVENT_PROP("keyCode", INT_TO_JSVAL(keycode));
    EVENT_PROP("location", INT_TO_JSVAL(location));
    EVENT_PROP("altKey", BOOLEAN_TO_JSVAL(!!(mod & NATIVE_KEY_ALT)));
    EVENT_PROP("ctrlKey", BOOLEAN_TO_JSVAL(!!(mod & NATIVE_KEY_CTRL)));
    EVENT_PROP("shiftKey", BOOLEAN_TO_JSVAL(!!(mod & NATIVE_KEY_SHIFT)));
    EVENT_PROP("metaKey", BOOLEAN_TO_JSVAL(!!(mod & NATIVE_KEY_META)));
    EVENT_PROP("spaceKey", BOOLEAN_TO_JSVAL(keycode == 32));
    EVENT_PROP("repeat", BOOLEAN_TO_JSVAL(!!(repeat)));

    jevent = OBJECT_TO_JSVAL(event);

    if (JS_GetProperty(m_Cx, m_JSObject,
        (state ? "_onkeydown" : "_onkeyup"), &onkeyupdown) &&
        !JSVAL_IS_PRIMITIVE(onkeyupdown) &&
        JS_ObjectIsCallable(m_Cx, &onkeyupdown.toObject())) {

        JS_CallFunctionValue(m_Cx, event, onkeyupdown, 1, &jevent, &rval);
    }
#undef EVENT_PROP
}

void NativeJSwindow::textInput(const char *data)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, nullptr, nullptr, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    JS::RootedObject event(m_Cx);
    JS::RootedValue jevent(m_Cx);
    JS::RootedValue ontextinput(m_Cx);
    JS::RootedValue rval(m_Cx);

    JSAutoRequest ar(m_Cx);

    event = JS_NewObject(m_Cx, &textEvent_class, nullptr, nullptr);

    EVENT_PROP("val",
        STRING_TO_JSVAL(NativeJSUtils::newStringWithEncoding(m_Cx, data,
        strlen(data), "utf8")));

    jevent = OBJECT_TO_JSVAL(event);

    if (JS_GetProperty(m_Cx, m_JSObject, "_ontextinput", &ontextinput) &&
        !JSVAL_IS_PRIMITIVE(ontextinput) &&
        JS_ObjectIsCallable(m_Cx, &ontextinput.toObject())) {

        JS_CallFunctionValue(m_Cx, event, ontextinput, 1, &jevent, &rval);
    }
}

void NativeJSwindow::systemMenuClicked(const char *id)
{
    JSContext *cx = m_Cx;
    JS::RootedObject event(cx, JS_NewObject(m_Cx, nullptr, JS::NullPtr(), JS::NullPtr()));

    JSOBJ_SET_PROP_CSTR(event, "id", id);
    JS::RootedValue ev(cx, OBJECT_TO_JSVAL(event));

    JSOBJ_CALLFUNCNAME(m_JSObject, "_onsystemtrayclick", 1, &ev);
}

void NativeJSwindow::mouseClick(int x, int y, int state, int button, int clicks)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, nullptr, nullptr, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    JS::RootedObject event(m_Cx);
    JS::RootedValue rval(m_Cx);
    JS::RootedValue jevent(m_Cx);
    JS::RootedValue onclick(m_Cx);

    JSAutoRequest ar(m_Cx);

    event = JS_NewObject(m_Cx, &mouseEvent_class, JS::NullPtr(), JS::NullPtr());

    NativeContext *nctx = NativeContext::getNativeClass(this->m_Cx);
    NativeInputEvent *ev = new NativeInputEvent(state ?
        NativeInputEvent::kMouseClick_Type :
        NativeInputEvent::kMouseClickRelease_Type, x, y);

    ev->data[0] = button;

    nctx->addInputEvent(ev);

    /*
        Handle double click.
        clicks receiv the number of successive clicks.
        Only trigger for even number on release.
    */
    if (clicks % 2 == 0 && !state) {
        NativeInputEvent *ev = new NativeInputEvent(NativeInputEvent::kMouseDoubleClick_Type, x, y);

        ev->data[0] = button;
        nctx->addInputEvent(ev);
    }

    EVENT_PROP("x", INT_TO_JSVAL(x));
    EVENT_PROP("y", INT_TO_JSVAL(y));
    EVENT_PROP("clientX", INT_TO_JSVAL(x));
    EVENT_PROP("clientY", INT_TO_JSVAL(y));
    EVENT_PROP("which", INT_TO_JSVAL(button));

    jevent = OBJECT_TO_JSVAL(event);
    if (JS_GetProperty(m_Cx, m_JSObject,
        (state ? "_onmousedown" : "_onmouseup"), &onclick) &&
        !onclick.isPrimitive() &&
        JS_ObjectIsCallable(m_Cx, &onclick.toObject())) {

        JS_CallFunctionValue(m_Cx, event, onclick, 1, &jevent, &rval);
    }
}

bool NativeJSwindow::dragEvent(const char *name, int x, int y)
{
    JS::RootedValue rval(m_Cx);
    JS::RootedValue jevent(m_Cx);
    JS::RootedValue ondragevent(m_Cx);
    JS::RootedObject event(m_Cx);

    JSAutoRequest ar(m_Cx);

    event = JS_NewObject(m_Cx, &dragEvent_class, JS::NullPtr(), JS::NullPtr());

    EVENT_PROP("x", INT_TO_JSVAL(x));
    EVENT_PROP("y", INT_TO_JSVAL(y));
    EVENT_PROP("clientX", INT_TO_JSVAL(x));
    EVENT_PROP("clientY", INT_TO_JSVAL(y));

    if (m_DragedFiles) {
        EVENT_PROP("files", OBJECT_TO_JSVAL(this->m_DragedFiles));
    }

    jevent = OBJECT_TO_JSVAL(event);

    if (JS_GetProperty(m_Cx, m_JSObject, name, &ondragevent) &&
        !ondragevent.isPrimitive() &&
        JS_ObjectIsCallable(m_Cx, &ondragevent.toObject())) {

        if (!JS_CallFunctionValue(m_Cx, event, ondragevent, 1, &jevent, &rval)) {
            printf("Failed to exec func\n");
            return false;
        }

        return rval.toBoolean();
    }

    return false;
}

bool NativeJSwindow::dragBegin(int x, int y, const char * const *files, size_t nfiles)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, nullptr, nullptr, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    if (m_Dragging) {
        return false;
    }

    m_Dragging = true;

    m_DragedFiles = JS_NewArrayObject(m_Cx, (int)nfiles);

    for (int i = 0; i < nfiles; i++) {
        JS::RootedValue val(m_Cx, OBJECT_TO_JSVAL(NativeJSFileIO::generateJSObject(m_Cx, files[i])));
        JS_SetElement(m_Cx, m_DragedFiles, i, val);
    }

    NativeJS::getNativeClass(m_Cx)->rootObjectUntilShutdown(m_DragedFiles);

    return this->dragEvent("_onFileDragEnter", x, y);

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

    NativeJS::getNativeClass(m_Cx)->unrootObject(m_DragedFiles);

    m_DragedFiles = NULL;
    m_Dragging = false;
}

void NativeJSwindow::resized(int width, int height)
{
    NativeContext::getNativeClass(m_Cx)->sizeChanged(width, height);
}

void NativeJSwindow::mouseMove(int x, int y, int xrel, int yrel)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, nullptr, nullptr, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    JS::RootedValue rval(m_Cx);
    JS::RootedValue jevent(m_Cx);
    JS::RootedValue onmove(m_Cx);;
    JS::RootedObject event(m_Cx);

    NativeContext *nctx = NativeContext::getNativeClass(this->m_Cx);

    NativeCanvasHandler *rootHandler = nctx->getRootHandler();

    rootHandler->mousePosition.x = x;
    rootHandler->mousePosition.y = y;
    rootHandler->mousePosition.xrel += xrel;
    rootHandler->mousePosition.yrel += yrel;
    rootHandler->mousePosition.consumed = false;

    NativeInputEvent *ev = new NativeInputEvent(NativeInputEvent::kMouseMove_Type, x, y);
    ev->data[0] = xrel;
    ev->data[1] = yrel;

    nctx->addInputEvent(ev);

    event = JS_NewObject(m_Cx, &mouseEvent_class, JS::NullPtr(), JS::NullPtr());

    EVENT_PROP("x", INT_TO_JSVAL(x));
    EVENT_PROP("y", INT_TO_JSVAL(y));
    EVENT_PROP("xrel", INT_TO_JSVAL(xrel));
    EVENT_PROP("yrel", INT_TO_JSVAL(yrel));
    EVENT_PROP("clientX", INT_TO_JSVAL(x));
    EVENT_PROP("clientY", INT_TO_JSVAL(y));

    jevent = OBJECT_TO_JSVAL(event);

    if (JS_GetProperty(m_Cx, m_JSObject, "_onmousemove", &onmove) &&
        !onmove.isPrimitive() &&
        JS_ObjectIsCallable(m_Cx, &onmove.toObject())) {

        JS_CallFunctionValue(m_Cx, event, onmove, 1, &jevent, &rval);
    }
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
    JS::HandleId id, JS::MutableHandleValue vp)
{
    NativeUIInterface *NUI = NativeContext::getNativeClass(m_Cx)->getUI();

    switch(JSID_TO_INT(id)) {
        case WINDOW_PROP_DEVICE_PIXELRATIO:
            /* TODO: Actual value */
            vp.setInt32(1);
            break;
        case WINDOW_PROP_LEFT:
        {
            int x;
            NUI->getWindowPosition(&x, NULL);
            vp.set(INT_TO_JSVAL(x));
            break;
        }
        case WINDOW_PROP_TOP:
        {
            int y;
            NUI->getWindowPosition(NULL, &y);
            vp.set(INT_TO_JSVAL(y));
            break;
        }
        case WINDOW_PROP_WIDTH:
            vp.set(INT_TO_JSVAL(NUI->getWidth()));
            break;
        case WINDOW_PROP_HEIGHT:
            vp.set(INT_TO_JSVAL(NUI->getHeight()));
            break;
        case WINDOW_PROP_TITLE:
        {
            const char *title =  NUI->getWindowTitle();
            JS::RootedString str(m_Cx, NativeJSUtils::newStringWithEncoding(m_Cx, title,
                strlen(title), "utf8"));
            vp.set(STRING_TO_JSVAL(str));
        }
        break;
    }

    return true;
}

static bool native_window_prop_set(JSContext *cx, JS::HandleObject obj,
    JS::HandleId id, bool strict, JS::MutableHandleValue vp)
{
    NativeUIInterface *NUI = NativeContext::getNativeClass(cx)->getUI();
    switch(JSID_TO_INT(id)) {
        case WINDOW_PROP_LEFT:
        {
            double dval;
            if (!vp.isNumber()) {

                return true;
            }

            dval = vp.toDouble();

            NUI->setWindowPosition((int)dval, NATIVE_WINDOWPOS_UNDEFINED_MASK);

            break;
        }

        case WINDOW_PROP_TOP:
        {
            double dval;
            if (!vp.isNumber()) {

                return true;
            }
            dval = vp.toDouble();

            NUI->setWindowPosition(NATIVE_WINDOWPOS_UNDEFINED_MASK, (int)dval);

            break;
        }
        case WINDOW_PROP_WIDTH:
        {
            double dval;
            if (!vp.isNumber()) {

                return true;
            }

            dval = vp.toDouble();
            NativeContext::getNativeClass(cx)->setWindowSize((int)dval, NUI->getHeight());

            break;
        }
        case WINDOW_PROP_HEIGHT:
        {
            double dval;
            if (!vp.isNumber()) {

                return true;
            }

            dval = vp.toDouble();
            NativeContext::getNativeClass(cx)->setWindowSize((int)NUI->getWidth(), (int)dval);

            break;
        }
        case WINDOW_PROP_TITLE:
        {
            if (!vp.isString()) {
                return true;
            }
            JSAutoByteString title(cx, vp.toString());
            NUI->setWindowTitle(title.ptr());
            break;
        }
        case WINDOW_PROP_CURSOR:
        {
            if (!vp.isString()) {

                return true;
            }
            JSAutoByteString type(cx, vp.toString());

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
            JSAutoByteString color(cx, vp.toString());
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
            dval = vp.toDouble();
            JS::RootedValue offsety(cx);

            if (JS_GetProperty(cx, obj, "titleBarControlsOffsetY", &offsety) == false) {
                offsety = DOUBLE_TO_JSVAL(0);
            }
            oval = offsety.toDouble();

            NUI->setWindowControlsOffset(dval, oval);
            break;
        }
        case WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETY:
        {
            double dval, oval;
            if (!vp.isNumber()) {

                return true;
            }
            dval = vp.toDouble();
            JS::RootedValue offsetx(cx);
            if (JS_GetProperty(cx, obj, "titleBarControlsOffsetX", &offsetx) == false) {
                offsetx = DOUBLE_TO_JSVAL(0);
            }
            oval = offsetx.toDouble();

            NUI->setWindowControlsOffset(oval, dval);
            break;
        }
        default:
            break;
    }
    return true;
}

static bool native_navigator_prop_get(JSContext *m_Cx, JS::HandleObject obj,
    JS::HandleId id, JS::MutableHandleValue vp)
{
    NativeUIInterface *NUI = NativeContext::getNativeClass(m_Cx)->getUI();

#define APP_NAME "nidium"
#define APP_LANGUAGE "en"
#define APP_LOCALE APP_LANGUAGE "-US"
    switch(JSID_TO_INT(id)) {
       case NAVIGATOR_PROP_LANGUAGE:
            {
            JS::RootedString jStr(m_Cx, JS_NewStringCopyZ(m_Cx, APP_LANGUAGE ));
            vp.set(STRING_TO_JSVAL(jStr));
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
            JS::RootedString jStr(m_Cx, JS_NewStringCopyZ(m_Cx, NATIVE_VERSION_STR ));
            vp.set(STRING_TO_JSVAL(jStr));
            }
            break;
        case NAVIGATOR_PROP_APPNAME:
            {
            JS::RootedString jStr(m_Cx, JS_NewStringCopyZ(m_Cx, APP_NAME ));
            vp.set(STRING_TO_JSVAL(jStr));
            }
            break;
        case NAVIGATOR_PROP_USERAGENT:
            {
            JS::RootedString jStr(m_Cx, JS_NewStringCopyZ(m_Cx, APP_NAME "/" NATIVE_VERSION_STR "(" APP_LOCALE "; rv:" NATIVE_BUILD ") " NATIVE_FRAMEWORK_STR ));
            vp.set(STRING_TO_JSVAL(jStr));
            }
            break;
        case NAVIGATOR_PROP_PLATFORM:
            {
            const char *platform;
            // http://stackoverflow.com/questions/19877924/what-is-the-list-of-possible-values-for-navigator-platform-as-of-today
#if defined( _WIN32 ) || defined( _WIN64 )
            platform = "Win32";
#elif defined( __APPLE ) || defined( _WIN64 )
            platform = "MacOSX";
#elif defined( __FreeBSD )
            platform = "FreeBSD";
#elif defined( __DragonFly )
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
            vp.set(STRING_TO_JSVAL(jStr));
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
    JS::Value cb;//@TODO :JS::Heap<JS::Value> cb;
};

static void native_window_openfilecb(void *_nof, const char *lst[], uint32_t len)
{
    struct _nativeopenfile *nof = (struct _nativeopenfile *)_nof;
    JS::RootedValue rval(nof->cx);
    JS::RootedValue cb(nof->cx, nof->cb);
    JS::RootedObject arr(nof->cx, JS_NewArrayObject(nof->cx, len));

    for (int i = 0; i < len; i++) {
        JS::RootedValue val(nof->cx, OBJECT_TO_JSVAL(NativeJSFileIO::generateJSObject(nof->cx, lst[i])));
        JS_SetElement(nof->cx, arr, i, val);
    }

    JS::RootedValue jarr(nof->cx, OBJECT_TO_JSVAL(arr));
    JS::RootedValue global(nof->cx, JS_GetGlobalObject(nof->cx));
    JS_CallFunctionValue(nof->cx, global, cb, 1, &jarr, &rval);

    JS::RemoveValueRoot(nof->cx, &nof->cb);
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

    if (!JS_ConvertArguments(cx, args, "S", &url)) {
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

    if (!JS_ConvertArguments(cx, args, "S", &url)) {
        return false;
    }

    JSAutoByteString curl(cx, url);

    const char *ret = NativeSystemInterface::getInstance()->execute(curl.ptr());
    args.rval().setString(JS_NewStringCopyZ(cx, ret));

    return true;
}

static bool native_window_openDirDialog(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedValue callback(cx);

    if (!JS_ConvertArguments(cx, args, "v", &callback)) {
        return false;
    }

    struct _nativeopenfile *nof = (struct _nativeopenfile *)malloc(sizeof(*nof));
    nof->cb = callback;
    nof->cx = cx;

    JS::AddValueRoot(cx, &nof->cb);

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

    if (!JS_ConvertArguments(cx, args, "ov", &types, &callback)) {
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

            if (JSVAL_IS_STRING(val)) {
                JS::RootedString str(cx, val.toString());
                ctypes[j] = JS_EncodeString(cx, str);
                j++;
            }
        }
        ctypes[j] = NULL;
    }

    struct _nativeopenfile *nof = (struct _nativeopenfile *)malloc(sizeof(*nof));
    nof->cb = callback;
    nof->cx = cx;

    JS::AddValueRoot(cx, &nof->cb);

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

    if (!JS_ConvertValue(cx, args.array()[0], JSTYPE_FUNCTION, &cb)) {
        return true;
    }
    NativeJSwindow::getNativeClass(cx)->addFrameCallback(cb);

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
    JS::RootedString title(cx);
    JS::RootedString body(cx);
    bool sound = false;

    if (!JS_ConvertArguments(cx, args, "SS/b", &title, &body, &sound)) {
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

    JS::RootedValue jobj(cx, args[0]);

    if (jobj.isNull() || !jobj.isObject()) {

        NUI->disableSysTray();

        return true;
    }

    JS_INITOPT();

    NativeSystemMenu &menu = NUI->getSystemMenu();
    menu.deleteItems();

    JSGET_OPT_TYPE(jobj.toObjectOrNull(), "icon", Object) {
        JS::RootedObject jsimg(cx, __curopt.toObjectOrNull());
        NativeSkImage *skimage;
        if (NativeJSImage::JSObjectIs(cx, jsimg) &&
            (skimage = NativeJSImage::JSObjectToNativeSkImage(jsimg))) {

            const uint8_t *pixels = skimage->getPixels(NULL);
            menu.setIcon(pixels, skimage->getWidth(), skimage->getHeight());
        }
    }

    JSGET_OPT_TYPE(jobj.toObjectOrNull(), "menu", Object) {
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
                    NativeSystemMenuItem *menuItem = new NativeSystemMenuItem();
                    JSGET_OPT_TYPE(val.toObjectOrNull(), "title", String) {

                        JSAutoByteString ctitle;
                        ctitle.encodeUtf8(cx, __curopt.toString());
                        menuItem->title(ctitle.ptr());
                    } else {
                        menuItem->title("");
                    }
                    JSGET_OPT_TYPE(val.toObjectOrNull(), "id", String) {
                        JSAutoByteString cid;
                        cid.encodeUtf8(cx, __curopt.toString());
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

    int x = 0, y = 0;

    if (args[0].isString()) {
        JS::RootedString xstr(cx, args[0].toString());
        JSAutoByteString cxstr(cx, xstr);

        if (strcmp(cxstr.ptr(), "center") == 0) {
            x = NATIVE_WINDOWPOS_CENTER_MASK;
        }

    } else if (args[0].isNumber()) {
        x = args[0].toInt32();
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
        y = args[1].toInt32();
    } else {
        JS_ReportError(cx, "setFrame() invalid position");
        return false;
    }

    NativeContext::getNativeClass(cx)->setWindowFrame(x, y,
        args[2].toInt32(), args[3].toInt32());

    return true;
}

void NativeJSwindow::addFrameCallback(JS::Value &cb)
{
    struct _requestedFrame *frame = new struct _requestedFrame;
    frame->next = this->m_RequestedFrame;
    frame->cb = cb;

    m_RequestedFrame = frame;

    JS::AddValueRoot(m_Cx, &frame->cb);
}

void NativeJSwindow::callFrameCallbacks(double ts, bool garbage)
{
    JS::RootedValue rval(m_Cx);
    JS::Value arg[1];
    struct _requestedFrame *frame = m_RequestedFrame;

    /* Set to NULL so that callbacks can "fork" the new chain */
    m_RequestedFrame = NULL;

    arg[0] = JS_NumberValue(ts/1000000L);

    while (frame != NULL) {
        if (!garbage) {
            JS_CallFunctionValue(m_Cx, JS_GetGlobalObject(m_Cx),
                frame->cb, 1, arg, &rval);
        }

        JS::RemoveValueRoot(m_Cx, &frame->cb);

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

void NativeJSwindow::createMainCanvas(int width, int height, JSObject *doc)
{
    JS::RootedObject canvas(m_Cx);

    canvas = NativeJSCanvas::generateJSObject(m_Cx, width,
        height, &m_handler);

    NativeContext::getNativeClass(m_Cx)->getRootHandler()->addChild(m_handler);

    JS_DefineProperty(m_Cx, doc, "canvas",
        OBJECT_TO_JSVAL(canvas), nullptr, nullptr, JSPROP_READONLY | JSPROP_PERMANENT);
}

void NativeJSwindow::createStorage()
{
    JS::RootedObject obj(m_Cx, m_JSObject);
    JS::RootedObject storage(m_Cx, JS_NewObject(m_Cx, &storage_class, JS::NullPtr(), JS::NullPtr()));

    JS_DefineFunctions(m_Cx, storage, storage_funcs);

    JS::RootedValue jsstorage(m_Cx, OBJECT_TO_JSVAL(storage));

    JS_SetProperty(m_Cx, obj, "storage", &jsstorage);
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

    NATIVE_CHECK_ARGS("get", 1);
    if (!args[0].isString()) {
        JS_ReportError(cx, "get() : key must be a string");
        return false;
    }

    JSAutoByteString key(cx, args[0].toString());
    NativeDB *db = NativeJSwindow::getNativeClass(cx)->getDataBase();

    std::string data;

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
        JS_STRUCTURED_CLONE_VERSION, &ret, nullptr, nullptr)) {

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
    int height, JSObject *doc)
{
    JS::RootedObject globalObj(cx, JS_GetGlobalObject(cx));
#if 0
    JS::RootedObject windowObj(cx, JS_DefineObject(cx, globalObj,
        NativeJSwindow::getJSObjectName(), &window_class, nullptr,
        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY));
#else
    JS::RootedObject windowObj(cx, globalObj);
#endif
    NativeJSwindow *jwin = new NativeJSwindow(globalObj, cx);

    //JS_SetPrivate(windowObj, jwin);

    jwin->initDataBase();

    jwin->createMainCanvas(width, height, doc);

    /*NativeJS::getNativeClass(cx)->jsobjects.set(
        NativeJSwindow::getJSObjectName(), windowObj);*/

    JS_DefineFunctions(cx, globalObj, window_funcs);
    JS_DefineProperties(cx, globalObj, window_props);

    JS::RootedValue val(cx);

    val = DOUBLE_TO_JSVAL(0);
    JS_SetProperty(cx, windowObj, "titleBarControlsOffsetX", &val);

    val = DOUBLE_TO_JSVAL(0);
    JS_SetProperty(cx, windowObj, "titleBarControlsOffsetY", &val);

    // Set the __nidium__ properties
    JS::RootedObject nidiumObj(cx, JS_NewObject(cx,  nullptr, JS::NullPtr(), JS::NullPtr()));

    JS::RootedString jVersion(cx, JS_NewStringCopyZ(cx, NATIVE_VERSION_STR));
    JS_DefineProperty(cx, nidiumObj, "version", STRING_TO_JSVAL(jVersion), nullptr,
        nullptr, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS::RootedString jFramework(cx, JS_NewStringCopyZ(cx, NATIVE_FRAMEWORK_STR));
    JS_DefineProperty(cx, nidiumObj, "build", STRING_TO_JSVAL(jFramework), nullptr,
        nullptr, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS::RootedString jRevision(cx, JS_NewStringCopyZ(cx, NATIVE_BUILD ));
    JS_DefineProperty(cx, nidiumObj, "revision", STRING_TO_JSVAL(jRevision), nullptr,
        nullptr, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    val = OBJECT_TO_JSVAL( nidiumObj);
    JS_SetProperty(cx, windowObj, "__nidium__", &val);

    //  Set the navigator properties
    JS::RootedObject navigatorObj(cx, JS_NewObject(cx, &navigator_class, nullptr, nullptr));
    JS_DefineProperties(cx, navigatorObj, navigator_props);

    val = OBJECT_TO_JSVAL( navigatorObj);
    JS_SetProperty(cx, windowObj, "navigator", &val);

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

