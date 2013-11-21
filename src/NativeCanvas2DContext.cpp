#include "NativeCanvas2DContext.h"
#include "NativeJSCanvas.h"
#include "NativeSkia.h"
#include "NativeSkGradient.h"
#include "NativeSkImage.h"
#include "NativeJSImage.h"
#include "SkDevice.h"
#include "SkGpuDevice.h"
#include "NativeSystemInterface.h"
#include "NativeMacros.h"

#include <SkDevice.h>
#define GL_GLEXT_PROTOTYPES
#if __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#define CANVASCTX_GETTER(obj) ((class NativeCanvas2DContext *)JS_GetPrivate(obj))
#define NSKIA_NATIVE_GETTER(obj) ((class NativeSkia *)((class NativeCanvas2DContext *)JS_GetPrivate(obj))->getSurface())
#define NSKIA_NATIVE ((class NativeSkia *)((class NativeCanvas2DContext *)JS_GetPrivate(JS_GetParent(JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)))))->getSurface())
#define HANDLER_GETTER(obj) ((class NativeCanvasHandler *)JS_GetPrivate(obj))
#define NCTX_NATIVE ((class NativeCanvas2DContext *)JS_GetPrivate(JS_GetParent(JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)))))

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

static JSClass canvasGLProgram_class = {
    "CanvasGLProgram", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
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

static JSBool native_canvas2dctx_breakText(JSContext *cx, unsigned argc, jsval *vp);
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
static JSBool native_canvas2dctx_iTransform(JSContext *cx, unsigned argc, jsval *vp);
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
static JSBool native_canvas2dctx_attachGLSLFragment(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas2dctx_detachGLSLFragment(JSContext *cx, unsigned argc,
    jsval *vp);

/* GLSL related */
static JSBool native_canvas2dctxGLProgram_getUniformLocation(JSContext *cx, unsigned argc,
    jsval *vp);

static JSBool native_canvas2dctxGLProgram_uniform1i(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas2dctxGLProgram_uniform1f(JSContext *cx, unsigned argc,
    jsval *vp);

static JSBool native_canvas2dctxGLProgram_uniform1iv(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas2dctxGLProgram_uniform2iv(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas2dctxGLProgram_uniform3iv(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas2dctxGLProgram_uniform4iv(JSContext *cx, unsigned argc,
    jsval *vp);

static JSBool native_canvas2dctxGLProgram_uniform1fv(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas2dctxGLProgram_uniform2fv(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas2dctxGLProgram_uniform3fv(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas2dctxGLProgram_uniform4fv(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas2dctxGLProgram_getActiveUniforms(JSContext *cx, unsigned argc,
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
    JS_FN("breakText", native_canvas2dctx_breakText, 2, 0),
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
    JS_FN("iTransform", native_canvas2dctx_iTransform, 0, 0),
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
    JS_FN("attachFragmentShader", native_canvas2dctx_attachGLSLFragment, 1, 0),
    JS_FN("detachFragmentShader", native_canvas2dctx_detachGLSLFragment, 0, 0),
    JS_FS_END
};

static JSFunctionSpec gradient_funcs[] = {
    
    JS_FN("addColorStop", native_canvas2dctxGradient_addColorStop, 2, 0),

    JS_FS_END
};

static JSFunctionSpec glprogram_funcs[] = {
    
    JS_FN("getUniformLocation", native_canvas2dctxGLProgram_getUniformLocation, 1, 0),
    JS_FN("getActiveUniforms", native_canvas2dctxGLProgram_getActiveUniforms, 0, 0),
    JS_FN("uniform1i", native_canvas2dctxGLProgram_uniform1i, 2, 0),
    JS_FN("uniform1f", native_canvas2dctxGLProgram_uniform1f, 2, 0),
    JS_FN("uniform1iv", native_canvas2dctxGLProgram_uniform1iv, 2, 0),
    JS_FN("uniform2iv", native_canvas2dctxGLProgram_uniform2iv, 2, 0),
    JS_FN("uniform3iv", native_canvas2dctxGLProgram_uniform3iv, 2, 0),
    JS_FN("uniform4iv", native_canvas2dctxGLProgram_uniform4iv, 2, 0),

    JS_FN("uniform1fv", native_canvas2dctxGLProgram_uniform1fv, 2, 0),
    JS_FN("uniform2fv", native_canvas2dctxGLProgram_uniform2fv, 2, 0),
    JS_FN("uniform3fv", native_canvas2dctxGLProgram_uniform3fv, 2, 0),
    JS_FN("uniform4fv", native_canvas2dctxGLProgram_uniform4fv, 2, 0),
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

static JSBool native_canvas2dctx_breakText(JSContext *cx,
    unsigned argc, jsval *vp)
{
#define SET_PROP(where, name, val) JS_DefineProperty(cx, where, \
    (const char *)name, val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | \
        JSPROP_ENUMERATE)
    JSString *str;
    double maxWidth;
    JSObject *res = NULL;
    int length = 0;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "Sd", &str, &maxWidth)) {
        return JS_TRUE;
    }

    res = JS_NewObject(cx, NULL, NULL, NULL);

    JSAutoByteString text(cx, str);
    size_t len = text.length();

    if (len == 0) {
        vp->setNull();
        return JS_TRUE;
    }

    struct _NativeLine *lines = new struct _NativeLine[len];

    if (!lines) {
        JS_ReportOutOfMemory(cx);
        return JS_FALSE;
    }

    memset(lines, 0, len * sizeof(struct _NativeLine));

    SkScalar ret = NSKIA_NATIVE->breakText(text.ptr(), len,
                    lines, maxWidth, &length);
    JSObject *alines = JS_NewArrayObject(cx, length, NULL);

    for (int i = 0; i < len && i < length; i++) {
        jsval val = STRING_TO_JSVAL(JS_NewStringCopyN(cx,
            lines[i].line, lines[i].len));
        JS_SetElement(cx, alines, i, &val);
    }

    SET_PROP(res, "height", DOUBLE_TO_JSVAL(SkScalarToDouble(ret)));
    SET_PROP(res, "lines", OBJECT_TO_JSVAL(alines));

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(res));

    delete[] lines;

    return JS_TRUE;
#undef SET_PROP
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
    double scalex, skewx, skewy, scaley, translatex, translatey, rotate;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddddd/d",
        &scalex, &skewx, &skewy, &scaley, &translatex, &translatey, &rotate)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->transform(scalex, skewx, skewy, scaley,
        translatex, translatey, 0);

    if (argc == 7) {
        NSKIA_NATIVE->rotate(rotate);
    }

    return JS_TRUE;
}

static JSBool native_canvas2dctx_iTransform(JSContext *cx, unsigned argc, jsval *vp)
{
    double scalex, skewx, skewy, scaley, translatex, translatey;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddddd",
        &scalex, &skewx, &skewy, &scaley, &translatex, &translatey)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->itransform(scalex, skewx, skewy, scaley,
        translatex, translatey);

    return JS_TRUE;
}

static JSBool native_canvas2dctx_setTransform(JSContext *cx, unsigned argc, jsval *vp)
{
    double scalex, skewx, skewy, scaley, translatex, translatey;

    NativeCanvasHandler *handler = NCTX_NATIVE->getHandler();

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddddd",
        &scalex, &skewx, &skewy, &scaley, &translatex, &translatey)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->transform(scalex, skewx, skewy, scaley,
        translatex+handler->padding.global, translatey+handler->padding.global, 1);

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

    /* The image is a Canvas */
    /*
        TODO: work with WebGL canvas
    */
    if (JS_InstanceOf(cx, jsimage, &Canvas_class, NULL)) {
        NativeCanvasContext *drawctx = HANDLER_GETTER(jsimage)->getContext();
        if (drawctx == NULL || drawctx->m_Mode != NativeCanvasContext::CONTEXT_2D) {
            JS_ReportError(cx, "Invalid image canvas (must be backed by a 2D context)");
            return false;
        }
        image = new NativeSkImage(((NativeCanvas2DContext *)drawctx)->getSurface()->canvas);
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
#define OBJ_PROP(name, val) JS_DefineProperty(cx, obj, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY)
    
    JS::CallArgs args = CallArgsFromVp(argc, vp);

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "S",
        &text)) {
        return true;
    }

    JSObject *obj = JS_NewObject(cx, NULL, NULL, NULL);

    JSAutoByteString ctext(cx, text);
    NativeSkia *n = NSKIA_NATIVE;
    
    OBJ_PROP("width", DOUBLE_TO_JSVAL(n->measureText(ctext.ptr(),
        strlen(ctext.ptr()))));

    args.rval().setObjectOrNull(obj);

    return true;
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

static JSBool native_canvas2dctx_detachGLSLFragment(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NCTX_NATIVE->detachShader();

    return true;
}

static JSBool native_canvas2dctx_attachGLSLFragment(JSContext *cx, unsigned argc,
    jsval *vp)
{
    JSString *glsl;
    NativeCanvas2DContext *nctx = NCTX_NATIVE;

    size_t program;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S",
        &glsl)) {
        return JS_TRUE;
    }

    JSAutoByteString cglsl(cx, glsl);

    if ((program = nctx->attachShader(cglsl.ptr())) == 0) {
        JS_ReportError(cx, "Failed to compile GLSL shader");
        return JS_FALSE;
    }
    JSObject *canvasProgram = JS_NewObject(cx, &canvasGLProgram_class, NULL, NULL);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(canvasProgram));

    JS_DefineFunctions(cx, canvasProgram, glprogram_funcs);

    JS_SetPrivate(canvasProgram, (void *)program);

    return JS_TRUE;
}

static JSBool native_canvas2dctxGLProgram_getUniformLocation(JSContext *cx, unsigned argc,
    jsval *vp)
{
    JSString *location;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    uint32_t program;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S",
        &location)) {
        return JS_TRUE;
    }

    JSAutoByteString clocation(cx, location);

    program = (size_t)JS_GetPrivate(caller);

    int ret = glGetUniformLocation(program, clocation.ptr());

    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(ret));

    return JS_TRUE;
}

