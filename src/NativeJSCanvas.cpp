#include "NativeJSCanvas.h"
#include "NativeSkia.h"
#include "NativeSkGradient.h"
#include "NativeSkImage.h"
#include "NativeJSImage.h"
#include "NativeCanvas2DContext.h"

#define CANVASCTX_GETTER(obj) ((class NativeCanvas2DContext *)JS_GetPrivate(obj))
#define NSKIA_NATIVE_GETTER(obj) ((class NativeSkia *)((class NativeCanvas2DContext *)JS_GetPrivate(obj))->skia)
#define NSKIA_NATIVE ((class NativeSkia *)((class NativeCanvas2DContext *)JS_GetPrivate(JS_GetParent(JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)))))->jsobj)
#define CANVASCTX_FROM_CALLEE ((class NativeCanvas2DContext *)JS_GetPrivate(JS_GetParent(JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)))))

extern jsval gfunc;

enum {
    CANVAS_PROP_WIDTH = 1,
    CANVAS_PROP_HEIGHT,
    CANVAS_PROP_POSITION
};

static void Canvas_Finalize(JSFreeOp *fop, JSObject *obj);

static JSClass Canvas_class = {
    "Canvas", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Canvas_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSBool native_canvas_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp);
static JSBool native_canvas_prop_get(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSMutableHandleValue vp);

static JSBool native_canvas_getContext(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_addSubCanvas(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_setPosition(JSContext *cx, unsigned argc,
    jsval *vp);

static JSPropertySpec canvas_props[] = {
    {"width", CANVAS_PROP_WIDTH, JSPROP_PERMANENT,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"height", CANVAS_PROP_HEIGHT, JSPROP_PERMANENT,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"position", CANVAS_PROP_POSITION, JSPROP_PERMANENT,
        JSOP_NULLWRAPPER, JSOP_WRAPPER(native_canvas_prop_set)},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSFunctionSpec canvas_funcs[] = {
    JS_FN("getContext", native_canvas_getContext, 1, 0),
    JS_FN("addSubCanvas", native_canvas_addSubCanvas, 1, 0),
    JS_FN("setPosition", native_canvas_setPosition, 2, 0),
    JS_FS_END
};

static JSBool native_canvas_addSubCanvas(JSContext *cx, unsigned argc,
    jsval *vp)
{
    JSObject *sub;
    NativeSkia *subskia;
    double left = 0.0, top = 0.0;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o/dd", &sub,
        &left, &top)) {
        return JS_TRUE;
    }

    if (!JS_InstanceOf(cx, sub, &Canvas_class, NULL)) {
        return JS_TRUE;
    }

    subskia = (NativeSkia *)JS_GetPrivate(sub);

    if (subskia == NULL) {
        return JS_TRUE;
    }

    if (NSKIA_NATIVE == subskia) {
        printf("Cant add canvas to itself\n");
        return JS_TRUE;
    }

    subskia->setPosition(left, top);

    NSKIA_NATIVE->addSubCanvas(subskia);

    return JS_TRUE;
}

static JSBool native_canvas_setPosition(JSContext *cx, unsigned argc,
    jsval *vp)
{
    double left = 0.0, top = 0.0;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dd", &left, &top)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->setPosition(left, top);

    return JS_TRUE;
}

static JSBool native_canvas_getContext(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NativeCanvas2DContext *canvasctx = CANVASCTX_FROM_CALLEE;

    if (canvasctx == NULL) {
        printf("Cant get context\n");
        return JS_TRUE;
    }

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(canvasctx->jsobj));

    return JS_TRUE;
}

/* TODO: do not change the value when a wrong type is set */
static JSBool native_canvas_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
    NativeSkia *curSkia = NSKIA_NATIVE_GETTER(obj.get());

    switch(JSID_TO_INT(id)) {

        case CANVAS_PROP_POSITION:
        {
            if (!JSVAL_IS_STRING(vp)) {
                vp.set(JSVAL_VOID);

                return JS_TRUE;
            }
            JSAutoByteString mode(cx, JSVAL_TO_STRING(vp));
            if (strcasecmp(mode.ptr(), "absolute") == 0) {
                curSkia->setPositioning(NativeCanvasHandler::COORD_ABSOLUTE);
            } else {
                curSkia->setPositioning(NativeCanvasHandler::COORD_RELATIVE);
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
    NativeSkia *curSkia = NSKIA_NATIVE_GETTER(obj.get());

    switch(JSID_TO_INT(id)) {
        case CANVAS_PROP_WIDTH:
        {
            vp.set(INT_TO_JSVAL(curSkia->getWidth()));
        }
        break;
        case CANVAS_PROP_HEIGHT:
        {
            vp.set(INT_TO_JSVAL(curSkia->getHeight()));
        }
        break;
        default:
            break;
    }

    return JS_TRUE;
}

static JSBool native_Canvas_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    int width, height;
    NativeCanvas2DContext *canvasctx;

    JSObject *ret = JS_NewObjectForConstructor(cx, &Canvas_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ii",
        &width, &height)) {
        return JS_TRUE;
    }

    canvasctx = new NativeCanvas2DContext(cx, width, height);

    /* Retain a ref to this, so that we are sure we can't get an undefined ctx */
    JS_AddObjectRoot(cx, &canvasctx->jsobj);

    JS_SetPrivate(ret, canvasctx);

    /* TODO: JS_IsConstructing() */
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

void Canvas_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeCanvas2DContext *canvasctx = CANVASCTX_GETTER(obj);
    if (canvasctx != NULL) {
        JS_RemoveObjectRoot(canvasctx->jscx, &canvasctx->jsobj);

        /* Don't delete canvasctx, otherwise
           ctx->jsobj's private would be undefined
        */
    }
}

void CanvasGradient_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeSkGradient *gradient = (class NativeSkGradient *)JS_GetPrivate(obj);
    if (gradient != NULL) {
        delete gradient;
    }
}

JSObject *NativeJSCanvas::generateJSObject(JSContext *cx, NativeSkia *skia)
{
    JSObject *ret;

    if (skia->cx && skia->obj) {
        printf("Warning generate already existing canvas obj\n");
        return skia->obj;
    }

    ret = JS_NewObject(cx, &Canvas_class, NULL, NULL);

    skia->obj = ret;
    skia->cx = cx;

    JS_DefineFunctions(cx, ret, canvas_funcs);
    JS_DefineProperties(cx, ret, canvas_props);

    JS_SetPrivate(ret, skia);

    /* Removed in NativeSkia destructor */
    JS_AddObjectRoot(cx, &skia->obj);

    return ret;
}

void NativeJSCanvas::registerObject(JSContext *cx)
{
    JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, &Canvas_class,
        native_Canvas_constructor,
        2, NULL, NULL, NULL, NULL);
}

