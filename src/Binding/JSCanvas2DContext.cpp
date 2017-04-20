/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifdef _MSC_VER
#include "Port/MSWindows.h"
#else
#include <strings.h>
#endif

#include <SkDevice.h>
#include <GrContext.h>

#include "Binding/JSCanvas2DContext.h"
#include "Binding/JSImageData.h"

#include "Interface/SystemInterface.h"

#include "Graphics/Image.h"
#include "Graphics/Gradient.h"
#include "Graphics/GLHeader.h"
#include "Graphics/SkiaContext.h"
#include "Binding/JSCanvas.h"
#include "Binding/JSDocument.h"
#include "Binding/JSImage.h"

using namespace Nidium::Graphics;
using Nidium::Interface::UIInterface;

namespace Nidium {
namespace Binding {

static const char *getCoopFragmentShader();

#if 0 && DEBUG
#define NIDIUM_LOG_2D_CALL()                                              \
    JS::RootedObject calObj(cx, &args.callee());                          \
    if (JS_ObjectIsFunction(cx, calObj)) {                                \
        unsigned lineno;                                                  \
        JS::AutoFilename filename;                                        \
        JS::DescribeScriptedCaller(cx, &filename, &lineno);               \
        JS::RootedValue calVal(cx, JS::ObjectValue(*calObj));              \
        JS::RootedString _fun_name(                                       \
            cx, JS_GetFunctionDisplayId(JS_ValueToFunction(cx, calVal))); \
        JSAutoByteString _fun_namec(cx, _fun_name);                       \
        ndm_printf("Canvas2D.%s()] called on %s:%d", _fun_namec.ptr(),       \
                filename.get(), lineno);                                  \
    }
#else
#define NIDIUM_LOG_2D_CALL()
#endif
// {{{ AutoGLProgram
AutoGLProgram::AutoGLProgram(int32_t program)
    : m_Program(program)
{
    NIDIUM_GL_CALL_MAIN(GetIntegerv(GL_CURRENT_PROGRAM, &m_PreviousProgram));
    NIDIUM_GL_CALL_MAIN(UseProgram(m_Program));
}

AutoGLProgram::~AutoGLProgram()
{
    NIDIUM_GL_CALL_MAIN(UseProgram(m_PreviousProgram));
}
// }}}

// {{{ JSCanvasGLProgram
JSFunctionSpec *JSCanvasGLProgram::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSCanvasGLProgram, getUniformLocation, 1),
        CLASSMAPPER_FN(JSCanvasGLProgram, getActiveUniforms, 0),
        CLASSMAPPER_FN(JSCanvasGLProgram, uniform1i, 2),
        CLASSMAPPER_FN(JSCanvasGLProgram, uniform1f, 2),
        CLASSMAPPER_FN(JSCanvasGLProgram, uniform1iv, 2),
        CLASSMAPPER_FN(JSCanvasGLProgram, uniform2iv, 2),
        CLASSMAPPER_FN(JSCanvasGLProgram, uniform3iv, 2),
        CLASSMAPPER_FN(JSCanvasGLProgram, uniform4iv, 2),
        CLASSMAPPER_FN(JSCanvasGLProgram, uniform1fv, 2),
        CLASSMAPPER_FN(JSCanvasGLProgram, uniform2fv, 2),
        CLASSMAPPER_FN(JSCanvasGLProgram, uniform3fv, 2),
        CLASSMAPPER_FN(JSCanvasGLProgram, uniform4fv, 2),
        JS_FS_END
    };

    return funcs;
}

JSCanvasGLProgram::JSCanvasGLProgram(JSContext *cx, int32_t program) :
    m_Program(program)
{
    JSCanvasGLProgram::CreateObject(cx, this);
}

bool JSCanvasGLProgram::JS_getUniformLocation(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString location(cx);
    int ret;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "S", location.address())) {
        return false;
    }

    JSAutoByteString clocation(cx, location);
    ret = glGetUniformLocation(m_Program, clocation.ptr());

    args.rval().setInt32(ret);

    return true;
}

bool JSCanvasGLProgram::JS_getActiveUniforms(JSContext *cx, JS::CallArgs &args)
{
    NIDIUM_LOG_2D_CALL();
    int nactives = 0;

    NIDIUM_GL_CALL_MAIN(GetProgramiv(m_Program, GL_ACTIVE_UNIFORMS, &nactives));

    JS::RootedObject arr(cx, JS_NewArrayObject(cx, nactives));
    args.rval().setObjectOrNull(arr);

    char name[512];
    for (int i = 0; i < nactives; i++) {
        int length = 0, size = 0;
        GLenum type = GL_ZERO;
        JS::RootedObject in(cx, JS_NewPlainObject(cx));

        glGetActiveUniform(m_Program, i, sizeof(name) - 1, &length, &size, &type,
                           name);
        name[length] = '\0';
        JS::RootedString nameStr(cx, JS_NewStringCopyN(cx, name, length));
        JS::RootedValue locationVal(
            cx, JS::Int32Value(glGetUniformLocation(m_Program, name)));
        JS::RootedValue inval(cx, JS::ObjectValue(*in));
        NIDIUM_JSOBJ_SET_PROP(in, "name", nameStr);
        NIDIUM_JSOBJ_SET_PROP(in, "location", locationVal);
        NIDIUM_JSOBJ_SET_PROP_INT(in, "type", type);

        JS_SetElement(cx, arr, i, inval);
    }

    return true;
}

bool JSCanvasGLProgram::JS_uniform1i(JSContext *cx, JS::CallArgs &args)
{
    int location, val;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "ii", &location, &val)) {
        return false;
    }

    if (location == -1) {
        return true;
    }

    AutoGLProgram autoProg(m_Program);
    
    NIDIUM_GL_CALL_MAIN(Uniform1i(location, val));

    return true;
}

bool JSCanvasGLProgram::JS_uniform1f(JSContext *cx, JS::CallArgs &args)
{
    int location;
    double val;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "id", &location, &val)) {
        return false;
    }

    if (location == -1) {
        return true;
    }

    AutoGLProgram autoProg(m_Program);

    NIDIUM_GL_CALL_MAIN(Uniform1f(location, (float)val));

    return true;
}

bool JSCanvasGLProgram::uniformXiv(JSContext *cx, JS::CallArgs &args, int nb)
{
    GLsizei length;
    GLint *carray;
    int location;
    bool isarray;

    NIDIUM_LOG_2D_CALL();
    JS::RootedObject array(cx);
    if (!JS_ConvertArguments(cx, args, "io", &location, array.address())) {
        return false;
    }

    if (location == -1) {
        return true;
    }
    if (JS_IsInt32Array(array)) {
        bool shared;
        JS::AutoCheckCannotGC nogc;

        carray = (GLint *)JS_GetInt32ArrayData(array, &shared, nogc);
        length = (GLsizei)JS_GetTypedArrayLength(array);
    } else if (JS_IsArrayObject(cx, array, &isarray) && isarray) {
        bool shared;
        JS::AutoCheckCannotGC nogc;

        JS::RootedObject tmp(cx, JS_NewInt32ArrayFromArray(cx, array));
        carray = (GLint *)JS_GetInt32ArrayData(tmp, &shared, nogc);
        length = (GLsizei)JS_GetTypedArrayLength(tmp);
    } else {
        JS_ReportError(cx, "Array is not a Int32 array");
        return false;
    }

    AutoGLProgram autoProg(m_Program);

    switch (nb) {
        case 1:
            NIDIUM_GL_CALL_MAIN(Uniform1iv(location, length, carray));
            break;
        case 2:
            NIDIUM_GL_CALL_MAIN(Uniform2iv(location, length / 2, carray));
            break;
        case 3:
            NIDIUM_GL_CALL_MAIN(Uniform3iv(location, length / 3, carray));
            break;
        case 4:
            NIDIUM_GL_CALL_MAIN(Uniform4iv(location, length / 4, carray));
            break;
        default:
            break;
    }

    return true;
}

