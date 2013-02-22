#include "NativeCanvas2DContext.h"
#include "NativeJSCanvas.h"
#include "NativeSkia.h"
#include "NativeSkGradient.h"
#include "NativeSkImage.h"
#include "NativeJSImage.h"

#include <SkDevice.h>

#define CANVASCTX_GETTER(obj) ((class NativeCanvas2DContext *)JS_GetPrivate(obj))
#define NSKIA_NATIVE_GETTER(obj) ((class NativeSkia *)((class NativeCanvas2DContext *)JS_GetPrivate(obj))->skia)
#define NSKIA_NATIVE ((class NativeSkia *)((class NativeCanvas2DContext *)JS_GetPrivate(JS_GetParent(JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)))))->skia)
#define HANDLER_GETTER(obj) ((class NativeCanvasHandler *)JS_GetPrivate(obj))

extern jsval gfunc;

enum {
#define CANVAS_2D_CTX_PROP(prop) CTX_PROP_ ## prop,
#define CANVAS_2D_CTX_PROP_GET(prop) CTX_PROP_ ## prop,
  #include "NativeCanvas2DContextProperties.h"
  CTX_PROP__NPROP
#undef CANVAS_2D_CTX_PROP
#undef CANVAS_2D_CTX_PROP_GET
};

void CanvasGradient_Finalize(JSFreeOp *fop, JSObject *obj);
void CanvasPattern_Finalize(JSFreeOp *fop, JSObject *obj);
void Canvas2DContext_finalize(JSFreeOp *fop, JSObject *obj);

extern JSClass Canvas_class;

