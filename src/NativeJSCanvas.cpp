#include "NativeJSCanvas.h"
#include "NativeSkia.h"
#include "NativeCanvas2DContext.h"
#include "NativeCanvas3DContext.h"
#include "NativeCanvasHandler.h"
#include "NativeContext.h"

#include <assert.h>

extern JSClass Canvas2DContext_class;

//#define HANDLER_GETTER(obj) ((NativeCanvasHandler *)((class NativeJSCanvas *)JS_GetPrivate(obj))->getHandler())

#define native_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))
#define native_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))

#define NATIVE_PROLOGUE(ofclass) \
    JS::CallArgs args = CallArgsFromVp(argc, vp); \
    JS::RootedObject thisobj(cx, JS_THIS_OBJECT(cx, vp)); \
    if (!thisobj) return false; \
    if (JS_GetClass(thisobj) != NativeLocalClass) { \
        JS_ReportError(cx, "Illegal invocation");\
        args.rval().setUndefined(); \
        return false; \
    } \
    ofclass *NativeObject = ((ofclass *)((class NativeJSCanvas *)JS_GetPrivate(thisobj))->getHandler())


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

extern jsval gfunc;

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
static void Canvas_Trace(JSTracer *trc, JSRawObject obj);

static JSBool CanvasInherit_get(JSContext *cx, JSHandleObject obj, JSHandleId id, JSMutableHandleValue vp);

JSClass Canvas_class = {
    "Canvas", JSCLASS_HAS_PRIVATE | JSCLASS_IMPLEMENTS_BARRIERS | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Canvas_Finalize,
    0,0,0,0, Canvas_Trace, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *NativeJSExposer<NativeJSCanvas>::jsclass = &Canvas_class;


JSClass Canvas_Inherit_class = {
    "CanvasInherit", JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,
        JS_PropertyStub,
        CanvasInherit_get,
        JS_StrictPropertyStub,
        JS_EnumerateStub,
        JS_ResolveStub,
        JS_ConvertStub
};


static JSClass *NativeLocalClass = &Canvas_class;

static JSBool native_canvas_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp);
static JSBool native_canvas_prop_get(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSMutableHandleValue vp);

