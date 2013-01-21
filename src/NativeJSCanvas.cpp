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
    CANVAS_PROP_POSITION,
    /* relative positions */
    CANVAS_PROP_TOP,
    CANVAS_PROP_LEFT,
    /* .show()/.hide() */
    CANVAS_PROP_VISIBLE,
    /* Element is going to be drawn */
    CANVAS_PROP___VISIBLE,
    /* Absolute positions */
    CANVAS_PROP___TOP,
    CANVAS_PROP___LEFT,
    /* conveniance getter for getContext("2D") */
    CANVAS_PROP_CTX,
    CANVAS_PROP_PADDING,
    CANVAS_PROP_CLIENTLEFT,
    CANVAS_PROP_CLIENTTOP,
    CANVAS_PROP_CLIENTWIDTH,
    CANVAS_PROP_CLIENTHEIGHT,
    CANVAS_PROP_OPACITY,
    CANVAS_PROP_OVERFLOW,
    CANVAS_PROP_CONTENTWIDTH,
    CANVAS_PROP_CONTENTHEIGHT,
    CANVAS_PROP_SCROLLTOP,
    CANVAS_PROP_SCROLLLEFT,
    CANVAS_PROP___FIXED
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
static JSBool native_canvas_removeFromParent(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_bringToFront(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_sendToBack(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_getParent(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_getChildren(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_show(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_hide(JSContext *cx, unsigned argc, jsval *vp);

static JSPropertySpec canvas_props[] = {
    {"opacity", CANVAS_PROP_OPACITY, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},
    {"overflow", CANVAS_PROP_OVERFLOW, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},  
    {"scrollLeft", CANVAS_PROP_SCROLLLEFT, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},
    {"scrollTop", CANVAS_PROP_SCROLLTOP, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},
    {"width", CANVAS_PROP_WIDTH, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},
    {"clientWidth", CANVAS_PROP_CLIENTWIDTH, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"clientHeight", CANVAS_PROP_CLIENTHEIGHT, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"clientTop", CANVAS_PROP_CLIENTTOP, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"clientLeft", CANVAS_PROP_CLIENTLEFT, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_NULLWRAPPER},
    {"padding", CANVAS_PROP_PADDING, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},

    {"height", CANVAS_PROP_HEIGHT, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get),
        JSOP_WRAPPER(native_canvas_prop_set)},

    {"position", CANVAS_PROP_POSITION, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_NULLWRAPPER, JSOP_WRAPPER(native_canvas_prop_set)},

    {"top", CANVAS_PROP_TOP, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},

    {"left", CANVAS_PROP_LEFT, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},

    {"visible", CANVAS_PROP_VISIBLE, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_WRAPPER(native_canvas_prop_set)},
    {"contentWidth", CANVAS_PROP_CONTENTWIDTH, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},
    {"contentHeight", CANVAS_PROP_CONTENTHEIGHT, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},
    {"__visible", CANVAS_PROP___VISIBLE, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},
    {"__top", CANVAS_PROP___TOP, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},
    {"__left", CANVAS_PROP___LEFT, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},
    {"__fixed", CANVAS_PROP___FIXED, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},
    {"ctx", CANVAS_PROP_CTX, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_canvas_prop_get), JSOP_NULLWRAPPER},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSFunctionSpec canvas_funcs[] = {
    JS_FN("getContext", native_canvas_getContext, 1, 0),
    JS_FN("add", native_canvas_addSubCanvas, 1, 0),
    JS_FN("removeFromParent", native_canvas_removeFromParent, 0, 0),
    JS_FN("show", native_canvas_show, 0, 0),
    JS_FN("hide", native_canvas_hide, 0, 0),
    JS_FN("bringToFront", native_canvas_bringToFront, 0, 0),
    JS_FN("sendToBack", native_canvas_sendToBack, 0, 0),
    JS_FN("getParent", native_canvas_getParent, 0, 0),
    JS_FN("getChildren", native_canvas_getChildren, 0, 0),
    JS_FS_END
};

static JSBool native_canvas_show(JSContext *cx, unsigned argc, jsval *vp)
{
    HANDLER_FROM_CALLEE->setHidden(false);

    return JS_TRUE;
}

static JSBool native_canvas_hide(JSContext *cx, unsigned argc, jsval *vp)
{
    HANDLER_FROM_CALLEE->setHidden(true);
    
    return JS_TRUE;
}

static JSBool native_canvas_removeFromParent(JSContext *cx, unsigned argc,
    jsval *vp)
{
    HANDLER_FROM_CALLEE->removeFromParent();
    
    return JS_TRUE;
}

static JSBool native_canvas_bringToFront(JSContext *cx, unsigned argc,
    jsval *vp)
{
    HANDLER_FROM_CALLEE->bringToFront();

    return JS_TRUE;
}

static JSBool native_canvas_sendToBack(JSContext *cx, unsigned argc,
    jsval *vp)
{
    HANDLER_FROM_CALLEE->sendToBack();

    return JS_TRUE;
}

static JSBool native_canvas_getParent(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NativeCanvasHandler *parent = HANDLER_FROM_CALLEE->getParent();

    if (parent == NULL) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(parent->jsobj));

    return JS_TRUE;
}

static JSBool native_canvas_getChildren(JSContext *cx, unsigned argc,
    jsval *vp)
{
    int32_t count = HANDLER_FROM_CALLEE->countChildren();

    if (!count) {
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(JS_NewArrayObject(cx, 0, NULL)));
        return JS_TRUE;
    }

    NativeCanvasHandler *list[count];
    jsval *jlist = (jsval *)malloc(sizeof(jsval) * count);

    HANDLER_FROM_CALLEE->getChildren(list);

    for (int i = 0; i < count; i++) {
        jlist[i] = OBJECT_TO_JSVAL(list[i]->jsobj);
    }

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(JS_NewArrayObject(cx, count, jlist)));

    free(jlist);
    return JS_TRUE;
}

static JSBool native_canvas_addSubCanvas(JSContext *cx, unsigned argc,
    jsval *vp)
{
    JSObject *sub;
    NativeCanvasHandler *handler;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &sub)) {
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

    HANDLER_FROM_CALLEE->addChild(handler);

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
            } else if(strcasecmp(mode.ptr(), "fixed") == 0) {
                handler->setPositioning(NativeCanvasHandler::COORD_FIXED);
            } else {
                handler->setPositioning(NativeCanvasHandler::COORD_RELATIVE);
            }
        }    
        break;
        case CANVAS_PROP_WIDTH:
        {
            double dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &dval);

            handler->setWidth((int)dval);
        }
        break;
        case CANVAS_PROP_HEIGHT:
        {
            double dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &dval);
            
            handler->setHeight((int)dval);
        }
        break;
        case CANVAS_PROP_LEFT:
        {
            double dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &dval);
            handler->left = dval;
        }
        break;
        case CANVAS_PROP_TOP:
        {
            double dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &dval);
            handler->top = dval;
        }
        break;
        case CANVAS_PROP_SCROLLLEFT:
        {
            double dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &dval);
            handler->setScrollLeft((int)dval);
        }
        break;
        case CANVAS_PROP_SCROLLTOP:
        {
            double dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &dval);
            handler->setScrollTop((int)dval);
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

            handler->overflow = JSVAL_TO_BOOLEAN(vp);
        }
        break;
        case CANVAS_PROP_PADDING:
        {
            double dval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &dval);

            handler->setPadding((int)dval);
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
        case CANVAS_PROP_OPACITY:
            vp.set(DOUBLE_TO_JSVAL(handler->opacity));
            break;
        case CANVAS_PROP_WIDTH:
            vp.set(INT_TO_JSVAL(handler->width));
            break;
        case CANVAS_PROP_CLIENTWIDTH:
            vp.set(INT_TO_JSVAL(handler->width + (handler->padding.global * 2)));
            break;
        case CANVAS_PROP_HEIGHT:
            vp.set(INT_TO_JSVAL(handler->height));
            break;
        case CANVAS_PROP_CLIENTHEIGHT:
            vp.set(INT_TO_JSVAL(handler->height + (handler->padding.global * 2)));
            break;
        case CANVAS_PROP_PADDING:
            vp.set(INT_TO_JSVAL(handler->padding.global));
            break;
        case CANVAS_PROP_LEFT:
            vp.set(DOUBLE_TO_JSVAL(handler->left));
            break;
        case CANVAS_PROP_CLIENTLEFT:
            vp.set(INT_TO_JSVAL(handler->left - handler->padding.global));
            break;
        case CANVAS_PROP_TOP:
            vp.set(DOUBLE_TO_JSVAL(handler->top));
            break;
        case CANVAS_PROP_CLIENTTOP:
            vp.set(INT_TO_JSVAL(handler->top - handler->padding.global));
            break;
        case CANVAS_PROP_VISIBLE:
            vp.set(BOOLEAN_TO_JSVAL(!handler->isHidden()));
            break;
        case CANVAS_PROP_OVERFLOW:
            vp.set(BOOLEAN_TO_JSVAL(handler->overflow));
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
        case CANVAS_PROP_SCROLLLEFT:
            vp.set(INT_TO_JSVAL(handler->content.scrollLeft));
            break;
        case CANVAS_PROP_SCROLLTOP:
            vp.set(INT_TO_JSVAL(handler->content.scrollTop));
            break;
        case CANVAS_PROP___FIXED:
            vp.set(JSVAL_NULL);
            break;
        case CANVAS_PROP___TOP:
        {
            handler->computeAbsolutePosition();
            vp.set(DOUBLE_TO_JSVAL(handler->a_top));
            break;
        }
        case CANVAS_PROP___LEFT:
        {
            handler->computeAbsolutePosition();
            vp.set(DOUBLE_TO_JSVAL(handler->a_left));
            break;
        }
        case CANVAS_PROP_CTX:
        {
            if (handler->context == NULL) {
                vp.set(JSVAL_NULL);
                break;
            }
            vp.set(OBJECT_TO_JSVAL(handler->context->jsobj));
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

    JSObject *ret = JS_NewObjectForConstructor(cx, &Canvas_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ii",
        &width, &height)) {
        return JS_TRUE;
    }

    handler = new NativeCanvasHandler(width, height);
    handler->context = new NativeCanvas2DContext(cx, width, height);
    handler->jsobj = ret;
    handler->jscx = cx;

    JS_AddObjectRoot(cx, &handler->jsobj);
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
    handler->jsobj = ret;
    handler->jscx = cx;
    
    JS_AddObjectRoot(cx, &handler->context->jsobj);
    JS_AddObjectRoot(cx, &handler->jsobj);

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