static JSClass Canvas2DContext_class = {
    "CanvasRenderingContext2D", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Canvas2DContext_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass canvasGradient_class = {
    "CanvasGradient", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, CanvasGradient_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass canvasPattern_class = {
    "CanvasPattern", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, CanvasPattern_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass imageData_class = {
    "ImageData", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSBool native_canvas2dctx_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp);
static JSBool native_canvas2dctx_prop_get(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSMutableHandleValue vp);

static JSBool native_canvas2dctx_shadow(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_fillRect(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_strokeRect(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_clearRect(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_fillText(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_beginPath(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_moveTo(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_lineTo(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_fill(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_stroke(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_closePath(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_arc(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_rect(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_quadraticCurveTo(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas2dctx_bezierCurveTo(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas2dctx_rotate(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_scale(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_save(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_restore(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_translate(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_transform(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_setTransform(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas2dctx_clip(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_createImageData(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_createPattern(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_putImageData(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_getImageData(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_createLinearGradient(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_createRadialGradient(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvas2dctxGradient_addColorStop(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_requestAnimationFrame(JSContext *cx,
    unsigned argc, jsval *vp);

static JSBool native_canvas2dctx_stub(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_drawImage(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas2dctx_measureText(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas2dctx_isPointInPath(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas2dctx_getPathBounds(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas2dctx_light(JSContext *cx, unsigned argc,
    jsval *vp);

static JSPropertySpec canvas2dctx_props[] = {
#define CANVAS_2D_CTX_PROP(prop) {#prop, CTX_PROP_ ## prop, JSPROP_PERMANENT | \
        JSPROP_ENUMERATE, JSOP_NULLWRAPPER, \
        JSOP_WRAPPER(native_canvas2dctx_prop_set)},
#define CANVAS_2D_CTX_PROP_GET(prop) {#prop, CTX_PROP_ ## prop, JSPROP_PERMANENT | \
        JSPROP_ENUMERATE, JSOP_WRAPPER(native_canvas2dctx_prop_get), \
        JSOP_NULLWRAPPER},

  #include "NativeCanvas2DContextProperties.h"
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
#undef CANVAS_2D_CTX_PROP
#undef CANVAS_2D_CTX_PROP_GET
};

static JSFunctionSpec canvas2dctx_funcs[] = {
    JS_FN("shadow", native_canvas2dctx_shadow, 0, 0),
    JS_FN("onerror", native_canvas2dctx_stub, 0, 0),
    JS_FN("fillRect", native_canvas2dctx_fillRect, 4, 0),
    JS_FN("fillText", native_canvas2dctx_fillText, 3, 0),
    JS_FN("strokeRect", native_canvas2dctx_strokeRect, 4, 0),
    JS_FN("clearRect", native_canvas2dctx_clearRect, 4, 0),
    JS_FN("beginPath", native_canvas2dctx_beginPath, 0, 0),
    JS_FN("moveTo", native_canvas2dctx_moveTo, 2, 0),
    JS_FN("lineTo", native_canvas2dctx_lineTo, 2, 0),
    JS_FN("fill", native_canvas2dctx_fill, 0, 0),
    JS_FN("stroke", native_canvas2dctx_stroke, 0, 0),
    JS_FN("closePath", native_canvas2dctx_closePath, 0, 0),
    JS_FN("clip", native_canvas2dctx_clip, 0, 0),
    JS_FN("arc", native_canvas2dctx_arc, 5, 0),
    JS_FN("rect", native_canvas2dctx_rect, 4, 0),
    JS_FN("quadraticCurveTo", native_canvas2dctx_quadraticCurveTo, 4, 0),
    JS_FN("bezierCurveTo", native_canvas2dctx_bezierCurveTo, 4, 0),
    JS_FN("rotate", native_canvas2dctx_rotate, 1, 0),
    JS_FN("scale", native_canvas2dctx_scale, 2, 0),
    JS_FN("save", native_canvas2dctx_save, 0, 0),
    JS_FN("restore", native_canvas2dctx_restore, 0, 0),
    JS_FN("translate", native_canvas2dctx_translate, 2, 0),
    JS_FN("transform", native_canvas2dctx_transform, 6, 0),
    JS_FN("setTransform", native_canvas2dctx_setTransform, 6, 0),
    JS_FN("createLinearGradient", native_canvas2dctx_createLinearGradient, 4, 0),
    JS_FN("createRadialGradient", native_canvas2dctx_createRadialGradient, 6, 0),
    JS_FN("createImageData", native_canvas2dctx_createImageData, 2, 0),
    JS_FN("createPattern", native_canvas2dctx_createPattern, 2, 0),
    JS_FN("putImageData", native_canvas2dctx_putImageData, 3, 0),
    JS_FN("getImageData", native_canvas2dctx_getImageData, 4, 0),
    JS_FN("requestAnimationFrame", native_canvas2dctx_requestAnimationFrame, 1, 0),
    JS_FN("drawImage", native_canvas2dctx_drawImage, 3, 0),
    JS_FN("measureText", native_canvas2dctx_measureText, 1, 0),
    JS_FN("isPointInPath", native_canvas2dctx_isPointInPath, 2, 0),
    JS_FN("getPathBounds", native_canvas2dctx_getPathBounds, 0, 0),
    JS_FN("light", native_canvas2dctx_light, 3, 0),
    JS_FS_END
};

static JSFunctionSpec gradient_funcs[] = {
    
    JS_FN("addColorStop", native_canvas2dctxGradient_addColorStop, 2, 0),

    JS_FS_END
};

static JSBool native_canvas2dctx_stub(JSContext *cx, unsigned argc, jsval *vp)
{
    return JS_TRUE;
}

static JSBool native_canvas2dctx_fillRect(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y, width, height, rx = 0, ry = 0;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddd/dd", &x, &y,
        &width, &height, &rx, &ry)) {
        return JS_TRUE;
    }

    if (argc > 4) {
        NSKIA_NATIVE->drawRect(x, y, width, height,
            rx, (argc == 5 ? rx : ry), 0);
    } else {
        NSKIA_NATIVE->drawRect(x, y, width, height, 0);
    }

    return JS_TRUE;
}

static JSBool native_canvas2dctx_strokeRect(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y, width, height, rx = 0, ry = 0;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddd/dd", &x, &y,
        &width, &height, &rx, &ry)) {
        return JS_TRUE;
    }

    if (argc > 4) {
        NSKIA_NATIVE->drawRect(x, y, width, height,
            rx, (argc == 5 ? rx : ry), 1);
    } else {
        NSKIA_NATIVE->drawRect(x, y, width, height, 1);
    }

    return JS_TRUE;
}

static JSBool native_canvas2dctx_clearRect(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y, width, height;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddd", &x, &y,
        &width, &height)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->clearRect(x, y, width, height);

    return JS_TRUE;
}

static JSBool native_canvas2dctx_fillText(JSContext *cx, unsigned argc, jsval *vp)
{
    int x, y, maxwidth;
    JSString *str;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "Sii/i",
            &str, &x, &y, &maxwidth)) {
        return JS_TRUE;
    }

    JSAutoByteString text(cx, str);

    NSKIA_NATIVE->drawText(text.ptr(), x, y);

    return JS_TRUE;
}

static JSBool native_canvas2dctx_shadow(JSContext *cx, unsigned argc, jsval *vp)
{
    //NSKIA_NATIVE->setShadow();
    return JS_TRUE;
}

static JSBool native_canvas2dctx_beginPath(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA_NATIVE->beginPath();

    return JS_TRUE;
}

static JSBool native_canvas2dctx_moveTo(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dd", &x, &y)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->moveTo(x, y);

    return JS_TRUE;
}

static JSBool native_canvas2dctx_lineTo(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dd", &x, &y)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->lineTo(x, y);

    return JS_TRUE;
}

static JSBool native_canvas2dctx_fill(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA_NATIVE->fill();

    return JS_TRUE;
}

static JSBool native_canvas2dctx_stroke(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA_NATIVE->stroke();

    return JS_TRUE;
}
static JSBool native_canvas2dctx_closePath(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA_NATIVE->closePath();

    return JS_TRUE;
}

static JSBool native_canvas2dctx_clip(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA_NATIVE->clip();

    return JS_TRUE;
}

static JSBool native_canvas2dctx_rect(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y, width, height;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddd", &x, &y,
        &width, &height)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->rect(x, y, width, height);

    return JS_TRUE;
}

static JSBool native_canvas2dctx_arc(JSContext *cx, unsigned argc, jsval *vp)
{
    int x, y, radius;
    double startAngle, endAngle;
    JSBool CCW = JS_FALSE;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "iiidd/b", &x, &y,
        &radius, &startAngle, &endAngle, &CCW)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->arc(x, y, radius, startAngle, endAngle, CCW);

    return JS_TRUE;
}

static JSBool native_canvas2dctx_quadraticCurveTo(JSContext *cx, unsigned argc,
    jsval *vp)
{
    double x, y, cpx, cpy;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddd", &cpx, &cpy,
        &x, &y)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->quadraticCurveTo(cpx, cpy, x, y);

    return JS_TRUE;
}

static JSBool native_canvas2dctx_bezierCurveTo(JSContext *cx, unsigned argc,
    jsval *vp)
{
    double x, y, cpx, cpy, cpx2, cpy2;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddddd", &cpx, &cpy,
        &cpx2, &cpy2, &x, &y)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->bezierCurveTo(cpx, cpy, cpx2, cpy2, x, y);

    return JS_TRUE;
}

static JSBool native_canvas2dctx_rotate(JSContext *cx, unsigned argc, jsval *vp)
{
    double angle;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "d", &angle)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->rotate(angle);

    return JS_TRUE;
}

static JSBool native_canvas2dctx_scale(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dd", &x, &y)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->scale(x, y);

    return JS_TRUE;
}

static JSBool native_canvas2dctx_translate(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dd", &x, &y)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->translate(x, y);

    return JS_TRUE;
}

static JSBool native_canvas2dctx_transform(JSContext *cx, unsigned argc, jsval *vp)
{
    double scalex, skewx, skewy, scaley, translatex, translatey;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddddd",
        &scalex, &skewx, &skewy, &scaley, &translatex, &translatey)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->transform(scalex, skewx, skewy, scaley,
        translatex, translatey, 0);

    return JS_TRUE;
}

static JSBool native_canvas2dctx_setTransform(JSContext *cx, unsigned argc, jsval *vp)
{
    double scalex, skewx, skewy, scaley, translatex, translatey;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddddd",
        &scalex, &skewx, &skewy, &scaley, &translatex, &translatey)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->transform(scalex, skewx, skewy, scaley,
        translatex, translatey, 1);

    return JS_TRUE;
}

static JSBool native_canvas2dctx_save(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *ctx = JS_GetParent(JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)));
    JSObject *savedArray = JSVAL_TO_OBJECT(JS_GetReservedSlot(ctx, 0));
    JSObject *saved = JS_NewObject(cx, NULL, NULL, NULL);
    jsval outval;

#define CANVAS_2D_CTX_PROP_GET(prop)
#define CANVAS_2D_CTX_PROP(prop)    JS_GetProperty(cx, ctx, #prop, &outval); \
                                    JS_SetProperty(cx, saved, #prop, &outval);

#include "NativeCanvas2DContextProperties.h"

#undef CANVAS_2D_CTX_PROP
#undef CANVAS_2D_CTX_PROP_GET
    uint32_t arr_length;
    jsval savedVal = OBJECT_TO_JSVAL(saved);

    JS_GetArrayLength(cx, savedArray, &arr_length);
    JS_SetElement(cx, savedArray, arr_length, &savedVal);

    NSKIA_NATIVE->save();

    return JS_TRUE;
}

static JSBool native_canvas2dctx_restore(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *ctx = JS_GetParent(JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)));
    JSObject *savedArray = JSVAL_TO_OBJECT(JS_GetReservedSlot(ctx, 0));
    NativeCanvas2DContext *NativeCtx = (NativeCanvas2DContext *)JS_GetPrivate(ctx);
    NSKIA_NATIVE->restore();

    uint32_t arr_length = 0;
    jsval saved, outval;
    if (JS_GetArrayLength(cx, savedArray, &arr_length) == JS_FALSE) {
        return JS_TRUE;
    }
    if (arr_length == 0) {
        return JS_TRUE;
    }
    JS_GetElement(cx, savedArray, arr_length-1, &saved);
    JSObject *savedObj = JSVAL_TO_OBJECT(saved);

#define CANVAS_2D_CTX_PROP_GET(prop)
#define CANVAS_2D_CTX_PROP(prop)    JS_GetProperty(cx, savedObj, #prop, &outval); \
                                    JS_SetProperty(cx, ctx, #prop, &outval);

    NativeCtx->setterDisabled = true;
#include "NativeCanvas2DContextProperties.h"
    NativeCtx->setterDisabled = false;

#undef CANVAS_2D_CTX_PROP
#undef CANVAS_2D_CTX_PROP_GET
    JS_SetArrayLength(cx, savedArray, arr_length-1);
    return JS_TRUE;
}

static JSBool native_canvas2dctx_createLinearGradient(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JSObject *linearObject;
    double x1, y1, x2, y2;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddd",
        &x1, &y1, &x2, &y2)) {
        return JS_TRUE;
    }

    linearObject = JS_NewObject(cx, &canvasGradient_class, NULL, NULL);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(linearObject));

    JS_SetPrivate(linearObject,
        new NativeSkGradient(x1, y1, x2, y2));

    JS_DefineFunctions(cx, linearObject, gradient_funcs);

    return JS_TRUE;
}

static JSBool native_canvas2dctx_getImageData(JSContext *cx,
    unsigned argc, jsval *vp)
{
    int left, top, width, height;
    JSObject *dataObject;
    JSObject *arrBuffer;
    uint8_t *data;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "iiii",
        &left, &top, &width, &height)) {
        return JS_TRUE;
    }

    dataObject = JS_NewObject(cx, &imageData_class, NULL, NULL);

    arrBuffer = JS_NewUint8ClampedArray(cx, width*height * 4);
    data = JS_GetUint8ClampedArrayData(arrBuffer);

    NSKIA_NATIVE->readPixels(top, left, width, height, data);

    JS_DefineProperty(cx, dataObject, "width", UINT_TO_JSVAL(width),
        NULL, NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "height", UINT_TO_JSVAL(height),
        NULL, NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "data", OBJECT_TO_JSVAL(arrBuffer),
        NULL, NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(dataObject));

    return JS_TRUE;
}

/* TODO: Huge memory leak? */
static JSBool native_canvas2dctx_putImageData(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JSObject *dataObject;
    int x, y;
    uint8_t *pixels;
    jsval jdata, jwidth, jheight;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oii",
        &dataObject, &x, &y)) {
        return JS_TRUE;
    }

    if (!JS_InstanceOf(cx, dataObject, &imageData_class, NULL)) {
        return JS_TRUE;
    }

    JS_GetProperty(cx, dataObject, "data", &jdata);
    JS_GetProperty(cx, dataObject, "width", &jwidth);
    JS_GetProperty(cx, dataObject, "height", &jheight);

    pixels = JS_GetUint8ClampedArrayData(JSVAL_TO_OBJECT(jdata));

    NSKIA_NATIVE->drawPixels(pixels, JSVAL_TO_INT(jwidth), JSVAL_TO_INT(jheight),
        x, y);

    return JS_TRUE;
}

/* TODO: clamp max size */
static JSBool native_canvas2dctx_createImageData(JSContext *cx,
    unsigned argc, jsval *vp)
{
    unsigned long x, y;
    JSObject *dataObject;
    JSObject *arrBuffer;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uu",
        &x, &y)) {
        return JS_TRUE;
    }

    if (x == 0) {
        x = 1;
    }
    if (y == 0) {
        y = 1;
    }
    
    arrBuffer = JS_NewUint8ClampedArray(cx, x*y * 4);
    if (arrBuffer == NULL) {
        JS_ReportOutOfMemory(cx);
        return JS_TRUE;
    }
    
    dataObject = JS_NewObject(cx, &imageData_class, NULL, NULL);

    JS_DefineProperty(cx, dataObject, "width", UINT_TO_JSVAL(x), NULL, NULL,
        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS_DefineProperty(cx, dataObject, "height", UINT_TO_JSVAL(y), NULL, NULL,
        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);


    JS_DefineProperty(cx, dataObject, "data", OBJECT_TO_JSVAL(arrBuffer), NULL,
        NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(dataObject));

    return JS_TRUE;
}

static JSBool native_canvas2dctx_createPattern(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JSObject *jsimage, *patternObject;
    JSString *mode;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oS",
        &jsimage, &mode)) {
        return JS_TRUE;
    }

    if (!NativeJSImage::JSObjectIs(cx, jsimage)) {
        JS_ReportError(cx, "First parameter is not an Image");
        return JS_FALSE;
    }

    patternObject = JS_NewObject(cx, &canvasPattern_class, NULL, NULL);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(patternObject));

    NativeJSImage *img = (NativeJSImage *)JS_GetPrivate(jsimage);

    JS_SetReservedSlot(patternObject, 0, OBJECT_TO_JSVAL(img->jsobj));

    JS_SetPrivate(patternObject,
        new NativeCanvasPattern((NativeJSImage *)JS_GetPrivate(jsimage),
        NativeCanvasPattern::PATTERN_REPEAT));

    return JS_TRUE;
}

static JSBool native_canvas2dctx_createRadialGradient(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JSObject *linearObject;
    double x1, y1, x2, y2, r1, r2;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddddd",
        &x1, &y1, &r1, &x2, &y2, &r2)) {
        return JS_TRUE;
    }

    linearObject = JS_NewObject(cx, &canvasGradient_class, NULL, NULL);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(linearObject));

    JS_SetPrivate(linearObject,
        new NativeSkGradient(x1, y1, r1, x2, y2, r2));

    JS_DefineFunctions(cx, linearObject, gradient_funcs);

    return JS_TRUE;
}

static JSBool native_canvas2dctxGradient_addColorStop(JSContext *cx,
    unsigned argc, jsval *vp)
{
    double position;
    JSString *color;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    NativeSkGradient *gradient;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dS",
        &position, &color)) {
        return JS_TRUE;
    }

    if ((gradient = (NativeSkGradient *)JS_GetPrivate(caller)) != NULL) {
        JSAutoByteString colorstr(cx, color);

        gradient->addColorStop(position, colorstr.ptr());
    }

    return JS_TRUE;
}

static JSBool native_canvas2dctx_requestAnimationFrame(JSContext *cx,
    unsigned argc, jsval *vp)
{

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[0], JSTYPE_FUNCTION, &gfunc)) {
        return JS_TRUE;
    }
    JS_AddValueRoot(cx, &gfunc);
    return JS_TRUE;
}


static JSBool native_canvas2dctx_drawImage(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *jsimage;
    NativeSkImage *image;
    double x, y, width, height;
    int sx, sy, swidth, sheight;
    int need_free = 0;

    if (argc == 9) {
         if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oiiiidddd",
            &jsimage, &sx, &sy, &swidth, &sheight, &x, &y, &width, &height)) {
            return JS_TRUE;
        }
    } else {

        if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "odd/dd",
            &jsimage, &x, &y, &width, &height)) {
            return JS_TRUE;
        }
    }

    if (JS_InstanceOf(cx, jsimage, &Canvas_class, NULL)) {
        image = new NativeSkImage(HANDLER_GETTER(jsimage)->context->skia->canvas);
        need_free = 1;

    } else if (!NativeJSImage::JSObjectIs(cx, jsimage) ||
        (image = NativeJSImage::JSObjectToNativeSkImage(jsimage)) == NULL) {
        return JS_TRUE;
    }

    switch(argc) {
        case 3:
            NSKIA_NATIVE->drawImage(image, x, y);
            break;
        case 5:
            NSKIA_NATIVE->drawImage(image, x, y, width, height);
            break;
        case 9:
            NSKIA_NATIVE->drawImage(image, sx, sy, swidth, sheight,
                x, y, width, height);
            break;
        default:
            break;
    }

    /* TODO: add cache (keep an SkBitmap for the canvas) */
    if (need_free) {
        delete image;
    }

    return JS_TRUE;
}