static JSBool native_canvas_getContext(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_setContext(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_addSubCanvas(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_insertBefore(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_insertAfter(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_removeFromParent(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_bringToFront(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_sendToBack(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_getParent(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_getFirstChild(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_getLastChild(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_getNextSibling(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_getPrevSibling(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_getChildren(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_getVisibleRect(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_setCoordinates(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_translate(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_show(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_hide(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_setSize(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_clear(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_setZoom(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_setScale(JSContext *cx, unsigned argc, jsval *vp);

#define NATIVE_JS_PROP JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED

static JSPropertySpec canvas_props[] = {
    {"opacity", CANVAS_PROP_OPACITY, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},
    {"overflow", CANVAS_PROP_OVERFLOW, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},  
    {"scrollLeft", CANVAS_PROP_SCROLLLEFT, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},
    {"scrollTop", CANVAS_PROP_SCROLLTOP, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},
    {"allowNegativeScroll", CANVAS_PROP_ALLOWNEGATIVESCROLL, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},
    {"width", CANVAS_PROP_WIDTH, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},
    {"clientWidth", CANVAS_PROP_CLIENTWIDTH, NATIVE_JS_PROP | JSPROP_READONLY,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"clientHeight", CANVAS_PROP_CLIENTHEIGHT, NATIVE_JS_PROP | JSPROP_READONLY,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"clientTop", CANVAS_PROP_CLIENTTOP, NATIVE_JS_PROP | JSPROP_READONLY,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"clientLeft", CANVAS_PROP_CLIENTLEFT, NATIVE_JS_PROP | JSPROP_READONLY,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"coating", CANVAS_PROP_COATING, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},

    {"height", CANVAS_PROP_HEIGHT, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},

    {"maxWidth", CANVAS_PROP_MAXWIDTH, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},

    {"maxHeight", CANVAS_PROP_MAXHEIGHT, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},

    {"minWidth", CANVAS_PROP_MINWIDTH, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},

    {"minHeight", CANVAS_PROP_MINHEIGHT, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},

    {"position", CANVAS_PROP_POSITION, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},

    {"top", CANVAS_PROP_TOP, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},

    {"left", CANVAS_PROP_LEFT, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},

    {"right", CANVAS_PROP_RIGHT, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},

    {"bottom", CANVAS_PROP_BOTTOM, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},

    {"visible", CANVAS_PROP_VISIBLE, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},
    {"contentWidth", CANVAS_PROP_CONTENTWIDTH, NATIVE_JS_PROP | JSPROP_READONLY,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},
    {"contentHeight", CANVAS_PROP_CONTENTHEIGHT, NATIVE_JS_PROP | JSPROP_READONLY,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},
    {"innerWidth", CANVAS_PROP_INNERWIDTH, NATIVE_JS_PROP | JSPROP_READONLY,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},
    {"innerHeight", CANVAS_PROP_INNERHEIGHT, NATIVE_JS_PROP | JSPROP_READONLY,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},
    {"__visible", CANVAS_PROP___VISIBLE, NATIVE_JS_PROP | JSPROP_READONLY,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},
    {"__top", CANVAS_PROP___TOP, NATIVE_JS_PROP | JSPROP_READONLY,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},
    {"__left", CANVAS_PROP___LEFT, NATIVE_JS_PROP | JSPROP_READONLY,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},
    {"__fixed", CANVAS_PROP___FIXED, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_SHARED,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},
    {"__outofbound", CANVAS_PROP___OUTOFBOUND, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_SHARED,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},
    {"ctx", CANVAS_PROP_CTX, NATIVE_JS_PROP | JSPROP_READONLY,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},

    {"staticLeft", CANVAS_PROP_STATICLEFT, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},
    {"staticRight", CANVAS_PROP_STATICRIGHT, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},

    {"staticTop", CANVAS_PROP_STATICTOP, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},
    {"staticBottom", CANVAS_PROP_STATICBOTTOM, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},

    {"fluidHeight", CANVAS_PROP_FLUIDHEIGHT, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},

    {"fluidWidth", CANVAS_PROP_FLUIDWIDTH, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},

    {"id", CANVAS_PROP_ID, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},

    {"marginLeft", CANVAS_PROP_MARGINLEFT, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},
    {"marginRight", CANVAS_PROP_MARGINRIGHT, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},
    {"marginTop", CANVAS_PROP_MARGINTOP, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},
    {"marginBottom", CANVAS_PROP_MARGINBOTTOM, NATIVE_JS_PROP,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},
    {"cursor", CANVAS_PROP_CURSOR, NATIVE_JS_PROP,
        JSOP_NULLWRAPPER,
        JSOP_WRAPPER(native_canvas_prop_set)},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
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

static JSBool CanvasInherit_get(JSContext *cx, JSHandleObject obj, JSHandleId id, JSMutableHandleValue vp)
{
    NativeJSCanvas *jscanvas = (class NativeJSCanvas *)JS_GetPrivate(obj);
    if (!jscanvas) {
        return true;
    }

    NativeCanvasHandler *handler = jscanvas->getHandler(), *parent;

    if (vp.get() == JSVAL_VOID) {

        if ((parent = handler->getParent()) == NULL || !parent->jsobj) {
            return true;
        }

        NativeJSCanvas *jscanvas_parent =
            (class NativeJSCanvas *)JS_GetPrivate(parent->jsobj);

        JS::Value ret;
        JS_GetPropertyById(cx, jscanvas_parent->getInherit(), id, &ret);
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

static NativeCanvasHandler *HANDLER_GETTER_SAFE(JSContext *cx, JSObject *obj, jsval *argv = NULL)
{
    NativeJSCanvas *jscanvas = (class NativeJSCanvas *)JS_GetInstancePrivate(cx, obj, &Canvas_class, argv);
    if (!jscanvas) {
        JSClass *cl = JS_GetClass(obj);
        printf("Missmatch classe %s\n", cl->name);
        //assert(1!=1);
        return NULL;
    }

    return jscanvas->getHandler();
}

static JSBool native_canvas_show(JSContext *cx, unsigned argc, jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeObject->setHidden(false);

    return true;
}

static JSBool native_canvas_hide(JSContext *cx, unsigned argc, jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeObject->setHidden(true);
    
    return true;
}

static JSBool native_canvas_clear(JSContext *cx, unsigned argc, jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    if (NativeObject->m_Context) {
        NativeObject->m_Context->clear(0x00000000);
    }
    
    return true;
}

static JSBool native_canvas_setZoom(JSContext *cx, unsigned argc, jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    double zoom;

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "d",
        &zoom)) {
        return false;
    }

    NativeObject->setZoom(zoom);
    
    return true;
}

static JSBool native_canvas_setScale(JSContext *cx, unsigned argc, jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    double x, y;

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "dd",
        &x, &y)) {
        return false;
    }

    NativeObject->setScale(x, y);
    
    return true;
}

static JSBool native_canvas_setSize(JSContext *cx, unsigned argc, jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    int width, height;

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "ii",
        &width, &height)) {
        return false;
    }

    NativeObject->setSize(width, height);
    
    return true;
}

static JSBool native_canvas_removeFromParent(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeObject->removeFromParent();
    
    return true;
}

static JSBool native_canvas_bringToFront(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeObject->bringToFront();

    return true;
}

static JSBool native_canvas_sendToBack(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeObject->sendToBack();

    return true;
}

static JSBool native_canvas_getParent(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeCanvasHandler *parent = NativeObject->getParent();

    args.rval().set(parent ? OBJECT_TO_JSVAL(parent->jsobj) : JSVAL_NULL);

    return true;
}

static JSBool native_canvas_getFirstChild(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeCanvasHandler *val = NativeObject->getFirstChild();

    args.rval().set(val ? OBJECT_TO_JSVAL(val->jsobj) : JSVAL_NULL);

    return true;
}

static JSBool native_canvas_getLastChild(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeCanvasHandler *val = NativeObject->getLastChild();

    args.rval().set(val ? OBJECT_TO_JSVAL(val->jsobj) : JSVAL_NULL);

    return true;
}

static JSBool native_canvas_getNextSibling(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeCanvasHandler *val = NativeObject->getNextSibling();

    args.rval().set(val ? OBJECT_TO_JSVAL(val->jsobj) : JSVAL_NULL);

    return true;
}

static JSBool native_canvas_getPrevSibling(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeCanvasHandler *val = NativeObject->getPrevSibling();

    args.rval().set(val ? OBJECT_TO_JSVAL(val->jsobj) : JSVAL_NULL);

    return true;
}

static JSBool native_canvas_getChildren(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    int32_t count = NativeObject->countChildren();

    if (!count) {
        args.rval().set(OBJECT_TO_JSVAL(JS_NewArrayObject(cx, 0, NULL)));
        return true;
    }

    NativeCanvasHandler *list[count];
    jsval *jlist = (jsval *)malloc(sizeof(jsval) * count);

    NativeObject->getChildren(list);

    for (int i = 0; i < count; i++) {
        jlist[i] = OBJECT_TO_JSVAL(list[i]->jsobj);
    }

    args.rval().set(OBJECT_TO_JSVAL(JS_NewArrayObject(cx, count, jlist)));

    free(jlist);

    return true;
}

static JSBool native_canvas_getVisibleRect(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    NativeRect rect = NativeObject->getVisibleRect();
    JSObject *ret;

    ret = JS_NewObject(cx, NULL, NULL, NULL);

#define SET_PROP(where, name, val) JS_DefineProperty(cx, where, \
    (const char *)name, val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | \
        JSPROP_ENUMERATE)
    
    SET_PROP(ret, "left", DOUBLE_TO_JSVAL(rect.fLeft));
    SET_PROP(ret, "top", DOUBLE_TO_JSVAL(rect.fTop));
    SET_PROP(ret, "width", DOUBLE_TO_JSVAL(native_max(0, rect.fRight-rect.fLeft)));
    SET_PROP(ret, "height", DOUBLE_TO_JSVAL(native_max(0, rect.fBottom-rect.fTop)));
#undef SET_PROP

    args.rval().set(OBJECT_TO_JSVAL(ret));

    return true;
}

static JSBool native_canvas_setCoordinates(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    double left, top;

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "dd",
        &left, &top)) {
        return false;
    }

    NativeObject->left = left;
    NativeObject->top = top;

    return true;
}

static JSBool native_canvas_translate(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    double left, top;

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "dd",
        &left, &top)) {
        return false;
    }

    NativeObject->translate(left, top);

    return true;
}

static JSBool native_canvas_addSubCanvas(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    JSObject *sub;
    NativeCanvasHandler *handler = NULL;

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "o", &sub)) {
        return false;
    }

    if (!JS_InstanceOf(cx, sub, &Canvas_class, NULL)) {
        JS_ReportError(cx, "add() First parameter is not a Canvas Object");
        return false;
    }

    handler = (NativeCanvasHandler *)((NativeJSCanvas *)JS_GetPrivate(sub))->getHandler();

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

static JSBool native_canvas_insertBefore(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    JSObject *insert, *ref;

    NativeCanvasHandler *handler_insert = NULL, *handler_ref = NULL;

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "oo", &insert, &ref)) {
        return false;
    }

    if (!JS_InstanceOf(cx, insert, &Canvas_class, NULL)) {
        JS_ReportError(cx, "add() First parameter is not a Canvas Object");
        return false;
    }

    handler_insert = (NativeCanvasHandler *)((NativeJSCanvas *)JS_GetPrivate(insert))->getHandler();

    if (handler_insert == NULL) {
        return true;
    }

    if (JS_InstanceOf(cx, ref, &Canvas_class, NULL)) {
        handler_ref = (NativeCanvasHandler *)((NativeJSCanvas *)JS_GetPrivate(ref))->getHandler();
    }

    if (NativeObject == handler_insert) {
        JS_ReportError(cx, "Canvas: can't add to itself");
        return false;
    }

    NativeObject->insertBefore(handler_insert, handler_ref);

    return true;
}

static JSBool native_canvas_insertAfter(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);

    JSObject *insert, *ref;
    NativeCanvasHandler *handler_insert = NULL, *handler_ref = NULL;

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "oo", &insert, &ref)) {
        return false;
    }

    if (!JS_InstanceOf(cx, insert, &Canvas_class, NULL)) {
        JS_ReportError(cx, "add() First parameter is not a Canvas Object");
        return false;
    }

    handler_insert = (NativeCanvasHandler *)((NativeJSCanvas *)JS_GetPrivate(insert))->getHandler();

    if (handler_insert == NULL) {
        return true;
    }

    if (JS_InstanceOf(cx, ref, &Canvas_class, NULL)) {
        handler_ref = (NativeCanvasHandler *)((NativeJSCanvas *)JS_GetPrivate(ref))->getHandler();
    }

    if (NativeObject == handler_insert) {
        JS_ReportError(cx, "Canvas: can't add to itself");
        return false;
    }

    NativeObject->insertAfter(handler_insert, handler_ref);

    return true;
}

static JSBool native_canvas_getContext(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);
    NATIVE_CHECK_ARGS("getContext", 1);

    NativeContext *nctx = NativeContext::getNativeClass(cx);

    JSString *mode = args[0].toString();
    JSAutoByteString cmode(cx, mode);

    NativeUIInterface *ui = nctx->getUI();

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
                        NativeObject->getWidth() + (NativeObject->padding.global * 2),
                        NativeObject->getHeight() + (NativeObject->padding.global * 2), ui);
                
                if (ctx2d->getSurface() == NULL) {
                    delete ctx2d;
                    JS_ReportError(cx, "Could not create 2D context for this canvas");
                    return false;
                }
                NativeObject->setContext(ctx2d);
                ctx2d->setGLState(nctx->getGLState());
                
                break;
            }
            case NativeCanvasContext::CONTEXT_WEBGL:
                /*
                    TODO :
                    NativeObject->setContext(new NativeCanvasWebGLContext(...))
                */
                NativeCanvas3DContext *ctx3d = new NativeCanvas3DContext(NativeObject, cx,
                        NativeObject->getWidth() + (NativeObject->padding.global * 2),
                        NativeObject->getHeight() + (NativeObject->padding.global * 2), ui);

                NativeObject->setContext(ctx3d);
                break;
        }

        canvasctx = NativeObject->getContext();

        /*  Protect against GC
            Canvas.slot[0] = context
        */
        JS_SetReservedSlot(NativeObject->jsobj, 0, OBJECT_TO_JSVAL(NativeObject->getContext()->jsobj));
    } else if (canvasctx->m_Mode != ctxmode) {
        JS_ReportWarning(cx, "Bad context requested");
        /* A mode is requested but another one was already created */
        args.rval().setNull();
        return true;        
    }

    args.rval().set(OBJECT_TO_JSVAL(canvasctx->jsobj));

    return true;
}

static JSBool native_canvas_setContext(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NATIVE_PROLOGUE(NativeCanvasHandler);
    NATIVE_CHECK_ARGS("setContext", 1);

    JSObject *obj = args[0].toObjectOrNull();
    if (!obj) {
        return true;
    }

    NativeCanvasContext *context;

    if (!(context = (NativeCanvasContext *)JS_GetInstancePrivate(cx,
            obj, &Canvas2DContext_class, NULL))) {
        JS_ReportError(cx, "setContext() argument must a CanvasRenderingContext2D object");
        return false;
    }

    NativeObject->setContext(context);

    /*
        If a context was already attached, it's going to be GC'd
        since it's not logner reachable from slot 0.
    */
    JS_SetReservedSlot(NativeObject->jsobj, 0, OBJECT_TO_JSVAL(context->jsobj));

    return true;
}


/* TODO: do not change the value when a wrong type is set */
static JSBool native_canvas_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
    NativeCanvasHandler *handler = HANDLER_GETTER_SAFE(cx, obj.get());
    if (!handler) {
        return true;
    }

    switch(JSID_TO_INT(id)) {

        case CANVAS_PROP_POSITION:
        {
            if (!JSVAL_IS_STRING(vp)) {
                vp.set(JSVAL_VOID);

                return JS_TRUE;
            }
            JSAutoByteString mode(cx, JSVAL_TO_STRING(vp));
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
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToECMAUint32(cx, vp, &dval);

            if (!handler->setWidth(dval)) {
                //JS_ReportError(cx, "Can't set canvas width (this canvas has a dynamic width)");
                return true;
            }
        }
        break;
        case CANVAS_PROP_MINWIDTH:
        {
            uint32_t dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToECMAUint32(cx, vp, &dval);

            if (!handler->setMinWidth(dval)) {
                return true;
            }
        }
        break;
        case CANVAS_PROP_MAXWIDTH:
        {
            uint32_t dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToECMAUint32(cx, vp, &dval);

            if (!handler->setMaxWidth(dval)) {
                return true;
            }
        }
        break;
        case CANVAS_PROP_HEIGHT:
        {
            uint32_t dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToECMAUint32(cx, vp, &dval);
            
            if (!handler->setHeight(dval)) {
                //JS_ReportError(cx, "Can't set canvas height (this canvas has a dynamic height)");
                return true;
            }
        }
        break;
        case CANVAS_PROP_MINHEIGHT:
        {
            uint32_t dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToECMAUint32(cx, vp, &dval);

            if (!handler->setMinHeight(dval)) {
                return true;
            }
        }
        break;
        case CANVAS_PROP_MAXHEIGHT:
        {
            uint32_t dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToECMAUint32(cx, vp, &dval);

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

            if (JSVAL_IS_NULL(vp)) {
                handler->unsetLeft();
                return JS_TRUE;
            }
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &dval);
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

            if (JSVAL_IS_NULL(vp)) {
                handler->unsetRight();
                return JS_TRUE;
            }
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &dval);
            handler->setRight(dval);
        }
        break;
        case CANVAS_PROP_TOP:
        {
            double dval;
            if (!handler->hasStaticTop()) {
                return true;
            }
            if (JSVAL_IS_NULL(vp)) {
                handler->unsetTop();
                return JS_TRUE;
            }
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &dval);
            handler->setTop(dval);
        }
        break;
        case CANVAS_PROP_BOTTOM:
        {
            double dval;
            if (!handler->hasStaticBottom()) {
                return true;
            }
            if (JSVAL_IS_NULL(vp)) {
                handler->unsetBottom();
                return JS_TRUE;
            }
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &dval);
            handler->setBottom(dval);
        }
        break;
        case CANVAS_PROP_SCROLLLEFT:
        {
            int32_t dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }

            JS_ValueToECMAInt32(cx, vp, &dval);

            handler->setScrollLeft(dval);
        }
        break;
        case CANVAS_PROP_SCROLLTOP:
        {
            int32_t dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }

            JS_ValueToECMAInt32(cx, vp, &dval);

            handler->setScrollTop(dval);
        }
        break;
        case CANVAS_PROP_ALLOWNEGATIVESCROLL:
        {
            if (!JSVAL_IS_BOOLEAN(vp)) {
                return JS_TRUE;
            }

            handler->setAllowNegativeScroll(JSVAL_TO_BOOLEAN(vp));
        }
        break;
        case CANVAS_PROP_VISIBLE:
        {
            if (!JSVAL_IS_BOOLEAN(vp)) {
                return JS_TRUE;
            }

            handler->setHidden(!JSVAL_TO_BOOLEAN(vp));
        }
        break;
        case CANVAS_PROP_OVERFLOW:
        {
            if (!JSVAL_IS_BOOLEAN(vp)) {
                return JS_TRUE;
            }

            handler->m_Overflow = JSVAL_TO_BOOLEAN(vp);
        }
        break;
        case CANVAS_PROP_COATING:
        {
            int32_t dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
             JS_ValueToECMAInt32(cx, vp, &dval);

            handler->setPadding(dval);
        }
        break;
        case CANVAS_PROP_OPACITY:
        {
            double dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &dval);
            handler->setOpacity(dval);      
        }
        break;
        case CANVAS_PROP_STATICLEFT:
        {
            if (!vp.isBoolean()) {
                return true;
            }

            if (vp.toBoolean()) {
                handler->setLeft(handler->left);
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
                handler->setRight(handler->right);
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
                handler->setTop(handler->top);
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
                handler->setBottom(handler->bottom);
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
            JSString *sid = JS_ValueToString(cx, vp);
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
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }

            JS_ValueToNumber(cx, vp, &dval);
            handler->setMargin(handler->m_Margin.top, handler->m_Margin.right,
                handler->m_Margin.bottom, dval);
        }
        break;
        case CANVAS_PROP_MARGINRIGHT:
        {
            double dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }

            JS_ValueToNumber(cx, vp, &dval);
            handler->setMargin(handler->m_Margin.top, dval,
                handler->m_Margin.bottom, handler->m_Margin.left);
        }
        break;
        case CANVAS_PROP_MARGINTOP:
        {
            double dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }

            JS_ValueToNumber(cx, vp, &dval);

            handler->setMargin(dval, handler->m_Margin.right,
                handler->m_Margin.bottom, handler->m_Margin.left);
        }
        break;
        case CANVAS_PROP_MARGINBOTTOM:
        {
            double dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }

            JS_ValueToNumber(cx, vp, &dval);
            
            handler->setMargin(handler->m_Margin.top, handler->m_Margin.right,
                dval, handler->m_Margin.left);
        }
        break;
        case CANVAS_PROP_CURSOR:
        {
            if (!JSVAL_IS_STRING(vp)) {
                return true;
            }
            JSAutoByteString type(cx, JSVAL_TO_STRING(vp));

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

    return JS_TRUE;
}

