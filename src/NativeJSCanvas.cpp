#include "NativeJSCanvas.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>

#include "NativeCanvas2DContext.h"
#include "NativeCanvas3DContext.h"
#include "NativeCanvasHandler.h"

extern JSClass Canvas2DContext_class;

#define native_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))
#define native_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))

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
JSClass *NativeJSExposer<NativeJSCanvas>::jsclass = &Canvas_class;


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
    {"opacity", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_OPACITY, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_OPACITY, native_canvas_prop_set)},
    {"overflow", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_OVERFLOW, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_OVERFLOW, native_canvas_prop_set)},
    {"scrollLeft", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_SCROLLLEFT, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_SCROLLLEFT, native_canvas_prop_set)},
    {"scrollTop", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_SCROLLTOP, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_SCROLLTOP, native_canvas_prop_set)},
    {"allowNegativeScroll", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_ALLOWNEGATIVESCROLL, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_ALLOWNEGATIVESCROLL, native_canvas_prop_set)},
    {"width", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_WIDTH, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_WIDTH, native_canvas_prop_set)},
    {"clientWidth",  NATIVE_JS_PROP | JSPROP_READONLY,
        NATIVE_JS_GETTER(CANVAS_PROP_CLIENTWIDTH, native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"clientHeight", NATIVE_JS_PROP | JSPROP_READONLY,
        NATIVE_JS_GETTER(CANVAS_PROP_CLIENTHEIGHT, native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"clientTop", NATIVE_JS_PROP | JSPROP_READONLY,
        NATIVE_JS_GETTER(CANVAS_PROP_CLIENTTOP, native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"clientLeft", NATIVE_JS_PROP | JSPROP_READONLY,
        NATIVE_JS_GETTER(CANVAS_PROP_CLIENTLEFT, native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"coating", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_COATING, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_COATING, native_canvas_prop_set)},
    {"height", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_HEIGHT, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_HEIGHT, native_canvas_prop_set)},
    {"maxWidth", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_MAXWIDTH, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_MAXWIDTH, native_canvas_prop_set)},
    {"maxHeight", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_MAXHEIGHT, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_MAXHEIGHT, native_canvas_prop_set)},
    {"minWidth", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_MINWIDTH, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_MINWIDTH, native_canvas_prop_set)},
    {"minHeight", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_MINHEIGHT, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_MINHEIGHT, native_canvas_prop_set)},
    {"position", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_POSITION, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_POSITION, native_canvas_prop_set)},
    {"top", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_TOP, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_TOP, native_canvas_prop_set)},
    {"left", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_LEFT, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_LEFT, native_canvas_prop_set)},
    {"right", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_RIGHT, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_RIGHT, native_canvas_prop_set)},
    {"bottom", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_BOTTOM, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_BOTTOM, native_canvas_prop_set)},
    {"visible", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_VISIBLE, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_VISIBLE, native_canvas_prop_set)},
    {"contentWidth", NATIVE_JS_PROP | JSPROP_READONLY,
        NATIVE_JS_GETTER(CANVAS_PROP_CONTENTWIDTH, native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"contentHeight", NATIVE_JS_PROP | JSPROP_READONLY,
        NATIVE_JS_GETTER(CANVAS_PROP_CONTENTHEIGHT, native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"innerWidth", NATIVE_JS_PROP | JSPROP_READONLY,
        NATIVE_JS_GETTER(CANVAS_PROP_INNERWIDTH, native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"innerHeight", NATIVE_JS_PROP | JSPROP_READONLY,
        NATIVE_JS_GETTER(CANVAS_PROP_INNERHEIGHT, native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"__visible", NATIVE_JS_PROP | JSPROP_READONLY,
        NATIVE_JS_GETTER(CANVAS_PROP___VISIBLE, native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"__top", NATIVE_JS_PROP | JSPROP_READONLY,
        NATIVE_JS_GETTER(CANVAS_PROP___TOP, native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"__left", NATIVE_JS_PROP | JSPROP_READONLY,
        NATIVE_JS_GETTER(CANVAS_PROP___LEFT, native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"__fixed", NATIVE_JS_PROP | JSPROP_READONLY,
        NATIVE_JS_GETTER(CANVAS_PROP___FIXED, native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"__outofbound", NATIVE_JS_PROP | JSPROP_READONLY,
        NATIVE_JS_GETTER(CANVAS_PROP___OUTOFBOUND, native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"ctx", NATIVE_JS_PROP | JSPROP_READONLY,
        NATIVE_JS_GETTER(CANVAS_PROP_CTX, native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"staticLeft", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_STATICLEFT, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_STATICLEFT, native_canvas_prop_set)},
    {"staticRight", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_STATICRIGHT, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_STATICRIGHT, native_canvas_prop_set)},
    {"staticTop", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_STATICTOP, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_STATICTOP, native_canvas_prop_set)},
    {"staticBottom", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_STATICBOTTOM, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_STATICBOTTOM, native_canvas_prop_set)},
    {"fluidHeight", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_FLUIDHEIGHT, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_FLUIDHEIGHT, native_canvas_prop_set)},
    {"fluidWidth", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_FLUIDWIDTH, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_FLUIDWIDTH, native_canvas_prop_set)},
    {"id", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_ID, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_ID, native_canvas_prop_set)},
    {"marginLeft", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_MARGINLEFT, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_MARGINLEFT, native_canvas_prop_set)},
    {"marginRight", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_MARGINRIGHT, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_MARGINRIGHT, native_canvas_prop_set)},
    {"marginTop", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_MARGINTOP, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_MARGINTOP, native_canvas_prop_set)},
    {"marginBottom", NATIVE_JS_PROP,
        NATIVE_JS_GETTER(CANVAS_PROP_MARGINBOTTOM, native_canvas_prop_get),
        NATIVE_JS_SETTER(CANVAS_PROP_MARGINBOTTOM, native_canvas_prop_set)},
    {"cursor", NATIVE_JS_PROP,
        NATIVE_JS_STUBGETTER(),
        NATIVE_JS_SETTER(CANVAS_PROP_CURSOR, native_canvas_prop_set)},
    JS_PS_END
};