static JSBool native_canvas2dctx_measureText(JSContext *cx, unsigned argc,
    jsval *vp)
{
    JSString *text;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S",
        &text)) {
        return JS_TRUE;
    }

    JSAutoByteString ctext(cx, text);
    NativeSkia *n = NSKIA_NATIVE;

    JS_SET_RVAL(cx, vp, DOUBLE_TO_JSVAL(n->measureText(ctext.ptr(),
        strlen(ctext.ptr()))));

    return JS_TRUE;
}

static JSBool native_canvas2dctx_isPointInPath(JSContext *cx, unsigned argc,
    jsval *vp)
{
    double x, y;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dd", &x, &y)) {
        vp->setBoolean(false);
        return JS_TRUE;
    }

    NativeSkia *n = NSKIA_NATIVE;

    vp->setBoolean(n->SkPathContainsPoint(x, y));

    return JS_TRUE;
}

/* TODO: return undefined if the path is invalid */
static JSBool native_canvas2dctx_getPathBounds(JSContext *cx, unsigned argc,
    jsval *vp)
{
#define OBJ_PROP(name, val) JS_DefineProperty(cx, obj, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY)

    double left = 0, right = 0, top = 0, bottom = 0;
    JSObject *obj = JS_NewObject(cx, NULL, NULL, NULL);

    NSKIA_NATIVE->getPathBounds(&left, &right, &top, &bottom);

    OBJ_PROP("left", DOUBLE_TO_JSVAL(left));
    OBJ_PROP("right", DOUBLE_TO_JSVAL(right));
    OBJ_PROP("top", DOUBLE_TO_JSVAL(top));
    OBJ_PROP("bottom", DOUBLE_TO_JSVAL(bottom));

    vp->setObject(*obj);

    return JS_TRUE;
}