bool JSCanvasGLProgram::uniformXfv(JSContext *cx, JS::CallArgs &args, int nb)
{
    GLsizei length;
    GLfloat *carray;
    int location;
    bool isarray;

    NIDIUM_LOG_2D_CALL();
    JS::RootedObject array(cx);
    if (!JS_ConvertArguments(cx, args, "io", &location, array.address())) {
        return false;
    }

    if (location == -1) {
        return true;
    }

    if (JS_IsFloat32Array(array)) {
        bool shared;
        JS::AutoCheckCannotGC nogc;

        carray = (GLfloat *)JS_GetFloat32ArrayData(array, &shared, nogc);
        length = (GLsizei)JS_GetTypedArrayLength(array);
    } else if (JS_IsArrayObject(cx, array, &isarray) && isarray) {
        bool shared;
        JS::AutoCheckCannotGC nogc;

        JS::RootedObject tmp(cx, JS_NewFloat32ArrayFromArray(cx, array));
        carray = (GLfloat *)JS_GetFloat32ArrayData(tmp, &shared, nogc);
        length = (GLsizei)JS_GetTypedArrayLength(tmp);
    } else {
        JS_ReportError(cx, "Array is not a Float32 array");
        return false;
    }

    AutoGLProgram autoProg(m_Program);

    switch (nb) {
        case 1:
            NIDIUM_GL_CALL_MAIN(Uniform1fv(location, length, carray));
            break;
        case 2:
            NIDIUM_GL_CALL_MAIN(Uniform2fv(location, length / 2, carray));
            break;
        case 3:
            NIDIUM_GL_CALL_MAIN(Uniform3fv(location, length / 3, carray));
            break;
        case 4:
            NIDIUM_GL_CALL_MAIN(Uniform4fv(location, length / 4, carray));
            break;
    }

    return true;
}

bool JSCanvasGLProgram::JS_uniform1iv(JSContext *cx, JS::CallArgs &args)
{
    return this->uniformXiv(cx, args, 1);
}

bool JSCanvasGLProgram::JS_uniform2iv(JSContext *cx, JS::CallArgs &args)
{
    return this->uniformXiv(cx, args, 2);
}

bool JSCanvasGLProgram::JS_uniform3iv(JSContext *cx, JS::CallArgs &args)
{
    return this->uniformXiv(cx, args, 3);
}

bool JSCanvasGLProgram::JS_uniform4iv(JSContext *cx, JS::CallArgs &args)
{
    return this->uniformXiv(cx, args, 4);
}

bool JSCanvasGLProgram::JS_uniform1fv(JSContext *cx, JS::CallArgs &args)
{
    return this->uniformXfv(cx, args, 1);
}

bool JSCanvasGLProgram::JS_uniform2fv(JSContext *cx, JS::CallArgs &args)
{
    return this->uniformXfv(cx, args, 2);
}

bool JSCanvasGLProgram::JS_uniform3fv(JSContext *cx, JS::CallArgs &args)
{
    return this->uniformXfv(cx, args, 3);
}

bool JSCanvasGLProgram::JS_uniform4fv(JSContext *cx, JS::CallArgs &args)
{
    return this->uniformXfv(cx, args, 4);
}
// }}}

// {{{ Canvas2DContext
bool Canvas2DContext::JS_onerror(JSContext *cx, JS::CallArgs &args)
{
    NIDIUM_LOG_2D_CALL();

    return true;
}

bool Canvas2DContext::JS_fillRect(JSContext *cx, JS::CallArgs &args)
{
    NIDIUM_LOG_2D_CALL();

    double x, y, width, height, rx = 0, ry = 0;

    if (!JS_ConvertArguments(cx, args, "dddd/dd", &x, &y, &width, &height, &rx,
                             &ry)) {
        return false;
    }

    if (args.length() > 4) {
        m_Skia->drawRect(x, y, width, height, rx, (args.length() == 5 ? rx : ry), 0);
    } else {
        m_Skia->drawRect(x, y, width, height, 0);
    }

    return true;
}

bool Canvas2DContext::JS_strokeRect(JSContext *cx, JS::CallArgs &args)
{
    double x, y, width, height, rx = 0, ry = 0;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dddd/dd", &x, &y, &width, &height, &rx,
                             &ry)) {
        return false;
    }

    if (args.length() > 4) {
        m_Skia->drawRect(x, y, width, height, rx, (args.length() == 5 ? rx : ry), 1);
    } else {
        m_Skia->drawRect(x, y, width, height, 1);
    }

    return true;
}

bool Canvas2DContext::JS_clearRect(JSContext *cx, JS::CallArgs &args)
{
    double x, y, width, height;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dddd", &x, &y, &width, &height)) {
        return false;
    }

    m_Skia->clearRect(x, y, width, height);

    return true;
}

bool Canvas2DContext::JS_breakText(JSContext *cx, JS::CallArgs &args)
{

    NIDIUM_LOG_2D_CALL();

#define SET_PROP(where, name, val)                        \
    JS_DefineProperty(cx, where, (const char *)name, val, \
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)
    double maxWidth;
    int length = 0;

    JS::RootedString str(cx);
    if (!JS_ConvertArguments(cx, args, "Sd", str.address(), &maxWidth)) {
        return false;
    }

    JSAutoByteString text(cx, str);
    size_t len = text.length();

    if (len == 0) {
        args.rval().setNull();
        return true;
    }

    struct _Line *lines = new struct _Line[len];

    if (!lines) {
        JS_ReportOutOfMemory(cx);
        return false;
    }

    memset(lines, 0, len * sizeof(struct _Line));

    SkScalar ret = m_Skia->breakText(text.ptr(), len, lines,
                                                   maxWidth, &length);
    JS::RootedObject alines(cx, JS_NewArrayObject(cx, length));
    for (int i = 0; i < len && i < length; i++) {
        JS::RootedString str(
            cx, JS_NewStringCopyN(cx, lines[i].m_Line, lines[i].m_Len));
        JS::RootedValue val(cx, JS::StringValue(str));
        JS_SetElement(cx, alines, i, val);
    }
    JS::RootedObject res(cx, JS_NewPlainObject(cx));
    JS::RootedValue heightVal(cx, JS::DoubleValue(SkScalarToDouble(ret)));
    JS::RootedValue linesVal(cx, JS::ObjectValue(*alines));
    SET_PROP(res, "height", heightVal);
    SET_PROP(res, "lines", linesVal);

    args.rval().setObjectOrNull(res);

    delete[] lines;

    return true;
#undef SET_PROP
}

bool Canvas2DContext::JS_fillText(JSContext *cx, JS::CallArgs &args)
{
    int x, y, maxwidth;

    NIDIUM_LOG_2D_CALL();
    JS::RootedString str(cx);
    if (!JS_ConvertArguments(cx, args, "Sii/i", str.address(), &x, &y,
                             &maxwidth)) {
        return false;
    }

    JSAutoByteString text;
    text.encodeUtf8(cx, str);

    m_Skia->drawText(text.ptr(), x, y);

    return true;
}

