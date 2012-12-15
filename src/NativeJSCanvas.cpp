#include "NativeJSCanvas.h"
#include "NativeSkia.h"
#include "NativeCanvas2DContext.h"
#include "NativeCanvasHandler.h"

#define HANDLER_GETTER(obj) ((class NativeCanvasHandler *)JS_GetPrivate(obj))
#define HANDLER_FROM_CALLEE ((class NativeCanvasHandler *)JS_GetPrivate(JS_GetParent(JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)))))

extern jsval gfunc;

enum {
    CANVAS_PROP_WIDTH = 1,
    CANVAS_PROP_HEIGHT,
    CANVAS_PROP_POSITION
};

static void Canvas_Finalize(JSFreeOp *fop, JSObject *obj);

JSClass Canvas_class = {
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
    NativeCanvasHandler *handler;
    double left = 0.0, top = 0.0;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o/dd", &sub,
        &left, &top)) {
        return JS_TRUE;
    }

    if (!JS_InstanceOf(cx, sub, &Canvas_class, NULL)) {
        return JS_TRUE;
    }

    handler = (NativeCanvasHandler *)JS_GetPrivate(sub);

    if (handler == NULL) {
        return JS_TRUE;
    }

    if (HANDLER_FROM_CALLEE == handler) {
        printf("Cant add canvas to itself\n");
        return JS_TRUE;
    }

    handler->setPosition(left, top);

    HANDLER_FROM_CALLEE->addChild(handler);

    return JS_TRUE;
}

static JSBool native_canvas_setPosition(JSContext *cx, unsigned argc,
    jsval *vp)
{
    double left = 0.0, top = 0.0;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dd", &left, &top)) {
        return JS_TRUE;
    }

    HANDLER_FROM_CALLEE->setPosition(left, top);

    return JS_TRUE;
}

static JSBool native_canvas_getContext(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NativeCanvas2DContext *canvasctx = HANDLER_FROM_CALLEE->context;

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
    NativeCanvasHandler *handler = HANDLER_GETTER(obj.get());

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
            } else {
                handler->setPositioning(NativeCanvasHandler::COORD_RELATIVE);
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
    NativeCanvasHandler *handler = HANDLER_GETTER(obj.get());

    switch(JSID_TO_INT(id)) {
        case CANVAS_PROP_WIDTH:
        {
            vp.set(INT_TO_JSVAL(handler->width));
        }
        break;
        case CANVAS_PROP_HEIGHT:
        {
            vp.set(INT_TO_JSVAL(handler->height));
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
    NativeCanvasHandler *handler;

    JSObject *ret = JS_NewObjectForConstructor(cx, &Canvas_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ii",
        &width, &height)) {
        return JS_TRUE;
    }

    handler = new NativeCanvasHandler(width, height);
    handler->context = new NativeCanvas2DContext(cx, width, height);

    /* Retain a ref to this, so that we are sure we can't get an undefined ctx */
    JS_AddObjectRoot(cx, &handler->context->jsobj);
    JS_SetPrivate(ret, handler);

    JS_DefineFunctions(cx, ret, canvas_funcs);
    JS_DefineProperties(cx, ret, canvas_props);    

    /* TODO: JS_IsConstructing() */
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

void Canvas_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeCanvasHandler *handler = HANDLER_GETTER(obj);
    if (handler != NULL) {
        delete handler;
    }
}

/* TODO: nuke this */
JSObject *NativeJSCanvas::generateJSObject(JSContext *cx, int width, int height)
{
    JSObject *ret;
    NativeCanvasHandler *handler;

    ret = JS_NewObject(cx, &Canvas_class, NULL, NULL);

/*
    skia->obj = ret;
    skia->cx = cx;
*/
    handler = new NativeCanvasHandler(width, height);
    handler->context = new NativeCanvas2DContext(cx, width, height);

    JS_AddObjectRoot(cx, &handler->context->jsobj);
    JS_SetPrivate(ret, handler);

    JS_DefineFunctions(cx, ret, canvas_funcs);
    JS_DefineProperties(cx, ret, canvas_props);


    /* Removed in NativeSkia destructor */
    //JS_AddObjectRoot(cx, &skia->obj);

    return ret;
}

void NativeJSCanvas::registerObject(JSContext *cx)
{
    JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, &Canvas_class,
        native_Canvas_constructor,
        2, NULL, NULL, NULL, NULL);
}