static JSBool native_canvas2dctx_light(JSContext *cx, unsigned argc,
    jsval *vp)
{
    double x, y, z;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ddd", &x, &y, &z)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->light(x, y, z);
    return JS_TRUE;
}

static JSBool native_canvas2dctx_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
#define CTX_PROP(prop) CTX_PROP_ ## prop

    if (CANVASCTX_GETTER(obj.get())->setterDisabled) {
        return JS_TRUE;
    }

    NativeSkia *curSkia = NSKIA_NATIVE_GETTER(obj.get());

    switch(JSID_TO_INT(id)) {
        case CTX_PROP(imageSmoothingEnabled):
        {
            if (!JSVAL_IS_BOOLEAN(vp)) {
                vp.set(JSVAL_FALSE);

                return JS_TRUE;
            }

            curSkia->setSmooth(vp.toBoolean());
            break;
        }
        case CTX_PROP(shadowOffsetX):
        {
            double ret;
            if (!JSVAL_IS_NUMBER(vp)) {
                vp.set(JSVAL_VOID);
                
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &ret);

            curSkia->setShadowOffsetX(ret);  
        }
        break;
        case CTX_PROP(shadowOffsetY):
        {
            double ret;
            if (!JSVAL_IS_NUMBER(vp)) {
                vp.set(JSVAL_VOID);
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &ret);

            curSkia->setShadowOffsetY(ret);  
        }
        break;
        case CTX_PROP(shadowBlur):
        {
            double ret;
            if (!JSVAL_IS_NUMBER(vp)) {
                vp.set(JSVAL_VOID);
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &ret);

            curSkia->setShadowBlur(ret);  
        }
        break;
        case CTX_PROP(shadowColor):
        {
            if (!JSVAL_IS_STRING(vp)) {
                vp.set(JSVAL_VOID);

                return JS_TRUE;
            }
            JSAutoByteString color(cx, JSVAL_TO_STRING(vp));
            curSkia->setShadowColor(color.ptr());          
        }
        break;
        case CTX_PROP(fontSize):
        {
            double ret;
            if (!JSVAL_IS_NUMBER(vp)) {
                vp.set(JSVAL_VOID);
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &ret);
            curSkia->setFontSize(ret);

        }
        break;
        case CTX_PROP(fontType):
        {
            if (!JSVAL_IS_STRING(vp)) {
                vp.set(JSVAL_VOID);

                return JS_TRUE;
            }
            JSAutoByteString font(cx, JSVAL_TO_STRING(vp));
            curSkia->setFontType(font.ptr());          
        }
        break;
        case CTX_PROP(fillStyle):
        {
            //printf("Fillstyle changed\n");
            if (JSVAL_IS_STRING(vp)) {

                JSAutoByteString colorName(cx, JSVAL_TO_STRING(vp));
                curSkia->setFillColor(colorName.ptr());
            } else if (!JSVAL_IS_PRIMITIVE(vp) && 
                JS_InstanceOf(cx, JSVAL_TO_OBJECT(vp),
                    &canvasGradient_class, NULL)) {

                NativeSkGradient *gradient = (class NativeSkGradient *)
                                            JS_GetPrivate(JSVAL_TO_OBJECT(vp));

                curSkia->setFillColor(gradient);

            } else if (!JSVAL_IS_PRIMITIVE(vp) && 
                JS_InstanceOf(cx, JSVAL_TO_OBJECT(vp),
                    &canvasPattern_class, NULL)) {

                NativeCanvasPattern *pattern = (class NativeCanvasPattern *)
                                            JS_GetPrivate(JSVAL_TO_OBJECT(vp));

                curSkia->setFillColor(pattern);
            } else {
                vp.set(JSVAL_VOID);

                return JS_TRUE;                
            }
        }
        break;
        case CTX_PROP(strokeStyle):
        {
            if (JSVAL_IS_STRING(vp)) {
                JSAutoByteString colorName(cx, JSVAL_TO_STRING(vp));
                curSkia->setStrokeColor(colorName.ptr());
            } else if (!JSVAL_IS_PRIMITIVE(vp) && 
                JS_InstanceOf(cx, JSVAL_TO_OBJECT(vp),
                    &canvasGradient_class, NULL)) {

                NativeSkGradient *gradient = (class NativeSkGradient *)
                                            JS_GetPrivate(JSVAL_TO_OBJECT(vp));

                curSkia->setStrokeColor(gradient);

            } else {
                vp.set(JSVAL_VOID);

                return JS_TRUE;                
            }    
        }
        break;
        case CTX_PROP(lineWidth):
        {
            double ret;
            if (!JSVAL_IS_NUMBER(vp)) {
                vp.set(JSVAL_VOID);
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &ret);
            curSkia->setLineWidth(ret);
        }
        break;
        case CTX_PROP(globalAlpha):
        {
            double ret;
            if (!JSVAL_IS_NUMBER(vp)) {
                vp.set(JSVAL_VOID);
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &ret);
            curSkia->setGlobalAlpha(ret);
        }
        break;
        case CTX_PROP(globalCompositeOperation):
        {
            if (!JSVAL_IS_STRING(vp)) {
                vp.set(JSVAL_VOID);

                return JS_TRUE;
            }
            JSAutoByteString composite(cx, JSVAL_TO_STRING(vp));
            curSkia->setGlobalComposite(composite.ptr());            
        }
        break;
        case CTX_PROP(lineCap):
        {
            if (!JSVAL_IS_STRING(vp)) {
                vp.set(JSVAL_VOID);

                return JS_TRUE;
            }
            JSAutoByteString lineCap(cx, JSVAL_TO_STRING(vp));
            curSkia->setLineCap(lineCap.ptr());                
        }
        break;
        case CTX_PROP(lineJoin):
        {
            if (!JSVAL_IS_STRING(vp)) {
                vp.set(JSVAL_VOID);

                return JS_TRUE;
            }
            JSAutoByteString lineJoin(cx, JSVAL_TO_STRING(vp));
            curSkia->setLineJoin(lineJoin.ptr());                
        }
        break;
        default:
            break;
    }

    return JS_TRUE;