bool Canvas2DContext::JS_strokeText(JSContext *cx, JS::CallArgs &args)
{
    int x, y, maxwidth;

    NIDIUM_LOG_2D_CALL();
    JS::RootedString str(cx);
    if (!JS_ConvertArguments(cx, args, "Sii/i", str.address(), &x, &y,
                             &maxwidth)) {
        return false;
    }

    JSAutoByteString text;
    text.encodeUtf8(cx, str);

    m_Skia->drawText(text.ptr(), x, y, true);

    return true;
}

bool Canvas2DContext::JS_shadow(JSContext *cx, JS::CallArgs &args)
{
    // m_Skia->setShadow();
    return true;
}

bool Canvas2DContext::JS_beginPath(JSContext *cx, JS::CallArgs &args)
{
    NIDIUM_LOG_2D_CALL();

    m_Skia->beginPath();

    return true;
}

bool Canvas2DContext::JS_moveTo(JSContext *cx, JS::CallArgs &args)
{
    double x, y;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        return false;
    }

    m_Skia->moveTo(x, y);

    return true;
}

bool Canvas2DContext::JS_lineTo(JSContext *cx, JS::CallArgs &args)
{
    double x, y;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        return false;
    }

    m_Skia->lineTo(x, y);

    return true;
}

bool Canvas2DContext::JS_fill(JSContext *cx, JS::CallArgs &args)
{
    NIDIUM_LOG_2D_CALL();
    m_Skia->fill();

    return true;
}

bool Canvas2DContext::JS_stroke(JSContext *cx, JS::CallArgs &args)
{
    NIDIUM_LOG_2D_CALL();
    m_Skia->stroke();

    return true;
}
bool Canvas2DContext::JS_closePath(JSContext *cx, JS::CallArgs &args)
{
    NIDIUM_LOG_2D_CALL();
    m_Skia->closePath();

    return true;
}

bool Canvas2DContext::JS_clip(JSContext *cx, JS::CallArgs &args)
{
    NIDIUM_LOG_2D_CALL();
    m_Skia->clip();

    return true;
}

bool Canvas2DContext::JS_rect(JSContext *cx, JS::CallArgs &args)
{
    double x, y, width, height;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dddd", &x, &y, &width, &height)) {
        return false;
    }

    m_Skia->rect(x, y, width, height);

    return true;
}

bool Canvas2DContext::JS_arc(JSContext *cx, JS::CallArgs &args)
{
    int x, y, radius;
    double startAngle, endAngle;
    bool CCW = false;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "iiidd/b", &x, &y, &radius, &startAngle,
                             &endAngle, &CCW)) {
        return false;
    }

    m_Skia->arc(x, y, radius, startAngle, endAngle, CCW);

    return true;
}

bool Canvas2DContext::JS_arcTo(JSContext *cx, JS::CallArgs &args)
{
    int x1, y1, x2, y2, radius;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "iiiii", &x1, &y1, &x2, &y2, &radius)) {
        return false;
    }

    m_Skia->arcTo(x1, y1, x2, y2, radius);

    return true;
}

bool Canvas2DContext::JS_quadraticCurveTo(JSContext *cx, JS::CallArgs &args)
{
    double x, y, cpx, cpy;
    if (!JS_ConvertArguments(cx, args, "dddd", &cpx, &cpy, &x, &y)) {
        return false;
    }

    m_Skia->quadraticCurveTo(cpx, cpy, x, y);

    return true;
}

bool Canvas2DContext::JS_bezierCurveTo(JSContext *cx, JS::CallArgs &args)
{
    double x, y, cpx, cpy, cpx2, cpy2;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dddddd", &cpx, &cpy, &cpx2, &cpy2, &x,
                             &y)) {
        return false;
    }

    m_Skia->bezierCurveTo(cpx, cpy, cpx2, cpy2, x, y);

    return true;
}

bool Canvas2DContext::JS_rotate(JSContext *cx, JS::CallArgs &args)
{
    double angle;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "d", &angle)) {
        return false;
    }

    m_Skia->rotate(angle);

    return true;
}

bool Canvas2DContext::JS_scale(JSContext *cx, JS::CallArgs &args)
{
    double x, y;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        return false;
    }

    m_Skia->scale(x, y);

    return true;
}

bool Canvas2DContext::JS_translate(JSContext *cx, JS::CallArgs &args)
{
    double x, y;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        return false;
    }

    m_Skia->translate(x, y);

    return true;
}

bool Canvas2DContext::JS_transform(JSContext *cx, JS::CallArgs &args)
{
    double scalex, skewx, skewy, scaley, translatex, translatey, rotate;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dddddd/d", &scalex, &skewx, &skewy,
                             &scaley, &translatex, &translatey, &rotate)) {
        return false;
    }

    m_Skia->transform(scalex, skewx, skewy, scaley, translatex, translatey, 0);

    if (args.length() == 7) {
        m_Skia->rotate(rotate);
    }

    return true;
}

bool Canvas2DContext::JS_iTransform(JSContext *cx, JS::CallArgs &args)
{
    double scalex, skewx, skewy, scaley, translatex, translatey;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dddddd", &scalex, &skewx, &skewy,
                             &scaley, &translatex, &translatey)) {
        return false;
    }

    m_Skia->itransform(scalex, skewx, skewy, scaley, translatex, translatey);

    return true;
}

bool Canvas2DContext::JS_setTransform(JSContext *cx, JS::CallArgs &args)
{
    double scalex, skewx, skewy, scaley, translatex, translatey;

    if (!JS_ConvertArguments(cx, args, "dddddd", &scalex, &skewx, &skewy,
                             &scaley, &translatex, &translatey)) {
        NIDIUM_LOG_2D_CALL();
        return false;
    }

    m_Skia->transform(scalex, skewx, skewy, scaley,
                       translatex + m_Handler->p_Coating,
                       translatey + m_Handler->p_Coating, 1);

    return true;
}

bool Canvas2DContext::JS_save(JSContext *cx, JS::CallArgs &args)
{
    NIDIUM_LOG_2D_CALL();

    /*
        TODO: limit? (avoid while(1) ctx.save())
    */
    this->pushNewState();
    m_Skia->save();

    return true;
}

bool Canvas2DContext::JS_restore(JSContext *cx, JS::CallArgs &args)
{
    NIDIUM_LOG_2D_CALL();

    this->popState();
    m_Skia->restore();

    return true;
}

bool Canvas2DContext::JS_createLinearGradient(JSContext *cx, JS::CallArgs &args)
{
    NIDIUM_LOG_2D_CALL();
    double x1, y1, x2, y2;

    if (!JS_ConvertArguments(cx, args, "dddd", &x1, &y1, &x2, &y2)) {
        return false;
    }

    Gradient *gradient = new Gradient(x1, y1, x2, y2);
    JS::RootedObject linearObject(cx,
        JSGradient::CreateObject(cx, new JSGradient(gradient)));

    args.rval().setObjectOrNull(linearObject);

    return true;
}

bool Canvas2DContext::JS_getImageData(JSContext *cx, JS::CallArgs &args)
{
    int left, top, width, height;
    uint8_t *data;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "iiii", &left, &top, &width, &height)) {
        return false;
    }

    JS::RootedObject arrBuffer(cx,
                               JS_NewUint8ClampedArray(cx, width * height * 4));
    {
        bool shared;
        JS::AutoCheckCannotGC nogc;

        data = JS_GetUint8ClampedArrayData(arrBuffer, &shared, nogc);
        m_Skia->readPixels(top, left, width, height, data);
    }
    JS::RootedValue widthVal(cx, JS::NumberValue(width));
    JS::RootedValue heightVal(cx, JS::NumberValue(height));
    JS::RootedValue arVal(cx, JS::ObjectValue(*arrBuffer));

    JS::RootedObject dataObject(cx,
      JSImageData::CreateObject(cx, new JSImageData()));

    JS_DefineProperty(cx, dataObject, "width", widthVal,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "height", heightVal,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "data", arVal,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    args.rval().setObjectOrNull(dataObject);

    return true;
}