static JSFunctionSpec canvas_funcs[] = {
    JS_FN("getContext", native_canvas_getContext, 1, 0),
    JS_FN("setContext", native_canvas_setContext, 1, 0),
    JS_FN("add", native_canvas_addSubCanvas, 1, 0),
    JS_FN("insertBefore", native_canvas_insertBefore, 2, 0),
    JS_FN("insertAfter", native_canvas_insertAfter, 2, 0),
    JS_FN("removeFromParent", native_canvas_removeFromParent, 0, 0),
    JS_FN("show", native_canvas_show, 0, 0),
    JS_FN("hide", native_canvas_hide, 0, 0),
    JS_FN("bringToFront", native_canvas_bringToFront, 0, 0),
    JS_FN("sendToBack", native_canvas_sendToBack, 0, 0),
    JS_FN("getParent", native_canvas_getParent, 0, 0),
    JS_FN("getFirstChild", native_canvas_getFirstChild, 0, 0),
    JS_FN("getLastChild", native_canvas_getLastChild, 0, 0),
    JS_FN("getNextSibling", native_canvas_getNextSibling, 0, 0),
    JS_FN("getPrevSibling", native_canvas_getPrevSibling, 0, 0),
    JS_FN("getChildren", native_canvas_getChildren, 0, 0),
    JS_FN("setCoordinates", native_canvas_setCoordinates, 2, 0),
    JS_FN("translate", native_canvas_translate, 2, 0),
    JS_FN("getVisibleRect", native_canvas_getVisibleRect, 0, 0),
    JS_FN("setSize", native_canvas_setSize, 2, 0),
    JS_FN("clear", native_canvas_clear, 0, 0),
    JS_FN("setZoom", native_canvas_setZoom, 1, 0),
    JS_FN("setScale", native_canvas_setScale, 2, 0),
    JS_FS_END
};

