#include "NativeCanvas2DContext.h"

#include <SkDevice.h>
#include <SkGpuDevice.h>

#include "NativeJSCanvas.h"
#include "NativeJSDocument.h"
#include "NativeSkia.h"
#include "NativeSkGradient.h"
#include "NativeSkImage.h"
#include "NativeJSImage.h"
#include "NativeSystemInterface.h"
#include "NativeMacros.h"
#include "NativeOpenGLHeader.h"

#define CANVASCTX_GETTER(obj) ((class NativeCanvas2DContext *)JS_GetPrivate(obj))
#define NSKIA_NATIVE_GETTER(obj) ((class NativeSkia *)((class NativeCanvas2DContext *)JS_GetPrivate(obj))->getSurface())
#define NSKIA_NATIVE (CppObj->getSurface())
#define HANDLER_GETTER(obj) ((NativeCanvasHandler *)((class NativeJSCanvas *)JS_GetPrivate(obj))->getHandler())

#define CANVASCTX_PROLOGUE

static JSClass imageData_class = {
    "ImageData", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

JSClass *NativeCanvas2DContext::ImageData_jsclass = &imageData_class;

enum {
#define CANVAS_2D_CTX_PROP(prop) CTX_PROP_ ## prop,
#define CANVAS_2D_CTX_PROP_GET(prop) CTX_PROP_ ## prop,
  #include "NativeCanvas2DContextProperties.h"
  CTX_PROP__NPROP
#undef CANVAS_2D_CTX_PROP
#undef CANVAS_2D_CTX_PROP_GET
};

#if 0
#define NATIVE_LOG_2D_CALL() \
    JS::RootedObject _callee(cx, JS_CALLEE(cx, vp).toObject()); \
    if (JS_ObjectIsFunction(cx, _callee)) { \
        JS::RootedString _fun_name(cx, JS_GetFunctionDisplayId(JS_ValueToFunction(cx, JS_CALLEE(cx, vp)))); \
        JSAutoByteString _fun_namec(cx, _fun_name); \
        NLOG("[Canvas2D] %s()", _fun_namec.ptr()); \
    }
#else
#define NATIVE_LOG_2D_CALL()
#endif

void CanvasGradient_Finalize(JSFreeOp *fop, JSObject *obj);
void CanvasPattern_Finalize(JSFreeOp *fop, JSObject *obj);
void Canvas2DContext_Finalize(JSFreeOp *fop, JSObject *obj);

extern JSClass Canvas_class;

JSClass Canvas2DContext_class = {
    "CanvasRenderingContext2D", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Canvas2DContext_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass canvasGradient_class = {
    "CanvasGradient", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, CanvasGradient_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass canvasGLProgram_class = {
    "CanvasGLProgram", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass canvasPattern_class = {
    "CanvasPattern", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, CanvasPattern_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static bool native_canvas2dctx_prop_set(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp);
static bool native_canvas2dctx_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp);

static bool native_canvas2dctx_breakText(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_shadow(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_fillRect(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_strokeRect(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_clearRect(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_fillText(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_strokeText(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_beginPath(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_moveTo(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_lineTo(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_fill(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_stroke(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_closePath(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_arc(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_arcTo(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_rect(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_quadraticCurveTo(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctx_bezierCurveTo(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctx_rotate(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_scale(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_save(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_restore(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_translate(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_transform(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_iTransform(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_setTransform(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctx_clip(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_createImageData(JSContext *cx,
    unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_createPattern(JSContext *cx,
    unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_putImageData(JSContext *cx,
    unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_getImageData(JSContext *cx,
    unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_createLinearGradient(JSContext *cx,
    unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_createRadialGradient(JSContext *cx,
    unsigned argc, JS::Value *vp);
static bool native_canvas2dctxGradient_addColorStop(JSContext *cx,
    unsigned argc, JS::Value *vp);

static bool native_canvas2dctx_stub(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_drawImage(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_canvas2dctx_measureText(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctx_isPointInPath(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctx_getPathBounds(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctx_light(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctx_attachGLSLFragment(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctx_detachGLSLFragment(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctx_setVertexOffset(JSContext *cx, unsigned argc,
    JS::Value *vp);

/* GLSL related */
static bool native_canvas2dctxGLProgram_getUniformLocation(JSContext *cx, unsigned argc,
    JS::Value *vp);

static bool native_canvas2dctxGLProgram_uniform1i(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctxGLProgram_uniform1f(JSContext *cx, unsigned argc,
    JS::Value *vp);

static bool native_canvas2dctxGLProgram_uniform1iv(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctxGLProgram_uniform2iv(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctxGLProgram_uniform3iv(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctxGLProgram_uniform4iv(JSContext *cx, unsigned argc,
    JS::Value *vp);

static bool native_canvas2dctxGLProgram_uniform1fv(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctxGLProgram_uniform2fv(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctxGLProgram_uniform3fv(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctxGLProgram_uniform4fv(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_canvas2dctxGLProgram_getActiveUniforms(JSContext *cx, unsigned argc,
    JS::Value *vp);

static JSPropertySpec canvas2dctx_props[] = {
#define CANVAS_2D_CTX_PROP(prop) {#prop, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS, \
    NATIVE_JS_STUBGETTER(), \
    NATIVE_JS_SETTER(CTX_PROP_ ## prop, native_canvas2dctx_prop_set)},
#define CANVAS_2D_CTX_PROP_GET(prop) {#prop, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS, \
    NATIVE_JS_GETTER(CTX_PROP_ ## prop, native_canvas2dctx_prop_get), \
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
    JS_FN("strokeText", native_canvas2dctx_strokeText, 3, 0),
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
    JS_FN("arcTo", native_canvas2dctx_arcTo, 5, 0),
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
    JS_FN("drawImage", native_canvas2dctx_drawImage, 3, 0),
    JS_FN("measureText", native_canvas2dctx_measureText, 1, 0),
    JS_FN("isPointInPath", native_canvas2dctx_isPointInPath, 2, 0),
    JS_FN("getPathBounds", native_canvas2dctx_getPathBounds, 0, 0),
    JS_FN("light", native_canvas2dctx_light, 3, 0),
    JS_FN("attachFragmentShader", native_canvas2dctx_attachGLSLFragment, 1, 0),
    JS_FN("detachFragmentShader", native_canvas2dctx_detachGLSLFragment, 0, 0),
    JS_FN("setVertexOffset", native_canvas2dctx_setVertexOffset, 3, 0),
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

static bool native_canvas2dctx_stub(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NATIVE_LOG_2D_CALL();

    return true;
}

static bool native_canvas2dctx_fillRect(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    double x, y, width, height, rx = 0, ry = 0;

    if (!JS_ConvertArguments(cx, args, "dddd/dd", &x, &y, &width, &height, &rx, &ry)) {
        return false;
    }

    if (argc > 4) {
        NSKIA_NATIVE->drawRect(x, y, width, height,
            rx, (argc == 5 ? rx : ry), 0);
    } else {
        NSKIA_NATIVE->drawRect(x, y, width, height, 0);
    }

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_strokeRect(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    double x, y, width, height, rx = 0, ry = 0;
    if (!JS_ConvertArguments(cx, args, "dddd/dd", &x, &y, &width, &height, &rx, &ry)) {
        return false;
    }

    if (argc > 4) {
        NSKIA_NATIVE->drawRect(x, y, width, height,
            rx, (argc == 5 ? rx : ry), 1);
    } else {
        NSKIA_NATIVE->drawRect(x, y, width, height, 1);
    }

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_clearRect(JSContext *cx, unsigned argc, JS::Value *vp)
{
    double x, y, width, height;

    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);

    if (!JS_ConvertArguments(cx, args, "dddd", &x, &y, &width, &height)) {
        return false;
    }

    CppObj->getSurface()->clearRect(x, y, width, height);

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_breakText(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);

#define SET_PROP(where, name, val) JS_DefineProperty(cx, where, \
    (const char *)name, val, JSPROP_PERMANENT | JSPROP_READONLY | \
        JSPROP_ENUMERATE)
    JS::RootedString str(cx);
    JS::RootedObject res(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));
    double maxWidth;
    int length = 0;

    if (!JS_ConvertArguments(cx, args, "Sd", str.address(), &maxWidth)) {
        return false;
    }

    JSAutoByteString text(cx, str);
    size_t len = text.length();

    if (len == 0) {
        vp->setNull();
        return true;
    }

    struct _NativeLine *lines = new struct _NativeLine[len];

    if (!lines) {
        JS_ReportOutOfMemory(cx);
        return false;
    }

    memset(lines, 0, len * sizeof(struct _NativeLine));

    SkScalar ret = CppObj->getSurface()->breakText(text.ptr(), len,
                    lines, maxWidth, &length);
    JS::RootedObject alines(cx, JS_NewArrayObject(cx, length));

    for (int i = 0; i < len && i < length; i++) {
        JS::RootedValue val(cx, STRING_TO_JSVAL(JS_NewStringCopyN(cx,
            lines[i].line, lines[i].len)));
        JS_SetElement(cx, alines, i, val);
    }
    JS::RootedValue heightVal(cx, DOUBLE_TO_JSVAL(SkScalarToDouble(ret)));
    JS::RootedValue linesVal(cx, OBJECT_TO_JSVAL(alines));
    SET_PROP(res, "height", heightVal);
    SET_PROP(res, "lines", linesVal);

    args.rval().set(OBJECT_TO_JSVAL(res));

    delete[] lines;

    NATIVE_LOG_2D_CALL();

    return true;
#undef SET_PROP
}

static bool native_canvas2dctx_fillText(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    int x, y, maxwidth;
    JS::RootedString str(cx);

    if (!JS_ConvertArguments(cx, args, "Sii/i", str.address(), &x, &y, &maxwidth)) {
        return false;
    }

    JSAutoByteString text;
    text.encodeUtf8(cx, str);

    NSKIA_NATIVE->drawText(text.ptr(), x, y);

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_strokeText(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    int x, y, maxwidth;
    JS::RootedString str(cx);

    if (!JS_ConvertArguments(cx, args, "Sii/i", str.address(), &x, &y, &maxwidth)) {
        return false;
    }

    JSAutoByteString text;
    text.encodeUtf8(cx, str);

    NSKIA_NATIVE->drawText(text.ptr(), x, y, true);

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_shadow(JSContext *cx, unsigned argc, JS::Value *vp)
{
    //NSKIA_NATIVE->setShadow();
    return true;
}

static bool native_canvas2dctx_beginPath(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    NSKIA_NATIVE->beginPath();

    return true;
}

static bool native_canvas2dctx_moveTo(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    double x, y;

    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        return false;
    }

    NSKIA_NATIVE->moveTo(x, y);

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_lineTo(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    double x, y;

    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        return false;
    }

    NSKIA_NATIVE->lineTo(x, y);

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_fill(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    NSKIA_NATIVE->fill();

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_stroke(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    NSKIA_NATIVE->stroke();

    NATIVE_LOG_2D_CALL();
    return true;
}
static bool native_canvas2dctx_closePath(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    NSKIA_NATIVE->closePath();

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_clip(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    NSKIA_NATIVE->clip();

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_rect(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    double x, y, width, height;

    if (!JS_ConvertArguments(cx, args, "dddd", &x, &y, &width, &height)) {
        return false;
    }

    NSKIA_NATIVE->rect(x, y, width, height);

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_arc(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    int x, y, radius;
    double startAngle, endAngle;
    bool CCW = false;

    if (!JS_ConvertArguments(cx, args, "iiidd/b", &x, &y, &radius, &startAngle,
        &endAngle, &CCW)) {
        return false;
    }

    NSKIA_NATIVE->arc(x, y, radius, startAngle, endAngle, CCW);

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_arcTo(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    int x1, y1, x2, y2, radius;

    if (!JS_ConvertArguments(cx, args, "iiiii", &x1, &y1, &x2, &y2, &radius)) {
        return false;
    }

    NSKIA_NATIVE->arcTo(x1, y1, x2, y2, radius);

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_quadraticCurveTo(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    double x, y, cpx, cpy;
    if (!JS_ConvertArguments(cx, args, "dddd", &cpx, &cpy, &x, &y)) {
        return false;
    }

    NSKIA_NATIVE->quadraticCurveTo(cpx, cpy, x, y);

    return true;
}

static bool native_canvas2dctx_bezierCurveTo(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    double x, y, cpx, cpy, cpx2, cpy2;
    if (!JS_ConvertArguments(cx, args, "dddddd", &cpx, &cpy, &cpx2, &cpy2,
        &x, &y)) {
        return false;
    }

    NSKIA_NATIVE->bezierCurveTo(cpx, cpy, cpx2, cpy2, x, y);

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_rotate(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    double angle;

    if (!JS_ConvertArguments(cx, args, "d", &angle)) {
        return false;
    }

    NSKIA_NATIVE->rotate(angle);

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_scale(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    double x, y;

    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        return false;
    }

    NSKIA_NATIVE->scale(x, y);

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_translate(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    double x, y;

    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        return false;
    }

    NSKIA_NATIVE->translate(x, y);

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_transform(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    double scalex, skewx, skewy, scaley, translatex, translatey, rotate;

    if (!JS_ConvertArguments(cx, args, "dddddd/d", &scalex, &skewx, &skewy,
        &scaley, &translatex, &translatey, &rotate)) {
        return false;
    }

    NSKIA_NATIVE->transform(scalex, skewx, skewy, scaley,
        translatex, translatey, 0);

    if (argc == 7) {
        NSKIA_NATIVE->rotate(rotate);
    }

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_iTransform(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    double scalex, skewx, skewy, scaley, translatex, translatey;

    if (!JS_ConvertArguments(cx, args, "dddddd", &scalex, &skewx, &skewy, &scaley, &translatex, &translatey)) {
        return false;
    }

    NSKIA_NATIVE->itransform(scalex, skewx, skewy, scaley,
        translatex, translatey);

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_setTransform(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    double scalex, skewx, skewy, scaley, translatex, translatey;

    NativeCanvasHandler *handler = CppObj->getHandler();

    if (!JS_ConvertArguments(cx, args, "dddddd", &scalex, &skewx, &skewy, &scaley, &translatex, &translatey)) {
        return false;
    }

    NSKIA_NATIVE->transform(scalex, skewx, skewy, scaley,
        translatex+handler->padding.global, translatey+handler->padding.global, 1);

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_save(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    JS::RootedValue slot(cx, JS_GetReservedSlot(thisobj, 0));
    JS::RootedObject savedArray(cx, &slot.toObject());
    JS::RootedObject saved(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));
    JS::RootedValue outval(cx);

#define CANVAS_2D_CTX_PROP_GET(prop)
#define CANVAS_2D_CTX_PROP(prop)    JS_GetProperty(cx, thisobj, #prop, &outval); \
                                    JS_SetProperty(cx, saved, #prop, outval);

#include "NativeCanvas2DContextProperties.h"

#undef CANVAS_2D_CTX_PROP
#undef CANVAS_2D_CTX_PROP_GET
    uint32_t arr_length;
    JS::RootedValue savedVal(cx, OBJECT_TO_JSVAL(saved));

    JS_GetArrayLength(cx, savedArray, &arr_length);
    JS_SetElement(cx, savedArray, arr_length, savedVal);

    CppObj->getSurface()->save();

    NATIVE_LOG_2D_CALL();

    return true;
}

static bool native_canvas2dctx_restore(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    JS::RootedValue slot(cx, JS_GetReservedSlot(thisobj, 0));
    JS::RootedObject savedArray(cx, &slot.toObject());
    NSKIA_NATIVE->restore();

    uint32_t arr_length = 0;
    JS::RootedValue saved(cx);
    JS::RootedValue  outval(cx);
    if (JS_GetArrayLength(cx, savedArray, &arr_length) == false) {
        return true;
    }
    if (arr_length == 0) {
        return true;
    }
    JS_GetElement(cx, savedArray, arr_length - 1, &saved);
    JS::RootedObject savedObj(cx, &saved.toObject());

#define CANVAS_2D_CTX_PROP_GET(prop)
#define CANVAS_2D_CTX_PROP(prop)    JS_GetProperty(cx, savedObj, #prop, &outval); \
                                    JS_SetProperty(cx, thisobj, #prop, outval);

    CppObj->setterDisabled = true;
#include "NativeCanvas2DContextProperties.h"
    CppObj->setterDisabled = false;

#undef CANVAS_2D_CTX_PROP
#undef CANVAS_2D_CTX_PROP_GET
    JS_SetArrayLength(cx, savedArray, arr_length-1);
    return true;
}

static bool native_canvas2dctx_createLinearGradient(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject linearObject(cx);
    double x1, y1, x2, y2;
    if (!JS_ConvertArguments(cx, args, "dddd", &x1, &y1, &x2, &y2)) {
        return false;
    }

    linearObject = JS_NewObject(cx, &canvasGradient_class, JS::NullPtr(), JS::NullPtr());

    args.rval().set(OBJECT_TO_JSVAL(linearObject));

    JS_SetPrivate(linearObject, new NativeSkGradient(x1, y1, x2, y2));

    JS_DefineFunctions(cx, linearObject, gradient_funcs);

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_getImageData(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    int left, top, width, height;
    JS::RootedObject dataObject(cx);
    JS::RootedObject arrBuffer(cx);
    uint8_t *data;

    if (!JS_ConvertArguments(cx, args, "iiii", &left, &top, &width, &height)) {
        return false;
    }

    dataObject = JS_NewObject(cx, &imageData_class, JS::NullPtr(), JS::NullPtr());

    arrBuffer = JS_NewUint8ClampedArray(cx, width*height * 4);
    data = JS_GetUint8ClampedArrayData(arrBuffer);

    NSKIA_NATIVE->readPixels(top, left, width, height, data);
    JS::RootedValue widthVal(cx, UINT_TO_JSVAL(width));
    JS::RootedValue heightVal(cx, UINT_TO_JSVAL(height));
    JS::RootedValue arVal(cx, OBJECT_TO_JSVAL(arrBuffer));
    JS_DefineProperty(cx, dataObject, "width", widthVal,
         JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "height", heightVal,
        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "data", arVal,
        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    args.rval().set(OBJECT_TO_JSVAL(dataObject));

    NATIVE_LOG_2D_CALL();
    return true;
}

/* TODO: Huge memory leak? */
static bool native_canvas2dctx_putImageData(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    JS::RootedObject dataObject(cx);
    int x, y;
    uint8_t *pixels;
    JS::RootedValue jdata(cx);
    JS::RootedValue jwidth(cx);
    JS::RootedValue jheight(cx);

    if (!JS_ConvertArguments(cx, args, "oii", dataObject.address(), &x, &y)) {
        return false;
    }
    if (JS_GetClass(dataObject) != &imageData_class) {
        JS_ReportError(cx, "First argument must be a imageData object");
        return false;
    }

    JS_GetProperty(cx, dataObject, "data", &jdata);
    JS_GetProperty(cx, dataObject, "width", &jwidth);
    JS_GetProperty(cx, dataObject, "height", &jheight);

    pixels = JS_GetUint8ClampedArrayData(&jdata.toObject());

    NSKIA_NATIVE->drawPixels(pixels, jwidth.toInt32(), jheight.toInt32(),
        x, y);

    NATIVE_LOG_2D_CALL();

    return true;
}

/* TODO: clamp max size */
static bool native_canvas2dctx_createImageData(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject dataObject(cx);
    JS::RootedObject arrBuffer(cx);
    unsigned long x, y;

    if (!JS_ConvertArguments(cx, args, "uu", &x, &y)) {
        return false;
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
        return false;
    }

    dataObject = JS_NewObject(cx, &imageData_class, JS::NullPtr(), JS::NullPtr());
    JS::RootedValue width(cx, UINT_TO_JSVAL(x));
    JS::RootedValue height(cx, UINT_TO_JSVAL(y));

    JS_DefineProperty(cx, dataObject, "width", width,
        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS_DefineProperty(cx, dataObject, "height", height,
        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS::RootedValue array(cx, OBJECT_TO_JSVAL(arrBuffer));
    JS_DefineProperty(cx, dataObject, "data", array,
        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    args.rval().set(OBJECT_TO_JSVAL(dataObject));

    NATIVE_LOG_2D_CALL();

    return true;
}

static bool native_canvas2dctx_createPattern(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject jsimage(cx);
    JS::RootedObject patternObject(cx);
    JS::RootedString mode(cx);

    if (!JS_ConvertArguments(cx, args, "oS", jsimage.address(), mode.address())) {
        return false;
    }

    if (!NativeJSImage::JSObjectIs(cx, jsimage)) {
        JS_ReportError(cx, "First parameter is not an Image");
        return false;
    }

    patternObject = JS_NewObject(cx, &canvasPattern_class, JS::NullPtr(), JS::NullPtr());

    args.rval().set(OBJECT_TO_JSVAL(patternObject));

    NativeJSImage *img = (NativeJSImage *)JS_GetPrivate(jsimage);

    JS_SetReservedSlot(patternObject, 0, OBJECT_TO_JSVAL(img->getJSObject()));

    NativeCanvasPattern::PATTERN_MODE pmode = NativeCanvasPattern::PATTERN_REPEAT;

    JSAutoByteString cmode(cx, mode);

    if (strcasecmp(cmode.ptr(), "repeat") == 0) {
        pmode = NativeCanvasPattern::PATTERN_REPEAT;
    } else if (strcasecmp(cmode.ptr(), "no-repeat") == 0) {
        pmode = NativeCanvasPattern::PATTERN_NOREPEAT;
    } else if (strcasecmp(cmode.ptr(), "repeat-x") == 0) {
        pmode = NativeCanvasPattern::PATTERN_REPEAT_X;
    } else if (strcasecmp(cmode.ptr(), "repeat-y") == 0) {
        pmode = NativeCanvasPattern::PATTERN_REPEAT_Y;
    } else if (strcasecmp(cmode.ptr(), "repeat-mirror") == 0) {
        pmode = NativeCanvasPattern::PATTERN_REPEAT_MIRROR;
    }

    JS_SetPrivate(patternObject,
        new NativeCanvasPattern((NativeJSImage *)JS_GetPrivate(jsimage), pmode));

    NATIVE_LOG_2D_CALL();

    return true;
}

static bool native_canvas2dctx_createRadialGradient(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject linearObject(cx);
    double x1, y1, x2, y2, r1, r2;

    if (!JS_ConvertArguments(cx, args, "dddddd", &x1, &y1, &r1, &x2, &y2, &r2)) {
        return false;
    }

    linearObject = JS_NewObject(cx, &canvasGradient_class, JS::NullPtr(), JS::NullPtr());

    args.rval().set(OBJECT_TO_JSVAL(linearObject));

    JS_SetPrivate(linearObject,
        new NativeSkGradient(x1, y1, r1, x2, y2, r2));

    JS_DefineFunctions(cx, linearObject, gradient_funcs);

    NATIVE_LOG_2D_CALL();

    return true;
}

static bool native_canvas2dctxGradient_addColorStop(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedString color(cx);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    double position;
    NativeSkGradient *gradient;

    if (!JS_ConvertArguments(cx, args, "dS", &position, color.address())) {
        return false;
    }

    if ((gradient = (NativeSkGradient *)JS_GetPrivate(caller)) != NULL) {
        JSAutoByteString colorstr(cx, color);

        gradient->addColorStop(position, colorstr.ptr());
    }

    NATIVE_LOG_2D_CALL();

    return true;
}


static bool native_canvas2dctx_drawImage(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    JS::RootedObject jsimage(cx);
    NativeSkImage *image;
    double x, y, width, height;
    int sx, sy, swidth, sheight;
    int need_free = 0;

    if (argc == 9) {
         if (!JS_ConvertArguments(cx, args, "oiiiidddd", jsimage.address(),
                &sx, &sy, &swidth, &sheight, &x, &y, &width, &height)) {
            return false;
        }
    } else {
        if (!JS_ConvertArguments(cx, args, "odd/dd", jsimage.address(), &x, &y, &width, &height)) {
            return false;
        }
    }

    /* The image is a Canvas */
    /*
        TODO: work with WebGL canvas
    */
    if (JS_GetClass(jsimage) == &Canvas_class) {
        NativeCanvasContext *drawctx = HANDLER_GETTER(jsimage)->getContext();

        if (drawctx == NULL || drawctx->getContextType() != NativeCanvasContext::CONTEXT_2D) {
            JS_ReportError(cx, "Invalid image canvas (must be backed by a 2D context)");
            return false;
        }
        image = new NativeSkImage(((NativeCanvas2DContext *)drawctx)->getSurface()->getCanvas());
        need_free = 1;
    } else if (!NativeJSImage::JSObjectIs(cx, jsimage) ||
        (image = NativeJSImage::JSObjectToNativeSkImage(jsimage)) == NULL) {

        JS_ReportWarning(cx, "Invalid image given");
        return true;
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

    NATIVE_LOG_2D_CALL();

    return true;
}

static bool native_canvas2dctx_measureText(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    JS::RootedString text(cx);
#define OBJ_PROP(name, val) JS_DefineProperty(cx, obj, name, \
    val, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY)

    if (!JS_ConvertArguments(cx, args, "S", text.address())) {
        return false;
    }

    JS::RootedObject obj(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));

    JSAutoByteString ctext;
    ctext.encodeUtf8(cx, text);

    NativeSkia *n = NSKIA_NATIVE;
    JS::RootedValue widthVal(cx, DOUBLE_TO_JSVAL(n->measureText(ctext.ptr(), strlen(ctext.ptr()))));
    OBJ_PROP("width", widthVal);

    args.rval().setObjectOrNull(obj);

    NATIVE_LOG_2D_CALL();

    return true;
}

static bool native_canvas2dctx_isPointInPath(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    double x, y;

    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        vp->setBoolean(false);
        return false;
    }

    NativeSkia *n = NSKIA_NATIVE;

    vp->setBoolean(n->SkPathContainsPoint(x, y));

    NATIVE_LOG_2D_CALL();

    return true;
}

/* TODO: return undefined if the path is invalid */
static bool native_canvas2dctx_getPathBounds(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
#define OBJ_PROP(name, val) JS_DefineProperty(cx, obj, name, \
    val, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY)
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    double left = 0, right = 0, top = 0, bottom = 0;
    JS::RootedObject obj(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));

    NSKIA_NATIVE->getPathBounds(&left, &right, &top, &bottom);
    JS::RootedValue leftVal(cx, DOUBLE_TO_JSVAL(left));
    JS::RootedValue rightVal(cx, DOUBLE_TO_JSVAL(right));
    JS::RootedValue topVal(cx, DOUBLE_TO_JSVAL(top));
    JS::RootedValue bottomVal(cx, DOUBLE_TO_JSVAL(bottom));
    OBJ_PROP("left", leftVal);
    OBJ_PROP("right", rightVal);
    OBJ_PROP("top", topVal);
    OBJ_PROP("bottom", bottomVal);

    vp->setObject(*obj);

    NATIVE_LOG_2D_CALL();

    return true;
}

static bool native_canvas2dctx_detachGLSLFragment(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    CppObj->detachShader();

    NATIVE_LOG_2D_CALL();

    return true;
}

static bool native_canvas2dctx_setVertexOffset(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);

    uint32_t vertex;
    double x, y;

    if (!JS_ConvertArguments(cx, args, "udd", &vertex, &x, &y)) {
        return false;
    }

    CppObj->setVertexDeformation(vertex, x, y);

    NATIVE_LOG_2D_CALL();

    return true;
}

static bool native_canvas2dctx_attachGLSLFragment(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    JS::RootedString glsl(cx);
    size_t program;

    if (!JS_ConvertArguments(cx, args, "S", glsl.address())) {
        return false;
    }

    JSAutoByteString cglsl(cx, glsl);

    if ((program = CppObj->attachShader(cglsl.ptr())) == 0) {
        JS_ReportError(cx, "Failed to compile GLSL shader");
        return false;
    }
    JS::RootedObject canvasProgram(cx, JS_NewObject(cx, &canvasGLProgram_class, JS::NullPtr(), JS::NullPtr()));

    args.rval().set(OBJECT_TO_JSVAL(canvasProgram));

    JS_DefineFunctions(cx, canvasProgram, glprogram_funcs);

    JS_SetPrivate(canvasProgram, (void *)program);

    NATIVE_LOG_2D_CALL();

    return true;
}

static bool native_canvas2dctxGLProgram_getUniformLocation(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedString location(cx);
    JS::RootedObject caller(cx,  JS_THIS_OBJECT(cx, vp));
    uint32_t program;

    if (!JS_ConvertArguments(cx, args, "S", location.address())) {
        return false;
    }

    JSAutoByteString clocation(cx, location);

    program = (size_t)JS_GetPrivate(caller);

    int ret = glGetUniformLocation(program, clocation.ptr());

    args.rval().set(INT_TO_JSVAL(ret));

    NATIVE_LOG_2D_CALL();

    return true;
}

static bool native_canvas2dctxGLProgram_uniform1i(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    int location, val;
    uint32_t program;

    if (!JS_ConvertArguments(cx, args, "ii", &location, &val)) {
        return false;
    }

    if (location == -1) {
        return true;
    }
    program = (size_t)JS_GetPrivate(caller);
    int32_t tmpProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &tmpProgram);
    glUseProgram(program);
    glUniform1i(location, val);
    glUseProgram(tmpProgram);

    NATIVE_LOG_2D_CALL();

    return true;
}

static bool native_canvas2dctxGLProgram_uniform1f(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    int location;
    double val;
    uint32_t program;

    if (!JS_ConvertArguments(cx, args, "id", &location, &val)) {
        return false;
    }

    if (location == -1) {
        return true;
    }

    program = (size_t)JS_GetPrivate(caller);
    int32_t tmpProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &tmpProgram);
    glUseProgram(program);
    glUniform1f(location, (float)val);
    glUseProgram(tmpProgram);

    NATIVE_LOG_2D_CALL();

    return true;
}

static bool native_canvas2dctxGLProgram_uniformXiv(JSContext *cx,
    unsigned int argc, JS::Value *vp, int nb)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject tmp(cx);
    JS::RootedObject array(cx);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    GLsizei length;
    GLint *carray;
    int location;
    uint32_t program;
    program = (size_t)JS_GetPrivate(caller);

    if (!JS_ConvertArguments(cx, args, "io", &location, array.address())) {
        return false;
    }

    if (location == -1) {
        return true;
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
        return false;
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

    NATIVE_LOG_2D_CALL();

    return true;
}

static bool native_canvas2dctxGLProgram_uniformXfv(JSContext *cx,
    unsigned int argc, JS::Value *vp, int nb)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject array(cx);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    GLsizei length;
    GLfloat *carray;
    int location;
    uint32_t program;
    program = (size_t)JS_GetPrivate(caller);

    if (!JS_ConvertArguments(cx, args, "io", &location, array.address())) {
        return false;
    }

    if (location == -1) {
        return true;
    }

    if (JS_IsFloat32Array(array)) {
        carray = (GLfloat *)JS_GetFloat32ArrayData(array);
        length = (GLsizei)JS_GetTypedArrayLength(array);
    } else if (JS_IsArrayObject(cx, array)) {
        JS::RootedObject tmp(cx, JS_NewFloat32ArrayFromArray(cx, array));
        carray = (GLfloat *)JS_GetFloat32ArrayData(tmp);
        length = (GLsizei)JS_GetTypedArrayLength(tmp);
    } else {
        JS_ReportError(cx, "Array is not a Float32 array");
        return false;
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

    NATIVE_LOG_2D_CALL();

    return true;
}

static bool native_canvas2dctxGLProgram_uniform1iv(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    return native_canvas2dctxGLProgram_uniformXiv(cx, argc, vp, 1);
}

static bool native_canvas2dctxGLProgram_uniform2iv(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    return native_canvas2dctxGLProgram_uniformXiv(cx, argc, vp, 2);
}

static bool native_canvas2dctxGLProgram_uniform3iv(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    return native_canvas2dctxGLProgram_uniformXiv(cx, argc, vp, 3);
}

static bool native_canvas2dctxGLProgram_uniform4iv(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    return native_canvas2dctxGLProgram_uniformXiv(cx, argc, vp, 4);
}

static bool native_canvas2dctxGLProgram_uniform1fv(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    return native_canvas2dctxGLProgram_uniformXfv(cx, argc, vp, 1);
}

static bool native_canvas2dctxGLProgram_uniform2fv(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    return native_canvas2dctxGLProgram_uniformXfv(cx, argc, vp, 2);
}

static bool native_canvas2dctxGLProgram_uniform3fv(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    return native_canvas2dctxGLProgram_uniformXfv(cx, argc, vp, 3);
}

static bool native_canvas2dctxGLProgram_uniform4fv(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    return native_canvas2dctxGLProgram_uniformXfv(cx, argc, vp, 4);
}

static bool native_canvas2dctxGLProgram_getActiveUniforms(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
#define SET_PROP(where, name, val) JS_DefineProperty(cx, where, \
    (const char *)name, val, JSPROP_PERMANENT | JSPROP_READONLY | \
        JSPROP_ENUMERATE)
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    uint32_t program;
    program = (size_t)JS_GetPrivate(caller);

    int nactives = 0;

    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &nactives);

    JS::RootedObject arr(cx, JS_NewArrayObject(cx, nactives));
    args.rval().set(OBJECT_TO_JSVAL(arr));

    char name[512];
    for (int i = 0; i < nactives; i++) {
        int length = 0, size = 0;
        GLenum type = GL_ZERO;
        JS::RootedObject in(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));

        glGetActiveUniform(program, i, sizeof(name)-1, &length, &size, &type, name);
        name[length] = '\0';
        JS::RootedString nameStr (cx, JS_NewStringCopyN(cx, name, length));
        JS::RootedValue locationVal(cx, INT_TO_JSVAL(glGetUniformLocation(program, name)));
        JS::RootedValue inval(cx, OBJECT_TO_JSVAL(in));
        SET_PROP(in, "name", nameStr);
        SET_PROP(in, "location", locationVal);
        JS_SetElement(cx, arr, i, inval);
    }

    NATIVE_LOG_2D_CALL();
    return true;
#undef SET_PROP
}

static bool native_canvas2dctx_light(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas2DContext, &Canvas2DContext_class);
    double x, y, z;

    if (!JS_ConvertArguments(cx, args, "ddd", &x, &y, &z)) {
        return false;
    }

    NSKIA_NATIVE->light(x, y, z);

    NATIVE_LOG_2D_CALL();
    return true;
}

static bool native_canvas2dctx_prop_set(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp)
{
#define CTX_PROP(prop) CTX_PROP_ ## prop

    if (CANVASCTX_GETTER(obj)->setterDisabled) {
        return true;
    }

    NativeSkia *curSkia = NSKIA_NATIVE_GETTER(obj);
    switch (id) {
        case CTX_PROP(imageSmoothingEnabled):
        {
            if (!vp.isBoolean()) {
                vp.set(JSVAL_FALSE);

                return true;
            }

            curSkia->setSmooth(vp.toBoolean());
            break;
        }
        case CTX_PROP(shadowOffsetX):
        {
            double ret;
            if (!vp.isNumber()) {
                vp.set(JS::NullHandleValue);

                return true;
            }
            ret = vp.toNumber();

            curSkia->setShadowOffsetX(ret);
        }
        break;
        case CTX_PROP(shadowOffsetY):
        {
            double ret;
            if (!vp.isNumber()) {
                vp.set(JS::NullHandleValue);

                return true;
            }
            ret = vp.toNumber();

            curSkia->setShadowOffsetY(ret);
        }
        break;
        case CTX_PROP(shadowBlur):
        {
            double ret;
            if (!vp.isNumber()) {
                vp.set(JS::NullHandleValue);

                return true;
            }
            ret = vp.toNumber();

            curSkia->setShadowBlur(ret);
        }
        break;
        case CTX_PROP(shadowColor):
        {
            if (!vp.isString()) {
                vp.set(JS::NullHandleValue);

                return true;
            }
            JSAutoByteString color(cx, vp.toString());
            curSkia->setShadowColor(color.ptr());
        }
        break;
        case CTX_PROP(fontSize):
        {
            double ret;
            if (!vp.isNumber()) {
                vp.set(JS::NullHandleValue);

                return true;
            }
            ret = vp.toNumber();
            curSkia->setFontSize(ret);

        }
        break;
        case CTX_PROP(fontStyle):
        {
            if (!vp.isString()) {
                vp.set(JS::NullHandleValue);

                return true;
            }
            JSAutoByteString style(cx, vp.toString());

            curSkia->setFontStyle(style.ptr());
        }
        break;
        case CTX_PROP(fontSkew):
        {
            double ret;
            if (!vp.isNumber()) {
                vp.set(JS::NullHandleValue);

                return true;
            }
            ret = vp.toNumber();

            curSkia->setFontSkew(ret);
        }
        break;
        case CTX_PROP(textBaseline):
        {
            if (!vp.isString()) {
                vp.set(JS::NullHandleValue);

                return true;
            }
            JSAutoByteString baseline(cx, vp.toString());
            curSkia->textBaseline(baseline.ptr());
        }
        break;
        case CTX_PROP(textAlign):
        {
            if (!vp.isString()) {
                vp.set(JS::NullHandleValue);

                return true;
            }

            JSAutoByteString font(cx, vp.toString());
            curSkia->textAlign(font.ptr());
        }
        break;
        case CTX_PROP(fontFamily):
        {
            if (!vp.isString()) {
                vp.set(JS::NullHandleValue);

                return true;
            }
            JSAutoByteString font(cx, vp.toString());
            curSkia->setFontType(font.ptr(), NativeJSdocument::getNativeClass(cx));
        }
        break;
        case CTX_PROP(fontFile):
        {
            if (!vp.isString()) {
                vp.set(JS::NullHandleValue);

                return true;
            }
            JSAutoByteString font(cx, vp.toString());
            if (!curSkia->setFontFile(font.ptr())) {
                JS_ReportError(cx, "Cannot set font (invalid file)");

                return false;
            }
        }
        break;
        case CTX_PROP(fillStyle):
        {
            JS::RootedObject vpObj(cx, vp.toObjectOrNull());
            if (vp.isString()) {

                JSAutoByteString colorName(cx, vp.toString());
                curSkia->setFillColor(colorName.ptr());
            } else if (!vp.isPrimitive() &&
                JS_GetClass(&vp.toObject()) == &canvasGradient_class) {
                NativeSkGradient *gradient = (class NativeSkGradient *)
                                            JS_GetPrivate(&vp.toObject());

                curSkia->setFillColor(gradient);

            } else if (!vp.isPrimitive() &&
                JS_GetClass(&vp.toObject()) == &canvasPattern_class) {
                NativeCanvasPattern *pattern = (class NativeCanvasPattern *)
                                            JS_GetPrivate(&vp.toObject());

                curSkia->setFillColor(pattern);
            } else {
                vp.set(JS::NullHandleValue);

                return true;
            }
        }
        break;
        case CTX_PROP(strokeStyle):
        {
            if (vp.isString()) {
                JSAutoByteString colorName(cx, vp.toString());
                curSkia->setStrokeColor(colorName.ptr());
            } else if (!vp.isPrimitive() &&
                JS_GetClass(&vp.toObject()) == &canvasGradient_class) {
                NativeSkGradient *gradient = (class NativeSkGradient *)
                                            JS_GetPrivate(&vp.toObject());

                curSkia->setStrokeColor(gradient);
            } else {

                vp.set(JS::NullHandleValue);
                return true;
            }
        }
        break;
        case CTX_PROP(lineWidth):
        {
            double ret;
            if (!vp.isNumber()) {
                vp.set(JS::NullHandleValue);

                return true;
            }
            ret = vp.toNumber();
            curSkia->setLineWidth(ret);
        }
        break;
        case CTX_PROP(miterLimit):
        {
            double ret;
            if (!vp.isNumber()) {
                vp.set(JS::NullHandleValue);

                return true;
            }
            ret = vp.toNumber();
            curSkia->setMiterLimit(ret);
        }
        break;
        case CTX_PROP(globalAlpha):
        {
            double ret;
            if (!vp.isNumber()) {
                vp.set(JS::NullHandleValue);

                return true;
            }
            ret = vp.toNumber();
            curSkia->setGlobalAlpha(ret);
        }
        break;
        case CTX_PROP(globalCompositeOperation):
        {
            if (!vp.isString()) {
                vp.set(JS::NullHandleValue);

                return true;
            }
            JSAutoByteString composite(cx, vp.toString());
            curSkia->setGlobalComposite(composite.ptr());
        }
        break;
        case CTX_PROP(lineCap):
        {
            if (!vp.isString()) {
                vp.set(JS::NullHandleValue);

                return true;
            }
            JSAutoByteString lineCap(cx, vp.toString());
            curSkia->setLineCap(lineCap.ptr());
        }
        break;
        case CTX_PROP(lineJoin):
        {
            if (!vp.isString()) {
                vp.set(JS::NullHandleValue);

                return true;
            }
            JSAutoByteString lineJoin(cx, vp.toString());
            curSkia->setLineJoin(lineJoin.ptr());
        }
        break;
        default:
            break;
    }

    return true;
#undef CTX_PROP
}

static bool native_canvas2dctx_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp)
{
#define CTX_PROP(prop) CTX_PROP_ ## prop
    NativeSkia *curSkia = NSKIA_NATIVE_GETTER(obj);

    switch (id) {
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

    return true;
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

void Canvas2DContext_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeCanvas2DContext *canvasctx = CANVASCTX_GETTER(obj);
    if (canvasctx != NULL) {
        delete canvasctx;
    }
}

void NativeCanvas2DContext::clear(uint32_t color)
{
    m_Skia->getCanvas()->clear(color);
}

char *NativeCanvas2DContext::genModifiedFragmentShader(const char *data)
{
    const char *prologue =
        //"#version 100\nprecision mediump float;\n"
        //"precision mediump float;\n"
        "vec4 _nm_gl_FragCoord;\n"
        "#define main _nm_main\n"
        "#define gl_FragCoord _nm_gl_FragCoord\n";

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

    GLuint programHandle;

    NativeGLContext *iface = m_GLState->getNativeGLContext();

    NATIVE_GL_CALL_RET(iface, CreateProgram(), programHandle);

    GLint linkSuccess;

    NATIVE_GL_CALL(iface, AttachShader(programHandle, vertex));
    NATIVE_GL_CALL(iface, AttachShader(programHandle, coop));
    NATIVE_GL_CALL(iface, AttachShader(programHandle, fragment));

    NATIVE_GL_CALL(iface, BindAttribLocation(programHandle,
        NativeCanvasContext::SH_ATTR_POSITION, "Position"));

    NATIVE_GL_CALL(iface, BindAttribLocation(programHandle,
        NativeCanvasContext::SH_ATTR_TEXCOORD, "TexCoordIn"));

    NATIVE_GL_CALL(iface, LinkProgram(programHandle));

    NATIVE_GL_CALL(iface, GetProgramiv(programHandle, GR_GL_LINK_STATUS, &linkSuccess));
    if (linkSuccess == GL_FALSE) {
        GLchar messages[256];
        NATIVE_GL_CALL(iface, GetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]));
        NLOG("createProgram error : %s", messages);
        return 0;
    }

    return programHandle;
}

uint32_t NativeCanvas2DContext::compileCoopFragmentShader()
{
    const char *coop =
        //"#version 100\nprecision mediump float;\n"
        //"precision mediump float;\n"
        "void _nm_main(void);\n"
        "uniform sampler2D Texture;\n"
        "uniform vec2 n_Position;\n"
        "uniform vec2 n_Resolution;\n"
        "uniform float u_opacity;\n"
        "uniform float n_Padding;\n"
        "varying vec2 TexCoordOut;\n"

        "vec4 _nm_gl_FragCoord = vec4(gl_FragCoord.x-n_Position.x-n_Padding, gl_FragCoord.y-n_Position.y-n_Padding, gl_FragCoord.wz);\n"

        "void main(void) {\n"
        "if (_nm_gl_FragCoord.x+n_Padding < n_Padding ||\n"
        "    _nm_gl_FragCoord.x > n_Resolution.x ||\n"
        "    _nm_gl_FragCoord.y+n_Padding < n_Padding ||\n"
        "    _nm_gl_FragCoord.y > n_Resolution.y) {\n"
        "     gl_FragColor = texture2D(Texture, TexCoordOut.xy);\n"
        "} else {\n"
        "_nm_main();\n"
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
                                        getDevice()->accessRenderTa;

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
            NULL);

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
                                        getDevice()->accessRenderTa;

    int width = backingTarget->asTexture()->width();
    int height =  backingTarget->asTexture()->height();

    SkDevice *dev = skia->canvas->createCompatibleDevice(SkBitmap::kARGB_8888_Config,
        width, height, false);

    gl.copy = new SkCanvas(dev);
    gl.texture = ((GrRenderTarget*)dev->accessRenderTa)
                    ->asTexture()->getTextureHandle();

    gl.fbo = static_cast<GLuint>(((GrRenderTarget*)dev->accessRenderTa)->
                    asTexture()->asRenderTa->getRenderTargetHandle());

    gl.textureWidth = width;
    gl.textureHeight = height;

    dev->unref();
}
#endif

uint32_t NativeCanvas2DContext::getMainFBO()
{
    GrRenderTarget* backingTarget = (GrRenderTarget*)m_Skia->getCanvas()->
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

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    /* Anti Aliasing */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

#if 0
void NativeCanvas2DContext::setupCommonDraw()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
#endif

void NativeCanvas2DContext::drawTexture(uint32_t textureID, uint32_t width,
    uint32_t height, uint32_t left, uint32_t top)
{
    NATIVE_GL_CALL_MAIN(BindTexture(GR_GL_TEXTURE_2D, textureID));

    NATIVE_GL_CALL_MAIN(TexParameteri(GR_GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
    NATIVE_GL_CALL_MAIN(TexParameteri(GR_GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));

    /* Anti Aliasing */
    NATIVE_GL_CALL_MAIN(TexParameteri(GR_GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    NATIVE_GL_CALL_MAIN(TexParameteri(GR_GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    NATIVE_GL_CALL_MAIN(DrawElements(GR_GL_TRIANGLE_STRIP, m_GLState->m_GLObjects.vtx->nindices, GL_UNSIGNED_INT, 0));

    NATIVE_GL_CALL_MAIN(BindTexture(GR_GL_TEXTURE_2D, 0));

    /* Unbind vertex array bound by resetGLContext() */
    NATIVE_GL_CALL_MAIN(BindVertexArray(0));
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
          glVertex3f(1.0f, 1.0f, 1.0f);

        glTexCoord3i(1, 0, 1);
          glVertex3f(1.0f, -1.0f, 1.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_TEXTURE_2D);
    glPopAttrib();
}
#endif

uint32_t NativeCanvas2DContext::getSkiaTextureID(int *width, int *height)
{
    GrRenderTarget* backingTarget = (GrRenderTarget*)m_Skia->getCanvas()->
                                        getDevice()->accessRenderTarget();

    if (width != NULL && height != NULL) {
        SkISize size = m_Skia->getCanvas()->getDeviceSize();

        *width = size.fWidth;
        *height = size.fHeight;
    }

    return backingTarget->asTexture()->getTextureHandle();
}

/* Ask skia to restore its GL state */
void NativeCanvas2DContext::resetSkiaContext(uint32_t flag)
{
    GrRenderTarget* backingTarget = (GrRenderTarget*)m_Skia->getCanvas()->
                                        getDevice()->accessRenderTarget();

    if (flag == 0) {
        flag = kProgram_GrGLBackendState
                | kTextureBinding_GrGLBackendState
                | kVertex_GrGLBackendState
                | kView_GrGLBackendState
                | kBlend_GrGLBackendState;
    }

    backingTarget->getContext()->resetContext(flag);
}

uint32_t NativeCanvas2DContext::attachShader(const char *string)
{
    uint32_t program = this->createProgram(string);

    if (program) {
        NativeUIInterface *ui = m_GLState->getNativeGLContext()->getUI();
        /* Destroy the old context (if it's not shared) */
        m_GLState->destroy();
        /* Create a new state without program */
        NativeGLState *nstate = new NativeGLState(ui, false);
        nstate->setShared(false);

        m_GLState = nstate;

        m_GLState->setProgram(program);

        NativeGLContext *iface = m_GLState->getNativeGLContext();

        NATIVE_GL_CALL_RET(iface,
            GetUniformLocation(program, "n_Resolution"),
            m_GLState->m_GLObjects.uniforms.u_resolution);

        NATIVE_GL_CALL_RET(iface,
            GetUniformLocation(program, "n_Position"),
            m_GLState->m_GLObjects.uniforms.u_position);

        NATIVE_GL_CALL_RET(iface,
            GetUniformLocation(program, "n_Padding"),
            m_GLState->m_GLObjects.uniforms.u_padding);
    }

    return program;
}

void NativeCanvas2DContext::detachShader()
{
#if 0
    /* TODO : shaders must be deleted */
    glDeleteProgram(m_GLObjects.program);
    m_GLObjects.program = 0;
#endif
}

void NativeCanvas2DContext::setVertexDeformation(uint32_t vertex,
    float x, float y)
{
    NativeGLState *state = m_GLState;

    /*
        If the GL state is shared among other Canvas, create a new one
    */
    if (state->isShared()) {
        NLOG("New GL state created !");
        state = new NativeGLState(m_GLState->getNativeGLContext()->getUI());
        state->setShared(false);

        m_GLState = state;
    }

    state->setVertexDeformation(vertex, x, y);
}

uint32_t NativeCanvas2DContext::getTextureID() const
{
    GrRenderTarget* backingTarget = (GrRenderTarget*)m_Skia->getCanvas()->
                                        getDevice()->accessRenderTarget();

    return backingTarget->asTexture()->getTextureHandle();
}

void NativeCanvas2DContext::flush()
{
    m_Skia->getCanvas()->flush();
}

void NativeCanvas2DContext::getSize(int *width, int *height) const
{
    SkISize size = this->m_Skia->getCanvas()->getDeviceSize();

    *width = size.width();
    *height = size.height();
}

void NativeCanvas2DContext::setSize(int width, int height, bool redraw)
{
    SkBaseDevice *ndev = NULL;
    SkCanvas *ncanvas;

    float ratio = NativeSystemInterface::getInstance()->backingStorePixelRatio();

    if (m_Skia->native_canvas_bind_mode == NativeSkia::BIND_GL) {
        if ((ncanvas = NativeSkia::createGLCanvas(width, height,
            __NativeUI->getNativeContext())) == NULL) {
            NLOG("[Error] Couldnt resize the canvas to %dx%d", width, height);
            return;
        }

        NativeSkia::glcontext = ncanvas;

    } else {
#if 1
        const SkImageInfo &info = SkImageInfo::MakeN32Premul(width*ratio, height*ratio);
        ndev = NativeSkia::glcontext->getDevice()->createCompatibleDevice(info);
#else
        GrContext *gr = ((SkGpuDevice *)NativeSkia::glcontext->getDevice())->context();
        ndev = m_Skia->createNewGPUDevice(gr, width*ratio, height*ratio);
#endif
        if (ndev == NULL) {
            printf("Cant create canvas of size %dx%d (backstore ratio : %f)\n", width, height, ratio);
            return;
        }

        ncanvas = new SkCanvas(ndev);
        ncanvas->clear(0x00000000);

        if (redraw) {
            const SkBitmap &bt = m_Skia->getCanvas()->getDevice()->accessBitmap(false);
            ncanvas->drawBitmap(bt, 0, 0);
        }
        SkSafeUnref(ndev);
    }

    //ncanvas->clipRegion(skia->getCanvas()->getTotalClip());
    ncanvas->setMatrix(m_Skia->getCanvas()->getTotalMatrix());

    /* Automatically unref the old one and ref the new one */
    m_Skia->setCanvas(ncanvas);
    ncanvas->unref();

    if (m_Skia->native_canvas_bind_mode == NativeSkia::BIND_GL) {
        m_Skia->drawRect(0, 0, 1, 1, 0);
    }
}

void NativeCanvas2DContext::translate(double x, double y)
{
    m_Skia->getCanvas()->translate(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

NativeCanvas2DContext::NativeCanvas2DContext(NativeCanvasHandler *handler,
    JSContext *cx, int width, int height, NativeUIInterface *ui) :
    NativeCanvasContext(handler),
    setterDisabled(false)
{
    m_Mode = CONTEXT_2D;

    JS::RootedObject jsobj(cx, JS_NewObject(cx, &Canvas2DContext_class, JS::NullPtr(), JS::NullPtr()));
    jscx  = cx;

    /*
        TODO: BUG: xxx setter doesn't work if we remove this the definesProperties
    */
    JS_DefineProperties(cx, jsobj, canvas2dctx_props);
    JS::RootedObject saved(cx, JS_NewArrayObject(cx, 0));
    JS_SetReservedSlot(jsobj, 0, OBJECT_TO_JSVAL(saved));

    m_Skia = new NativeSkia();
    if (!m_Skia->bindOnScreen(width, height)) {
        delete m_Skia;
        m_Skia = NULL;
        return;
    }

    JS_SetPrivate(jsobj, this);

    /* Vertex buffers were unbound by parent constructor */
    this->resetSkiaContext(kVertex_GrGLBackendState);
}

NativeCanvas2DContext::NativeCanvas2DContext(NativeCanvasHandler *handler,
    int width, int height, NativeUIInterface *ui, bool isGL) :
    NativeCanvasContext(handler)
{
    m_Mode = CONTEXT_2D;

    m_Skia = new NativeSkia();
    int state;

    if (isGL) {

        state = m_Skia->bindGL(width, height, ui->getNativeContext());
    } else {
        state = m_Skia->bindOnScreen(width, height);
    }

    if (!state) {
        NLOG("Failed to create canvas");
        delete m_Skia;
        m_Skia = NULL;
        return;
    }

    /* Vertex buffers were unbound by parent constructor */
    this->resetSkiaContext(kVertex_GrGLBackendState);
}

void NativeCanvas2DContext::setScale(double x, double y,
    double px, double py)
{
    m_Skia->scale(1./px, 1./py);

    m_Skia->scale(x, y);
}

uint8_t *NativeCanvas2DContext::getPixels()
{
    this->flush();

    return (uint8_t *)m_Skia->getCanvas()->getDevice()->
        accessBitmap(false).getPixels();
}

NativeCanvas2DContext::~NativeCanvas2DContext()
{
    //NLOG("Delete skia %p", skia);
    if (m_Skia) delete m_Skia;
}

static bool native_Canvas2DContext_constructor(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS_ReportError(cx, "Illegal constructor");
    return false;
}

void NativeCanvas2DContext::registerObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS_InitClass(cx, global, JS::NullPtr(), &Canvas2DContext_class,
    native_Canvas2DContext_constructor, 0, canvas2dctx_props,
    canvas2dctx_funcs, nullptr, nullptr);
}