#undef CTX_PROP
}

static JSBool native_canvas2dctx_prop_get(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSMutableHandleValue vp)
{
#define CTX_PROP(prop) CTX_PROP_ ## prop
    NativeSkia *curSkia = NSKIA_NATIVE_GETTER(obj.get());

    switch(JSID_TO_INT(id)) {
        case CTX_PROP(width):
        {
            vp.set(INT_TO_JSVAL(curSkia->getWidth()));
        }
        break;
        case CTX_PROP(height):
        {
            vp.set(INT_TO_JSVAL(curSkia->getHeight()));
        }
        break;
        default:
            break;
    }

    return JS_TRUE;
#undef CTX_PROP
}

void CanvasGradient_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeSkGradient *gradient = (class NativeSkGradient *)JS_GetPrivate(obj);
    if (gradient != NULL) {
        printf("Deleting gradient\n");
        delete gradient;
    }
}

void CanvasPattern_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeCanvasPattern *pattern = (class NativeCanvasPattern *)JS_GetPrivate(obj);
    if (pattern != NULL) {
        delete pattern;
    }
}

void Canvas2DContext_finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeCanvas2DContext *canvasctx = CANVASCTX_GETTER(obj);
    if (canvasctx != NULL) {
        delete canvasctx;
    }
}