/* TODO: Huge memory leak? */
bool Canvas2DContext::JS_putImageData(JSContext *cx, JS::CallArgs &args)
{
    int x, y;
    uint8_t *pixels;
    int32_t w, h;

    NIDIUM_LOG_2D_CALL();
    JS::RootedObject dataObject(cx);
    if (!JS_ConvertArguments(cx, args, "oii", dataObject.address(), &x, &y)) {
        return false;
    }

    if (!dataObject || !JSImageData::InstanceOf(dataObject)) {
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

    JS::ToInt32(cx, jwidth, &w);
    JS::ToInt32(cx, jheight, &h);

    bool shared;
    JS::AutoCheckCannotGC nogc;

    pixels = JS_GetUint8ClampedArrayData(jObj, &shared, nogc);

    m_Skia->drawPixels(pixels, w, h, x, y);

    return true;
}

/* TODO: clamp max size */
bool Canvas2DContext::JS_createImageData(JSContext *cx, JS::CallArgs &args)
{
    uint32_t x, y;

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

    JS::RootedObject arrBuffer(cx, JS_NewUint8ClampedArray(cx, x * y * 4));
    if (!arrBuffer.get()) {
        JS_ReportOutOfMemory(cx);
        return false;
    }

    JS::RootedValue array(cx, JS::ObjectValue(*arrBuffer));
    JS::RootedValue width(cx, JS::NumberValue(x));
    JS::RootedValue height(cx, JS::NumberValue(y));

    JS::RootedObject dataObject(cx,
      JSImageData::CreateObject(cx, new JSImageData()));

    JS_DefineProperty(cx, dataObject, "width", width,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "height", height,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "data", array,
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    args.rval().setObjectOrNull(dataObject);

    return true;
}

bool Canvas2DContext::JS_createPattern(JSContext *cx, JS::CallArgs &args)
{
    NIDIUM_LOG_2D_CALL();

    JS::RootedObject jsimage(cx);
    JS::RootedString mode(cx);
    if (!JS_ConvertArguments(cx, args, "oS", jsimage.address(),
                             mode.address())) {
        return false;
    }

    if (!JSImage::InstanceOf(jsimage)) {
        JS_ReportError(cx, "First parameter is not an Image");
        return false;
    }

    JSImage *img = JSImage::GetInstanceUnsafe(jsimage);

    JSCanvasPattern::PATTERN_MODE pmode = JSCanvasPattern::PATTERN_REPEAT;

    JSAutoByteString cmode(cx, mode);
    if (strcasecmp(cmode.ptr(), "repeat") == 0) {
        pmode = JSCanvasPattern::PATTERN_REPEAT;
    } else if (strcasecmp(cmode.ptr(), "no-repeat") == 0) {
        pmode = JSCanvasPattern::PATTERN_NOREPEAT;
    } else if (strcasecmp(cmode.ptr(), "repeat-x") == 0) {
        pmode = JSCanvasPattern::PATTERN_REPEAT_X;
    } else if (strcasecmp(cmode.ptr(), "repeat-y") == 0) {
        pmode = JSCanvasPattern::PATTERN_REPEAT_Y;
    } else if (strcasecmp(cmode.ptr(), "repeat-mirror") == 0) {
        pmode = JSCanvasPattern::PATTERN_REPEAT_MIRROR;
    }

    JS::RootedObject patternObject(cx,
      JSCanvasPattern::CreateObject(cx, new JSCanvasPattern(img, pmode)));

    /*
        The pattern object retains a reference to the JSImage object
    */
    JS_SetReservedSlot(patternObject, 0, JS::ObjectValue(*img->getJSObject()));

    args.rval().setObjectOrNull(patternObject);

    return true;
}

bool Canvas2DContext::JS_createRadialGradient(JSContext *cx, JS::CallArgs &args)
{
    double x1, y1, x2, y2, r1, r2;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dddddd", &x1, &y1, &r1, &x2, &y2,
                             &r2)) {
        return false;
    }

    Gradient *gradient = new Gradient(x1, y1, r1, x2, y2, r2);
    JS::RootedObject radialObject(cx,
        JSGradient::CreateObject(cx, new JSGradient(gradient)));

    args.rval().setObjectOrNull(radialObject);

    return true;
}

bool Canvas2DContext::JS_drawImage(JSContext *cx, JS::CallArgs &args)
{
    Image *image;
    double x, y, width, height;
    int sx, sy, swidth, sheight;
    int need_free = 0;

    NIDIUM_LOG_2D_CALL();
    JS::RootedObject jsimage(cx);
    if (args.length() == 9) {
        if (!JS_ConvertArguments(cx, args, "oiiiidddd", jsimage.address(), &sx,
                                 &sy, &swidth, &sheight, &x, &y, &width,
                                 &height)) {
            return false;
        }
    } else {
        if (!JS_ConvertArguments(cx, args, "odd/dd", jsimage.address(), &x, &y,
                                 &width, &height)) {
            return false;
        }
    }
    /* The image is a Canvas */
    /*
        TODO: work with WebGL canvas
    */
    if (jsimage && JSCanvas::InstanceOf(jsimage)) {
        CanvasContext *drawctx = JSCanvas::GetInstanceUnsafe(jsimage)->getHandler()->getContext();

        if (drawctx == NULL
            || drawctx->getContextType() != CanvasContext::CONTEXT_2D) {
            JS_ReportError(
                cx, "Invalid image canvas (must be backed by a 2D context)");
            return false;
        }
        image = Image::CreateFromSurface((static_cast<Canvas2DContext *>(drawctx))
                              ->getSkiaContext()
                              ->getSurface());
        need_free = 1;
    } else if (!jsimage || !JSImage::InstanceOf(jsimage)
               || (image = JSImage::JSObjectToImage(jsimage)) == NULL) {

        JS_ReportWarning(cx, "Invalid image given");
        return true;
    }

    switch (args.length()) {
        case 3:
            m_Skia->drawImage(image, x, y);
            break;
        case 5:
            m_Skia->drawImage(image, x, y, width, height);
            break;
        case 9:
            m_Skia->drawImage(image, sx, sy, swidth, sheight, x, y, width,
                               height);
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

bool Canvas2DContext::JS_measureText(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString text(cx);
#define OBJ_PROP(name, val)               \
    JS_DefineProperty(cx, obj, name, val, \
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY)

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "S", text.address())) {
        return false;
    }

    JS::RootedObject obj(cx, JS_NewPlainObject(cx));

    JSAutoByteString ctext;
    ctext.encodeUtf8(cx, text);

    JS::RootedValue widthVal(
        cx, JS::DoubleValue(m_Skia->measureText(ctext.ptr(), strlen(ctext.ptr()))));
    OBJ_PROP("width", widthVal);

    args.rval().setObjectOrNull(obj);

    return true;
}

bool Canvas2DContext::JS_isPointInPath(JSContext *cx, JS::CallArgs &args)
{
    double x, y;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dd", &x, &y)) {
        args.rval().setBoolean(false);
        return false;
    }

    args.rval().setBoolean(m_Skia->SkPathContainsPoint(x, y));

    return true;
}