static JSBool native_canvas_prop_get(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSMutableHandleValue vp)
{
    NativeCanvasHandler *handler = HANDLER_GETTER_SAFE(cx, obj.get());
    if (!handler) {
        return true;
    }

    switch(JSID_TO_INT(id)) {
        case CANVAS_PROP_OPACITY:
            vp.set(DOUBLE_TO_JSVAL(handler->getOpacity()));
            break;
        case CANVAS_PROP_WIDTH:
            vp.set(INT_TO_JSVAL(handler->getWidth()));
            break;
        case CANVAS_PROP_MINWIDTH:
            vp.set(INT_TO_JSVAL(handler->getMinWidth()));
            break;
        case CANVAS_PROP_MAXWIDTH:
            vp.set(INT_TO_JSVAL(handler->getMaxWidth()));
            break;
        case CANVAS_PROP_CLIENTWIDTH:
            vp.set(INT_TO_JSVAL(handler->getWidth() + (handler->padding.global * 2)));
            break;
        case CANVAS_PROP_HEIGHT:
            vp.set(INT_TO_JSVAL(handler->getHeight()));
            break;
        case CANVAS_PROP_MINHEIGHT:
            vp.set(INT_TO_JSVAL(handler->getMinHeight()));
            break;
        case CANVAS_PROP_MAXHEIGHT:
            vp.set(INT_TO_JSVAL(handler->getMaxHeight()));
            break;
        case CANVAS_PROP_CLIENTHEIGHT:
            vp.set(INT_TO_JSVAL(handler->getHeight() + (handler->padding.global * 2)));
            break;
        case CANVAS_PROP_COATING:
            vp.set(INT_TO_JSVAL(handler->padding.global));
            break;
        case CANVAS_PROP_LEFT:
            vp.set(DOUBLE_TO_JSVAL(handler->getLeft()));
            break;
        case CANVAS_PROP_RIGHT:
            vp.set(DOUBLE_TO_JSVAL(handler->getRight()));
            break;
        case CANVAS_PROP_BOTTOM:
            vp.set(DOUBLE_TO_JSVAL(handler->getBottom()));
            break;
        case CANVAS_PROP_CLIENTLEFT:
            vp.set(INT_TO_JSVAL(handler->getLeft() - handler->padding.global));
            break;
        case CANVAS_PROP_TOP:
            vp.set(DOUBLE_TO_JSVAL(handler->getTop()));
            break;
        case CANVAS_PROP_CLIENTTOP:
            vp.set(INT_TO_JSVAL(handler->getTop() - handler->padding.global));
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
            vp.set(BOOLEAN_TO_JSVAL(!handler->isHidden()));
            break;
        case CANVAS_PROP_OVERFLOW:
            vp.set(BOOLEAN_TO_JSVAL(handler->m_Overflow));
            break;
        case CANVAS_PROP___VISIBLE:
            vp.set(BOOLEAN_TO_JSVAL(handler->isDisplayed()));
            break;
        case CANVAS_PROP_CONTENTWIDTH:
            vp.set(INT_TO_JSVAL(handler->getContentWidth()));
            break;
        case CANVAS_PROP_CONTENTHEIGHT:
            vp.set(INT_TO_JSVAL(handler->getContentHeight()));
            break;
        case CANVAS_PROP_INNERWIDTH:
            vp.set(INT_TO_JSVAL(handler->getContentWidth(true)));
            break;
        case CANVAS_PROP_INNERHEIGHT:
            vp.set(INT_TO_JSVAL(handler->getContentHeight(true)));
            break;
        case CANVAS_PROP_SCROLLLEFT:
            vp.set(INT_TO_JSVAL(handler->content.scrollLeft));
            break;
        case CANVAS_PROP_SCROLLTOP:
            vp.set(INT_TO_JSVAL(handler->content.scrollTop));
            break;
        case CANVAS_PROP_ALLOWNEGATIVESCROLL:
            vp.set(BOOLEAN_TO_JSVAL(handler->getAllowNegativeScroll()));
            break;
        case CANVAS_PROP___FIXED:
            vp.set(BOOLEAN_TO_JSVAL(handler->hasAFixedAncestor()));
            break;
        case CANVAS_PROP___TOP:
        {
            handler->computeAbsolutePosition();
            vp.set(DOUBLE_TO_JSVAL(handler->getTop(true)));
            break;
        }
        case CANVAS_PROP___LEFT:
        {
            handler->computeAbsolutePosition();
            vp.set(DOUBLE_TO_JSVAL(handler->getLeft(true)));
            break;
        }
        case CANVAS_PROP_CTX:
        {
            if (handler->m_Context == NULL) {
                vp.set(JSVAL_NULL);
                break;
            }
            vp.set(OBJECT_TO_JSVAL(handler->m_Context->jsobj));
            break;
        }
        case CANVAS_PROP___OUTOFBOUND:
        {
            vp.set(BOOLEAN_TO_JSVAL(handler->isOutOfBound()));
            break;
        }
        case CANVAS_PROP_POSITION:
        {
            switch (handler->getPositioning()) {
                case NativeCanvasHandler::COORD_RELATIVE:
                    if (handler->getFlowMode() & NativeCanvasHandler::kFlowBreakPreviousSibling) {
                        vp.set(STRING_TO_JSVAL(JS_NewStringCopyZ(cx, "inline-break")));
                    } else if (handler->getFlowMode() & NativeCanvasHandler::kFlowInlinePreviousSibling) {
                        vp.set(STRING_TO_JSVAL(JS_NewStringCopyZ(cx, "inline")));
                    } else {
                        vp.set(STRING_TO_JSVAL(JS_NewStringCopyZ(cx, "relative")));
                    }
                    break;
                case NativeCanvasHandler::COORD_ABSOLUTE:
                    vp.set(STRING_TO_JSVAL(JS_NewStringCopyZ(cx, "absolute")));
                    break;
                case NativeCanvasHandler::COORD_FIXED:
                    vp.set(STRING_TO_JSVAL(JS_NewStringCopyZ(cx, "fixed")));
                    break;
                case NativeCanvasHandler::COORD_INLINE:
                    vp.set(STRING_TO_JSVAL(JS_NewStringCopyZ(cx, "inline")));
                    break;
                case NativeCanvasHandler::COORD_INLINEBREAK:
                    vp.set(STRING_TO_JSVAL(JS_NewStringCopyZ(cx, "inline-break")));
                    break; 
            }
            break;
        }
        case CANVAS_PROP_ID:
        {
            char *id;
            uint64_t numid = handler->getIdentifier(&id);

            vp.setString(JS_NewStringCopyZ(cx, id));

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

    return JS_TRUE;
}

static JSBool native_Canvas_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    int width, height;
    NativeCanvasHandler *handler;
    JSObject *opt = NULL;

    if (!JS_IsConstructing(cx, vp)) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    JSObject *ret = JS_NewObjectForConstructor(cx, &Canvas_class, vp);

    JSObject *inherit = JS_DefineObject(cx, ret, "inherit", &Canvas_Inherit_class, NULL,
        JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ii/o",
        &width, &height, &opt)) {
        return false;
    }

    bool lazyLoad = false;

    JS_INITOPT();

    /*
        Unused
    */
    JSGET_OPT_TYPE(opt, "lazy", Boolean) {
        lazyLoad = __curopt.toBoolean();
    }
    /*
        Always lazy load for now.

    */
    handler = new NativeCanvasHandler(width, height,
        NativeContext::getNativeClass(cx), true);

    handler->m_Context = NULL;
    handler->jsobj = ret;
    handler->jscx = cx;

    NativeJSCanvas *jscanvas = new NativeJSCanvas(ret, cx, handler);
    jscanvas->setInherit(inherit);

    JS_SetPrivate(ret, jscanvas);
    JS_SetPrivate(inherit, jscanvas);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

void Canvas_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSCanvas *jscanvas = ((class NativeJSCanvas *)JS_GetPrivate(obj));

    if (jscanvas != NULL) {
        delete jscanvas;
    }
}

