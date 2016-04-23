#include "Binding/JSCanvas.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>

#include "Graphics/Canvas3DContext.h"
#include "Graphics/CanvasHandler.h"
#include "Binding/JSCanvas2DContext.h"

namespace Nidium {
namespace Binding {

extern JSClass Canvas2DContext_class;

#define NATIVE_PROLOGUE(ofclass) \
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp); \
    JS::RootedObject thisobj(cx, JS_THIS_OBJECT(cx, vp)); \
    if (!thisobj.get()) return false; \
    if (JS_GetClass(thisobj) != NativeLocalClass) { \
        JS_ReportError(cx, "Illegal invocation");\
        args.rval().setUndefined(); \
        return false; \
    } \
    ofclass *NativeObject = (static_cast<class NativeJSCanvas *>(JS_GetPrivate(thisobj)))->getHandler();


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

enum {
    CANVAS_PROP_WIDTH = 1,
    CANVAS_PROP_HEIGHT,
    CANVAS_PROP_POSITION,
    /* relative positions */
    CANVAS_PROP_TOP,
    CANVAS_PROP_LEFT,
    CANVAS_PROP_RIGHT,
    CANVAS_PROP_BOTTOM,
    /* .show()/.hide() */
    CANVAS_PROP_VISIBLE,
    /* Element is going to be drawn */
    CANVAS_PROP___VISIBLE,
    /* Absolute positions */
    CANVAS_PROP___TOP,
    CANVAS_PROP___LEFT,
    /* conveniance getter for getContext("2D") */
    CANVAS_PROP_CTX,