void NativeCanvas2DContext::clear(uint32_t color)
{
    skia->canvas->clear(color);
}

void NativeCanvas2DContext::composeWith(NativeCanvas2DContext *layer,
    double left, double top, double opacity, const NativeRect *rclip)
{
    SkPaint pt;
    pt.setAlpha(opacity * (double)255.);

    if (rclip != NULL) {
        SkRect r;
        r.set(SkDoubleToScalar(rclip->fLeft), SkDoubleToScalar(rclip->fTop),
            SkDoubleToScalar(rclip->fRight), SkDoubleToScalar(rclip->fBottom));

        skia->canvas->save(SkCanvas::kClip_SaveFlag);

        skia->canvas->clipRect(r);
        skia->canvas->drawBitmap(layer->skia->canvas->getDevice()->accessBitmap(false),
            left, top, &pt);

        skia->canvas->restore();
    } else {
        skia->canvas->drawBitmap(layer->skia->canvas->getDevice()->accessBitmap(false),
            left, top, &pt);        
    }
    skia->canvas->flush();
}

void NativeCanvas2DContext::flush()
{
    skia->canvas->flush();
}

void NativeCanvas2DContext::setSize(int width, int height)
{
    SkDevice *ndev;
    SkCanvas *ncanvas;

    SkBitmap bt = skia->canvas->getDevice()->accessBitmap(false);
 
    ndev = skia->canvas->createCompatibleDevice(SkBitmap::kARGB_8888_Config,
                                width, height, false);

    ncanvas = new SkCanvas(ndev);

    ncanvas->drawBitmap(bt, 0, 0);
    //ncanvas->clipRegion(skia->canvas->getTotalClip());
    ncanvas->setMatrix(skia->canvas->getTotalMatrix());

    SkSafeUnref(ndev);
    skia->canvas->unref();
    skia->canvas = ncanvas;

}