static bool CanvasInherit_get(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
{
    NativeJSCanvas *jscanvas = (class NativeJSCanvas *)JS_GetPrivate(obj);
    if (!jscanvas) {
        return true;
    }

    NativeCanvasHandler *handler = jscanvas->getHandler(), *parent;

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

NativeCanvasHandler *HANDLER_GETTER(JSObject *obj)
{
    NativeJSCanvas *jscanvas = (class NativeJSCanvas *)JS_GetPrivate(obj);
    if (!jscanvas) {
        return NULL;
    }

    return jscanvas->getHandler();
}

static NativeCanvasHandler *HANDLER_GETTER_SAFE(JSContext *cx, JS::HandleObject obj)
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
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeObject->setHidden(false);

    return true;
}

static bool native_canvas_hide(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeObject->setHidden(true);

    return true;
}

static bool native_canvas_clear(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    if (NativeObject->m_Context) {
        NativeObject->m_Context->clear(0x00000000);
    }

    return true;
}

static bool native_canvas_setZoom(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    double zoom;

    if (!JS_ConvertArguments(cx, args, "d", &zoom)) {
        return false;
    }

    NativeObject->setZoom(zoom);

    return true;
}

static bool native_canvas_setScale(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    double x, y;

    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        return false;
    }

    NativeObject->setScale(x, y);

    return true;
}

static bool native_canvas_setSize(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

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
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeObject->removeFromParent();

    return true;
}

static bool native_canvas_bringToFront(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeObject->bringToFront();

    return true;
}

static bool native_canvas_sendToBack(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeObject->sendToBack();

    return true;
}

static bool native_canvas_getParent(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeCanvasHandler *parent = NativeObject->getParent();
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
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeCanvasHandler *val = NativeObject->getFirstChild();
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
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeCanvasHandler *val = NativeObject->getLastChild();
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
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeCanvasHandler *val = NativeObject->getNextSibling();
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
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeCanvasHandler *val = NativeObject->getPrevSibling();
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
    NATIVE_PROLOGUE(NativeCanvasHandler);

    uint32_t i;
    int32_t count = NativeObject->countChildren();

    if (!count) {
        JS::RootedObject retObj(cx, JS_NewArrayObject(cx, 0));
        args.rval().setObjectOrNull(retObj);
        return true;
    }

    NativeCanvasHandler *list[count];

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
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeRect rect = NativeObject->getVisibleRect();
    JS::RootedObject ret(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));

#define SET_PROP(where, name, val) JS_DefineProperty(cx, where, \
    (const char *)name, val, JSPROP_PERMANENT | JSPROP_READONLY | \
        JSPROP_ENUMERATE)

    SET_PROP(ret, "left", rect.m_fLeft);
    SET_PROP(ret, "top", rect.m_fTop);
    SET_PROP(ret, "width", native_max(0, rect.m_fRight - rect.m_fLeft));
    SET_PROP(ret, "height", native_max(0, rect.m_fBottom-rect.m_fTop));
#undef SET_PROP

    args.rval().setObjectOrNull(ret);

    return true;
}