#ifdef DEBUG
static void PrintGetTraceName(JSTracer* trc, char *buf, size_t bufsize)
{
    snprintf(buf, bufsize, "[0x%p].mJSVal", trc->debugPrintArg);
}
#endif

static void Canvas_Trace(JSTracer *trc, JSRawObject obj)
{
    NativeCanvasHandler *handler = HANDLER_GETTER(obj);

    if (handler != NULL) {
        NativeCanvasHandler *cur;

        for (cur = handler->getFirstChild(); cur != NULL; cur = cur->m_Next) {
            if (cur->jsobj) {
#ifdef DEBUG
                JS_SET_TRACING_DETAILS(trc, PrintGetTraceName, cur, 0);
#endif

                JS_CallObjectTracer(trc, (JSObject *)cur->jsobj, "nativecanvasroot");
            }
        }
    }
}

JSObject *NativeJSCanvas::generateJSObject(JSContext *cx, int width, 
    int height, NativeCanvasHandler **out)
{
    JSObject *ret;
    NativeCanvasHandler *handler;
    NativeContext *nctx = NativeContext::getNativeClass(cx);
    NativeUIInterface *ui = nctx->getUI();

    ret = JS_NewObject(cx, &Canvas_class, NULL, NULL);
    JSObject *inherit = JS_DefineObject(cx, ret, "inherit", &Canvas_Inherit_class, NULL,
        JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);

    handler = new NativeCanvasHandler(width, height, nctx);
    handler->setContext(new NativeCanvas2DContext(handler, cx, width, height, ui));
    handler->getContext()->setGLState(nctx->getGLState());

    /* window.canvas.overflow default to false */
    handler->m_Overflow = false;

    handler->jsobj = ret;
    handler->jscx = cx;

    JS_SetReservedSlot(ret, 0, OBJECT_TO_JSVAL(handler->m_Context->jsobj));

    NativeJSCanvas *jscanvas = new NativeJSCanvas(ret, cx, handler);

    jscanvas->setInherit(inherit);

    JS_SetPrivate(ret, jscanvas);
    JS_SetPrivate(inherit, jscanvas);

    *out = handler;

    return ret;
}

