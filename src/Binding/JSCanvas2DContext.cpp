#include "Binding/JSCanvas2DContext.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>

#include <SkDevice.h>

#include <SystemInterface.h>

#include "Graphics/Skia.h"
#include "Graphics/SkGradient.h"
#include "Graphics/SkImage.h"
#include "Graphics/OpenGLHeader.h"
#include "Binding/JSCanvas.h"
#include "Binding/JSDocument.h"

namespace Nidium {
namespace Binding {

// {{{ Preamble
#define CANVASCTX_GETTER(obj) ((class Canvas2DContext *)JS_GetPrivate(obj))
#define NSKIA_NATIVE_GETTER(obj) ((class Nidium::Graphics::Skia *)((class Canvas2DContext *)JS_GetPrivate(obj))->getSurface())
#define NSKIA_NATIVE (CppObj->getSurface())
#define HANDLER_GETTER(obj) ((Nidium::Graphics::CanvasHandler *)((class JSCanvas *)JS_GetPrivate(obj))->getHandler())

static JSClass ImageData_class = {
    "ImageData", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

JSClass *Canvas2DContext::jsclass = &ImageData_class;

enum {
#define CANVAS_2D_CTX_PROP(prop) CTX_PROP_ ## prop,
#define CANVAS_2D_CTX_PROP_GET(prop) CTX_PROP_ ## prop,
  #include "Graphics/Canvas2DContextProperties.h"
  CTX_PROP__NPROP
#undef CANVAS_2D_CTX_PROP
#undef CANVAS_2D_CTX_PROP_GET
};

#if 0 && DEBUG
#define NIDIUM_LOG_2D_CALL() \
    JS::RootedObject calObj(cx, &args.callee()); \
    if (JS_ObjectIsFunction(cx, calObj)) { \
        unsigned lineno; \
        JS::AutoFilename filename; \
        JS::DescribeScriptedCaller(cx, &filename, &lineno); \
        JS::RootedValue calVal(cx, OBJECT_TO_JSVAL(calObj)); \
        JS::RootedString _fun_name(cx, JS_GetFunctionDisplayId(JS_ValueToFunction(cx, calVal))); \
        JSAutoByteString _fun_namec(cx, _fun_name); \
        NLOG("Canvas2D.%s()] called on %s:%d", _fun_namec.ptr(), filename.get(), lineno); \
    }
#else
#define NIDIUM_LOG_2D_CALL()
#endif

static void CanvasGradient_Finalize(JSFreeOp *fop, JSObject *obj);
static void CanvasPattern_Finalize(JSFreeOp *fop, JSObject *obj);
static void Canvas2DContext_Finalize(JSFreeOp *fop, JSObject *obj);

static void Canvas2DContext_Trace(JSTracer *trc, JSObject *obj);

extern JSClass Canvas_class;

JSClass Canvas2DContext_class = {
    "CanvasRenderingContext2D", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Canvas2DContext_Finalize,
    nullptr, nullptr, nullptr, Canvas2DContext_Trace, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass CanvasGradient_class = {
    "CanvasGradient", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, CanvasGradient_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass CanvasGLProgram_class = {
    "CanvasGLProgram", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass CanvasPattern_class = {
    "CanvasPattern", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, CanvasPattern_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static bool nidium_canvas2dctx_prop_set(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp);
static bool nidium_canvas2dctx_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp);

static bool nidium_canvas2dctx_breakText(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_shadow(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_fillRect(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_strokeRect(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_clearRect(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_fillText(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_strokeText(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_beginPath(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_moveTo(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_lineTo(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_fill(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_stroke(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_closePath(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_arc(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_arcTo(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_rect(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_quadraticCurveTo(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctx_bezierCurveTo(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctx_rotate(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_scale(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_save(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_restore(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_translate(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_transform(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_iTransform(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_setTransform(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctx_clip(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_createImageData(JSContext *cx,
    unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_createPattern(JSContext *cx,
    unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_putImageData(JSContext *cx,
    unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_getImageData(JSContext *cx,
    unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_createLinearGradient(JSContext *cx,
    unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_createRadialGradient(JSContext *cx,
    unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctxGradient_addColorStop(JSContext *cx,
    unsigned argc, JS::Value *vp);

static bool nidium_canvas2dctx_stub(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_drawImage(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_canvas2dctx_measureText(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctx_isPointInPath(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctx_getPathBounds(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctx_light(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctx_attachGLSLFragment(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctx_detachGLSLFragment(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctx_setVertexOffset(JSContext *cx, unsigned argc,
    JS::Value *vp);

/* GLSL related */
static bool nidium_canvas2dctxGLProgram_getUniformLocation(JSContext *cx, unsigned argc,
    JS::Value *vp);

static bool nidium_canvas2dctxGLProgram_uniform1i(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctxGLProgram_uniform1f(JSContext *cx, unsigned argc,
    JS::Value *vp);

static bool nidium_canvas2dctxGLProgram_uniform1iv(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctxGLProgram_uniform2iv(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctxGLProgram_uniform3iv(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctxGLProgram_uniform4iv(JSContext *cx, unsigned argc,
    JS::Value *vp);

static bool nidium_canvas2dctxGLProgram_uniform1fv(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctxGLProgram_uniform2fv(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctxGLProgram_uniform3fv(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctxGLProgram_uniform4fv(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool nidium_canvas2dctxGLProgram_getActiveUniforms(JSContext *cx, unsigned argc,
    JS::Value *vp);

static JSPropertySpec canvas2dctx_props[] = {
#define CANVAS_2D_CTX_PROP(prop) NIDIUM_JS_PSGS(#prop, CTX_PROP_ ## prop, nidium_canvas2dctx_prop_get, nidium_canvas2dctx_prop_set),
#define CANVAS_2D_CTX_PROP_GET(prop) NIDIUM_JS_PSG(#prop, CTX_PROP_ ## prop, nidium_canvas2dctx_prop_get),
  #include "Graphics/Canvas2DContextProperties.h"
  JS_PS_END
#undef CANVAS_2D_CTX_PROP
#undef CANVAS_2D_CTX_PROP_GET
};

static JSFunctionSpec canvas2dctx_funcs[] = {
    JS_FN("breakText", nidium_canvas2dctx_breakText, 2, NIDIUM_JS_FNPROPS),
    JS_FN("shadow", nidium_canvas2dctx_shadow, 0, NIDIUM_JS_FNPROPS),
    JS_FN("onerror", nidium_canvas2dctx_stub, 0, NIDIUM_JS_FNPROPS),
    JS_FN("fillRect", nidium_canvas2dctx_fillRect, 4, NIDIUM_JS_FNPROPS),
    JS_FN("fillText", nidium_canvas2dctx_fillText, 3, NIDIUM_JS_FNPROPS),
    JS_FN("strokeText", nidium_canvas2dctx_strokeText, 3, NIDIUM_JS_FNPROPS),
    JS_FN("strokeRect", nidium_canvas2dctx_strokeRect, 4, NIDIUM_JS_FNPROPS),
    JS_FN("clearRect", nidium_canvas2dctx_clearRect, 4, NIDIUM_JS_FNPROPS),
    JS_FN("beginPath", nidium_canvas2dctx_beginPath, 0, NIDIUM_JS_FNPROPS),
    JS_FN("moveTo", nidium_canvas2dctx_moveTo, 2, NIDIUM_JS_FNPROPS),
    JS_FN("lineTo", nidium_canvas2dctx_lineTo, 2, NIDIUM_JS_FNPROPS),
    JS_FN("fill", nidium_canvas2dctx_fill, 0, NIDIUM_JS_FNPROPS),
    JS_FN("stroke", nidium_canvas2dctx_stroke, 0, NIDIUM_JS_FNPROPS),
    JS_FN("closePath", nidium_canvas2dctx_closePath, 0, NIDIUM_JS_FNPROPS),
    JS_FN("clip", nidium_canvas2dctx_clip, 0, NIDIUM_JS_FNPROPS),
    JS_FN("arc", nidium_canvas2dctx_arc, 5, NIDIUM_JS_FNPROPS),
    JS_FN("arcTo", nidium_canvas2dctx_arcTo, 5, NIDIUM_JS_FNPROPS),
    JS_FN("rect", nidium_canvas2dctx_rect, 4, NIDIUM_JS_FNPROPS),
    JS_FN("quadraticCurveTo", nidium_canvas2dctx_quadraticCurveTo, 4, NIDIUM_JS_FNPROPS),
    JS_FN("bezierCurveTo", nidium_canvas2dctx_bezierCurveTo, 4, NIDIUM_JS_FNPROPS),
    JS_FN("rotate", nidium_canvas2dctx_rotate, 1, NIDIUM_JS_FNPROPS),
    JS_FN("scale", nidium_canvas2dctx_scale, 2, NIDIUM_JS_FNPROPS),
    JS_FN("save", nidium_canvas2dctx_save, 0, NIDIUM_JS_FNPROPS),
    JS_FN("restore", nidium_canvas2dctx_restore, 0, NIDIUM_JS_FNPROPS),
    JS_FN("translate", nidium_canvas2dctx_translate, 2, NIDIUM_JS_FNPROPS),
    JS_FN("transform", nidium_canvas2dctx_transform, 6, NIDIUM_JS_FNPROPS),
    JS_FN("iTransform", nidium_canvas2dctx_iTransform, 0, NIDIUM_JS_FNPROPS),
    JS_FN("setTransform", nidium_canvas2dctx_setTransform, 6, NIDIUM_JS_FNPROPS),
    JS_FN("createLinearGradient", nidium_canvas2dctx_createLinearGradient, 4, NIDIUM_JS_FNPROPS),
    JS_FN("createRadialGradient", nidium_canvas2dctx_createRadialGradient, 6, NIDIUM_JS_FNPROPS),
    JS_FN("createImageData", nidium_canvas2dctx_createImageData, 2, NIDIUM_JS_FNPROPS),
    JS_FN("createPattern", nidium_canvas2dctx_createPattern, 2, NIDIUM_JS_FNPROPS),
    JS_FN("putImageData", nidium_canvas2dctx_putImageData, 3, NIDIUM_JS_FNPROPS),
    JS_FN("getImageData", nidium_canvas2dctx_getImageData, 4, NIDIUM_JS_FNPROPS),
    JS_FN("drawImage", nidium_canvas2dctx_drawImage, 3, NIDIUM_JS_FNPROPS),
    JS_FN("measureText", nidium_canvas2dctx_measureText, 1, NIDIUM_JS_FNPROPS),
    JS_FN("isPointInPath", nidium_canvas2dctx_isPointInPath, 2, NIDIUM_JS_FNPROPS),
    JS_FN("getPathBounds", nidium_canvas2dctx_getPathBounds, 0, NIDIUM_JS_FNPROPS),
    JS_FN("light", nidium_canvas2dctx_light, 3, NIDIUM_JS_FNPROPS),
    JS_FN("attachFragmentShader", nidium_canvas2dctx_attachGLSLFragment, 1, NIDIUM_JS_FNPROPS),
    JS_FN("detachFragmentShader", nidium_canvas2dctx_detachGLSLFragment, 0, NIDIUM_JS_FNPROPS),
    JS_FN("setVertexOffset", nidium_canvas2dctx_setVertexOffset, 3, NIDIUM_JS_FNPROPS),
    JS_FS_END
};

static JSFunctionSpec gradient_funcs[] = {

    JS_FN("addColorStop", nidium_canvas2dctxGradient_addColorStop, 2, NIDIUM_JS_FNPROPS),

    JS_FS_END
};

static JSFunctionSpec glprogram_funcs[] = {

    JS_FN("getUniformLocation", nidium_canvas2dctxGLProgram_getUniformLocation, 1, NIDIUM_JS_FNPROPS),
    JS_FN("getActiveUniforms", nidium_canvas2dctxGLProgram_getActiveUniforms, 0, NIDIUM_JS_FNPROPS),
    JS_FN("uniform1i", nidium_canvas2dctxGLProgram_uniform1i, 2, NIDIUM_JS_FNPROPS),
    JS_FN("uniform1f", nidium_canvas2dctxGLProgram_uniform1f, 2, NIDIUM_JS_FNPROPS),
    JS_FN("uniform1iv", nidium_canvas2dctxGLProgram_uniform1iv, 2, NIDIUM_JS_FNPROPS),
    JS_FN("uniform2iv", nidium_canvas2dctxGLProgram_uniform2iv, 2, NIDIUM_JS_FNPROPS),
    JS_FN("uniform3iv", nidium_canvas2dctxGLProgram_uniform3iv, 2, NIDIUM_JS_FNPROPS),
    JS_FN("uniform4iv", nidium_canvas2dctxGLProgram_uniform4iv, 2, NIDIUM_JS_FNPROPS),

    JS_FN("uniform1fv", nidium_canvas2dctxGLProgram_uniform1fv, 2, NIDIUM_JS_FNPROPS),
    JS_FN("uniform2fv", nidium_canvas2dctxGLProgram_uniform2fv, 2, NIDIUM_JS_FNPROPS),
    JS_FN("uniform3fv", nidium_canvas2dctxGLProgram_uniform3fv, 2, NIDIUM_JS_FNPROPS),
    JS_FN("uniform4fv", nidium_canvas2dctxGLProgram_uniform4fv, 2, NIDIUM_JS_FNPROPS),
    JS_FS_END
};
// }}}

// {{{ Implementation
static bool nidium_canvas2dctx_stub(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    NIDIUM_LOG_2D_CALL();

    return true;
}

static bool nidium_canvas2dctx_fillRect(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    NIDIUM_LOG_2D_CALL();

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

    return true;
}

static bool nidium_canvas2dctx_strokeRect(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    double x, y, width, height, rx = 0, ry = 0;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dddd/dd", &x, &y, &width, &height, &rx, &ry)) {
        return false;
    }

    if (argc > 4) {
        NSKIA_NATIVE->drawRect(x, y, width, height,
            rx, (argc == 5 ? rx : ry), 1);
    } else {
        NSKIA_NATIVE->drawRect(x, y, width, height, 1);
    }

    return true;
}

static bool nidium_canvas2dctx_clearRect(JSContext *cx, unsigned argc, JS::Value *vp)
{
    double x, y, width, height;

    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dddd", &x, &y, &width, &height)) {
        return false;
    }

    CppObj->getSurface()->clearRect(x, y, width, height);

    return true;
}

static bool nidium_canvas2dctx_breakText(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);

    NIDIUM_LOG_2D_CALL();

#define SET_PROP(where, name, val) JS_DefineProperty(cx, where, \
    (const char *)name, val, JSPROP_PERMANENT | JSPROP_READONLY | \
        JSPROP_ENUMERATE)
    double maxWidth;
    int length = 0;

    JS::RootedString str(cx);
    if (!JS_ConvertArguments(cx, args, "Sd", str.address(), &maxWidth)) {
        return false;
    }

    JSAutoByteString text(cx, str);
    size_t len = text.length();

    if (len == 0) {
        vp->setNull();
        return true;
    }

    struct Nidium::Graphics::_NativeLine *lines = new struct Nidium::Graphics::_NativeLine[len];

    if (!lines) {
        JS_ReportOutOfMemory(cx);
        return false;
    }

    memset(lines, 0, len * sizeof(struct Nidium::Graphics::_NativeLine));

    SkScalar ret = CppObj->getSurface()->breakText(text.ptr(), len,
                    lines, maxWidth, &length);
    JS::RootedObject alines(cx, JS_NewArrayObject(cx, length));
    for (int i = 0; i < len && i < length; i++) {
        JS::RootedString str(cx, JS_NewStringCopyN(cx, lines[i].line, lines[i].len));
        JS::RootedValue val(cx, STRING_TO_JSVAL(str));
        JS_SetElement(cx, alines, i, val);
    }
    JS::RootedObject res(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));
    JS::RootedValue heightVal(cx, DOUBLE_TO_JSVAL(SkScalarToDouble(ret)));
    JS::RootedValue linesVal(cx, OBJECT_TO_JSVAL(alines));
    SET_PROP(res, "height", heightVal);
    SET_PROP(res, "lines", linesVal);

    args.rval().setObjectOrNull(res);

    delete[] lines;

    return true;
#undef SET_PROP
}

static bool nidium_canvas2dctx_fillText(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    int x, y, maxwidth;

    NIDIUM_LOG_2D_CALL();
    JS::RootedString str(cx);
    if (!JS_ConvertArguments(cx, args, "Sii/i", str.address(), &x, &y, &maxwidth)) {
        return false;
    }

    JSAutoByteString text;
    text.encodeUtf8(cx, str);

    NSKIA_NATIVE->drawText(text.ptr(), x, y);

    return true;
}

static bool nidium_canvas2dctx_strokeText(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    int x, y, maxwidth;

    NIDIUM_LOG_2D_CALL();
    JS::RootedString str(cx);
    if (!JS_ConvertArguments(cx, args, "Sii/i", str.address(), &x, &y, &maxwidth)) {
        return false;
    }

    JSAutoByteString text;
    text.encodeUtf8(cx, str);

    NSKIA_NATIVE->drawText(text.ptr(), x, y, true);

    return true;
}

static bool nidium_canvas2dctx_shadow(JSContext *cx, unsigned argc, JS::Value *vp)
{
    //NSKIA_NATIVE->setShadow();
    return true;
}

static bool nidium_canvas2dctx_beginPath(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    NIDIUM_LOG_2D_CALL();

    NSKIA_NATIVE->beginPath();

    return true;
}

static bool nidium_canvas2dctx_moveTo(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    double x, y;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        return false;
    }

    NSKIA_NATIVE->moveTo(x, y);

    return true;
}

static bool nidium_canvas2dctx_lineTo(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    double x, y;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        return false;
    }

    NSKIA_NATIVE->lineTo(x, y);

    return true;
}

static bool nidium_canvas2dctx_fill(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    NIDIUM_LOG_2D_CALL();
    NSKIA_NATIVE->fill();

    return true;
}

static bool nidium_canvas2dctx_stroke(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    NIDIUM_LOG_2D_CALL();
    NSKIA_NATIVE->stroke();

    return true;
}
static bool nidium_canvas2dctx_closePath(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    NIDIUM_LOG_2D_CALL();
    NSKIA_NATIVE->closePath();

    return true;
}

static bool nidium_canvas2dctx_clip(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    NIDIUM_LOG_2D_CALL();
    NSKIA_NATIVE->clip();

    return true;
}

static bool nidium_canvas2dctx_rect(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    double x, y, width, height;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dddd", &x, &y, &width, &height)) {
        return false;
    }

    NSKIA_NATIVE->rect(x, y, width, height);

    return true;
}

static bool nidium_canvas2dctx_arc(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    int x, y, radius;
    double startAngle, endAngle;
    bool CCW = false;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "iiidd/b", &x, &y, &radius, &startAngle,
        &endAngle, &CCW)) {
        return false;
    }

    NSKIA_NATIVE->arc(x, y, radius, startAngle, endAngle, CCW);

    return true;
}

static bool nidium_canvas2dctx_arcTo(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    int x1, y1, x2, y2, radius;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "iiiii", &x1, &y1, &x2, &y2, &radius)) {
        return false;
    }

    NSKIA_NATIVE->arcTo(x1, y1, x2, y2, radius);

    return true;
}

static bool nidium_canvas2dctx_quadraticCurveTo(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    double x, y, cpx, cpy;
    if (!JS_ConvertArguments(cx, args, "dddd", &cpx, &cpy, &x, &y)) {
        return false;
    }

    NSKIA_NATIVE->quadraticCurveTo(cpx, cpy, x, y);

    return true;
}

static bool nidium_canvas2dctx_bezierCurveTo(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    double x, y, cpx, cpy, cpx2, cpy2;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dddddd", &cpx, &cpy, &cpx2, &cpy2,
        &x, &y)) {
        return false;
    }

    NSKIA_NATIVE->bezierCurveTo(cpx, cpy, cpx2, cpy2, x, y);

    return true;
}

static bool nidium_canvas2dctx_rotate(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    double angle;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "d", &angle)) {
        return false;
    }

    NSKIA_NATIVE->rotate(angle);

    return true;
}

static bool nidium_canvas2dctx_scale(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    double x, y;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        return false;
    }

    NSKIA_NATIVE->scale(x, y);

    return true;
}

static bool nidium_canvas2dctx_translate(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    double x, y;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        return false;
    }

    NSKIA_NATIVE->translate(x, y);

    return true;
}

static bool nidium_canvas2dctx_transform(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    double scalex, skewx, skewy, scaley, translatex, translatey, rotate;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dddddd/d", &scalex, &skewx, &skewy,
        &scaley, &translatex, &translatey, &rotate)) {
        return false;
    }

    NSKIA_NATIVE->transform(scalex, skewx, skewy, scaley,
        translatex, translatey, 0);

    if (argc == 7) {
        NSKIA_NATIVE->rotate(rotate);
    }

    return true;
}

static bool nidium_canvas2dctx_iTransform(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    double scalex, skewx, skewy, scaley, translatex, translatey;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dddddd", &scalex, &skewx, &skewy, &scaley, &translatex, &translatey)) {
        return false;
    }

    NSKIA_NATIVE->itransform(scalex, skewx, skewy, scaley,
        translatex, translatey);

    return true;
}

static bool nidium_canvas2dctx_setTransform(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    double scalex, skewx, skewy, scaley, translatex, translatey;

    Nidium::Graphics::CanvasHandler *handler = CppObj->getHandler();

    if (!JS_ConvertArguments(cx, args, "dddddd", &scalex, &skewx, &skewy, &scaley, &translatex, &translatey)) {
    NIDIUM_LOG_2D_CALL();
        return false;
    }

    NSKIA_NATIVE->transform(scalex, skewx, skewy, scaley,
        translatex+handler->m_Padding.global, translatey+handler->m_Padding.global, 1);

    return true;
}

static bool nidium_canvas2dctx_save(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    NIDIUM_LOG_2D_CALL();

    /*
        TODO: limit? (avoid while(1) ctx.save())
    */
    CppObj->pushNewState();
    CppObj->getSurface()->save();

    return true;
}

static bool nidium_canvas2dctx_restore(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    NIDIUM_LOG_2D_CALL();

    CppObj->popState();
    NSKIA_NATIVE->restore();

    return true;
}

static bool nidium_canvas2dctx_createLinearGradient(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    NIDIUM_LOG_2D_CALL();
    double x1, y1, x2, y2;

    if (!JS_ConvertArguments(cx, args, "dddd", &x1, &y1, &x2, &y2)) {
        return false;
    }

    JS::RootedObject linearObject(cx, JS_NewObject(cx, &CanvasGradient_class, JS::NullPtr(), JS::NullPtr()));
    JS_SetPrivate(linearObject, new Nidium::Graphics::Gradient(x1, y1, x2, y2));
    JS_DefineFunctions(cx, linearObject, gradient_funcs);

    args.rval().setObjectOrNull(linearObject);

    return true;
}

static bool nidium_canvas2dctx_getImageData(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    int left, top, width, height;
    uint8_t *data;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "iiii", &left, &top, &width, &height)) {
        return false;
    }

    JS::RootedObject arrBuffer(cx, JS_NewUint8ClampedArray(cx, width*height * 4));
    data = JS_GetUint8ClampedArrayData(arrBuffer);

    NSKIA_NATIVE->readPixels(top, left, width, height, data);
    JS::RootedValue widthVal(cx, UINT_TO_JSVAL(width));
    JS::RootedValue heightVal(cx, UINT_TO_JSVAL(height));
    JS::RootedValue arVal(cx, OBJECT_TO_JSVAL(arrBuffer));
    JS::RootedObject dataObject(cx, JS_NewObject(cx, &ImageData_class, JS::NullPtr(), JS::NullPtr()));
    JS_DefineProperty(cx, dataObject, "width", widthVal, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "height", heightVal, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "data", arVal, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    args.rval().setObjectOrNull(dataObject);

    return true;
}

/* TODO: Huge memory leak? */
static bool nidium_canvas2dctx_putImageData(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    int x, y;
    uint8_t *pixels;
    int32_t w, h;

    NIDIUM_LOG_2D_CALL();
    JS::RootedObject dataObject(cx);
    if (!JS_ConvertArguments(cx, args, "oii", dataObject.address(), &x, &y)) {
        return false;
    }

    if (!dataObject || JS_GetClass(dataObject) != &ImageData_class) {
        JS_ReportError(cx, "First argument must be a imageData object");
        return false;
    }

    JS::RootedValue jdata(cx);
    JS::RootedValue jwidth(cx);
    JS::RootedValue jheight(cx);
    JS_GetProperty(cx, dataObject, "data", &jdata);
    JS_GetProperty(cx, dataObject, "width", &jwidth);
    JS_GetProperty(cx, dataObject, "height", &jheight);

    JS::RootedObject jObj(cx);
    JS_ValueToObject(cx, jdata, &jObj);
    pixels = JS_GetUint8ClampedArrayData(jObj);
    JS::ToInt32(cx, jwidth, &w);
    JS::ToInt32(cx, jheight, &h);

    NSKIA_NATIVE->drawPixels(pixels, w, h, x, y);

    return true;
}

/* TODO: clamp max size */
static bool nidium_canvas2dctx_createImageData(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    unsigned long x, y;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "uu", &x, &y)) {
        return false;
    }

    if (x == 0) {
        x = 1;
    }
    if (y == 0) {
        y = 1;
    }

    JS::RootedObject arrBuffer(cx, JS_NewUint8ClampedArray(cx, x*y * 4));
    if (!arrBuffer.get()) {
        JS_ReportOutOfMemory(cx);
        return false;
    }

    JS::RootedValue array(cx, OBJECT_TO_JSVAL(arrBuffer));
    JS::RootedValue width(cx, UINT_TO_JSVAL(x));
    JS::RootedValue height(cx, UINT_TO_JSVAL(y));
    JS::RootedObject dataObject(cx, JS_NewObject(cx, &ImageData_class, JS::NullPtr(), JS::NullPtr()));
    JS_DefineProperty(cx, dataObject, "width", width, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "height", height, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "data", array, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    args.rval().setObjectOrNull(dataObject);


    return true;
}

static bool nidium_canvas2dctx_createPattern(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    NIDIUM_LOG_2D_CALL();
    JS::RootedObject jsimage(cx);
    JS::RootedString mode(cx);
    if (!JS_ConvertArguments(cx, args, "oS", jsimage.address(), mode.address())) {
        return false;
    }

    if (!JSImage::JSObjectIs(cx, jsimage)) {
        JS_ReportError(cx, "First parameter is not an Image");
        return false;
    }

    JS::RootedObject patternObject(cx, JS_NewObject(cx, &CanvasPattern_class, JS::NullPtr(), JS::NullPtr()));
    JSImage *img = static_cast<JSImage *>(JS_GetPrivate(jsimage));
    JS_SetReservedSlot(patternObject, 0, OBJECT_TO_JSVAL(img->getJSObject()));
    CanvasPattern::PATTERN_MODE pmode = CanvasPattern::PATTERN_REPEAT;

    args.rval().setObjectOrNull(patternObject);

    JSAutoByteString cmode(cx, mode);
    if (strcasecmp(cmode.ptr(), "repeat") == 0) {
        pmode = CanvasPattern::PATTERN_REPEAT;
    } else if (strcasecmp(cmode.ptr(), "no-repeat") == 0) {
        pmode = CanvasPattern::PATTERN_NOREPEAT;
    } else if (strcasecmp(cmode.ptr(), "repeat-x") == 0) {
        pmode = CanvasPattern::PATTERN_REPEAT_X;
    } else if (strcasecmp(cmode.ptr(), "repeat-y") == 0) {
        pmode = CanvasPattern::PATTERN_REPEAT_Y;
    } else if (strcasecmp(cmode.ptr(), "repeat-mirror") == 0) {
        pmode = CanvasPattern::PATTERN_REPEAT_MIRROR;
    }

    JS_SetPrivate(patternObject,
        new CanvasPattern(static_cast<JSImage *>(JS_GetPrivate(jsimage)), pmode));

    return true;
}

static bool nidium_canvas2dctx_createRadialGradient(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    double x1, y1, x2, y2, r1, r2;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dddddd", &x1, &y1, &r1, &x2, &y2, &r2)) {
        return false;
    }

    JS::RootedObject linearObject(cx, JS_NewObject(cx, &CanvasGradient_class, JS::NullPtr(), JS::NullPtr()));
    JS_SetPrivate(linearObject, new Nidium::Graphics::Gradient(x1, y1, r1, x2, y2, r2));
    JS_DefineFunctions(cx, linearObject, gradient_funcs);
    args.rval().setObjectOrNull(linearObject);

    return true;
}

static bool nidium_canvas2dctxGradient_addColorStop(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    JS::RootedString color(cx);
    double position;
    Nidium::Graphics::Gradient *gradient;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dS", &position, color.address())) {
        return false;
    }

    if ((gradient = static_cast<Nidium::Graphics::Gradient *>(JS_GetPrivate(caller))) != NULL) {
        JSAutoByteString colorstr(cx, color);

        gradient->addColorStop(position, colorstr.ptr());
    }

    return true;
}


static bool nidium_canvas2dctx_drawImage(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    Nidium::Graphics::Image *image;
    double x, y, width, height;
    int sx, sy, swidth, sheight;
    int need_free = 0;

    NIDIUM_LOG_2D_CALL();
    JS::RootedObject jsimage(cx);
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
    if (jsimage && JS_GetClass(jsimage) == &Canvas_class) {
        Nidium::Graphics::CanvasContext *drawctx = HANDLER_GETTER(jsimage)->getContext();

        if (drawctx == NULL || drawctx->getContextType() != Nidium::Graphics::CanvasContext::CONTEXT_2D) {
            JS_ReportError(cx, "Invalid image canvas (must be backed by a 2D context)");
            return false;
        }
        image = new Nidium::Graphics::Image((static_cast<Canvas2DContext *>(drawctx))->getSurface()->getCanvas());
        need_free = 1;
    } else if (!jsimage || !JSImage::JSObjectIs(cx, jsimage) ||
        (image = JSImage::JSObjectToImage(jsimage)) == NULL) {

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

    return true;
}

static bool nidium_canvas2dctx_measureText(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    JS::RootedString text(cx);
#define OBJ_PROP(name, val) JS_DefineProperty(cx, obj, name, \
    val, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY)

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "S", text.address())) {
        return false;
    }

    JS::RootedObject obj(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));

    JSAutoByteString ctext;
    ctext.encodeUtf8(cx, text);

    Nidium::Graphics::Skia *n = NSKIA_NATIVE;
    JS::RootedValue widthVal(cx, DOUBLE_TO_JSVAL(n->measureText(ctext.ptr(), strlen(ctext.ptr()))));
    OBJ_PROP("width", widthVal);

    args.rval().setObjectOrNull(obj);

    return true;
}

static bool nidium_canvas2dctx_isPointInPath(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    double x, y;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        vp->setBoolean(false);
        return false;
    }

    Nidium::Graphics::Skia *n = NSKIA_NATIVE;

    vp->setBoolean(n->SkPathContainsPoint(x, y));

    return true;
}

/* TODO: return undefined if the path is invalid */
static bool nidium_canvas2dctx_getPathBounds(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
#define OBJ_PROP(name, val) JS_DefineProperty(cx, obj, name, \
    val, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY)
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    double left = 0, right = 0, top = 0, bottom = 0;
    JS::RootedObject obj(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));

    NIDIUM_LOG_2D_CALL();
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

    return true;
}

static bool nidium_canvas2dctx_detachGLSLFragment(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    NIDIUM_LOG_2D_CALL();

    CppObj->detachShader();

    return true;
}

static bool nidium_canvas2dctx_setVertexOffset(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);

    uint32_t vertex;
    double x, y;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "udd", &vertex, &x, &y)) {
        return false;
    }

    CppObj->setVertexDeformation(vertex, x, y);

    return true;
}

static bool nidium_canvas2dctx_attachGLSLFragment(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    size_t program;

    NIDIUM_LOG_2D_CALL();
    JS::RootedString glsl(cx);
    if (!JS_ConvertArguments(cx, args, "S", glsl.address())) {
        return false;
    }

    JSAutoByteString cglsl(cx, glsl);
    if ((program = CppObj->attachShader(cglsl.ptr())) == 0) {
        JS_ReportError(cx, "Failed to compile GLSL shader");
        return false;
    }
    JS::RootedObject canvasProgram(cx, JS_NewObject(cx, &CanvasGLProgram_class, JS::NullPtr(), JS::NullPtr()));
    JS_DefineFunctions(cx, canvasProgram, glprogram_funcs);
    JS_SetPrivate(canvasProgram, (void *)program);

    args.rval().setObjectOrNull(canvasProgram);

    return true;
}

static bool nidium_canvas2dctxGLProgram_getUniformLocation(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx,  JS_THIS_OBJECT(cx, vp));
    uint32_t program;
    int ret;

    NIDIUM_LOG_2D_CALL();
    JS::RootedString location(cx);
    if (!JS_ConvertArguments(cx, args, "S", location.address())) {
        return false;
    }

    JSAutoByteString clocation(cx, location);
    program = (size_t)JS_GetPrivate(caller);
    ret = glGetUniformLocation(program, clocation.ptr());

    args.rval().setInt32(ret);

    return true;
}

static bool nidium_canvas2dctxGLProgram_uniform1i(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    int location, val;
    uint32_t program;

    NIDIUM_LOG_2D_CALL();
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

    return true;
}

static bool nidium_canvas2dctxGLProgram_uniform1f(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    int location;
    double val;
    uint32_t program;

    NIDIUM_LOG_2D_CALL();
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

    return true;
}

static bool nidium_canvas2dctxGLProgram_uniformXiv(JSContext *cx,
    unsigned int argc, JS::Value *vp, int nb)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    GLsizei length;
    GLint *carray;
    int location;
    uint32_t program;
    program = (size_t)JS_GetPrivate(caller);

    NIDIUM_LOG_2D_CALL();
    JS::RootedObject array(cx);
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
        JS::RootedObject tmp(cx, JS_NewInt32ArrayFromArray(cx, array));
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

    return true;
}

static bool nidium_canvas2dctxGLProgram_uniformXfv(JSContext *cx,
    unsigned int argc, JS::Value *vp, int nb)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    GLsizei length;
    GLfloat *carray;
    int location;
    uint32_t program;
    program = (size_t)JS_GetPrivate(caller);

    NIDIUM_LOG_2D_CALL();
    JS::RootedObject array(cx);
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

    return true;
}

static bool nidium_canvas2dctxGLProgram_uniform1iv(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    return nidium_canvas2dctxGLProgram_uniformXiv(cx, argc, vp, 1);
}

static bool nidium_canvas2dctxGLProgram_uniform2iv(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    return nidium_canvas2dctxGLProgram_uniformXiv(cx, argc, vp, 2);
}

static bool nidium_canvas2dctxGLProgram_uniform3iv(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    return nidium_canvas2dctxGLProgram_uniformXiv(cx, argc, vp, 3);
}

static bool nidium_canvas2dctxGLProgram_uniform4iv(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    return nidium_canvas2dctxGLProgram_uniformXiv(cx, argc, vp, 4);
}

static bool nidium_canvas2dctxGLProgram_uniform1fv(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    return nidium_canvas2dctxGLProgram_uniformXfv(cx, argc, vp, 1);
}

static bool nidium_canvas2dctxGLProgram_uniform2fv(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    return nidium_canvas2dctxGLProgram_uniformXfv(cx, argc, vp, 2);
}

static bool nidium_canvas2dctxGLProgram_uniform3fv(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    return nidium_canvas2dctxGLProgram_uniformXfv(cx, argc, vp, 3);
}

static bool nidium_canvas2dctxGLProgram_uniform4fv(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    return nidium_canvas2dctxGLProgram_uniformXfv(cx, argc, vp, 4);
}

static bool nidium_canvas2dctxGLProgram_getActiveUniforms(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
#define SET_PROP(where, name, val) JS_DefineProperty(cx, where, \
    (const char *)name, val, JSPROP_PERMANENT | JSPROP_READONLY | \
        JSPROP_ENUMERATE)
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    NIDIUM_LOG_2D_CALL();
    uint32_t program = (size_t)JS_GetPrivate(caller);
    int nactives = 0;

    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &nactives);

    JS::RootedObject arr(cx, JS_NewArrayObject(cx, nactives));
    args.rval().setObjectOrNull(arr);

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

    return true;
#undef SET_PROP
}

static bool nidium_canvas2dctx_light(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(Canvas2DContext, &Canvas2DContext_class);
    double x, y, z;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "ddd", &x, &y, &z)) {
        return false;
    }

    NSKIA_NATIVE->light(x, y, z);

    return true;
}

static bool nidium_canvas2dctx_prop_set(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp)
{
#define CTX_PROP(prop) CTX_PROP_ ## prop

    if (CANVASCTX_GETTER(obj)->m_SetterDisabled) {
        return true;
    }

    Nidium::Graphics::Skia *curSkia = NSKIA_NATIVE_GETTER(obj);
    switch (id) {
        case CTX_PROP(imageSmoothingEnabled):
        {
            if (!vp.isBoolean()) {
                vp.setBoolean(false);

                return true;
            }

            curSkia->setSmooth(vp.toBoolean());
            break;
        }
        case CTX_PROP(shadowOffsetX):
        {
            double ret;
            if (!vp.isNumber()) {
                vp.setNull();

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
                vp.setNull();

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
                vp.setNull();

                return true;
            }

            ret = vp.toNumber();
            curSkia->setShadowBlur(ret);
        }
        break;
        case CTX_PROP(shadowColor):
        {
            if (!vp.isString()) {
                vp.setNull();

                return true;
            }

            JS::RootedString vpStr(cx, JS::ToString(cx, vp));
            JSAutoByteString color(cx, vpStr);
            curSkia->setShadowColor(color.ptr());
        }
        break;
        case CTX_PROP(fontSize):
        {
            double ret;
            if (!vp.isNumber()) {
                vp.setNull();

                return true;
            }

            ret = vp.toNumber();
            curSkia->setFontSize(ret);

        }
        break;
        case CTX_PROP(fontStyle):
        {
            if (!vp.isString()) {
                vp.setNull();

                return true;
            }
            JS::RootedString vpStr(cx, JS::ToString(cx, vp));
            JSAutoByteString style(cx, vpStr);
            curSkia->setFontStyle(style.ptr());
        }
        break;
        case CTX_PROP(fontSkew):
        {
            double ret;
            if (!vp.isNumber()) {
                vp.setNull();

                return true;
            }

            ret = vp.toNumber();
            curSkia->setFontSkew(ret);
        }
        break;
        case CTX_PROP(textBaseline):
        {
            if (!vp.isString()) {
                vp.setNull();

                return true;
            }

            JS::RootedString vpStr(cx, JS::ToString(cx, vp));
            JSAutoByteString baseline(cx, vpStr);
            curSkia->textBaseline(baseline.ptr());
        }
        break;
        case CTX_PROP(textAlign):
        {
            if (!vp.isString()) {
                vp.setNull();

                return true;
            }

            JS::RootedString vpStr(cx, JS::ToString(cx, vp));
            JSAutoByteString font(cx, vpStr);
            curSkia->textAlign(font.ptr());
        }
        break;
        case CTX_PROP(fontFamily):
        {
            if (!vp.isString()) {
                vp.setNull();

                return true;
            }

            JS::RootedString vpStr(cx, JS::ToString(cx, vp));
            JSAutoByteString font(cx, vpStr);
            curSkia->setFontType(font.ptr(), JSDocument::GetObject(cx));
        }
        break;
        case CTX_PROP(fontFile):
        {
            if (!vp.isString()) {
                vp.setNull();

                return true;
            }

            JS::RootedString vpStr(cx, JS::ToString(cx, vp));
            JSAutoByteString font(cx, vpStr);
            if (!curSkia->setFontFile(font.ptr())) {
                JS_ReportError(cx, "Cannot set font (invalid file)");

                return false;
            }
        }
        break;
        case CTX_PROP(fillStyle):
        {
            Canvas2DContextState *state = CANVASCTX_GETTER(obj)->getCurrentState();

            if (vp.isString()) {
                JS::RootedString vpStr(cx, JS::ToString(cx, vp));
                JSAutoByteString colorName(cx, vpStr);
                curSkia->setFillColor(colorName.ptr());

                state->m_CurrentShader.setUndefined();

            } else if (vp.isObject() &&
                JS_GetClass(&vp.toObject()) == &CanvasGradient_class) {

                JS::RootedObject vpObj(cx, &vp.toObject());
                Nidium::Graphics::Gradient *gradient = (class Nidium::Graphics::Gradient *) JS_GetPrivate(vpObj);

                curSkia->setFillColor(gradient);

                /* Since out obj doesn't store the actual value (JSPROP_SHARED),
                   we implicitly store and root our pattern obj */
                state->m_CurrentShader.set(vp);

            } else if (vp.isObject() &&
                JS_GetClass(&vp.toObject()) == &CanvasPattern_class) {

                JS::RootedObject vpObj(cx, &vp.toObject());
                CanvasPattern *pattern = (class CanvasPattern *) JS_GetPrivate(vpObj);

                curSkia->setFillColor(pattern);

                state->m_CurrentShader.set(vp);
            } else {
                vp.setNull();

                state->m_CurrentShader.setUndefined();
                return true;
            }
        }
        break;
        case CTX_PROP(strokeStyle):
        {
            Canvas2DContextState *state = CANVASCTX_GETTER(obj)->getCurrentState();

            if (vp.isString()) {
                JS::RootedString vpStr(cx, JS::ToString(cx, vp));
                JSAutoByteString colorName(cx, vpStr);
                curSkia->setStrokeColor(colorName.ptr());

                state->m_CurrentStrokeShader.setUndefined();

            } else if (vp.isObject() &&
                JS_GetClass(&vp.toObject()) == &CanvasGradient_class) {
                JS::RootedObject vpObj(cx, &vp.toObject());
                Nidium::Graphics::Gradient *gradient = (class Nidium::Graphics::Gradient *) JS_GetPrivate(vpObj);

                curSkia->setStrokeColor(gradient);

                /* Since out obj doesn't store the actual value (JSPROP_SHARED),
                   we implicitly store and root our pattern obj */
                state->m_CurrentStrokeShader.set(vp);
            } else {
                vp.setNull();
                state->m_CurrentStrokeShader.setUndefined();

                return true;
            }
        }
        break;
        case CTX_PROP(lineWidth):
        {
            double ret;
            if (!vp.isNumber()) {
                vp.setNull();

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
                vp.setNull();

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
                vp.setNull();

                return true;
            }

            ret = vp.toNumber();
            curSkia->setGlobalAlpha(ret);
        }
        break;
        case CTX_PROP(globalCompositeOperation):
        {
            if (!vp.isString()) {
                vp.setNull();

                return true;
            }

            JS::RootedString vpStr(cx, JS::ToString(cx, vp));
            JSAutoByteString composite(cx, vpStr);
            curSkia->setGlobalComposite(composite.ptr());
        }
        break;
        case CTX_PROP(lineCap):
        {
            if (!vp.isString()) {
                vp.setNull();

                return true;
            }

            JS::RootedString vpStr(cx, JS::ToString(cx, vp));
            JSAutoByteString lineCap(cx, vpStr);
            curSkia->setLineCap(lineCap.ptr());
        }
        break;
        case CTX_PROP(lineJoin):
        {
            if (!vp.isString()) {
                vp.setNull();

                return true;
            }

            JS::RootedString vpStr(cx, JS::ToString(cx, vp));
            JSAutoByteString lineJoin(cx, vpStr);
            curSkia->setLineJoin(lineJoin.ptr());
        }
        break;
        default:
            break;
    }

    return true;
#undef CTX_PROP
}

static bool nidium_canvas2dctx_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp)
{
#define CTX_PROP(prop) CTX_PROP_ ## prop
    Nidium::Graphics::Skia *curSkia = NSKIA_NATIVE_GETTER(obj);
    Canvas2DContext *ctx = CANVASCTX_GETTER(obj);

    switch (id) {
        case CTX_PROP(width):
        {
            vp.setInt32(curSkia->getWidth());
        }
        break;
        case CTX_PROP(height):
        {
            vp.setInt32(curSkia->getHeight());
        }
        break;
        case CTX_PROP(fillStyle):
        case CTX_PROP(strokeStyle):
        {
            JS::RootedValue ret(cx);
            uint32_t curColor;

            if (id == CTX_PROP(fillStyle)) {
                ret = ctx->getCurrentState()->m_CurrentShader;
                curColor = curSkia->getFillColor();
            } else {
                ret = ctx->getCurrentState()->m_CurrentStrokeShader;
                curColor = curSkia->getStrokeColor();
            }

            if (ret.isUndefined()) {
                char rgba_str[64];

                Nidium::Graphics::Skia::GetStringColor(curColor, rgba_str);

                vp.setString(JS_NewStringCopyZ(cx, rgba_str));
            } else {
                vp.set(ret);
            }
        }
        break;
        case CTX_PROP(lineWidth):
        {
            vp.setDouble(curSkia->getLineWidth());
        }
        break;
        case CTX_PROP(miterLimit):
        {
            vp.setDouble(curSkia->getMiterLimit());
        }
        break;
        case CTX_PROP(globalAlpha):
        {
            vp.setDouble(curSkia->getGlobalAlpha());
        }
        break;
        case CTX_PROP(imageSmoothingEnabled):
        {
            vp.setBoolean(!!curSkia->getSmooth());
        }
        break;
        case CTX_PROP(shadowOffsetX):
        {
            vp.setDouble(curSkia->getShadowOffsetX());
        }
        break;
        case CTX_PROP(shadowOffsetY):
        {
            vp.setDouble(curSkia->getShadowOffsetY());
        }
        break;
        case CTX_PROP(shadowBlur):
        {
            vp.setDouble(curSkia->getShadowBlur());
        }
        break;
        case CTX_PROP(lineCap):
        {
            vp.setString(JS_NewStringCopyZ(cx, curSkia->getLineCap()));
        }
        break;
        case CTX_PROP(lineJoin):
        {
            vp.setString(JS_NewStringCopyZ(cx, curSkia->getLineJoin()));
        }
        break;
        case CTX_PROP(shadowColor):
        {
            char rgba_str[64];

            Nidium::Graphics::Skia::GetStringColor(curSkia->getShadowColor(), rgba_str);

            vp.setString(JS_NewStringCopyZ(cx, rgba_str));
        }
        break;
        default:
            vp.setUndefined();
            break;
    }

    return true;
#undef CTX_PROP
}

void CanvasGradient_Finalize(JSFreeOp *fop, JSObject *obj)
{
    Nidium::Graphics::Gradient *gradient = (class Nidium::Graphics::Gradient *)JS_GetPrivate(obj);
    if (gradient != NULL) {
        delete gradient;
    }
}

void CanvasPattern_Finalize(JSFreeOp *fop, JSObject *obj)
{
    CanvasPattern *pattern = (class CanvasPattern *)JS_GetPrivate(obj);
    if (pattern != NULL) {
        delete pattern;
    }
}

void Canvas2DContext_Finalize(JSFreeOp *fop, JSObject *obj)
{
    Canvas2DContext *canvasctx = CANVASCTX_GETTER(obj);
    if (canvasctx != NULL) {
        delete canvasctx;
    }
}

void Canvas2DContext_Trace(JSTracer *trc, JSObject *obj)
{
    Canvas2DContext *canvasctx = CANVASCTX_GETTER(obj);

    if (!canvasctx) {
        return;
    }

    for (Canvas2DContextState *state = canvasctx->getCurrentState();
        state != NULL; state = state->m_Next) {

        /* Does this matter if we trace an UndefinedValue? */
        JS_CallHeapValueTracer(trc, &state->m_CurrentShader, "Canvas2DContextShader");
        JS_CallHeapValueTracer(trc, &state->m_CurrentStrokeShader, "Canvas2DContextShader");
    }
}

void Canvas2DContext::clear(uint32_t color)
{
    m_Skia->getCanvas()->clear(color);
}

char *Canvas2DContext::genModifiedFragmentShader(const char *data)
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

uint32_t Canvas2DContext::createProgram(const char *data)
{
    char *pdata = Nidium::Graphics::CanvasContext::ProcessShader(data, Nidium::Graphics::CanvasContext::SHADER_FRAGMENT);

    if (pdata == NULL) {
        return 0;
    }

    char *nshader = this->genModifiedFragmentShader(pdata);

    uint32_t fragment = Nidium::Graphics::CanvasContext::CompileShader(nshader, GL_FRAGMENT_SHADER);
    uint32_t coop = this->compileCoopFragmentShader();
    uint32_t vertex = this->CreatePassThroughVertex();

    free(nshader);

    if (fragment == 0) {
        return 0;
    }

    GLuint programHandle;

    Nidium::Graphics::GLContext *iface = m_GLState->getNativeGLContext();

    NIDIUM_GL_CALL_RET(iface, CreateProgram(), programHandle);

    GLint linkSuccess;

    NIDIUM_GL_CALL(iface, AttachShader(programHandle, vertex));
    NIDIUM_GL_CALL(iface, AttachShader(programHandle, coop));
    NIDIUM_GL_CALL(iface, AttachShader(programHandle, fragment));

    NIDIUM_GL_CALL(iface, BindAttribLocation(programHandle,
        Nidium::Graphics::CanvasContext::SH_ATTR_POSITION, "Position"));

    NIDIUM_GL_CALL(iface, BindAttribLocation(programHandle,
        Nidium::Graphics::CanvasContext::SH_ATTR_TEXCOORD, "TexCoordIn"));

    NIDIUM_GL_CALL(iface, LinkProgram(programHandle));

    NIDIUM_GL_CALL(iface, GetProgramiv(programHandle, GR_GL_LINK_STATUS, &linkSuccess));
    if (linkSuccess == GL_FALSE) {
        GLchar messages[256];
        NIDIUM_GL_CALL(iface, GetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]));
        NLOG("createProgram error : %s", messages);
        return 0;
    }

    return programHandle;
}

uint32_t Canvas2DContext::compileCoopFragmentShader()
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
        "    _nm_main();\n"
        "}\n"
        "gl_FragColor = gl_FragColor * u_opacity;"
        "}\n";

    return this->CompileShader(coop, GL_FRAGMENT_SHADER);
}

#if 0
void Canvas2DContext::initCopyTex()
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
void Canvas2DContext::initCopyTex(uint32_t textureID)
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

uint32_t Canvas2DContext::getMainFBO()
{
    GrRenderTarget* backingTarget = (GrRenderTarget*)m_Skia->getCanvas()->
                                        getDevice()->accessRenderTarget();

    return (uint32_t)backingTarget->getRenderTargetHandle();
}

#if 0
void Canvas2DContext::drawTexIDToFBO(uint32_t textureID, uint32_t width,
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
void Canvas2DContext::setupCommonDraw()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
#endif

void Canvas2DContext::drawTexture(uint32_t textureID, uint32_t width,
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
void Canvas2DContext::drawTexToFBO(uint32_t textureID)
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

uint32_t Canvas2DContext::getSkiaTextureID(int *width, int *height)
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
void Canvas2DContext::resetSkiaContext(uint32_t flag)
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

uint32_t Canvas2DContext::attachShader(const char *string)
{
    uint32_t program = this->createProgram(string);

    if (program) {
        Nidium::Interface::NativeUIInterface *ui = m_GLState->getNativeGLContext()->getUI();
        /* Destroy the old context (if it's not shared) */
        m_GLState->destroy();
        /* Create a new state without program */
        Nidium::Graphics::GLState *nstate = new Nidium::Graphics::GLState(ui, false);
        nstate->setShared(false);

        m_GLState = nstate;

        m_GLState->setProgram(program);

        Nidium::Graphics::GLContext *iface = m_GLState->getNativeGLContext();

        NIDIUM_GL_CALL_RET(iface,
            GetUniformLocation(program, "n_Resolution"),
            m_GLState->m_GLObjects.uniforms.u_resolution);

        NIDIUM_GL_CALL_RET(iface,
            GetUniformLocation(program, "n_Position"),
            m_GLState->m_GLObjects.uniforms.u_position);

        NIDIUM_GL_CALL_RET(iface,
            GetUniformLocation(program, "n_Padding"),
            m_GLState->m_GLObjects.uniforms.u_padding);
    }

    return program;
}

void Canvas2DContext::detachShader()
{
#if 0
    /* TODO : shaders must be deleted */
    glDeleteProgram(m_GLObjects.program);
    m_GLObjects.program = 0;
#endif
}

void Canvas2DContext::setVertexDeformation(uint32_t vertex,
    float x, float y)
{
    Nidium::Graphics::GLState *state = m_GLState;

    /*
        If the GL state is shared among other Canvas, create a new one
    */
    if (state->isShared()) {
        NLOG("New GL state created !");
        state = new Nidium::Graphics::GLState(m_GLState->getNativeGLContext()->getUI());
        state->setShared(false);

        m_GLState = state;
    }

    state->setVertexDeformation(vertex, x, y);
}

uint32_t Canvas2DContext::getTextureID() const
{
    GrRenderTarget* backingTarget = (GrRenderTarget*)m_Skia->getCanvas()->
                                        getDevice()->accessRenderTarget();

    return backingTarget->asTexture()->getTextureHandle();
}

void Canvas2DContext::flush()
{
    m_Skia->getCanvas()->flush();
}

void Canvas2DContext::getSize(int *width, int *height) const
{
    SkISize size = m_Skia->getCanvas()->getDeviceSize();

    *width = size.width();
    *height = size.height();
}

void Canvas2DContext::setSize(int width, int height, bool redraw)
{
    SkBaseDevice *ndev = NULL;
    SkCanvas *ncanvas;

    float ratio = Nidium::Interface::NativeSystemInterface::GetInstance()->backingStorePixelRatio();

    if (m_Skia->m_NativeCanvasBindMode == Nidium::Graphics::Skia::BIND_GL) {
        if ((ncanvas = Nidium::Graphics::Skia::CreateGLCanvas(width, height,
            Nidium::Interface::__NativeUI->getNativeContext())) == NULL) {
            NLOG("[Error] Couldnt resize the canvas to %dx%d", width, height);
            return;
        }

        Nidium::Graphics::Skia::m_GlContext = ncanvas;

    } else {
#if 1
        const SkImageInfo &info = SkImageInfo::MakeN32Premul(width*ratio, height*ratio);
        ndev = Nidium::Graphics::Skia::m_GlContext->getDevice()->createCompatibleDevice(info);
#else
        GrContext *gr = ((SkGpuDevice *)Nidium::Graphics::Skia::m_GlContext->getDevice())->context();
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

    if (m_Skia->m_NativeCanvasBindMode == Nidium::Graphics::Skia::BIND_GL) {
        m_Skia->drawRect(0, 0, 1, 1, 0);
    }
}

void Canvas2DContext::translate(double x, double y)
{
    m_Skia->getCanvas()->translate(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

Canvas2DContext::Canvas2DContext(Nidium::Graphics::CanvasHandler *handler,
    JSContext *cx, int width, int height, Nidium::Interface::NativeUIInterface *ui) :
    Nidium::Graphics::CanvasContext(handler),
    m_SetterDisabled(false), m_CurrentState(NULL)
{
    m_Mode = CONTEXT_2D;

    this->pushNewState();

    JS::RootedObject jsobj(cx, JS_NewObject(cx, &Canvas2DContext_class, JS::NullPtr(), JS::NullPtr()));
    m_JsObj = jsobj;
    m_JsCx  = cx;

    /*
        TODO: BUG: xxx setter doesn't work if we remove this the definesProperties
    */
    JS_DefineProperties(cx, jsobj, canvas2dctx_props);

    m_Skia = new Nidium::Graphics::Skia();
    if (!m_Skia->bindOnScreen(width, height)) {
        delete m_Skia;
        m_Skia = NULL;
        return;
    }
    JS_SetPrivate(jsobj, this);

    /* Vertex buffers were unbound by parent constructor */
    this->resetSkiaContext(kVertex_GrGLBackendState);
}

Canvas2DContext::Canvas2DContext(Nidium::Graphics::CanvasHandler *handler,
    int width, int height, Nidium::Interface::NativeUIInterface *ui, bool isGL) :
    Nidium::Graphics::CanvasContext(handler), m_SetterDisabled(false)
{
    m_Mode = CONTEXT_2D;

    m_Skia = new Nidium::Graphics::Skia();
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

void Canvas2DContext::setScale(double x, double y,
    double px, double py)
{
    m_Skia->scale(1./px, 1./py);

    m_Skia->scale(x, y);
}

uint8_t *Canvas2DContext::getPixels()
{
    this->flush();

    return (uint8_t *)m_Skia->getCanvas()->getDevice()->
        accessBitmap(false).getPixels();
}

Canvas2DContext::~Canvas2DContext()
{
    //NLOG("Delete skia %p", skia);
    if (m_Skia) delete m_Skia;
}

static bool nidium_Canvas2DContext_constructor(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS_ReportError(cx, "Illegal constructor");
    return false;
}
// }}}

// {{{ Registration
void Canvas2DContext::RegisterObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS_InitClass(cx, global, JS::NullPtr(), &Canvas2DContext_class,
    nidium_Canvas2DContext_constructor, 0, canvas2dctx_props,
    canvas2dctx_funcs, nullptr, nullptr);
}
// }}}

} // namespace Binding
} // namespace Nidium