/* TODO: return undefined if the path is invalid */
bool Canvas2DContext::JS_getPathBounds(JSContext *cx, JS::CallArgs &args)
{
#define OBJ_PROP(name, val)               \
    JS_DefineProperty(cx, obj, name, val, \
                      JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY)
    double left = 0, right = 0, top = 0, bottom = 0;
    JS::RootedObject obj(cx, JS_NewPlainObject(cx));

    NIDIUM_LOG_2D_CALL();
    m_Skia->getPathBounds(&left, &right, &top, &bottom);
    JS::RootedValue leftVal(cx, JS::DoubleValue(left));
    JS::RootedValue rightVal(cx, JS::DoubleValue(right));
    JS::RootedValue topVal(cx, JS::DoubleValue(top));
    JS::RootedValue bottomVal(cx, JS::DoubleValue(bottom));
    OBJ_PROP("left", leftVal);
    OBJ_PROP("right", rightVal);
    OBJ_PROP("top", topVal);
    OBJ_PROP("bottom", bottomVal);

    args.rval().setObject(*obj);

    return true;
}

bool Canvas2DContext::JS_detachFragmentShader(JSContext *cx, JS::CallArgs &args)
{
    NIDIUM_LOG_2D_CALL();

    this->detachShader();

    return true;
}

bool Canvas2DContext::JS_setVertexOffset(JSContext *cx, JS::CallArgs &args)
{
    uint32_t vertex;
    double x, y;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "udd", &vertex, &x, &y)) {
        return false;
    }

    this->setVertexDeformation(vertex, x, y);

    return true;
}

bool Canvas2DContext::JS_attachFragmentShader(JSContext *cx, JS::CallArgs &args)
{
    uint32_t program;

    NIDIUM_LOG_2D_CALL();
    JS::RootedString glsl(cx);
    if (!JS_ConvertArguments(cx, args, "S", glsl.address())) {
        return false;
    }

    JSAutoByteString cglsl(cx, glsl);
    if ((program = this->attachShader(cglsl.ptr())) == 0) {
        JS_ReportError(cx, "Failed to compile GLSL shader");
        return false;
    }

    JSCanvasGLProgram *glProgram = new JSCanvasGLProgram(cx, program);
    JS::RootedObject canvasProgram(cx, glProgram->getJSObject());

    args.rval().setObjectOrNull(canvasProgram);

    return true;
}

bool Canvas2DContext::JSSetter_imageSmoothingEnabled(JSContext *cx,
    JS::MutableHandleValue vp)
{
    if (vp.isBoolean()) {
        m_Skia->setSmooth(vp.toBoolean());
    }

    return true;
}

bool Canvas2DContext::JSSetter_shadowOffsetX(JSContext *cx,
    JS::MutableHandleValue vp)
{
    if (vp.isNumber()) {
        m_Skia->setShadowOffsetX(vp.toNumber());
    }

    return true;
}

bool Canvas2DContext::JSSetter_shadowOffsetY(JSContext *cx,
    JS::MutableHandleValue vp)
{
    if (vp.isNumber()) {
        m_Skia->setShadowOffsetY(vp.toNumber());
    }

    return true;
}

bool Canvas2DContext::JSSetter_shadowBlur(JSContext *cx,
    JS::MutableHandleValue vp)
{
    if (vp.isNumber()) {
        m_Skia->setShadowBlur(vp.toNumber());
    }

    return true;
}

bool Canvas2DContext::JSSetter_shadowColor(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS::RootedString vpStr(cx, JS::ToString(cx, vp));
    JSAutoByteString color(cx, vpStr);
    m_Skia->setShadowColor(color.ptr());

    return true;
}

bool Canvas2DContext::JSSetter_fontSize(JSContext *cx,
    JS::MutableHandleValue vp)
{
    if (vp.isNumber()) {
        m_Skia->setFontSize(vp.toNumber());
    }

    return true;
}

bool Canvas2DContext::JSSetter_fontStyle(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS::RootedString vpStr(cx, JS::ToString(cx, vp));
    JSAutoByteString style(cx, vpStr);
    m_Skia->setFontStyle(style.ptr());

    return true;
}

bool Canvas2DContext::JSSetter_fontSkew(JSContext *cx,
    JS::MutableHandleValue vp)
{
    if (vp.isNumber()) {
        m_Skia->setFontSkew(vp.toNumber());
    }

    return true;
}

bool Canvas2DContext::JSSetter_textBaseline(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS::RootedString vpStr(cx, JS::ToString(cx, vp));
    JSAutoByteString baseline(cx, vpStr);
    m_Skia->textBaseline(baseline.ptr());

    return true;
}

bool Canvas2DContext::JSSetter_textAlign(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS::RootedString vpStr(cx, JS::ToString(cx, vp));
    JSAutoByteString align(cx, vpStr);
    m_Skia->textAlign(align.ptr());

    return true;
}

bool Canvas2DContext::JSSetter_fontFamily(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS::RootedString vpStr(cx, JS::ToString(cx, vp));
    JSAutoByteString font(cx, vpStr);
    m_Skia->setFontType(font.ptr(), JSDocument::GetInstanceSingleton());;

    return true;
}

bool Canvas2DContext::JSSetter_fontFile(JSContext *cx,
    JS::MutableHandleValue vp)
{

    JS::RootedString vpStr(cx, JS::ToString(cx, vp));
    JSAutoByteString font(cx, vpStr);

    if (!m_Skia->setFontFile(font.ptr())) {
        JS_ReportError(cx, "Can't set font (invalid file)");

        return false;
    }

    return true;
}

bool Canvas2DContext::JSSetter_fillStyle(JSContext *cx,
    JS::MutableHandleValue vp)
{
    Canvas2DContextState *state = getCurrentState();

    if (vp.isString()) {
        JS::RootedString vpStr(cx, JS::ToString(cx, vp));
        JSAutoByteString colorName(cx, vpStr);
        m_Skia->setFillColor(colorName.ptr());

        state->m_CurrentShader.setUndefined();
    } else if (vp.isObject() && JSGradient::InstanceOf(vp)) {

        m_Skia->setFillColor(JSGradient::GetInstanceUnsafe(
                vp.toObjectOrNull())->getGradient());

        /* Since out obj doesn't store the actual value (JSPROP_SHARED),
           we implicitly store and root our pattern obj */
        state->m_CurrentShader = vp;

    } else if (vp.isObject() && JSCanvasPattern::InstanceOf(vp)) {

        m_Skia->setFillColor(JSCanvasPattern::GetInstanceUnsafe(vp.toObjectOrNull()));

        state->m_CurrentShader = vp;
    } else {
        vp.setNull();

        state->m_CurrentShader.setUndefined();
        return true;
    }
    return true;
}

bool Canvas2DContext::JSSetter_strokeStyle(JSContext *cx,
    JS::MutableHandleValue vp)
{
    Canvas2DContextState *state = getCurrentState();

    if (vp.isString()) {
        JS::RootedString vpStr(cx, JS::ToString(cx, vp));
        JSAutoByteString colorName(cx, vpStr);
        m_Skia->setStrokeColor(colorName.ptr());

        state->m_CurrentStrokeShader.setUndefined();

    } else if (vp.isObject() && JSGradient::InstanceOf(vp)) {

        m_Skia->setStrokeColor(JSGradient::GetInstanceUnsafe(
                vp.toObjectOrNull())->getGradient());

        /* Since out obj doesn't store the actual value (JSPROP_SHARED),
           we implicitly store and root our pattern obj */
        state->m_CurrentStrokeShader = vp;
    } else {
        vp.setNull();
        state->m_CurrentStrokeShader.setUndefined();

        return true;
    }

    return true;
}