void NativeJSCanvas::registerObject(JSContext *cx)
{
    JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, &Canvas_class,
        native_Canvas_constructor,
        2, canvas_props, canvas_funcs, NULL, NULL);
}

void NativeJSCanvas::onMessage(const NativeSharedMessages::Message &msg)
{
    JSContext *cx = m_Cx;
    switch (msg.event()) {
        case NATIVE_EVENT(NativeCanvasHandler, RESIZE_EVENT):
        {
            JSOBJ_CALLFUNCNAME(m_JSObject, "onresize", 0, NULL);
            break;
        }
        case NATIVE_EVENT(NativeCanvasHandler, LOADED_EVENT):
        {
            JSOBJ_CALLFUNCNAME(m_JSObject, "onload", 0, NULL);
            break;
        }
        case NATIVE_EVENT(NativeCanvasHandler, CHANGE_EVENT):
        {
            JSObject *ev = JS_NewObject(cx, NULL, NULL, NULL);
            const char *name = NULL;
            JS::Value value, arg;

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

            JSOBJ_SET_PROP_CSTR(ev, "property", name);
            JSOBJ_SET_PROP(ev, "value", value);

            arg.setObjectOrNull(ev);

            JSOBJ_CALLFUNCNAME(m_JSObject, "onchange", 1, &arg);
            break;
        }
        case NATIVE_EVENT(NativeCanvasHandler, DRAG_EVENT):
        {
            printf("Drag event detected\n");
        } 
        case NATIVE_EVENT(NativeCanvasHandler, MOUSE_EVENT):
        {
            NativeJSObjectBuilder obj = NativeJSObjectBuilder(m_Cx,
                NativeJSEvents::CreateEventObject(m_Cx));

            NativeCanvasHandler *target = (NativeCanvasHandler *)msg.args[8].toPtr();

            obj.set("x", msg.args[2].toInt());
            obj.set("y", msg.args[3].toInt());
            obj.set("clientX", msg.args[2].toInt());
            obj.set("clientY", msg.args[3].toInt());
            obj.set("layerX", msg.args[6].toInt());
            obj.set("layerY", msg.args[7].toInt());
            obj.set("target", OBJECT_TO_JSVAL(target->jsobj));

            switch ((NativeInputEvent::Type)msg.args[1].toInt()) {
                case NativeInputEvent::kMouseClick_Type:
                case NativeInputEvent::kMouseDoubleClick_Type:
                case NativeInputEvent::kMouseClickRelease_Type:
                    obj.set("which", msg.args[4].toInt());
                    break;
                case NativeInputEvent::kMouseMove_Type:
                    obj.set("xrel", msg.args[4].toInt());
                    obj.set("yrel", msg.args[5].toInt());
                    break;
                case NativeInputEvent::kMouseDragOver_Type:
                case NativeInputEvent::kMouseDragStart_Type:
                case NativeInputEvent::kMouseDragEnd_Type:
                    obj.set("source", OBJECT_TO_JSVAL(this->getJSObject()));
                    break;
                case NativeInputEvent::kMouseDrop_Type:
                    obj.set("source", OBJECT_TO_JSVAL(target->jsobj));
                    obj.set("target", OBJECT_TO_JSVAL(this->getJSObject()));
                    break;
                default:
                    break;
            }

            if (!this->fireJSEvent(NativeInputEvent::getName(msg.args[1].toInt()),
                obj.jsval())) {

                break;
            }

            JS::Value cancelBubble;
            if (JS_GetProperty(cx, obj.obj(), "cancelBubble", &cancelBubble)) {
                if (cancelBubble.isBoolean() && cancelBubble.toBoolean()) {
                    /* TODO: sort out this dirty hack */
                    NativeSharedMessages::Message *nonconstmsg = (NativeSharedMessages::Message *)&msg;
                    nonconstmsg->priv = 1;
                }
            }
        }
        default:
            break;
    }
}

void NativeJSCanvas::onMessageLost(const NativeSharedMessages::Message &msg)
{

}

NativeJSCanvas::NativeJSCanvas(JSObject *obj, JSContext *cx,
    NativeCanvasHandler *handler) :
    NativeJSExposer<NativeJSCanvas>(obj, cx),
    m_CanvasHandler(handler), m_Inherit(NULL)
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