static JSBool native_canvas2dctxGLProgram_uniform1i(JSContext *cx, unsigned argc,
    jsval *vp)
{
    int location, val;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    uint32_t program;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ii",
        &location, &val)) {
        return JS_TRUE;
    }

    if (location == -1) {
        return JS_TRUE;
    }
    program = (size_t)JS_GetPrivate(caller);
    int32_t tmpProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &tmpProgram);
    glUseProgram(program);
    glUniform1i(location, val);
    glUseProgram(tmpProgram);

    return JS_TRUE;
}

static JSBool native_canvas2dctxGLProgram_uniform1f(JSContext *cx, unsigned argc,
    jsval *vp)
{
    int location;
    double val;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    uint32_t program;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "id",
        &location, &val)) {
        return JS_TRUE;
    }

    if (location == -1) {
        return JS_TRUE;
    }

    program = (size_t)JS_GetPrivate(caller);
    int32_t tmpProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &tmpProgram);
    glUseProgram(program);
    glUniform1f(location, (float)val);
    glUseProgram(tmpProgram);

    return JS_TRUE;
}

static JSBool native_canvas2dctxGLProgram_uniformXiv(JSContext *cx,
    unsigned int argc, jsval *vp, int nb) 
{
    GLsizei length;
    GLint *carray;
    JSObject *tmp;
    JSObject *array;
    int location;
    uint32_t program;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    program = (size_t)JS_GetPrivate(caller);
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "io", &location, &array)) {
        return JS_TRUE;
    }

    if (location == -1) {
        return JS_TRUE;
    }

    if (JS_IsInt32Array(array)) {
        carray = (GLint *)JS_GetInt32ArrayData(array);
        length = (GLsizei)JS_GetTypedArrayLength(array);
    } else if (JS_IsArrayObject(cx, array)) {
        tmp = JS_NewInt32ArrayFromArray(cx, array); 
        carray = (GLint *)JS_GetInt32ArrayData(tmp);
        length = (GLsizei)JS_GetTypedArrayLength(tmp);
    } else {
        JS_ReportError(cx, "Array is not a Int32 array");
        return JS_FALSE;
    }
    int32_t tmpProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &tmpProgram);

    glUseProgram(program);

    switch (nb) {
        case 1:
            glUniform1iv(location, length, carray);
            break;
        case 2:
            glUniform2iv(location, length/2, carray);
            break;
        case 3:
            glUniform3iv(location, length/3, carray);
            break;
        case 4:
            glUniform4iv(location, length/4, carray);
            break;
        default:
            break;
    }

    glUseProgram(tmpProgram);
    
    return JS_TRUE;
}