void NativeCanvas2DContext::translate(double x, double y)
{
    skia->canvas->translate(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

NativeCanvas2DContext::NativeCanvas2DContext(JSContext *cx, int width, int height) :
    setterDisabled(false)
{
    jsobj = JS_NewObject(cx, &Canvas2DContext_class, NULL, NULL);
    jscx  = cx;

    JS_DefineFunctions(cx, jsobj, canvas2dctx_funcs);
    JS_DefineProperties(cx, jsobj, canvas2dctx_props);
    JSObject *saved = JS_NewArrayObject(cx, 0, NULL);
    JS_SetReservedSlot(jsobj, 0, OBJECT_TO_JSVAL(saved));

    skia = new NativeSkia();
    skia->bindOnScreen(width, height);

    JS_SetPrivate(jsobj, this);
}

NativeCanvas2DContext::NativeCanvas2DContext(int width, int height) :
    jsobj(NULL), jscx(NULL)
{
    skia = new NativeSkia();
    skia->bindGL(width, height);
}

NativeCanvas2DContext::~NativeCanvas2DContext()
{
    delete skia;
}

static JSBool native_Canvas2DContext_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JS_ReportError(cx, "Illegal constructor");
    return JS_FALSE;
}

void NativeCanvas2DContext::registerObject(JSContext *cx)
{
    JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, &Canvas2DContext_class,
                native_Canvas2DContext_constructor,
                0, NULL, NULL, NULL, NULL);
}