    CANVAS_PROP_COATING,
    CANVAS_PROP_CLIENTLEFT,
    CANVAS_PROP_CLIENTTOP,
    CANVAS_PROP_CLIENTWIDTH,
    CANVAS_PROP_CLIENTHEIGHT,
    CANVAS_PROP_OPACITY,
    CANVAS_PROP_OVERFLOW,
    CANVAS_PROP_CONTENTWIDTH,
    CANVAS_PROP_INNERWIDTH,
    CANVAS_PROP_CONTENTHEIGHT,
    CANVAS_PROP_INNERHEIGHT,
    CANVAS_PROP_SCROLLTOP,
    CANVAS_PROP_SCROLLLEFT,
    CANVAS_PROP___FIXED,
    CANVAS_PROP___OUTOFBOUND,
    CANVAS_PROP_ALLOWNEGATIVESCROLL,
    CANVAS_PROP_STATICLEFT,
    CANVAS_PROP_STATICRIGHT,
    CANVAS_PROP_STATICTOP,
    CANVAS_PROP_STATICBOTTOM,
    CANVAS_PROP_MINWIDTH,
    CANVAS_PROP_MINHEIGHT,
    CANVAS_PROP_MAXWIDTH,
    CANVAS_PROP_MAXHEIGHT,
    CANVAS_PROP_FLUIDHEIGHT,
    CANVAS_PROP_FLUIDWIDTH,
    CANVAS_PROP_ID,
    CANVAS_PROP_MARGINLEFT,
    CANVAS_PROP_MARGINRIGHT,
    CANVAS_PROP_MARGINTOP,
    CANVAS_PROP_MARGINBOTTOM,
    CANVAS_PROP_CURSOR
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
JSClass *JSExposer<NativeJSCanvas>::jsclass = &Canvas_class;


static JSClass Canvas_Inherit_class = {
    "CanvasInherit", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, CanvasInherit_get, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass *NativeLocalClass = &Canvas_class;

static bool native_canvas_prop_set(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp);
static bool native_canvas_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp);

static bool native_canvas_getContext(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_setContext(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_addSubCanvas(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_insertBefore(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_insertAfter(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_removeFromParent(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_bringToFront(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_sendToBack(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_getParent(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_getFirstChild(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_getLastChild(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_getNextSibling(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_getPrevSibling(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_getChildren(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_getVisibleRect(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_setCoordinates(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_translate(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas_show(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas_hide(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas_setSize(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas_clear(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas_setZoom(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas_setScale(JSContext *cx, unsigned argc, JS::Value *vp);

#define NATIVE_JS_PROP JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS

static JSPropertySpec canvas_props[] = {
    NIDIUM_JS_PSGS("opacity", CANVAS_PROP_OPACITY, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("overflow", CANVAS_PROP_OVERFLOW, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("scrollLeft", CANVAS_PROP_SCROLLLEFT, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("scrollTop", CANVAS_PROP_SCROLLTOP, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("allowNegativeScroll", CANVAS_PROP_ALLOWNEGATIVESCROLL, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("width", CANVAS_PROP_WIDTH, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("coating", CANVAS_PROP_COATING, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("height", CANVAS_PROP_HEIGHT, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("maxwidth", CANVAS_PROP_MAXWIDTH, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("maxHeight", CANVAS_PROP_MAXHEIGHT, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("minWidth", CANVAS_PROP_MINWIDTH, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("minHeight", CANVAS_PROP_MINHEIGHT, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("position", CANVAS_PROP_POSITION, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("top", CANVAS_PROP_TOP, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("left", CANVAS_PROP_LEFT, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("right", CANVAS_PROP_RIGHT, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("bottom", CANVAS_PROP_BOTTOM, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("visible", CANVAS_PROP_VISIBLE, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("staticLeft", CANVAS_PROP_STATICLEFT, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("staticRight", CANVAS_PROP_STATICRIGHT, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("staticTop", CANVAS_PROP_STATICTOP, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("staticBottom", CANVAS_PROP_STATICBOTTOM, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("fluidHeight", CANVAS_PROP_FLUIDHEIGHT, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("fluidWidth", CANVAS_PROP_FLUIDWIDTH, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("id", CANVAS_PROP_ID, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("marginLeft", CANVAS_PROP_MARGINLEFT, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("marginRight", CANVAS_PROP_MARGINRIGHT, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("marginTop", CANVAS_PROP_MARGINTOP, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("marginBottom", CANVAS_PROP_MARGINBOTTOM, native_canvas_prop_get, native_canvas_prop_set),
    NIDIUM_JS_PSGS("cursor", CANVAS_PROP_CURSOR, native_canvas_prop_get, native_canvas_prop_set),


    NIDIUM_JS_PSG("clientWidth", CANVAS_PROP_CLIENTWIDTH, native_canvas_prop_get),
    NIDIUM_JS_PSG("clientHeight", CANVAS_PROP_CLIENTHEIGHT, native_canvas_prop_get),
    NIDIUM_JS_PSG("clientTop", CANVAS_PROP_CLIENTTOP, native_canvas_prop_get),
    NIDIUM_JS_PSG("clientLeft", CANVAS_PROP_CLIENTLEFT, native_canvas_prop_get),
    NIDIUM_JS_PSG("contentWidth", CANVAS_PROP_CONTENTWIDTH, native_canvas_prop_get),
    NIDIUM_JS_PSG("contentHeight", CANVAS_PROP_CONTENTHEIGHT, native_canvas_prop_get),
    NIDIUM_JS_PSG("innerWidth", CANVAS_PROP_INNERWIDTH, native_canvas_prop_get),
    NIDIUM_JS_PSG("innerHeight", CANVAS_PROP_INNERHEIGHT, native_canvas_prop_get),
    NIDIUM_JS_PSG("__visible", CANVAS_PROP___VISIBLE, native_canvas_prop_get),
    NIDIUM_JS_PSG("__top", CANVAS_PROP___TOP, native_canvas_prop_get),
    NIDIUM_JS_PSG("__left", CANVAS_PROP___LEFT, native_canvas_prop_get),
    NIDIUM_JS_PSG("__fixed", CANVAS_PROP___FIXED, native_canvas_prop_get),
    NIDIUM_JS_PSG("__outofbound", CANVAS_PROP___OUTOFBOUND, native_canvas_prop_get),
    NIDIUM_JS_PSG("ctx", CANVAS_PROP_CTX, native_canvas_prop_get),
    JS_PS_END
};

static JSFunctionSpec canvas_funcs[] = {
    JS_FN("getContext", native_canvas_getContext, 1, NIDIUM_JS_FNPROPS),
    JS_FN("setContext", native_canvas_setContext, 1, NIDIUM_JS_FNPROPS),
    JS_FN("add", native_canvas_addSubCanvas, 1, NIDIUM_JS_FNPROPS),
    JS_FN("insertBefore", native_canvas_insertBefore, 2, NIDIUM_JS_FNPROPS),
    JS_FN("insertAfter", native_canvas_insertAfter, 2, NIDIUM_JS_FNPROPS),
    JS_FN("removeFromParent", native_canvas_removeFromParent, 0, NIDIUM_JS_FNPROPS),
    JS_FN("show", native_canvas_show, 0, NIDIUM_JS_FNPROPS),
    JS_FN("hide", native_canvas_hide, 0, NIDIUM_JS_FNPROPS),
    JS_FN("bringToFront", native_canvas_bringToFront, 0, NIDIUM_JS_FNPROPS),
    JS_FN("sendToBack", native_canvas_sendToBack, 0, NIDIUM_JS_FNPROPS),
    JS_FN("getParent", native_canvas_getParent, 0, NIDIUM_JS_FNPROPS),
    JS_FN("getFirstChild", native_canvas_getFirstChild, 0, NIDIUM_JS_FNPROPS),
    JS_FN("getLastChild", native_canvas_getLastChild, 0, NIDIUM_JS_FNPROPS),
    JS_FN("getNextSibling", native_canvas_getNextSibling, 0, NIDIUM_JS_FNPROPS),
    JS_FN("getPrevSibling", native_canvas_getPrevSibling, 0, NIDIUM_JS_FNPROPS),
    JS_FN("getChildren", native_canvas_getChildren, 0, NIDIUM_JS_FNPROPS),
    JS_FN("setCoordinates", native_canvas_setCoordinates, 2, NIDIUM_JS_FNPROPS),
    JS_FN("translate", native_canvas_translate, 2, NIDIUM_JS_FNPROPS),
    JS_FN("getVisibleRect", native_canvas_getVisibleRect, 0, NIDIUM_JS_FNPROPS),
    JS_FN("setSize", native_canvas_setSize, 2, NIDIUM_JS_FNPROPS),
    JS_FN("clear", native_canvas_clear, 0, NIDIUM_JS_FNPROPS),
    JS_FN("setZoom", native_canvas_setZoom, 1, NIDIUM_JS_FNPROPS),
    JS_FN("setScale", native_canvas_setScale, 2, NIDIUM_JS_FNPROPS),
    JS_FS_END
};

static bool CanvasInherit_get(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
{
    NativeJSCanvas *jscanvas = (class NativeJSCanvas *)JS_GetPrivate(obj);
    if (!jscanvas) {
        return true;
    }

    Nidium::Graphics::NativeCanvasHandler *handler = jscanvas->getHandler(), *parent;

    if (vp.isNull()) {
        if ((parent = handler->getParent()) == NULL || !parent->m_JsObj) {
            return true;
        }

        NativeJSCanvas *jscanvas_parent =
            (class NativeJSCanvas *)JS_GetPrivate(parent->m_JsObj);

        JS::RootedValue ret(cx);
        JS::RootedObject parentObj(cx, jscanvas_parent->getInherit());
        JS_GetPropertyById(cx, parentObj, id, &ret);
        vp.set(ret);
    }

    return true;
}

Nidium::Graphics::NativeCanvasHandler *HANDLER_GETTER(JSObject *obj)
{
    NativeJSCanvas *jscanvas = (class NativeJSCanvas *)JS_GetPrivate(obj);
    if (!jscanvas) {
        return NULL;
    }

    return jscanvas->getHandler();
}

static Nidium::Graphics::NativeCanvasHandler *HANDLER_GETTER_SAFE(JSContext *cx, JS::HandleObject obj)
{
    NativeJSCanvas *jscanvas = (class NativeJSCanvas *)JS_GetPrivate(obj);

    const JSClass *cl = JS_GetClass(obj);
    if (!jscanvas || cl != &Canvas_class) {
        printf("Missmatch class %s\n", cl->name); //TODO error reporting
        return NULL;
    }

    return jscanvas->getHandler();
}

static bool native_canvas_show(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    NativeObject->setHidden(false);

    return true;
}

static bool native_canvas_hide(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    NativeObject->setHidden(true);

    return true;
}

static bool native_canvas_clear(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    if (NativeObject->m_Context) {
        NativeObject->m_Context->clear(0x00000000);
    }

    return true;
}

static bool native_canvas_setZoom(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    double zoom;

    if (!JS_ConvertArguments(cx, args, "d", &zoom)) {
        return false;
    }

    NativeObject->setZoom(zoom);

    return true;
}

static bool native_canvas_setScale(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    double x, y;

    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        return false;
    }

    NativeObject->setScale(x, y);

    return true;
}

static bool native_canvas_setSize(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    int width, height;

    if (!JS_ConvertArguments(cx, args, "ii", &width, &height)) {
        return false;
    }

    NativeObject->setSize(width, height);

    return true;
}

static bool native_canvas_removeFromParent(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    NativeObject->removeFromParent();

    return true;
}

static bool native_canvas_bringToFront(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    NativeObject->bringToFront();

    return true;
}

static bool native_canvas_sendToBack(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    NativeObject->sendToBack();

    return true;
}

static bool native_canvas_getParent(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    Nidium::Graphics::NativeCanvasHandler *parent = NativeObject->getParent();
    if (parent) {
        args.rval().setObjectOrNull(parent->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

static bool native_canvas_getFirstChild(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    Nidium::Graphics::NativeCanvasHandler *val = NativeObject->getFirstChild();
    if (val) {
        args.rval().setObjectOrNull(val->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

static bool native_canvas_getLastChild(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    Nidium::Graphics::NativeCanvasHandler *val = NativeObject->getLastChild();
    if (val) {
        args.rval().setObjectOrNull(val->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

static bool native_canvas_getNextSibling(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    Nidium::Graphics::NativeCanvasHandler *val = NativeObject->getNextSibling();
    if (val) {
        args.rval().setObjectOrNull(val->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

static bool native_canvas_getPrevSibling(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    Nidium::Graphics::NativeCanvasHandler *val = NativeObject->getPrevSibling();
    if (val) {
        args.rval().setObjectOrNull(val->m_JsObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

static bool native_canvas_getChildren(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    uint32_t i;
    int32_t count = NativeObject->countChildren();

    if (!count) {
        JS::RootedObject retObj(cx, JS_NewArrayObject(cx, 0));
        args.rval().setObjectOrNull(retObj);
        return true;
    }

    Nidium::Graphics::NativeCanvasHandler *list[count];

    NativeObject->getChildren(list);
    JS::RootedObject jlist(cx, JS_NewArrayObject(cx, count));
    for (i = 0; i < count; i++) {
        JS::RootedValue objVal(cx, OBJECT_TO_JSVAL(list[i]->m_JsObj));
        JS_SetElement(cx, jlist, i, objVal);
    }
    JS::RootedValue val(cx, OBJECT_TO_JSVAL(jlist));

    args.rval().set(val);

    return true;
}

static bool native_canvas_getVisibleRect(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    Nidium::Graphics::NativeRect rect = NativeObject->getVisibleRect();
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

static bool native_canvas_setCoordinates(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    double left, top;

    if (!JS_ConvertArguments(cx, args, "dd", &left, &top)) {
        return false;
    }

    NativeObject->m_Left = left;
    NativeObject->m_Top = top;

    return true;
}

static bool native_canvas_translate(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    double left, top;

    if (!JS_ConvertArguments(cx, args, "dd", &left, &top)) {
        return false;
    }

    NativeObject->translate(left, top);

    return true;
}

static bool native_canvas_addSubCanvas(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    Nidium::Graphics::NativeCanvasHandler *handler = NULL;

    JS::RootedObject sub(cx);
    if (!JS_ConvertArguments(cx, args, "o", sub.address())) {
        return false;
    }

    if (sub && JS_GetClass(sub) != &Canvas_class) {
        JS_ReportError(cx, "add() First parameter is not a Canvas Object");
        return false;
    }

    handler = (static_cast<NativeJSCanvas *>(JS_GetPrivate(sub)))->getHandler();

    if (handler == NULL) {
        return true;
    }

    if (NativeObject == handler) {
        JS_ReportError(cx, "Canvas: can't add to itself");
        return false;
    }

    NativeObject->addChild(handler);

    return true;
}

static bool native_canvas_insertBefore(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    Nidium::Graphics::NativeCanvasHandler *handler_insert = NULL, *handler_ref = NULL;

    JS::RootedObject ref(cx);
    JS::RootedObject insert(cx);
    if (!JS_ConvertArguments(cx, args, "oo", insert.address(), ref.address())) {
        return false;
    }

    if (!insert || JS_GetClass(insert) != &Canvas_class) {
        JS_ReportError(cx, "add() First parameter is not a Canvas Object");
        return false;
    }

    handler_insert = (static_cast<NativeJSCanvas *>(JS_GetPrivate(insert)))->getHandler();
    if (handler_insert == NULL) {
        return true;
    }

    if (ref && JS_GetClass(ref) == &Canvas_class) {
        handler_ref= (static_cast<NativeJSCanvas *>(JS_GetPrivate(ref)))->getHandler();
    }

    if (NativeObject == handler_insert) {
        JS_ReportError(cx, "Canvas: can't add to itself");
        return false;
    }

    NativeObject->insertBefore(handler_insert, handler_ref);

    return true;
}

static bool native_canvas_insertAfter(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);

    Nidium::Graphics::NativeCanvasHandler *handler_insert = NULL, *handler_ref = NULL;

    JS::RootedObject insert(cx);
    JS::RootedObject ref(cx);
    if (!JS_ConvertArguments(cx, args, "oo", insert.address(), ref.address())) {
        return false;
    }

    if (!insert || JS_GetClass(insert) != &Canvas_class) {
        JS_ReportError(cx, "add() First parameter is not a Canvas Object");
        return false;
    }

    handler_insert= (static_cast<NativeJSCanvas *>(JS_GetPrivate(insert)))->getHandler();
    if (handler_insert == NULL) {
        return true;
    }

    if (ref && JS_GetClass(ref) == &Canvas_class) {
        handler_ref= (static_cast<NativeJSCanvas *>(JS_GetPrivate(ref)))->getHandler();
    }

    if (NativeObject == handler_insert) {
        JS_ReportError(cx, "Canvas: can't add to itself");
        return false;
    }

    NativeObject->insertAfter(handler_insert, handler_ref);

    return true;
}

static bool native_canvas_getContext(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);
    NIDIUM_JS_CHECK_ARGS("getContext", 1);

    Nidium::NML::NativeContext *nctx = Nidium::NML::NativeContext::GetObject(cx);
    NativeUIInterface *ui = nctx->getUI();

    JS::RootedString mode(cx, args[0].toString());
    JSAutoByteString cmode(cx, mode);
    Nidium::Graphics::NativeCanvasContext::mode ctxmode = Nidium::Graphics::NativeCanvasContext::CONTEXT_2D;
    if (strncmp(cmode.ptr(), "2d", 2) == 0) {
        ctxmode = Nidium::Graphics::NativeCanvasContext::CONTEXT_2D;
    } else if (strncmp(cmode.ptr(), "webgl", 5) == 0) {
        ctxmode = Nidium::Graphics::NativeCanvasContext::CONTEXT_WEBGL;
    } else {
        args.rval().setNull();
        return true;
    }

    Nidium::Graphics::NativeCanvasContext *canvasctx = NativeObject->getContext();

    /* The context is lazy-created */
    if (canvasctx == NULL) {
        switch(ctxmode) {
            case Nidium::Graphics::NativeCanvasContext::CONTEXT_2D:
            {
                NativeCanvas2DContext *ctx2d = new NativeCanvas2DContext(NativeObject, cx,
                        NativeObject->getWidth() + (NativeObject->m_Padding.global * 2),
                        NativeObject->getHeight() + (NativeObject->m_Padding.global * 2), ui);

                if (ctx2d->getSurface() == NULL) {
                    delete ctx2d;
                    JS_ReportError(cx, "Could not create 2D context for this canvas");
                    return false;
                }
                NativeObject->setContext(ctx2d);

                /* Inherit from the Nidium::NML::NativeContext glstate */
                ctx2d->setGLState(nctx->getGLState());

                break;
            }
            case Nidium::Graphics::NativeCanvasContext::CONTEXT_WEBGL:
                /*
                    TODO :
                    NativeObject->setContext(new NativeCanvasWebGLContext(...))
                */
                Nidium::Graphics::NativeCanvas3DContext *ctx3d = new Nidium::Graphics::NativeCanvas3DContext(NativeObject, cx,
                        NativeObject->getWidth() + (NativeObject->m_Padding.global * 2),
                        NativeObject->getHeight() + (NativeObject->m_Padding.global * 2), ui);

                NativeObject->setContext(ctx3d);
                break;
        }

        canvasctx = NativeObject->getContext();

        /*  Protect against GC
            Canvas.slot[0] = context
        */
        JS::RootedValue slot(cx, OBJECT_TO_JSVAL(NativeObject->getContext()->m_JsObj));
        JS_SetReservedSlot(NativeObject->m_JsObj, 0, slot);
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

static bool native_canvas_setContext(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(Nidium::Graphics::NativeCanvasHandler);
    NIDIUM_JS_CHECK_ARGS("setContext", 1);

    JS::RootedObject obj(cx, args[0].toObjectOrNull());
    if (!obj.get()) {
        return true;
    }

    Nidium::Graphics::NativeCanvasContext *context;

    if (!(context = static_cast<Nidium::Graphics::NativeCanvasContext *>(JS_GetInstancePrivate(cx,
            obj, &Canvas2DContext_class, &args)))) {
        JS_ReportError(cx, "setContext() argument must a CanvasRenderingContext2D object");
        return false;
    }

    NativeObject->setContext(context);

    /*
        If a context was already attached, it's going to be GC'd
        since it's not longer reachable from slot 0.
    */
    JS::RootedValue slot(cx, OBJECT_TO_JSVAL(context->m_JsObj));
    JS_SetReservedSlot(NativeObject->m_JsObj, 0, slot);

    return true;
}


/* TODO: do not change the value when a wrong type is set */
static bool native_canvas_prop_set(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp)
{
    Nidium::Graphics::NativeCanvasHandler *handler = HANDLER_GETTER_SAFE(cx, obj);
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
                handler->setPositioning(Nidium::Graphics::NativeCanvasHandler::COORD_ABSOLUTE);
            } else if(strcasecmp(mode.ptr(), "fixed") == 0) {
                handler->setPositioning(Nidium::Graphics::NativeCanvasHandler::COORD_FIXED);
            } else if (strcasecmp(mode.ptr(), "inline") == 0) {
                handler->setPositioning(Nidium::Graphics::NativeCanvasHandler::COORD_INLINE);
            } else if (strcasecmp(mode.ptr(), "inline-break") == 0) {
                handler->setPositioning(Nidium::Graphics::NativeCanvasHandler::COORD_INLINEBREAK);
            } else {
                handler->setPositioning(Nidium::Graphics::NativeCanvasHandler::COORD_RELATIVE);
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
            for (int i = 0; native_cursors_list[i].str != NULL; i++) {
                if (strncasecmp(native_cursors_list[i].str, type.ptr(),
                    strlen(native_cursors_list[i].str)) == 0) {
                    handler->setCursor(native_cursors_list[i].type);
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

static bool native_canvas_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp)
{
    Nidium::Graphics::NativeCanvasHandler *handler = HANDLER_GETTER_SAFE(cx, obj);
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
                case Nidium::Graphics::NativeCanvasHandler::COORD_RELATIVE:
                    if (handler->getFlowMode() & Nidium::Graphics::NativeCanvasHandler::kFlowBreakPreviousSibling) {
                        jstr = JS_NewStringCopyZ(cx, "inline-break");
                        vp.setString(jstr);
                    } else if (handler->getFlowMode() & Nidium::Graphics::NativeCanvasHandler::kFlowInlinePreviousSibling) {
                        jstr = JS_NewStringCopyZ(cx, "inline");
                        vp.setString(jstr);
                    } else {
                        jstr = JS_NewStringCopyZ(cx, "relative");
                        vp.setString(jstr);
                    }
                    break;
                case Nidium::Graphics::NativeCanvasHandler::COORD_ABSOLUTE:
                    jstr = JS_NewStringCopyZ(cx, "absolute");
                    vp.setString(jstr);
                    break;
                case Nidium::Graphics::NativeCanvasHandler::COORD_FIXED:
                    jstr = JS_NewStringCopyZ(cx, "fixed");
                    vp.setString(jstr);
                    break;
                case Nidium::Graphics::NativeCanvasHandler::COORD_INLINE:
                    jstr = JS_NewStringCopyZ(cx, "inline");
                    vp.setString(jstr);
                    break;
                case Nidium::Graphics::NativeCanvasHandler::COORD_INLINEBREAK:
                    jstr = JS_NewStringCopyZ(cx, "inline-break");
                    vp.setString(jstr);
                    break;
            }
            break;
        }
        case CANVAS_PROP_ID:
        {
            char *id;
            handler->getIdentifier(&id);
            JS::RootedString jstr(cx, JS_NewStringCopyZ(cx, id));
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

            for (int i = 0; native_cursors_list[i].str != NULL; i++) {
                if (native_cursors_list[i].type == cursor) {
                    vp.setString(JS_NewStringCopyZ(cx, native_cursors_list[i].str));

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

static bool native_Canvas_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    Nidium::Graphics::NativeCanvasHandler *handler;
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
    handler = new Nidium::Graphics::NativeCanvasHandler(width, height, Nidium::NML::NativeContext::GetObject(cx), true);
    handler->m_Context = NULL;
    handler->m_JsObj = ret;
    handler->m_JsCx = cx;

    NativeJSCanvas *jscanvas = new NativeJSCanvas(ret, cx, handler);
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
    NativeJSCanvas *jscanvas = ((class NativeJSCanvas *)JS_GetPrivate(obj));

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
    Nidium::Graphics::NativeCanvasHandler *handler = HANDLER_GETTER(obj);
    if (handler != NULL) {
        Nidium::Graphics::NativeCanvasHandler *cur;

        for (cur = handler->getFirstChild(); cur != NULL; cur = cur->m_Next) {
            if (cur->m_JsObj) {

                JS_CallHeapObjectTracer(trc, &cur->m_JsObj, "nativecanvasroot");
            }
        }
    }
}

JSObject *NativeJSCanvas::generateJSObject(JSContext *cx, int width,
    int height, Nidium::Graphics::NativeCanvasHandler **out)
{
    Nidium::Graphics::NativeCanvasHandler *handler;
    Nidium::NML::NativeContext *nctx = Nidium::NML::NativeContext::GetObject(cx);
    NativeUIInterface *ui = nctx->getUI();

    JS::RootedObject ret(cx, JS_NewObject(cx, &Canvas_class, JS::NullPtr(), JS::NullPtr()));
    JS::RootedObject inherit(cx, JS_DefineObject(cx, ret, "inherit",
        &Canvas_Inherit_class, nullptr,
        JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT));

    handler = new Nidium::Graphics::NativeCanvasHandler(width, height, nctx);
    handler->setContext(new NativeCanvas2DContext(handler, cx, width, height, ui));
    handler->getContext()->setGLState(nctx->getGLState());

    /* window.canvas.overflow default to false */
    handler->m_Overflow = false;
    handler->m_JsObj = ret;
    handler->m_JsCx = cx;
    JS::RootedValue val(cx, OBJECT_TO_JSVAL(handler->m_Context->m_JsObj));
    JS_SetReservedSlot(ret, 0, val);

    NativeJSCanvas *jscanvas = new NativeJSCanvas(ret, cx, handler);

    jscanvas->setInherit(inherit);

    JS_SetPrivate(ret, jscanvas);
    JS_SetPrivate(inherit, jscanvas);

    *out = handler;

    return ret;
}

void NativeJSCanvas::registerObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS_InitClass(cx, global, JS::NullPtr(), &Canvas_class, native_Canvas_constructor, 2, canvas_props, canvas_funcs,
        nullptr, nullptr);
}

void NativeJSCanvas::onMessage(const Core::SharedMessages::Message &msg)
{
    JSContext *cx = m_Cx;
    JS::RootedObject ro(cx, m_JSObject);
    switch (msg.event()) {
        case NIDIUM_EVENT(Nidium::Graphics::NativeCanvasHandler, RESIZE_EVENT):
        {
            // TODO : fireEvent
            JSOBJ_CALLFUNCNAME(ro, "onresize", JS::HandleValueArray::empty());
            break;
        }
        case NIDIUM_EVENT(Nidium::Graphics::NativeCanvasHandler, LOADED_EVENT):
        {
            // TODO : fireEvent
            JSOBJ_CALLFUNCNAME(ro, "onload", JS::HandleValueArray::empty());
            break;
        }
        case NIDIUM_EVENT(Nidium::Graphics::NativeCanvasHandler, CHANGE_EVENT):
        {
            const char *name = NULL;
            JS::RootedValue value(cx);

            switch (msg.args[1].toInt()) {
                case Nidium::Graphics::NativeCanvasHandler::kContentWidth_Changed:
                    name = "contentWidth";
                    value.setInt32(msg.args[2].toInt());
                    break;
                case Nidium::Graphics::NativeCanvasHandler::kContentHeight_Changed:
                    name = "contentHeight";
                    value.setInt32(msg.args[2].toInt());
                    break;
            }

            JS::RootedObject ev(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));
            NIDIUM_JSOBJ_SET_PROP_CSTR(ev, "property", name);
            NIDIUM_JSOBJ_SET_PROP(ev, "value", value);

            JS::AutoValueArray<1> arg(cx);
            arg[0].set(OBJECT_TO_JSVAL(ev));
            // TODO : fireEvent
            JSOBJ_CALLFUNCNAME(ro, "onchange", arg);
            break;
        }
        case NIDIUM_EVENT(Nidium::Graphics::NativeCanvasHandler, DRAG_EVENT):
        {
            printf("Drag event detected\n");
        }
        case NIDIUM_EVENT(Nidium::Graphics::NativeCanvasHandler, MOUSE_EVENT):
        {
            JS::RootedObject eventObj(m_Cx, JSEvents::CreateEventObject(m_Cx));
            Nidium::Graphics::NativeCanvasHandler *target = static_cast<Nidium::Graphics::NativeCanvasHandler *>(msg.args[8].toPtr());
            JSObjectBuilder obj(m_Cx, eventObj);
            obj.set("x", msg.args[2].toInt());
            obj.set("y", msg.args[3].toInt());
            obj.set("clientX", msg.args[2].toInt());
            obj.set("clientY", msg.args[3].toInt());
            obj.set("layerX", msg.args[6].toInt());
            obj.set("layerY", msg.args[7].toInt());
            obj.set("target", target->m_JsObj);

            int evtype = (Nidium::NML::NativeInputEvent::Type)msg.args[1].toInt();

            switch (evtype) {
                case Nidium::NML::NativeInputEvent::kMouseMove_Type:
                case Nidium::NML::NativeInputEvent::kMouseDrag_Type:
                case Nidium::NML::NativeInputEvent::kMouseDragOver_Type:
                    obj.set("xrel", msg.args[4].toInt());
                    obj.set("yrel", msg.args[5].toInt());
                    break;
                default:
                    break;
            }

            switch (evtype) {
                case Nidium::NML::NativeInputEvent::kMouseClick_Type:
                case Nidium::NML::NativeInputEvent::kMouseDoubleClick_Type:
                case Nidium::NML::NativeInputEvent::kMouseClickRelease_Type:
                    obj.set("which", msg.args[4].toInt());
                    break;
                case Nidium::NML::NativeInputEvent::kMouseDrag_Type:
                case Nidium::NML::NativeInputEvent::kMouseDragStart_Type:
                case Nidium::NML::NativeInputEvent::kMouseDragEnd_Type:
                    obj.set("source", this->getJSObject());
                    break;
                case Nidium::NML::NativeInputEvent::kMouseDragOver_Type:
                case Nidium::NML::NativeInputEvent::kMouseDrop_Type:
                    obj.set("source", target->m_JsObj);
                    obj.set("target", this->getJSObject());
                    break;
                default:
                    break;
            }
            JS::RootedValue rval(cx, obj.jsval());
            if (!this->fireJSEvent(Nidium::NML::NativeInputEvent::getName(msg.args[1].toInt()), &rval)) {
                break;
            }

            JS::RootedValue cancelBubble(cx);
            JS::RootedObject robj(cx, obj.obj());
            if (JS_GetProperty(cx, robj, "cancelBubble", &cancelBubble)) {
                if (cancelBubble.isBoolean() && cancelBubble.toBoolean()) {
                    /* TODO: sort out this dirty hack */
                    Core::SharedMessages::Message *nonconstmsg = (Core::SharedMessages::Message *)&msg;
                    nonconstmsg->priv = 1;
                }
            }
        }
            break;
        default:
            break;
    }
}

void NativeJSCanvas::onMessageLost(const Core::SharedMessages::Message &msg)
{

}

NativeJSCanvas::NativeJSCanvas(JS::HandleObject obj, JSContext *cx,
    Nidium::Graphics::NativeCanvasHandler *handler) :
    JSExposer<NativeJSCanvas>(obj, cx),
    m_CanvasHandler(handler)
{
    m_CanvasHandler->addListener(this);

    /*
        Trigger "loaded" event if not lazy loaded
    */
    m_CanvasHandler->checkLoaded();
}

NativeJSCanvas::~NativeJSCanvas()
{
    delete m_CanvasHandler;
}

} // namespace Nidium
} // namespace Binding