static JSBool native_canvas2dctxGLProgram_uniformXfv(JSContext *cx,
    unsigned int argc, jsval *vp, int nb) 
{
    GLsizei length;
    GLfloat *carray;
    JSObject *array;
    int location;
    uint32_t program;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    program = (size_t)JS_GetPrivate(caller);
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "io", &location, &array)) {
        return JS_TRUE;
    }

    if (location == -1) {
        return JS_TRUE;
    }

    if (JS_IsFloat32Array(array)) {
        carray = (GLfloat *)JS_GetFloat32ArrayData(array);
        length = (GLsizei)JS_GetTypedArrayLength(array);
    } else if (JS_IsArrayObject(cx, array)) {
        JSObject *tmp;
        tmp = JS_NewFloat32ArrayFromArray(cx, array); 
        carray = (GLfloat *)JS_GetFloat32ArrayData(tmp);
        length = (GLsizei)JS_GetTypedArrayLength(tmp);
    } else {
        JS_ReportError(cx, "Array is not a Float32 array");
        return JS_FALSE;
    }
    int32_t tmpProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &tmpProgram);

    glUseProgram(program);

    switch (nb) {
        case 1:
            glUniform1fv(location, length, carray);
            break;
        case 2:
            glUniform2fv(location, length/2, carray);
            break;
        case 3:
            glUniform3fv(location, length/3, carray);
            break;
        case 4:
            glUniform4fv(location, length/4, carray);
            break;
    }

    glUseProgram(tmpProgram);
    
    return JS_TRUE;
}