static bool native_canvas_setCoordinates(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

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
    NATIVE_PROLOGUE(NativeCanvasHandler);

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
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeCanvasHandler *handler = NULL;

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
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeCanvasHandler *handler_insert = NULL, *handler_ref = NULL;

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
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeCanvasHandler *handler_insert = NULL, *handler_ref = NULL;

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
    NATIVE_PROLOGUE(NativeCanvasHandler);
    NATIVE_CHECK_ARGS("getContext", 1);

    NativeContext *nctx = NativeContext::getNativeClass(cx);
    NativeUIInterface *ui = nctx->getUI();

    JS::RootedString mode(cx, args[0].toString());
    JSAutoByteString cmode(cx, mode);
    NativeCanvasContext::mode ctxmode = NativeCanvasContext::CONTEXT_2D;
    if (strncmp(cmode.ptr(), "2d", 2) == 0) {
        ctxmode = NativeCanvasContext::CONTEXT_2D;
    } else if (strncmp(cmode.ptr(), "webgl", 5) == 0) {
        ctxmode = NativeCanvasContext::CONTEXT_WEBGL;
    } else {
        args.rval().setNull();
        return true;
    }

    NativeCanvasContext *canvasctx = NativeObject->getContext();

    /* The context is lazy-created */
    if (canvasctx == NULL) {
        switch(ctxmode) {
            case NativeCanvasContext::CONTEXT_2D:
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

                /* Inherit from the NativeContext glstate */
                ctx2d->setGLState(nctx->getGLState());

                break;
            }
            case NativeCanvasContext::CONTEXT_WEBGL:
                /*
                    TODO :
                    NativeObject->setContext(new NativeCanvasWebGLContext(...))
                */
                NativeCanvas3DContext *ctx3d = new NativeCanvas3DContext(NativeObject, cx,
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
    NATIVE_PROLOGUE(NativeCanvasHandler);
    NATIVE_CHECK_ARGS("setContext", 1);

    JS::RootedObject obj(cx, args[0].toObjectOrNull());
    if (!obj.get()) {
        return true;
    }

    NativeCanvasContext *context;

    if (!(context = static_cast<NativeCanvasContext *>(JS_GetInstancePrivate(cx,
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
    NativeCanvasHandler *handler = HANDLER_GETTER_SAFE(cx, obj);
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
            JS::RootedString vpStr(cx, JSVAL_TO_STRING(vp));
            JSAutoByteString mode(cx, vpStr);
            if (strcasecmp(mode.ptr(), "absolute") == 0) {
                handler->setPositioning(NativeCanvasHandler::COORD_ABSOLUTE);
            } else if(strcasecmp(mode.ptr(), "fixed") == 0) {
                handler->setPositioning(NativeCanvasHandler::COORD_FIXED);
            } else if (strcasecmp(mode.ptr(), "inline") == 0) {
                handler->setPositioning(NativeCanvasHandler::COORD_INLINE);
            } else if (strcasecmp(mode.ptr(), "inline-break") == 0) {
                handler->setPositioning(NativeCanvasHandler::COORD_INLINEBREAK);
            } else {
                handler->setPositioning(NativeCanvasHandler::COORD_RELATIVE);
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
            JS::ToNumber(cx, vp, &dval);
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
            JS::ToNumber(cx, vp, &dval);
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
            JS::ToNumber(cx, vp, &dval);
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
            JS::ToNumber(cx, vp, &dval);
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
            handler->setAllowNegativeScroll(JSVAL_TO_BOOLEAN(vp));
        }
        break;
        case CANVAS_PROP_VISIBLE:
        {
            if (!vp.isBoolean()) {
                return true;
            }

            handler->setHidden(!JSVAL_TO_BOOLEAN(vp));
        }
        break;
        case CANVAS_PROP_OVERFLOW:
        {
            if (!vp.isBoolean()) {
                return true;
            }

            handler->m_Overflow = JSVAL_TO_BOOLEAN(vp);
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
            JS::ToNumber(cx, vp, &dval);
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
            JS::RootedString sid(cx, vp.toString());
            if (!sid) {
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

            JS::ToNumber(cx, vp, &dval);
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

            JS::ToNumber(cx, vp, &dval);
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

            JS::ToNumber(cx, vp, &dval);
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

            JS::ToNumber(cx, vp, &dval);
            handler->setMargin(handler->m_Margin.top, handler->m_Margin.right,
                dval, handler->m_Margin.left);
        }
        break;
        case CANVAS_PROP_CURSOR:
        {
            if (!vp.isString()) {
                return true;
            }

            JS::RootedString vpStr(cx, JSVAL_TO_STRING(vp));
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
    NativeCanvasHandler *handler = HANDLER_GETTER_SAFE(cx, obj);
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
                case NativeCanvasHandler::COORD_RELATIVE:
                    if (handler->getFlowMode() & NativeCanvasHandler::kFlowBreakPreviousSibling) {
                        jstr = JS_NewStringCopyZ(cx, "inline-break");
                        vp.setString(jstr);
                    } else if (handler->getFlowMode() & NativeCanvasHandler::kFlowInlinePreviousSibling) {
                        jstr = JS_NewStringCopyZ(cx, "inline");
                        vp.setString(jstr);
                    } else {
                        jstr = JS_NewStringCopyZ(cx, "relative");
                        vp.setString(jstr);
                    }
                    break;
                case NativeCanvasHandler::COORD_ABSOLUTE:
                    jstr = JS_NewStringCopyZ(cx, "absolute");
                    vp.setString(jstr);
                    break;
                case NativeCanvasHandler::COORD_FIXED:
                    jstr = JS_NewStringCopyZ(cx, "fixed");
                    vp.setString(jstr);
                    break;
                case NativeCanvasHandler::COORD_INLINE:
                    jstr = JS_NewStringCopyZ(cx, "inline");
                    vp.setString(jstr);
                    break;
                case NativeCanvasHandler::COORD_INLINEBREAK:
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
        default:
            break;
    }

    return true;
}

static bool native_Canvas_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativeCanvasHandler *handler;
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

    JS_INITOPT();
    JSGET_OPT_TYPE(opt, "lazy", Boolean) {
        lazyLoad = __curopt.toBoolean();
    }
    lazyLoad = false; /* Always lazy load for now.  */
    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &Canvas_class, args));
    handler = new NativeCanvasHandler(width, height, NativeContext::getNativeClass(cx), true);
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
    NativeCanvasHandler *handler = HANDLER_GETTER(obj);
    if (handler != NULL) {
        NativeCanvasHandler *cur;

        for (cur = handler->getFirstChild(); cur != NULL; cur = cur->m_Next) {
            if (cur->m_JsObj) {
#if 0 && defined(DEBUG)
                trc->debugPrinter = PrintGetTraceName;
                trc->debugPrintArg = cur;
                JS_CallObjectTracer(trc, (JSObject **)&cur->m_JsObj, "nativecanvasroot");
#endif
            }
        }
    }
}

JSObject *NativeJSCanvas::generateJSObject(JSContext *cx, int width,
    int height, NativeCanvasHandler **out)
{
    NativeCanvasHandler *handler;
    NativeContext *nctx = NativeContext::getNativeClass(cx);
    NativeUIInterface *ui = nctx->getUI();

    JS::RootedObject ret(cx, JS_NewObject(cx, &Canvas_class, JS::NullPtr(), JS::NullPtr()));
    JS::RootedObject inherit(cx, JS_DefineObject(cx, ret, "inherit",
        &Canvas_Inherit_class, nullptr,
        JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT));

    handler = new NativeCanvasHandler(width, height, nctx);
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

void NativeJSCanvas::onMessage(const NativeSharedMessages::Message &msg)
{
    JSContext *cx = m_Cx;
    JS::RootedObject ro(cx, m_JSObject);
    switch (msg.event()) {
        case NATIVE_EVENT(NativeCanvasHandler, RESIZE_EVENT):
        {
            // TODO : fireEvent
            JSOBJ_CALLFUNCNAME(ro, "onresize", JS::HandleValueArray::empty());
            break;
        }
        case NATIVE_EVENT(NativeCanvasHandler, LOADED_EVENT):
        {
            // TODO : fireEvent
            JSOBJ_CALLFUNCNAME(ro, "onload", JS::HandleValueArray::empty());
            break;
        }
        case NATIVE_EVENT(NativeCanvasHandler, CHANGE_EVENT):
        {
            const char *name = NULL;
            JS::RootedValue value(cx);

            switch (msg.args[1].toInt()) {
                case NativeCanvasHandler::kContentWidth_Changed:
                    name = "contentWidth";
                    value.setInt32(msg.args[2].toInt());
                    break;
                case NativeCanvasHandler::kContentHeight_Changed:
                    name = "contentHeight";
                    value.setInt32(msg.args[2].toInt());
                    break;
            }

            JS::RootedObject ev(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));
            JSOBJ_SET_PROP_CSTR(ev, "property", name);
            JSOBJ_SET_PROP(ev, "value", value);

            JS::AutoValueArray<1> arg(cx);
            arg[0].set(OBJECT_TO_JSVAL(ev));
            // TODO : fireEvent
            JSOBJ_CALLFUNCNAME(ro, "onchange", arg);
            break;
        }
        case NATIVE_EVENT(NativeCanvasHandler, DRAG_EVENT):
        {
            printf("Drag event detected\n");
        }
        case NATIVE_EVENT(NativeCanvasHandler, MOUSE_EVENT):
        {
            JS::RootedObject eventObj(m_Cx, NativeJSEvents::CreateEventObject(m_Cx));
            NativeCanvasHandler *target = static_cast<NativeCanvasHandler *>(msg.args[8].toPtr());
            NativeJSObjectBuilder obj(m_Cx, eventObj);
            obj.set("x", msg.args[2].toInt());
            obj.set("y", msg.args[3].toInt());
            obj.set("clientX", msg.args[2].toInt());
            obj.set("clientY", msg.args[3].toInt());
            obj.set("layerX", msg.args[6].toInt());
            obj.set("layerY", msg.args[7].toInt());
            obj.set("target", target->m_JsObj);

            int evtype = (NativeInputEvent::Type)msg.args[1].toInt();

            switch (evtype) {
                case NativeInputEvent::kMouseMove_Type:
                case NativeInputEvent::kMouseDrag_Type:
                case NativeInputEvent::kMouseDragOver_Type:
                    obj.set("xrel", msg.args[4].toInt());
                    obj.set("yrel", msg.args[5].toInt());
                    break;
                default:
                    break;
            }

            switch (evtype) {
                case NativeInputEvent::kMouseClick_Type:
                case NativeInputEvent::kMouseDoubleClick_Type:
                case NativeInputEvent::kMouseClickRelease_Type:
                    obj.set("which", msg.args[4].toInt());
                    break;
                case NativeInputEvent::kMouseDrag_Type:
                case NativeInputEvent::kMouseDragStart_Type:
                case NativeInputEvent::kMouseDragEnd_Type:
                    obj.set("source", this->getJSObject());
                    break;
                case NativeInputEvent::kMouseDragOver_Type:
                case NativeInputEvent::kMouseDrop_Type:
                    obj.set("source", target->m_JsObj);
                    obj.set("target", this->getJSObject());
                    break;
                default:
                    break;
            }
            JS::RootedValue rval(cx, obj.jsval());
            if (!this->fireJSEvent(NativeInputEvent::getName(msg.args[1].toInt()), &rval)) {
                break;
            }

            JS::RootedValue cancelBubble(cx);
            JS::RootedObject robj(cx, obj.obj());
            if (JS_GetProperty(cx, robj, "cancelBubble", &cancelBubble)) {
                if (cancelBubble.isBoolean() && cancelBubble.toBoolean()) {
                    /* TODO: sort out this dirty hack */
                    NativeSharedMessages::Message *nonconstmsg = (NativeSharedMessages::Message *)&msg;
                    nonconstmsg->priv = 1;
                }
            }
        }
            break;
        default:
            break;
    }
}

void NativeJSCanvas::onMessageLost(const NativeSharedMessages::Message &msg)
{

}

NativeJSCanvas::NativeJSCanvas(JS::HandleObject obj, JSContext *cx,
    NativeCanvasHandler *handler) :
    NativeJSExposer<NativeJSCanvas>(obj, cx),
    m_CanvasHandler(handler), m_Inherit(cx)
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