bool Canvas2DContext::JSSetter_lineWidth(JSContext *cx,
    JS::MutableHandleValue vp)
{
    if (vp.isNumber()) {
        m_Skia->setLineWidth(vp.toNumber());
    }

    return true;
}

bool Canvas2DContext::JSSetter_miterLimit(JSContext *cx,
    JS::MutableHandleValue vp)
{
    if (vp.isNumber()) {
        m_Skia->setMiterLimit(vp.toNumber());
    }

    return true;
}

bool Canvas2DContext::JSSetter_globalAlpha(JSContext *cx,
    JS::MutableHandleValue vp)
{
    if (vp.isNumber()) {
        m_Skia->setGlobalAlpha(vp.toNumber());
    }

    return true;
}

bool Canvas2DContext::JSSetter_globalCompositeOperation(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS::RootedString vpStr(cx, JS::ToString(cx, vp));
    JSAutoByteString composite(cx, vpStr);
    m_Skia->setGlobalComposite(composite.ptr());

    return true;
}

bool Canvas2DContext::JSSetter_lineCap(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS::RootedString vpStr(cx, JS::ToString(cx, vp));
    JSAutoByteString lineCap(cx, vpStr);
    m_Skia->setLineCap(lineCap.ptr());

    return true;
}

bool Canvas2DContext::JSSetter_lineJoin(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS::RootedString vpStr(cx, JS::ToString(cx, vp));
    JSAutoByteString lineJoin(cx, vpStr);
    m_Skia->setLineJoin(lineJoin.ptr());

    return true;
}

bool Canvas2DContext::JSGetter_width(JSContext *cx,
    JS::MutableHandleValue vp)
{
    vp.setInt32(m_Skia->getWidth());

    return true;
}

bool Canvas2DContext::JSGetter_height(JSContext *cx,
    JS::MutableHandleValue vp)
{
    vp.setInt32(m_Skia->getHeight());

    return true;
}

bool Canvas2DContext::JSGetter_fillStyle(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS::RootedValue ret(cx);
    uint32_t curColor;

    ret      = getCurrentState()->m_CurrentShader;
    curColor = m_Skia->getFillColor();

    if (ret.isUndefined()) {
        char rgba_str[64];

        SkiaContext::GetStringColor(curColor, rgba_str);

        vp.setString(JS_NewStringCopyZ(cx, rgba_str));
    } else {
        vp.set(ret);
    }

    return true;
}

bool Canvas2DContext::JSGetter_strokeStyle(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS::RootedValue ret(cx);
    uint32_t curColor;

    ret      = getCurrentState()->m_CurrentStrokeShader;
    curColor = m_Skia->getStrokeColor();

    if (ret.isUndefined()) {
        char rgba_str[64];

        SkiaContext::GetStringColor(curColor, rgba_str);

        vp.setString(JS_NewStringCopyZ(cx, rgba_str));
    } else {
        vp.set(ret);
    }

    return true;
}

bool Canvas2DContext::JSGetter_lineWidth(JSContext *cx,
    JS::MutableHandleValue vp)
{
    vp.setDouble(m_Skia->getLineWidth());

    return true;
}

bool Canvas2DContext::JSGetter_miterLimit(JSContext *cx,
    JS::MutableHandleValue vp)
{
    vp.setDouble(m_Skia->getMiterLimit());

    return true;
}

bool Canvas2DContext::JSGetter_globalAlpha(JSContext *cx,
    JS::MutableHandleValue vp)
{
    vp.setDouble(m_Skia->getGlobalAlpha());

    return true;
}

bool Canvas2DContext::JSGetter_imageSmoothingEnabled(JSContext *cx,
    JS::MutableHandleValue vp)
{
    vp.setBoolean(!!m_Skia->getSmooth());

    return true;
}

bool Canvas2DContext::JSGetter_shadowOffsetX(JSContext *cx,
    JS::MutableHandleValue vp)
{
    vp.setDouble(m_Skia->getShadowOffsetX());

    return true;
}

bool Canvas2DContext::JSGetter_shadowOffsetY(JSContext *cx,
    JS::MutableHandleValue vp)
{
    vp.setDouble(m_Skia->getShadowOffsetY());

    return true;
}

bool Canvas2DContext::JSGetter_shadowBlur(JSContext *cx,
    JS::MutableHandleValue vp)
{
    vp.setDouble(m_Skia->getShadowBlur());

    return true;
}

bool Canvas2DContext::JSGetter_lineCap(JSContext *cx,
    JS::MutableHandleValue vp)
{
    vp.setString(JS_NewStringCopyZ(cx, m_Skia->getLineCap()));

    return true;
}

bool Canvas2DContext::JSGetter_lineJoin(JSContext *cx,
    JS::MutableHandleValue vp)
{
    vp.setString(JS_NewStringCopyZ(cx, m_Skia->getLineJoin()));

    return true;
}

bool Canvas2DContext::JSGetter_shadowColor(JSContext *cx,
    JS::MutableHandleValue vp)
{
    char rgba_str[64];

    SkiaContext::GetStringColor(m_Skia->getShadowColor(), rgba_str);

    vp.setString(JS_NewStringCopyZ(cx, rgba_str));

    return true;
}

bool Canvas2DContext::JSGetter_globalCompositeOperation(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS_ReportWarning(cx, "Not implemented");
    vp.setUndefined();

    return true;
}

bool Canvas2DContext::JSGetter_fontSize(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS_ReportWarning(cx, "Not implemented");
    vp.setUndefined();

    return true;
}

bool Canvas2DContext::JSGetter_textAlign(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS_ReportWarning(cx, "Not implemented");
    vp.setUndefined();

    return true;
}

bool Canvas2DContext::JSGetter_textBaseline(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS_ReportWarning(cx, "Not implemented");
    vp.setUndefined();

    return true;
}

bool Canvas2DContext::JSGetter_fontFamily(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS_ReportWarning(cx, "Not implemented");
    vp.setUndefined();

    return true;
}

bool Canvas2DContext::JSGetter_fontStyle(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS_ReportWarning(cx, "Not implemented");
    vp.setUndefined();

    return true;
}

bool Canvas2DContext::JSGetter_fontSkew(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS_ReportWarning(cx, "Not implemented");
    vp.setUndefined();

    return true;
}

bool Canvas2DContext::JSGetter_fontFile(JSContext *cx,
    JS::MutableHandleValue vp)
{
    JS_ReportWarning(cx, "Not implemented");
    vp.setUndefined();

    return true;
}

void Canvas2DContext::jsTrace(class JSTracer *trc)
{
    for (Canvas2DContextState *state = this->getCurrentState();
         state != NULL; state = state->m_Next) {

        /* Does this matter if we trace an UndefinedValue? */
        JS_CallValueTracer(trc, &state->m_CurrentShader,
                               "Canvas2DContextShader");
        JS_CallValueTracer(trc, &state->m_CurrentStrokeShader,
                               "Canvas2DContextShader");
    }
}

void Canvas2DContext::clear(uint32_t color)
{
    m_Skia->getCanvas()->clear(color);
}

char *Canvas2DContext::genModifiedFragmentShader(const char *data, const char *glslversion)
{
    const char *prologue =
        "vec4 _nm_gl_FragCoord;\n"
        "void main(void);\n"
        "#define main _nm_main\n"
        "#define gl_FragCoord _nm_gl_FragCoord\n";

    char *ret;

    asprintf(&ret, "%s\n%s%s", glslversion, prologue, data);

    return ret;
}