static JSBool native_canvas2dctxGLProgram_uniform1iv(JSContext *cx, unsigned argc,
    jsval *vp)
{   
    return native_canvas2dctxGLProgram_uniformXiv(cx, argc, vp, 1);
}

static JSBool native_canvas2dctxGLProgram_uniform2iv(JSContext *cx, unsigned argc,
    jsval *vp)
{   
    return native_canvas2dctxGLProgram_uniformXiv(cx, argc, vp, 2);
}

static JSBool native_canvas2dctxGLProgram_uniform3iv(JSContext *cx, unsigned argc,
    jsval *vp)
{   
    return native_canvas2dctxGLProgram_uniformXiv(cx, argc, vp, 3);
}

static JSBool native_canvas2dctxGLProgram_uniform4iv(JSContext *cx, unsigned argc,
    jsval *vp)
{   
    return native_canvas2dctxGLProgram_uniformXiv(cx, argc, vp, 4);
}

static JSBool native_canvas2dctxGLProgram_uniform1fv(JSContext *cx, unsigned argc,
    jsval *vp)
{   
    return native_canvas2dctxGLProgram_uniformXfv(cx, argc, vp, 1);
}

static JSBool native_canvas2dctxGLProgram_uniform2fv(JSContext *cx, unsigned argc,
    jsval *vp)
{   
    return native_canvas2dctxGLProgram_uniformXfv(cx, argc, vp, 2);
}

static JSBool native_canvas2dctxGLProgram_uniform3fv(JSContext *cx, unsigned argc,
    jsval *vp)
{   
    return native_canvas2dctxGLProgram_uniformXfv(cx, argc, vp, 3);
}

static JSBool native_canvas2dctxGLProgram_uniform4fv(JSContext *cx, unsigned argc,
    jsval *vp)
{   
    return native_canvas2dctxGLProgram_uniformXfv(cx, argc, vp, 4);
}


static JSBool native_canvas2dctxGLProgram_getActiveUniforms(JSContext *cx, unsigned argc,
    jsval *vp)
{
#define SET_PROP(where, name, val) JS_DefineProperty(cx, where, \
    (const char *)name, val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | \
        JSPROP_ENUMERATE)
    uint32_t program;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    program = (size_t)JS_GetPrivate(caller);

    int nactives = 0;

    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &nactives);

    JSObject *arr = JS_NewArrayObject(cx, nactives, NULL);
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(arr));

    char name[512];
    for (int i = 0; i < nactives; i++) {
        int length = 0, size = 0;
        GLenum type = GL_ZERO;

        JSObject *in = JS_NewObject(cx, NULL, NULL, NULL);

        glGetActiveUniform(program, i, sizeof(name)-1, &length, &size, &type, name);
        name[length] = '\0';
        SET_PROP(in, "name", STRING_TO_JSVAL(JS_NewStringCopyN(cx, name, length)));
        SET_PROP(in, "location", INT_TO_JSVAL(glGetUniformLocation(program, name)));
        jsval inval = OBJECT_TO_JSVAL(in);
        JS_SetElement(cx, arr, i, &inval);
    }

    return JS_TRUE;
#undef SET_PROP
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
        case CTX_PROP(textBaseline):
        {
            if (!JSVAL_IS_STRING(vp)) {
                vp.set(JSVAL_VOID);

                return JS_TRUE;
            }
            JSAutoByteString baseline(cx, JSVAL_TO_STRING(vp));
            curSkia->textBaseline(baseline.ptr());
        }
        break;
        case CTX_PROP(textAlign):
        {
            if (!JSVAL_IS_STRING(vp)) {
                vp.set(JSVAL_VOID);

                return JS_TRUE;
            }

            JSAutoByteString font(cx, JSVAL_TO_STRING(vp));
            curSkia->textAlign(font.ptr());
        }
        break;
        case CTX_PROP(fontFamily):
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
    m_Skia->canvas->clear(color);
}

char *NativeCanvas2DContext::genModifiedFragmentShader(const char *data)
{
    const char *prologue =
        "#version 100\nprecision mediump float;\n"
        "vec4 ___gl_FragCoord;\n"
        "#define main ___main\n"
        "#define gl_FragCoord ___gl_FragCoord\n";

    char *ret;

    asprintf(&ret, "%s%s", prologue, data);

    return ret;
}

uint32_t NativeCanvas2DContext::createProgram(const char *data)
{
    char *pdata = NativeCanvasContext::processShader(data, NativeCanvasContext::SHADER_FRAGMENT);

    if (pdata == NULL) {
        return 0;
    }
    
    char *nshader = this->genModifiedFragmentShader(pdata);

    uint32_t fragment = NativeCanvasContext::compileShader(nshader, GL_FRAGMENT_SHADER);
    uint32_t coop = this->compileCoopFragmentShader();
    uint32_t vertex = this->createPassThroughVertex();

    free(nshader);

    if (fragment == 0) {
        return 0;
    }

    GLuint programHandle = glCreateProgram();
    GLint linkSuccess;

    glAttachShader(programHandle, vertex);
    glAttachShader(programHandle, coop);
    glAttachShader(programHandle, fragment);

    glBindAttribLocation(programHandle,
        NativeCanvasContext::SH_ATTR_POSITION, "Position");

    glBindAttribLocation(programHandle,
        NativeCanvasContext::SH_ATTR_TEXCOORD, "TexCoordIn");

    glLinkProgram(programHandle);

    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
        NLOG("createProgram error : %s", messages);
        return 0;
    }

    return programHandle;
}

uint32_t NativeCanvas2DContext::compileCoopFragmentShader()
{
    const char *coop =
        "#version 100\nprecision mediump float;\n"
        "void ___main(void);\n"
        "uniform sampler2D Texture;\n"
        "uniform vec2 n_Position;\n"
        "uniform vec2 n_Resolution;\n"
        "uniform float u_opacity;\n"
        "uniform float n_Padding;\n"
        "varying vec2 TexCoordOut;\n"

        "vec4 ___gl_FragCoord = vec4(gl_FragCoord.x-n_Position.x-n_Padding, gl_FragCoord.y-n_Position.y-n_Padding, gl_FragCoord.wz);\n"

        "void main(void) {\n"
        "if (___gl_FragCoord.x+n_Padding < n_Padding ||\n"
        "    ___gl_FragCoord.x > n_Resolution.x ||\n"
        "    ___gl_FragCoord.y+n_Padding < n_Padding ||\n"
        "    ___gl_FragCoord.y > n_Resolution.y) {\n"
        "     gl_FragColor = texture2D(Texture, TexCoordOut.xy);\n"
        "} else {\n"
        "___main();\n"
        "}\n"
        "gl_FragColor = gl_FragColor * u_opacity;"
        "}\n";
    
    return this->compileShader(coop, GL_FRAGMENT_SHADER);
}

#if 0
void NativeCanvas2DContext::initCopyTex()
{
    glEnable(GL_TEXTURE_2D);
    GrRenderTarget* backingTarget = (GrRenderTarget*)m_Skia->canvas->
                                        getDevice()->accessRenderTarget();

    int width = backingTarget->asTexture()->width();
    int height =  backingTarget->asTexture()->height();

    glGenTextures(1, &gl.texture);
    glBindTexture(GL_TEXTURE_2D, gl.texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* Allocate memory for the new texture */
    glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            width, height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            NULL
    );

    glBindTexture(GL_TEXTURE_2D, 0);        

    /* Generate the FBO */
    glGenFramebuffers(1, &gl.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, gl.fbo);

    /* Set the FBO backing store using the new texture */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, gl.texture, 0);

    GLenum status;
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    switch(status) {
        case GL_FRAMEBUFFER_COMPLETE:
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            printf("fbo unsupported\n");
            return;
        default:
            printf("fbo fatal error\n");
            return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    gl.textureWidth = width;
    gl.textureHeight = height;
    glDisable(GL_TEXTURE_2D);
}
#endif
#if 0
void NativeCanvas2DContext::initCopyTex(uint32_t textureID)
{
    GrRenderTarget* backingTarget = (GrRenderTarget*)skia->canvas->
                                        getDevice()->accessRenderTarget();

    int width = backingTarget->asTexture()->width();
    int height =  backingTarget->asTexture()->height();

    SkDevice *dev = skia->canvas->createCompatibleDevice(SkBitmap::kARGB_8888_Config,
        width, height, false);

    gl.copy = new SkCanvas(dev);
    gl.texture = ((GrRenderTarget*)dev->accessRenderTarget())
                    ->asTexture()->getTextureHandle();

    gl.fbo = static_cast<GLuint>(((GrRenderTarget*)dev->accessRenderTarget())->
                    asTexture()->asRenderTarget()->getRenderTargetHandle());

    gl.textureWidth = width;
    gl.textureHeight = height;

    dev->unref();
}
#endif