uint32_t Canvas2DContext::createProgram(const char *data)
{
    char *pdata = CanvasContext::ProcessShader(data,
        CanvasContext::SHADER_FRAGMENT, SH_GLSL_COMPATIBILITY_OUTPUT);

    if (!pdata) {
        return 0;
    }

    std::string tshader = pdata;
    free(pdata);

    int fline = tshader.find("\n");
    if (!fline) {
        return 0;
    }

    std::string glslversion = tshader.substr(0, fline);

    tshader.erase(0, fline + 1);

    char *nshader = this->genModifiedFragmentShader(tshader.c_str(), glslversion.c_str());
    uint32_t fragment = CanvasContext::CompileShader((const char **)&nshader, 1, GL_FRAGMENT_SHADER);

    if (fragment == 0) {
        free(nshader);
        return 0;
    }

    uint32_t coop   = this->compileCoopFragmentShader(glslversion.c_str());
    uint32_t vertex = this->CreatePassThroughVertex();

    free(nshader);

    GLuint programHandle;

    GLContext *iface = m_GLState->getNidiumGLContext();

    NIDIUM_GL_CALL_RET(iface, CreateProgram(), programHandle);

    GLint linkSuccess;

    NIDIUM_GL_CALL(iface, AttachShader(programHandle, vertex));
    NIDIUM_GL_CALL(iface, AttachShader(programHandle, coop));
    NIDIUM_GL_CALL(iface, AttachShader(programHandle, fragment));

    NIDIUM_GL_CALL(iface, BindAttribLocation(programHandle,
                                             CanvasContext::SH_ATTR_POSITION,
                                             "Position"));

    NIDIUM_GL_CALL(iface, BindAttribLocation(programHandle,
                                             CanvasContext::SH_ATTR_TEXCOORD,
                                             "TexCoordIn"));

    NIDIUM_GL_CALL(iface, LinkProgram(programHandle));

    NIDIUM_GL_CALL(
        iface, GetProgramiv(programHandle, GR_GL_LINK_STATUS, &linkSuccess));
    if (linkSuccess == GL_FALSE) {
        GLchar messages[1024];
        NIDIUM_GL_CALL(iface, GetProgramInfoLog(programHandle, sizeof(messages),
                                                0, &messages[0]));
        ndm_logf(NDM_LOG_ERROR, "JSCanvas2DContext", "createProgram error : %s", messages);
        return 0;
    }

    return programHandle;
}

static const char *getCoopFragmentShader()
{
    return
        "uniform sampler2D Texture;\n"
        "uniform vec2 n_Position;\n"
        "uniform vec2 n_Resolution;\n"
        "uniform float u_opacity;\n"
        "uniform float n_Padding;\n"
        "varying vec2 TexCoordOut;\n"
        "vec4 _nm_gl_FragCoord;\n"
        "void _nm_main(void);\n"

        "void main(void) {\n"
        "_nm_gl_FragCoord = vec4(gl_FragCoord.x-n_Position.x-n_Padding, "
        "gl_FragCoord.y-n_Position.y-n_Padding, gl_FragCoord.wz);\n"

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
}

uint32_t Canvas2DContext::compileCoopFragmentShader(const char *glslversion)
{
    const char *shader = getCoopFragmentShader();

    return this->CompileShader(&shader, 1, GL_FRAGMENT_SHADER);
}

void Canvas2DContext::drawTexture(uint32_t textureID)
{
    NIDIUM_GL_CALL_MAIN(BindTexture(GR_GL_TEXTURE_2D, textureID));

    NIDIUM_GL_CALL_MAIN(
        TexParameteri(GR_GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
    NIDIUM_GL_CALL_MAIN(
        TexParameteri(GR_GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));

    /* Anti Aliasing */
    NIDIUM_GL_CALL_MAIN(
        TexParameteri(GR_GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    NIDIUM_GL_CALL_MAIN(
        TexParameteri(GR_GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    NIDIUM_GL_CALL_MAIN(DrawElements(GR_GL_TRIANGLE_STRIP,
                                     m_GLState->m_GLObjects.vtx->nindices,
                                     GL_UNSIGNED_INT, 0));

    NIDIUM_GL_CALL_MAIN(BindTexture(GR_GL_TEXTURE_2D, 0));

    /* Unbind vertex array bound by resetGLContext() */
    NIDIUM_GL_CALL_MAIN(BindVertexArray(0));
}

uint32_t Canvas2DContext::attachShader(const char *string)
{
    uint32_t program = this->createProgram(string);

    if (program) {
        UIInterface *ui = m_GLState->getNidiumGLContext()->getUI();
        /* Destroy the old context (if it's not shared) */
        m_GLState->destroy();
        /* Create a new state without program */
        GLState *nstate = new GLState(ui, false);
        nstate->setShared(false);

        m_GLState = nstate;

        m_GLState->setProgram(program);

        GLContext *iface = m_GLState->getNidiumGLContext();

        NIDIUM_GL_CALL_RET(iface, GetUniformLocation(program, "n_Resolution"),
                           m_GLState->m_GLObjects.uniforms.u_resolution);

        NIDIUM_GL_CALL_RET(iface, GetUniformLocation(program, "n_Position"),
                           m_GLState->m_GLObjects.uniforms.u_position);

        NIDIUM_GL_CALL_RET(iface, GetUniformLocation(program, "n_Padding"),
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

void Canvas2DContext::setVertexDeformation(uint32_t vertex, float x, float y)
{
    GLState *state = m_GLState;

    /*
        If the GL state is shared among other Canvas, create a new one
    */
    if (state->isShared()) {
        ndm_logf(NDM_LOG_INFO, "JScanvas2DContext", "New GL state created !");
        state = new GLState(m_GLState->getNidiumGLContext()->getUI());
        state->setShared(false);

        m_GLState = state;
    }

    state->setVertexDeformation(vertex, x, y);
}

uint32_t Canvas2DContext::getTextureID() const
{
    return m_Skia->getOpenGLTextureId();
}

void Canvas2DContext::flush()
{
    m_Skia->getCanvas()->flush();
}

void Canvas2DContext::getSize(int *width, int *height) const
{
    SkISize size = m_Skia->getCanvas()->getDeviceSize();

    *width  = size.width();
    *height = size.height();
}

void Canvas2DContext::setSize(float width, float height, bool redraw)
{
    m_Skia->setSize(width, height, redraw);
}

void Canvas2DContext::translate(double x, double y)
{
    m_Skia->getCanvas()->translate(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

Canvas2DContext::Canvas2DContext(CanvasHandler *handler,
                                 JSContext *cx,
                                 int width,
                                 int height,
                                 UIInterface *ui)
    : CanvasContext(handler), m_SetterDisabled(false), m_CurrentState(NULL)
{
    m_Mode = CONTEXT_2D;

    this->pushNewState();

    width = nidium_max(width, 1);
    height = nidium_max(height, 1);

    m_Skia = SkiaContext::CreateWithTextureBackend(ui->getNidiumContext(), width, height);

    /* Vertex buffers were unbound by parent constructor */
    m_Skia->resetGrBackendContext(kVertex_GrGLBackendState);
}

Canvas2DContext::Canvas2DContext(
    CanvasHandler *handler, int width, int height, UIInterface *ui, bool isGL)
    : CanvasContext(handler), m_SetterDisabled(false)
{
    m_Mode = CONTEXT_2D;

    width = nidium_max(width, 1);
    height = nidium_max(height, 1);

    if (isGL) {
        m_Skia = SkiaContext::CreateWithFBOBackend(ui->getNidiumContext(), width, height);
    } else {
        if (!ui) {
            ui = Interface::__NidiumUI;
        }
        m_Skia = SkiaContext::CreateWithTextureBackend(ui->getNidiumContext(), width, height);
    }

    /* Vertex buffers were unbound by parent constructor */
    m_Skia->resetGrBackendContext(kVertex_GrGLBackendState);
}

void Canvas2DContext::setScale(double x, double y, double px, double py)
{
    m_Skia->scale(1. / px, 1. / py);

    m_Skia->scale(x, y);
}

uint8_t *Canvas2DContext::getPixels()
{
    this->flush();
    ndm_log(NDM_LOG_INFO, "JSCanvas2DContext", "Get Pixel unimplemented");

    return nullptr;
#if 0

    return (uint8_t *)m_Skia->getCanvas()
        ->getDevice()
        ->accessBitmap(false)
        .getPixels();
#endif
}

Canvas2DContext::~Canvas2DContext()
{
    if (m_Skia) {
        delete m_Skia;
    }
}
// }}}

// {{{ JSGradient
bool JSGradient::JS_addColorStop(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString color(cx);
    double position;

    NIDIUM_LOG_2D_CALL();
    if (!JS_ConvertArguments(cx, args, "dS", &position, color.address())) {
        return false;
    }

    JSAutoByteString colorstr(cx, color);

    m_Gradient->addColorStop(position, colorstr.ptr());

    return true;
}
// }}}

// {{{ Registration
JSPropertySpec *Canvas2DContext::ListProperties()
{
    static JSPropertySpec props[] = {
        CLASSMAPPER_PROP_GS(Canvas2DContext, fillStyle),
        CLASSMAPPER_PROP_GS(Canvas2DContext, strokeStyle),
        CLASSMAPPER_PROP_GS(Canvas2DContext, lineWidth),
        CLASSMAPPER_PROP_GS(Canvas2DContext, miterLimit),
        CLASSMAPPER_PROP_GS(Canvas2DContext, globalAlpha),
        CLASSMAPPER_PROP_GS(Canvas2DContext, globalCompositeOperation),
        CLASSMAPPER_PROP_GS(Canvas2DContext, fontSize),
        CLASSMAPPER_PROP_GS(Canvas2DContext, textAlign),
        CLASSMAPPER_PROP_GS(Canvas2DContext, textBaseline),
        CLASSMAPPER_PROP_GS(Canvas2DContext, fontFamily),
        CLASSMAPPER_PROP_GS(Canvas2DContext, fontStyle),
        CLASSMAPPER_PROP_GS(Canvas2DContext, fontSkew),
        CLASSMAPPER_PROP_GS(Canvas2DContext, fontFile),
        CLASSMAPPER_PROP_GS(Canvas2DContext, lineCap),
        CLASSMAPPER_PROP_GS(Canvas2DContext, lineJoin),
        CLASSMAPPER_PROP_GS(Canvas2DContext, shadowOffsetX),
        CLASSMAPPER_PROP_GS(Canvas2DContext, shadowOffsetY),
        CLASSMAPPER_PROP_GS(Canvas2DContext, shadowBlur),
        CLASSMAPPER_PROP_GS(Canvas2DContext, shadowColor),
        CLASSMAPPER_PROP_GS(Canvas2DContext, imageSmoothingEnabled),
        CLASSMAPPER_PROP_G(Canvas2DContext, width),
        CLASSMAPPER_PROP_G(Canvas2DContext, height),
        JS_PS_END
    };

    return props;
}

JSFunctionSpec *Canvas2DContext::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(Canvas2DContext, breakText, 2),
        CLASSMAPPER_FN(Canvas2DContext, shadow, 0),
        CLASSMAPPER_FN(Canvas2DContext, onerror, 0),
        CLASSMAPPER_FN(Canvas2DContext, fillRect, 4),
        CLASSMAPPER_FN(Canvas2DContext, fillText, 3),
        CLASSMAPPER_FN(Canvas2DContext, strokeText, 3),
        CLASSMAPPER_FN(Canvas2DContext, strokeRect, 4),
        CLASSMAPPER_FN(Canvas2DContext, clearRect, 4),
        CLASSMAPPER_FN(Canvas2DContext, beginPath, 0),
        CLASSMAPPER_FN(Canvas2DContext, moveTo, 2),
        CLASSMAPPER_FN(Canvas2DContext, lineTo, 2),
        CLASSMAPPER_FN(Canvas2DContext, fill, 0),
        CLASSMAPPER_FN(Canvas2DContext, stroke, 0),
        CLASSMAPPER_FN(Canvas2DContext, closePath, 0),
        CLASSMAPPER_FN(Canvas2DContext, clip, 0),
        CLASSMAPPER_FN(Canvas2DContext, arc, 5),
        CLASSMAPPER_FN(Canvas2DContext, arcTo, 5),
        CLASSMAPPER_FN(Canvas2DContext, rect, 4),
        CLASSMAPPER_FN(Canvas2DContext, quadraticCurveTo, 4),
        CLASSMAPPER_FN(Canvas2DContext, bezierCurveTo, 6),
        CLASSMAPPER_FN(Canvas2DContext, rotate, 1),
        CLASSMAPPER_FN(Canvas2DContext, scale, 2),
        CLASSMAPPER_FN(Canvas2DContext, save, 0),
        CLASSMAPPER_FN(Canvas2DContext, restore, 0),
        CLASSMAPPER_FN(Canvas2DContext, translate, 2),
        CLASSMAPPER_FN(Canvas2DContext, transform, 6),
        CLASSMAPPER_FN(Canvas2DContext, iTransform, 6),
        CLASSMAPPER_FN(Canvas2DContext, setTransform, 6),
        CLASSMAPPER_FN(Canvas2DContext, createLinearGradient, 4),
        CLASSMAPPER_FN(Canvas2DContext, createRadialGradient, 6),
        CLASSMAPPER_FN(Canvas2DContext, createImageData, 2),
        CLASSMAPPER_FN(Canvas2DContext, createPattern, 2),
        CLASSMAPPER_FN(Canvas2DContext, putImageData, 3),
        CLASSMAPPER_FN(Canvas2DContext, getImageData, 4),
        CLASSMAPPER_FN(Canvas2DContext, drawImage, 3),
        CLASSMAPPER_FN(Canvas2DContext, measureText, 1),
        CLASSMAPPER_FN(Canvas2DContext, isPointInPath, 2),
        CLASSMAPPER_FN(Canvas2DContext, getPathBounds, 0),
        CLASSMAPPER_FN(Canvas2DContext, attachFragmentShader, 1),
        CLASSMAPPER_FN(Canvas2DContext, detachFragmentShader, 0),
        CLASSMAPPER_FN(Canvas2DContext, setVertexOffset, 3),
        JS_FS_END
    };

    return funcs;
}

void Canvas2DContext::RegisterObject(JSContext *cx)
{
    Canvas2DContext::ExposeClass(cx, "CanvasRenderingContext2D",
        0, kJSTracer_ExposeFlag);

    JSGradient::ExposeClass(cx, "CanvasGradient");

    JSCanvasGLProgram::ExposeClass(cx, "CanvasGLProgram");

    JSCanvasPattern::ExposeClass(cx, "CanvasPattern",
      JSCLASS_HAS_RESERVED_SLOTS(1));
}
// }}}

} // namespace Binding
} // namespace Nidium