uint32_t NativeCanvas2DContext::getMainFBO()
{
    GrRenderTarget* backingTarget = (GrRenderTarget*)m_Skia->canvas->
                                        getDevice()->accessRenderTarget();

    return (uint32_t)backingTarget->getRenderTargetHandle();
}

#if 0
void NativeCanvas2DContext::drawTexIDToFBO(uint32_t textureID, uint32_t width,
    uint32_t height, uint32_t left, uint32_t top, uint32_t fbo)
{
    SkISize size = m_Skia->canvas->getDeviceSize();

    GLenum err;
    if ((err = glGetError()) != GL_NO_ERROR) {
        printf("got a gl error %d\n", err);
    }

    /* save the old viewport size */
    glPushAttrib(GL_VIEWPORT_BIT);

    /* set the viewport with the texture size */
    glViewport(left, (float)size.fHeight-(height+top), width, height);

    glEnable(GL_TEXTURE_2D);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );    

    /* Anti Aliasing */
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_NOTEQUAL, 0.0f);
#if 1
    glBegin(GL_QUADS);
        /*
            (-1, 1)...........(1, 1)
                .               .
                .               .
                .               .
            (-1, -1)...........(1, -1)
        */
        glTexCoord3i(0, 0, 1);
          glVertex3f(-1., -1., 1.0f);

        glTexCoord3i(0, 1, 1);
          glVertex3f(-1, 1, 1.0f);

        glTexCoord3i(1, 1, 1);
          glVertex3f(1, 1, 1.0f);

        glTexCoord3i(1, 0, 1);
          glVertex3f(1, -1, 1.0f);
    glEnd();
#endif
    glDisable(GL_ALPHA_TEST);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glDisable(GL_SCISSOR_TEST);
    glDisable(GL_TEXTURE_2D);
    glPopAttrib();

}
#endif

void NativeCanvas2DContext::drawTexIDToFBO2(uint32_t textureID, uint32_t width,
    uint32_t height, uint32_t left, uint32_t top, uint32_t fbo)
{
    GLenum err;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );    

    /* Anti Aliasing */
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    //glEnable(GL_ALPHA_TEST);
    //glAlphaFunc(GL_NOTEQUAL, 0.0f);

    glDrawElements(GL_TRIANGLE_STRIP, m_GLObjects.vtx->nindices, GL_UNSIGNED_INT, 0);
    //glDisable(GL_ALPHA_TEST);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}


#if 0
void NativeCanvas2DContext::drawTexToFBO(uint32_t textureID)
{
    glEnable(GL_TEXTURE_2D);
    glClearColor(0, 0, 0, 0);

    if (!gl.fbo) {
        this->initCopyTex();
    }

    /* Use the current FBO */
    glBindFramebuffer(GL_FRAMEBUFFER, gl.fbo);

    /* save the old viewport size */
    glPushAttrib(GL_VIEWPORT_BIT);

    /* set the viewport with the texture size */
    glViewport(0, 0, gl.textureWidth, gl.textureHeight);

    /* clear the FBO */
    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, textureID);

    /* draw a textured quad on the new texture using textureID */
    glBegin(GL_QUADS);
        /*
            (-1, 1)...........(1, 1)
                .               .
                .               .
                .               .
            (-1, -1)...........(1, -1)
        */
        glTexCoord3i(0, 0, 1);
          glVertex3f(-1.0f, -1.0f, 1.0f);

        glTexCoord3i(0, 1, 1);
          glVertex3f(-1.0f, 1.0f, 1.0f);

        glTexCoord3i(1, 1, 1);
          glVertex3f( 1.0f, 1.0f, 1.0f);

        glTexCoord3i(1, 0, 1);
          glVertex3f( 1.0f, -1.0f, 1.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_TEXTURE_2D);
    glPopAttrib();
}
#endif

uint32_t NativeCanvas2DContext::getSkiaTextureID(int *width, int *height)
{
    GrRenderTarget* backingTarget = (GrRenderTarget*)m_Skia->canvas->
                                        getDevice()->accessRenderTarget();

    if (width != NULL && height != NULL) {
        SkISize size = m_Skia->canvas->getDeviceSize();

        *width = size.fWidth;
        *height = size.fHeight;
    }

    return backingTarget->asTexture()->getTextureHandle();
}

/* Ask skia to restore its GL state */
void NativeCanvas2DContext::resetSkiaContext(uint32_t flag)
{
    GrRenderTarget* backingTarget = (GrRenderTarget*)m_Skia->canvas->
                                        getDevice()->accessRenderTarget();

    if (flag == 0) {
        flag = kProgram_GrGLBackendState
                | kTextureBinding_GrGLBackendState
                | kVertex_GrGLBackendState
                | kView_GrGLBackendState;
    }

    backingTarget->getContext()->resetContext(flag);
}

uint32_t NativeCanvas2DContext::attachShader(const char *string)
{
    if ((m_GLObjects.program = this->createProgram(string))) {
        this->setupUniforms();

        m_GL.shader.uniformResolution = glGetUniformLocation(m_GLObjects.program,
                                    "n_Resolution");
        m_GL.shader.uniformPosition = glGetUniformLocation(m_GLObjects.program,
                                    "n_Position");
        m_GL.shader.uniformPadding = glGetUniformLocation(m_GLObjects.program,
                                    "n_Padding");        
    }

    return m_GLObjects.program;
}

void NativeCanvas2DContext::detachShader()
{
    /* TODO : shaders must be deleted */
    glDeleteProgram(m_GLObjects.program);
    m_GLObjects.program = 0;
}

void NativeCanvas2DContext::setupShader(float opacity, int width, int height,
    int left, int top, int wWidth, int wHeight)
{
    uint32_t program = this->getProgram();
    glUseProgram(program);
    float ratio = NativeSystemInterface::getInstance()->backingStorePixelRatio();

    if (program > 0) {
        if (m_GLObjects.uniforms.u_opacity != -1) {
            glUniform1f(m_GLObjects.uniforms.u_opacity, opacity);
        }
#if 1
        float padding = this->getHandler()->padding.global * ratio;

        if (m_GL.shader.uniformResolution != -1)
            glUniform2f(m_GL.shader.uniformResolution, (width)-(padding*2), (height)-(padding*2));
        if (m_GL.shader.uniformPosition != -1)
            glUniform2f(m_GL.shader.uniformPosition, ratio*left, ratio*wHeight - (height+ratio*top));
        if (m_GL.shader.uniformPadding != -1)
            glUniform1f(m_GL.shader.uniformPadding, padding);
#endif
    }

}

void NativeCanvas2DContext::composeWith(NativeCanvas2DContext *layer,
    double left, double top, double opacity,
    double zoom, const NativeRect *rclip)
{
    SkPaint pt;
    pt.setAlpha(opacity * (double)255.);
    float ratio = NativeSystemInterface::getInstance()->backingStorePixelRatio();

    NativeSkia *skia = layer->getSurface();
    SkISize layerSize = skia->canvas->getDeviceSize();

    if (rclip != NULL) {
        SkRect r;
        r.set(SkDoubleToScalar(rclip->fLeft*(double)ratio),
            SkDoubleToScalar(rclip->fTop*(double)ratio),
            SkDoubleToScalar(rclip->fRight*(double)ratio),
            SkDoubleToScalar(rclip->fBottom*(double)ratio));
        glEnable(GL_SCISSOR_TEST);
        glScissor(r.left(), layerSize.height()-(r.top()+r.height()), r.width(), r.height());
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
    /* TODO: disable alpha testing? */
    if (this->hasShader() && !commonDraw) {
        int width, height;
        //this->resetSkiaContext();
        //layer->flush();
        //this->flush();

        /* get the layer's Texture ID */
        uint32_t textureID = this->getSkiaTextureID(&width, &height);
        /* Use our custom shader */
        this->resetGLContext();

        this->setupShader((float)opacity, width, height,
            left, top,
            (int)layer->getHandler()->getWidth(),
            (int)layer->getHandler()->getHeight());

        //glDisable(GL_ALPHA_TEST);
        
        this->updateMatrix(left*ratio, top*ratio, layerSize.width(), layerSize.height());

        /* draw layer->skia->canvas (textureID) in skia->canvas (getMainFBO) */
        layer->drawTexIDToFBO2(textureID, width, height, left*ratio, top*ratio, layer->getMainFBO());

        /* Reset skia GL context */
        this->resetSkiaContext();

        return;

        /* Draw the temporary FBO into main canvas (root) */
        // /bitmapLayer = layer->gl.copy->getDevice()->accessBitmap(false);
    }

    if (this->commonDraw) {
        //return;
        const SkBitmap &bitmapLayer = this->getSurface()->canvas->getDevice()->accessBitmap(false);

        this->resetSkiaContext();
        layer->flush();
        this->flush();
        skia->canvas->scale(SkDoubleToScalar(zoom), SkDoubleToScalar(zoom));
        skia->canvas->drawBitmap(bitmapLayer,
            left*ratio, top*ratio, &pt);
        skia->canvas->scale(SkDoubleToScalar(1./zoom), SkDoubleToScalar(1./zoom));
        //skia->canvas->flush();
    } else {
        int width, height;
        skia->canvas->flush();
        this->getSurface()->canvas->flush();
        /* get the layer's Texture ID */
        uint32_t textureID = this->getSkiaTextureID(&width, &height);
        //printf("Texture size : %dx%d (%d)\n", width, height, textureID);
        glUseProgram(0);
        //NLOG("Composing...");
        //layer->drawTexIDToFBO(textureID, width, height, left*ratio, top*ratio, layer->getMainFBO());
        //this->resetSkiaContext();            
    }
    
    
}

void NativeCanvas2DContext::flush()
{
    m_Skia->canvas->flush();
}

void NativeCanvas2DContext::getSize(int *width, int *height) const
{
    SkISize size = this->m_Skia->canvas->getDeviceSize();

    *width = size.width();
    *height = size.height();
}

void NativeCanvas2DContext::setSize(int width, int height)
{
    SkBaseDevice *ndev = NULL;
    SkCanvas *ncanvas;

    float ratio = NativeSystemInterface::getInstance()->backingStorePixelRatio();

    const SkBitmap &bt = m_Skia->canvas->getDevice()->accessBitmap(false);

    if (m_Skia->native_canvas_bind_mode == NativeSkia::BIND_GL) {
        ncanvas = NativeSkia::createGLCanvas(width, height);
        NativeSkia::glcontext = ncanvas;
    } else {
        ndev = NativeSkia::glcontext->createCompatibleDevice(SkBitmap::kARGB_8888_Config,
                                    width*ratio, height*ratio, false);

        if (ndev == NULL) {
            printf("Cant create canvas of size %dx%d (backstore ratio : %f)\n", width, height, ratio);
            return;
        }

        ncanvas = new SkCanvas(ndev);
    }
    
    ncanvas->drawBitmap(bt, 0, 0);
    //ncanvas->clipRegion(skia->canvas->getTotalClip());
    ncanvas->setMatrix(m_Skia->canvas->getTotalMatrix());

    SkSafeUnref(ndev);
    m_Skia->canvas->unref();
    m_Skia->canvas = ncanvas;

}

void NativeCanvas2DContext::translate(double x, double y)
{
    m_Skia->canvas->translate(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

NativeCanvas2DContext::NativeCanvas2DContext(NativeCanvasHandler *handler,
    JSContext *cx, int width, int height) :
    NativeCanvasContext(handler),
    setterDisabled(false), commonDraw(true)
{
    m_Mode = CONTEXT_2D;

    jsobj = JS_NewObject(cx, &Canvas2DContext_class, NULL, NULL);
    jscx  = cx;

    JS_DefineFunctions(cx, jsobj, canvas2dctx_funcs);
    JS_DefineProperties(cx, jsobj, canvas2dctx_props);
    JSObject *saved = JS_NewArrayObject(cx, 0, NULL);
    JS_SetReservedSlot(jsobj, 0, OBJECT_TO_JSVAL(saved));

    m_Skia = new NativeSkia();
    m_Skia->bindOnScreen(width, height);

    JS_SetPrivate(jsobj, this);

    memset(&this->m_GL, 0, sizeof(this->m_GL));
    memset(&this->m_GL.shader, -1, sizeof(this->m_GL.shader));

    /* Vertex buffers were unbound by parent constructor */
    this->resetSkiaContext(kVertex_GrGLBackendState);
}

NativeCanvas2DContext::NativeCanvas2DContext(NativeCanvasHandler *handler,
    int width, int height, bool isGL) :
    NativeCanvasContext(handler),
    commonDraw(true)
{
    m_Mode = CONTEXT_2D;
    
    m_Skia = new NativeSkia();
    if (isGL) {
        NLOG("Window framebuffer is at %p", this);
        m_Skia->bindGL(width, height);
    } else {
        m_Skia->bindOnScreen(width, height);
    }
    memset(&this->m_GL, 0, sizeof(this->m_GL));

    /* Vertex buffers were unbound by parent constructor */
    this->resetSkiaContext(kVertex_GrGLBackendState);
}

void NativeCanvas2DContext::setScale(double x, double y,
    double px, double py)
{
    m_Skia->scale(1./px, 1./py);

    m_Skia->scale(x, y);
}

NativeCanvas2DContext::~NativeCanvas2DContext()
{
    //NLOG("Delete skia %p", skia);
    delete m_Skia;
}

static JSBool native_Canvas2DContext_constructor(JSContext *cx,
    unsigned argc, jsval *vp)
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
