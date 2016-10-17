/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSWebGL.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "SystemInterface.h"

#include "Graphics/Image.h"
#include "Graphics/SkiaContext.h"
#include "Graphics/Canvas3DContext.h"
#include "Binding/JSCanvas.h"
#include "Binding/JSCanvas2DContext.h"
#include "Macros.h"

using Nidium::Frontend::Context;
using Nidium::Interface::SystemInterface;
using Nidium::Graphics::Image;
using Nidium::Graphics::GLContext;
using Nidium::Graphics::CanvasContext;
using Nidium::Graphics::Canvas3DContext;
using Nidium::Graphics::CanvasHandler;

namespace Nidium {
namespace Binding {

// {{{ Helper Macros
#define GL_CALL(IFACE, FN)                                         \
    /*printf(#FN "\n");*/                                          \
    NIDIUM_GL_CALL((IFACE)->getGLContext(), FN);                   \
    {                                                              \
        GLint err = glGetError();                                  \
        if (err != 0) NUI_LOG("err = %d / call = %s\n", err, #FN); \
    }

#define GL_CALL_RET(IFACE, FN, RET)                                \
    /*printf(#FN "\n");*/                                          \
    NIDIUM_GL_CALL_RET((IFACE)->getGLContext(), FN, RET);          \
    {                                                              \
        GLint err = glGetError();                                  \
        if (err != 0) NUI_LOG("err = %d / call = %s\n", err, #FN); \
    }

#define NGL_JS_FN_DELETE_X(FUNC_NAME, RESOURCE_CLASS)                         \
    bool JSWebGLRenderingContext::JS_##FUNC_NAME(JSContext *cx,               \
                                                 JS::CallArgs &args)          \
    {                                                                         \
        JS::RootedObject obj(cx);                                             \
        if (!args[0].isObject()) {                                            \
            JS_ReportError(cx, "Invalid argument");                           \
            return false;                                                     \
        }                                                                     \
        JSWebGL##RESOURCE_CLASS *cppObj                                       \
            = JSWebGL##RESOURCE_CLASS::GetInstance(args[0].toObjectOrNull()); \
        if (cppObj) {                                                         \
            cppObj->unbind();                                                 \
            return true;                                                      \
        } else {                                                              \
            JS_ReportError(cx, "Invalid argument");                           \
            return false;                                                     \
        }                                                                     \
    }
/// }}}

// {{{ WebGLRenderingContext Methods & Constants definition
// clang-format off
JSFunctionSpec *JSWebGLRenderingContext::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSWebGLRenderingContext, isContextLost, 0),
        CLASSMAPPER_FN(JSWebGLRenderingContext, getExtension, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, activeTexture, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, attachShader, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, bindAttribLocation, 3),
        CLASSMAPPER_FN(JSWebGLRenderingContext, bindBuffer, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, bindFramebuffer, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, bindRenderbuffer, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, bindTexture, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, copyTexImage2D, 8),
        CLASSMAPPER_FN(JSWebGLRenderingContext, copyTexSubImage2D, 8),
        CLASSMAPPER_FN(JSWebGLRenderingContext, blendEquation, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, blendEquationSeparate, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, blendFunc, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, blendFuncSeparate, 4),
        CLASSMAPPER_FN(JSWebGLRenderingContext, bufferData, 3),
        CLASSMAPPER_FN(JSWebGLRenderingContext, bufferSubData, 3),
        CLASSMAPPER_FN(JSWebGLRenderingContext, clear, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, clearColor, 4),
        CLASSMAPPER_FN(JSWebGLRenderingContext, clearDepth, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, clearStencil, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, colorMask, 0),
        CLASSMAPPER_FN(JSWebGLRenderingContext, compileShader, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, texImage2D, 6),
        CLASSMAPPER_FN(JSWebGLRenderingContext, createBuffer, 0),
        CLASSMAPPER_FN(JSWebGLRenderingContext, createFramebuffer, 0),
        CLASSMAPPER_FN(JSWebGLRenderingContext, createRenderbuffer, 0),
        CLASSMAPPER_FN(JSWebGLRenderingContext, createProgram, 0),
        CLASSMAPPER_FN(JSWebGLRenderingContext, createShader, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, createTexture, 0),
        CLASSMAPPER_FN(JSWebGLRenderingContext, cullFace, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, deleteBuffer, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, deleteFramebuffer, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, deleteProgram, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, deleteRenderbuffer, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, deleteShader, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, deleteTexture, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, depthFunc, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, depthMask, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, depthRange, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, detachShader, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, disable, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, disableVertexAttribArray, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, drawArrays, 3),
        CLASSMAPPER_FN(JSWebGLRenderingContext, drawElements, 4),
        CLASSMAPPER_FN(JSWebGLRenderingContext, enable, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, enableVertexAttribArray, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, finish, 0),
        CLASSMAPPER_FN(JSWebGLRenderingContext, flush, 0),
        CLASSMAPPER_FN(JSWebGLRenderingContext, framebufferRenderbuffer, 4),
        CLASSMAPPER_FN(JSWebGLRenderingContext, framebufferTexture2D, 5),
        CLASSMAPPER_FN(JSWebGLRenderingContext, frontFace, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, generateMipmap, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, getActiveAttrib, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, getActiveUniform, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, getAttribLocation, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, getParameter, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, getProgramParameter, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, getProgramInfoLog, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, getShaderParameter, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, getShaderInfoLog, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, getUniformLocation, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, getShaderPrecisionFormat, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, lineWidth, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, linkProgram, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, pixelStorei, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, renderbufferStorage, 4),
        CLASSMAPPER_FN(JSWebGLRenderingContext, scissor, 4),
        CLASSMAPPER_FN(JSWebGLRenderingContext, shaderSource, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, texParameteri, 3),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniform1f, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniform1fv, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniform1i, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniform1iv, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniform2f, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniform2fv, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniform2i, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniform2iv, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniform3f, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniform3fv, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniform3i, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniform3iv, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniform4f, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniform4fv, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniform4i, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniform4iv, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniformMatrix2fv, 3),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniformMatrix3fv, 3),
        CLASSMAPPER_FN(JSWebGLRenderingContext, uniformMatrix4fv, 3),
        CLASSMAPPER_FN(JSWebGLRenderingContext, vertexAttrib1f, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, vertexAttrib1fv, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, vertexAttrib2f, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, vertexAttrib2fv, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, vertexAttrib3f, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, vertexAttrib3fv, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, vertexAttrib4f, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, vertexAttrib4fv, 2),
        CLASSMAPPER_FN(JSWebGLRenderingContext, vertexAttribPointer, 6),
        CLASSMAPPER_FN(JSWebGLRenderingContext, viewport, 4),
        CLASSMAPPER_FN(JSWebGLRenderingContext, useProgram, 1),
        CLASSMAPPER_FN(JSWebGLRenderingContext, getError, 0),
        CLASSMAPPER_FN(JSWebGLRenderingContext, swapBuffer, 0),
        JS_FS_END
    };

    return funcs;
}

JSConstDoubleSpec *JSWebGLRenderingContext::ListConstDoubles()
{
    static JSConstDoubleSpec constDoubles[] = {
        //{NGL_ES_VERSION_2_0, "ES_VERSION_2_0", 0, {0, 0, 0}},
        { NGL_DEPTH_BUFFER_BIT, "DEPTH_BUFFER_BIT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_BUFFER_BIT, "STENCIL_BUFFER_BIT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_COLOR_BUFFER_BIT, "COLOR_BUFFER_BIT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_POINTS, "POINTS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_LINES, "LINES", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_LINE_LOOP, "LINE_LOOP", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_LINE_STRIP, "LINE_STRIP", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TRIANGLES, "TRIANGLES", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TRIANGLE_STRIP, "TRIANGLE_STRIP", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TRIANGLE_FAN, "TRIANGLE_FAN", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ZERO, "ZERO", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ONE, "ONE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_SRC_COLOR, "SRC_COLOR", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ONE_MINUS_SRC_COLOR, "ONE_MINUS_SRC_COLOR", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_SRC_ALPHA, "SRC_ALPHA", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ONE_MINUS_SRC_ALPHA, "ONE_MINUS_SRC_ALPHA", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DST_ALPHA, "DST_ALPHA", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ONE_MINUS_DST_ALPHA, "ONE_MINUS_DST_ALPHA", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DST_COLOR, "DST_COLOR", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ONE_MINUS_DST_COLOR, "ONE_MINUS_DST_COLOR", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_SRC_ALPHA_SATURATE, "SRC_ALPHA_SATURATE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FUNC_ADD, "FUNC_ADD", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BLEND_EQUATION, "BLEND_EQUATION", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BLEND_EQUATION_RGB, "BLEND_EQUATION_RGB", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BLEND_EQUATION_ALPHA, "BLEND_EQUATION_ALPHA", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FUNC_SUBTRACT, "FUNC_SUBTRACT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FUNC_REVERSE_SUBTRACT, "FUNC_REVERSE_SUBTRACT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BLEND_DST_RGB, "BLEND_DST_RGB", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BLEND_SRC_RGB, "BLEND_SRC_RGB", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BLEND_DST_ALPHA, "BLEND_DST_ALPHA", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BLEND_SRC_ALPHA, "BLEND_SRC_ALPHA", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_CONSTANT_COLOR, "CONSTANT_COLOR", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ONE_MINUS_CONSTANT_COLOR, "ONE_MINUS_CONSTANT_COLOR", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_CONSTANT_ALPHA, "CONSTANT_ALPHA", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ONE_MINUS_CONSTANT_ALPHA, "ONE_MINUS_CONSTANT_ALPHA", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BLEND_COLOR, "BLEND_COLOR", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ARRAY_BUFFER, "ARRAY_BUFFER", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ELEMENT_ARRAY_BUFFER, "ELEMENT_ARRAY_BUFFER", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ARRAY_BUFFER_BINDING, "ARRAY_BUFFER_BINDING", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ELEMENT_ARRAY_BUFFER_BINDING, "ELEMENT_ARRAY_BUFFER_BINDING", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STREAM_DRAW, "STREAM_DRAW", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STATIC_DRAW, "STATIC_DRAW", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DYNAMIC_DRAW, "DYNAMIC_DRAW", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BUFFER_SIZE, "BUFFER_SIZE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BUFFER_USAGE, "BUFFER_USAGE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_CURRENT_VERTEX_ATTRIB, "CURRENT_VERTEX_ATTRIB", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FRONT, "FRONT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BACK, "BACK", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FRONT_AND_BACK, "FRONT_AND_BACK", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE_2D, "TEXTURE_2D", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_CULL_FACE, "CULL_FACE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BLEND, "BLEND", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DITHER, "DITHER", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_TEST, "STENCIL_TEST", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DEPTH_TEST, "DEPTH_TEST", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_SCISSOR_TEST, "SCISSOR_TEST", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_POLYGON_OFFSET_FILL, "POLYGON_OFFSET_FILL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_SAMPLE_ALPHA_TO_COVERAGE, "SAMPLE_ALPHA_TO_COVERAGE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_SAMPLE_COVERAGE, "SAMPLE_COVERAGE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_NO_ERROR, "NO_ERROR", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_INVALID_ENUM, "INVALID_ENUM", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_INVALID_VALUE, "INVALID_VALUE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_INVALID_OPERATION, "INVALID_OPERATION", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_OUT_OF_MEMORY, "OUT_OF_MEMORY", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_CW, "CW", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_CCW, "CCW", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_LINE_WIDTH, "LINE_WIDTH", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ALIASED_POINT_SIZE_RANGE, "ALIASED_POINT_SIZE_RANGE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ALIASED_LINE_WIDTH_RANGE, "ALIASED_LINE_WIDTH_RANGE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_CULL_FACE_MODE, "CULL_FACE_MODE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FRONT_FACE, "FRONT_FACE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DEPTH_RANGE, "DEPTH_RANGE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DEPTH_WRITEMASK, "DEPTH_WRITEMASK", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DEPTH_CLEAR_VALUE, "DEPTH_CLEAR_VALUE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DEPTH_FUNC, "DEPTH_FUNC", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_CLEAR_VALUE, "STENCIL_CLEAR_VALUE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_FUNC, "STENCIL_FUNC", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_FAIL, "STENCIL_FAIL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_PASS_DEPTH_FAIL, "STENCIL_PASS_DEPTH_FAIL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_PASS_DEPTH_PASS, "STENCIL_PASS_DEPTH_PASS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_REF, "STENCIL_REF", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_VALUE_MASK, "STENCIL_VALUE_MASK", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_WRITEMASK, "STENCIL_WRITEMASK", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_BACK_FUNC, "STENCIL_BACK_FUNC", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_BACK_FAIL, "STENCIL_BACK_FAIL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_BACK_PASS_DEPTH_FAIL, "STENCIL_BACK_PASS_DEPTH_FAIL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_BACK_PASS_DEPTH_PASS, "STENCIL_BACK_PASS_DEPTH_PASS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_BACK_REF, "STENCIL_BACK_REF", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_BACK_VALUE_MASK, "STENCIL_BACK_VALUE_MASK", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_BACK_WRITEMASK, "STENCIL_BACK_WRITEMASK", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_VIEWPORT, "VIEWPORT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_SCISSOR_BOX, "SCISSOR_BOX", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_COLOR_CLEAR_VALUE, "COLOR_CLEAR_VALUE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_COLOR_WRITEMASK, "COLOR_WRITEMASK", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_UNPACK_ALIGNMENT, "UNPACK_ALIGNMENT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_PACK_ALIGNMENT, "PACK_ALIGNMENT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_MAX_TEXTURE_SIZE, "MAX_TEXTURE_SIZE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_MAX_VIEWPORT_DIMS, "MAX_VIEWPORT_DIMS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_SUBPIXEL_BITS, "SUBPIXEL_BITS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RED_BITS, "RED_BITS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_GREEN_BITS, "GREEN_BITS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BLUE_BITS, "BLUE_BITS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ALPHA_BITS, "ALPHA_BITS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DEPTH_BITS, "DEPTH_BITS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_BITS, "STENCIL_BITS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_POLYGON_OFFSET_UNITS, "POLYGON_OFFSET_UNITS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_POLYGON_OFFSET_FACTOR, "POLYGON_OFFSET_FACTOR", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE_BINDING_2D, "TEXTURE_BINDING_2D", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_SAMPLE_BUFFERS, "SAMPLE_BUFFERS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_SAMPLES, "SAMPLES", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_SAMPLE_COVERAGE_VALUE, "SAMPLE_COVERAGE_VALUE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_SAMPLE_COVERAGE_INVERT, "SAMPLE_COVERAGE_INVERT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        //{NGL_NUM_COMPRESSED_TEXTURE_FORMATS, "NUM_COMPRESSED_TEXTURE_FORMATS", JSPROP_ENUMERATE, {0,0, 0}},
        { NGL_COMPRESSED_TEXTURE_FORMATS, "COMPRESSED_TEXTURE_FORMATS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DONT_CARE, "DONT_CARE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FASTEST, "FASTEST", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_NICEST, "NICEST", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_GENERATE_MIPMAP_HINT, "GENERATE_MIPMAP_HINT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BYTE, "BYTE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_UNSIGNED_BYTE, "UNSIGNED_BYTE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_SHORT, "SHORT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_UNSIGNED_SHORT, "UNSIGNED_SHORT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_INT, "INT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_UNSIGNED_INT, "UNSIGNED_INT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FLOAT, "FLOAT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        //{NGL_FIXED, "FIXED", JSPROP_ENUMERATE, {0, 0, 0}},
        { NGL_DEPTH_COMPONENT, "DEPTH_COMPONENT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ALPHA, "ALPHA", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RGB, "RGB", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RGBA, "RGBA", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_LUMINANCE, "LUMINANCE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_LUMINANCE_ALPHA, "LUMINANCE_ALPHA", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_UNSIGNED_SHORT_4_4_4_4, "UNSIGNED_SHORT_4_4_4_4", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_UNSIGNED_SHORT_5_5_5_1, "UNSIGNED_SHORT_5_5_5_1", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_UNSIGNED_SHORT_5_6_5, "UNSIGNED_SHORT_5_6_5", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FRAGMENT_SHADER, "FRAGMENT_SHADER", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_VERTEX_SHADER, "VERTEX_SHADER", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_MAX_VERTEX_ATTRIBS, "MAX_VERTEX_ATTRIBS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_MAX_VERTEX_UNIFORM_VECTORS, "MAX_VERTEX_UNIFORM_VECTORS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_MAX_VARYING_VECTORS, "MAX_VARYING_VECTORS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, "MAX_COMBINED_TEXTURE_IMAGE_UNITS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, "MAX_VERTEX_TEXTURE_IMAGE_UNITS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_MAX_TEXTURE_IMAGE_UNITS, "MAX_TEXTURE_IMAGE_UNITS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_MAX_FRAGMENT_UNIFORM_VECTORS, "MAX_FRAGMENT_UNIFORM_VECTORS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_SHADER_TYPE, "SHADER_TYPE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DELETE_STATUS, "DELETE_STATUS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_LINK_STATUS, "LINK_STATUS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_VALIDATE_STATUS, "VALIDATE_STATUS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ATTACHED_SHADERS, "ATTACHED_SHADERS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ACTIVE_UNIFORMS, "ACTIVE_UNIFORMS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        //{NGL_ACTIVE_UNIFORM_MAX_LENGTH, "ACTIVE_UNIFORM_MAX_LENGTH",
        // JSPROP_ENUMERATE, {0, 0, 0}},
        { NGL_ACTIVE_ATTRIBUTES, "ACTIVE_ATTRIBUTES", JSPROP_ENUMERATE, { 0, 0, 0 } },
        //{NGL_ACTIVE_ATTRIBUTE_MAX_LENGTH, "ACTIVE_ATTRIBUTE_MAX_LENGTH",
        // JSPROP_ENUMERATE, {0, 0, 0}},
        { NGL_SHADING_LANGUAGE_VERSION, "SHADING_LANGUAGE_VERSION", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_CURRENT_PROGRAM, "CURRENT_PROGRAM", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_NEVER, "NEVER", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_LESS, "LESS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_EQUAL, "EQUAL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_LEQUAL, "LEQUAL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_GREATER, "GREATER", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_NOTEQUAL, "NOTEQUAL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_GEQUAL, "GEQUAL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ALWAYS, "ALWAYS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_KEEP, "KEEP", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_REPLACE, "REPLACE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_INCR, "INCR", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DECR, "DECR", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_INVERT, "INVERT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_INCR_WRAP, "INCR_WRAP", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DECR_WRAP, "DECR_WRAP", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_VENDOR, "VENDOR", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RENDERER, "RENDERER", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_VERSION, "VERSION", JSPROP_ENUMERATE, { 0, 0, 0 } },
        //{NGL_EXTENSIONS, "EXTENSIONS", JSPROP_ENUMERATE, {0, 0, 0}},
        { NGL_NEAREST, "NEAREST", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_LINEAR, "LINEAR", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_NEAREST_MIPMAP_NEAREST, "NEAREST_MIPMAP_NEAREST", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_LINEAR_MIPMAP_NEAREST, "LINEAR_MIPMAP_NEAREST", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_NEAREST_MIPMAP_LINEAR, "NEAREST_MIPMAP_LINEAR", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_LINEAR_MIPMAP_LINEAR, "LINEAR_MIPMAP_LINEAR", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE_MAG_FILTER, "TEXTURE_MAG_FILTER", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE_MIN_FILTER, "TEXTURE_MIN_FILTER", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE_WRAP_S, "TEXTURE_WRAP_S", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE_WRAP_T, "TEXTURE_WRAP_T", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE, "TEXTURE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE_CUBE_MAP, "TEXTURE_CUBE_MAP", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE_BINDING_CUBE_MAP, "TEXTURE_BINDING_CUBE_MAP", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE_CUBE_MAP_POSITIVE_X, "TEXTURE_CUBE_MAP_POSITIVE_X", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE_CUBE_MAP_NEGATIVE_X, "TEXTURE_CUBE_MAP_NEGATIVE_X", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE_CUBE_MAP_POSITIVE_Y, "TEXTURE_CUBE_MAP_POSITIVE_Y", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "TEXTURE_CUBE_MAP_NEGATIVE_Y", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE_CUBE_MAP_POSITIVE_Z, "TEXTURE_CUBE_MAP_POSITIVE_Z", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "TEXTURE_CUBE_MAP_NEGATIVE_Z", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_MAX_CUBE_MAP_TEXTURE_SIZE, "MAX_CUBE_MAP_TEXTURE_SIZE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE0, "TEXTURE0", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE1, "TEXTURE1", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE2, "TEXTURE2", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE3, "TEXTURE3", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE4, "TEXTURE4", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE5, "TEXTURE5", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE6, "TEXTURE6", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE7, "TEXTURE7", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE8, "TEXTURE8", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE9, "TEXTURE9", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE10, "TEXTURE10", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE11, "TEXTURE11", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE12, "TEXTURE12", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE13, "TEXTURE13", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE14, "TEXTURE14", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE15, "TEXTURE15", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE16, "TEXTURE16", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE17, "TEXTURE17", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE18, "TEXTURE18", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE19, "TEXTURE19", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE20, "TEXTURE20", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE21, "TEXTURE21", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE22, "TEXTURE22", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE23, "TEXTURE23", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE24, "TEXTURE24", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE25, "TEXTURE25", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE26, "TEXTURE26", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE27, "TEXTURE27", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE28, "TEXTURE28", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE29, "TEXTURE29", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE30, "TEXTURE30", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_TEXTURE31, "TEXTURE31", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_ACTIVE_TEXTURE, "ACTIVE_TEXTURE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_REPEAT, "REPEAT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_CLAMP_TO_EDGE, "CLAMP_TO_EDGE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_MIRRORED_REPEAT, "MIRRORED_REPEAT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FLOAT_VEC2, "FLOAT_VEC2", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FLOAT_VEC3, "FLOAT_VEC3", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FLOAT_VEC4, "FLOAT_VEC4", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_INT_VEC2, "INT_VEC2", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_INT_VEC3, "INT_VEC3", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_INT_VEC4, "INT_VEC4", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BOOL, "BOOL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BOOL_VEC2, "BOOL_VEC2", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BOOL_VEC3, "BOOL_VEC3", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BOOL_VEC4, "BOOL_VEC4", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FLOAT_MAT2, "FLOAT_MAT2", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FLOAT_MAT3, "FLOAT_MAT3", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FLOAT_MAT4, "FLOAT_MAT4", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_SAMPLER_2D, "SAMPLER_2D", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_SAMPLER_CUBE, "SAMPLER_CUBE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_VERTEX_ATTRIB_ARRAY_ENABLED, "VERTEX_ATTRIB_ARRAY_ENABLED", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_VERTEX_ATTRIB_ARRAY_SIZE, "VERTEX_ATTRIB_ARRAY_SIZE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_VERTEX_ATTRIB_ARRAY_STRIDE, "VERTEX_ATTRIB_ARRAY_STRIDE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_VERTEX_ATTRIB_ARRAY_TYPE, "VERTEX_ATTRIB_ARRAY_TYPE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_VERTEX_ATTRIB_ARRAY_NORMALIZED, "VERTEX_ATTRIB_ARRAY_NORMALIZED", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_VERTEX_ATTRIB_ARRAY_POINTER, "VERTEX_ATTRIB_ARRAY_POINTER", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, "VERTEX_ATTRIB_ARRAY_BUFFER_BINDING", JSPROP_ENUMERATE, { 0, 0, 0 } },
        //{NGL_IMPLEMENTATION_COLOR_READ_TYPE, "IMPLEMENTATION_COLOR_READ_TYPE", JSPROP_ENUMERATE, {0, 0, 0}},
        //{NGL_IMPLEMENTATION_COLOR_READ_FORMAT, "IMPLEMENTATION_COLOR_READ_FORMAT", JSPROP_ENUMERATE, {0, 0, 0}},
        { NGL_COMPILE_STATUS, "COMPILE_STATUS", JSPROP_ENUMERATE, { 0, 0, 0 } },
        //{NGL_INFO_LOG_LENGTH, "INFO_LOG_LENGTH", JSPROP_ENUMERATE, {0, 0, 0}},
        //{NGL_SHADER_SOURCE_LENGTH, "SHADER_SOURCE_LENGTH", JSPROP_ENUMERATE, {0, 0, 0}},
        //{NGL_SHADER_COMPILER, "SHADER_COMPILER", JSPROP_ENUMERATE, {0, 0, 0}},
        { NGL_LOW_FLOAT, "LOW_FLOAT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_MEDIUM_FLOAT, "MEDIUM_FLOAT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_HIGH_FLOAT, "HIGH_FLOAT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_LOW_INT, "LOW_INT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_MEDIUM_INT, "MEDIUM_INT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_HIGH_INT, "HIGH_INT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FRAMEBUFFER, "FRAMEBUFFER", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RENDERBUFFER, "RENDERBUFFER", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RGBA4, "RGBA4", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RGB5_A1, "RGB5_A1", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RGB565, "RGB565", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DEPTH_COMPONENT16, "DEPTH_COMPONENT16", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_INDEX, "STENCIL_INDEX", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_INDEX8, "STENCIL_INDEX8", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DEPTH_STENCIL, "DEPTH_STENCIL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RENDERBUFFER_WIDTH, "RENDERBUFFER_WIDTH", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RENDERBUFFER_HEIGHT, "RENDERBUFFER_HEIGHT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RENDERBUFFER_INTERNAL_FORMAT, "RENDERBUFFER_INTERNAL_FORMAT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RENDERBUFFER_RED_SIZE, "RENDERBUFFER_RED_SIZE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RENDERBUFFER_GREEN_SIZE, "RENDERBUFFER_GREEN_SIZE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RENDERBUFFER_BLUE_SIZE, "RENDERBUFFER_BLUE_SIZE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RENDERBUFFER_ALPHA_SIZE, "RENDERBUFFER_ALPHA_SIZE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RENDERBUFFER_DEPTH_SIZE, "RENDERBUFFER_DEPTH_SIZE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RENDERBUFFER_STENCIL_SIZE, "RENDERBUFFER_STENCIL_SIZE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, "FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, "FRAMEBUFFER_ATTACHMENT_OBJECT_NAME", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, "FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE, "FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_COLOR_ATTACHMENT0, "COLOR_ATTACHMENT0", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DEPTH_ATTACHMENT, "DEPTH_ATTACHMENT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_STENCIL_ATTACHMENT, "STENCIL_ATTACHMENT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_DEPTH_STENCIL_ATTACHMENT, "DEPTH_STENCIL_ATTACHMENT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_NONE, "NONE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FRAMEBUFFER_COMPLETE, "FRAMEBUFFER_COMPLETE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT, "FRAMEBUFFER_INCOMPLETE_ATTACHMENT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT, "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT", JSPROP_ENUMERATE, { 0, 0, 0 } },
        //{NGL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT, "FRAMEBUFFER_INCOMPLETE_DIMENSIONS", JSPROP_ENUMERATE, {0, 0, 0}},
        { NGL_FRAMEBUFFER_UNSUPPORTED, "FRAMEBUFFER_UNSUPPORTED", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_FRAMEBUFFER_BINDING, "FRAMEBUFFER_BINDING", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_RENDERBUFFER_BINDING, "RENDERBUFFER_BINDING", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_MAX_RENDERBUFFER_SIZE, "MAX_RENDERBUFFER_SIZE", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_INVALID_FRAMEBUFFER_OPERATION, "INVALID_FRAMEBUFFER_OPERATION", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_UNPACK_FLIP_Y_WEBGL, "UNPACK_FLIP_Y_WEBGL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_UNPACK_PREMULTIPLY_ALPHA_WEBGL, "UNPACK_PREMULTIPLY_ALPHA_WEBGL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_CONTEXT_LOST_WEBGL, "CONTEXT_LOST_WEBGL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_UNPACK_COLORSPACE_CONVERSION_WEBGL, "UNPACK_COLORSPACE_CONVERSION_WEBGL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { NGL_BROWSER_DEFAULT_WEBGL, "BROWSER_DEFAULT_WEBGL", JSPROP_ENUMERATE, { 0, 0, 0 } },
        { 0, nullptr, 0, { 0, 0, 0 } }
    };

    return constDoubles;
}
// clang-format on
// }}}

// {{{ WebGLResource Implementation
WebGLResource::~WebGLResource()
{
    switch (m_Type) {
        case kProgram:
            GL_CALL(m_WebGLCtx, DeleteProgram(m_GlIdentifier));
            break;
        case kShader:
            GL_CALL(m_WebGLCtx, DeleteShader(m_GlIdentifier));
            break;
        case kTexture:
            GL_CALL(m_WebGLCtx, DeleteTextures(1, &m_GlIdentifier));
            break;
        case kBuffer:
            GL_CALL(m_WebGLCtx, DeleteBuffers(1, &m_GlIdentifier));
            break;
        case kVertexArray:
            GL_CALL(m_WebGLCtx, DeleteVertexArrays(1, &m_GlIdentifier));
            break;
        case kFramebuffer:
            GL_CALL(m_WebGLCtx, DeleteFramebuffers(1, &m_GlIdentifier));
            break;
        case kRenderbuffer:
            GL_CALL(m_WebGLCtx, DeleteRenderbuffers(1, &m_GlIdentifier));
            break;
        default:
            break;
    }
}

void WebGLResource::bind()
{
    JS_SetReservedSlot(m_WebGLCtx->getJSObject(), m_Type,
                       OBJECT_TO_JSVAL(this->getJSObject()));

    m_IsBound = true;
}

void WebGLResource::bindTo(GLenum target)
{
    JS::RootedObject webglObj(m_JSCx, m_WebGLCtx->getJSObject());
    JS::RootedValue slot(m_JSCx, JS_GetReservedSlot(webglObj, m_Type));
    if (slot.isUndefined()) slot.setNull();

    JS_SetReservedSlot(webglObj, m_Type, JS::NullValue());
    JS::RootedObject bindObject(m_JSCx, slot.toObjectOrNull());

    if (!bindObject) {
        bindObject
            = JS_NewObject(m_JSCx, nullptr, JS::NullPtr(), JS::NullPtr());
        JS_SetReservedSlot(webglObj, m_Type, OBJECT_TO_JSVAL(bindObject));
    }

    JS::RootedValue val(m_JSCx, OBJECT_TO_JSVAL(this->getJSObject()));

    char targetStr[16];
    snprintf(targetStr, 15, "%d", target);

    JS_SetProperty(m_JSCx, bindObject, targetStr, val);

    m_IsBound = true;
    m_Target  = target;
}

void WebGLResource::unbind()
{
    if (!m_IsBound) return;

    JS::RootedObject obj(m_JSCx, m_WebGLCtx->getJSObject());

    if (m_Target != NGL_NONE) {
        WebGLResource::UnbindFrom(m_JSCx, obj, m_Type, m_Target);
    } else {
        WebGLResource::Unbind(obj, m_Type);
    }
}

void WebGLResource::Unbind(JS::HandleObject JSGLCtx, ResourceType type)
{
    JS_SetReservedSlot(JSGLCtx, type, JS::NullHandleValue);
}

void WebGLResource::UnbindFrom(JSContext *cx,
                               JS::HandleObject JSGLCtx,
                               ResourceType type,
                               GLenum target)
{
    char targetStr[11];
    snprintf(targetStr, 11, "%d", target);

    JS::RootedValue boundValue(cx, JS_GetReservedSlot(JSGLCtx, type));
    if (boundValue.isNullOrUndefined()) return;

    JS::RootedObject bound(cx, boundValue.toObjectOrNull());
    if (!bound) return;

    JS_SetProperty(cx, bound, targetStr, JS::NullHandleValue);
}
// }}}

// {{{ JSWebGLActiveInfo Implementation
JSObject *JSWebGLActiveInfo::createObject(JSContext *cx,
                                          GLint csize,
                                          GLenum ctype,
                                          const char *cname)
{
    JS::RootedObject obj(cx, JSWebGLActiveInfo::CreateObject(cx, this));

    JS::RootedValue size(cx);
    JS::RootedValue type(cx);
    JS::RootedValue name(cx);

    size.setInt32(csize);
    type.setNumber(ctype);
    name.setString(JS_NewStringCopyZ(cx, cname));

    JS_DefineProperty(cx, obj, "size", size,
                      JSPROP_READONLY | JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineProperty(cx, obj, "type", type,
                      JSPROP_READONLY | JSPROP_ENUMERATE | JSPROP_PERMANENT);
    JS_DefineProperty(cx, obj, "name", name,
                      JSPROP_READONLY | JSPROP_ENUMERATE | JSPROP_PERMANENT);
    return obj;
}
// }}}

// {{{ JSWebGLShader Implementation
void JSWebGLShader::setShaderSource(JS::HandleString str)
{
    m_ShaderData.source = JS_EncodeString(this->getJSContext(), str);
}

void JSWebGLShader::freeShaderSource()
{
    if (m_ShaderData.source != nullptr) {
        JS_free(this->getJSContext(), const_cast<char *>(m_ShaderData.source));
        m_ShaderData.source = nullptr;
    }
}

JSWebGLShader::~JSWebGLShader()
{
    this->freeShaderSource();
}
// }}}

// {{{ WebGLRenderingContext Implementation
bool JSWebGLRenderingContext::JS_isContextLost(JSContext *cx,
                                               JS::CallArgs &args)
{
    args.rval().setBoolean(false);

    return true;
}

bool JSWebGLRenderingContext::JS_getExtension(JSContext *cx, JS::CallArgs &args)
{
    args.rval().setNull();

    return true;
}


bool JSWebGLRenderingContext::JS_activeTexture(JSContext *cx,
                                               JS::CallArgs &args)
{
    GLuint texture;

    if (!JS_ConvertArguments(cx, args, "u", &texture)) {
        return false;
    }

    GL_CALL(this, ActiveTexture(texture));

    return true;
}

bool JSWebGLRenderingContext::JS_attachShader(JSContext *cx, JS::CallArgs &args)
{
    JSWebGLProgram *webglProgram;
    JSWebGLShader *webglShader;

    JS::RootedObject program(cx);
    JS::RootedObject shader(cx);

    if (!JS_ConvertArguments(cx, args, "oo", program.address(),
                             shader.address())) {
        return false;
    }

    webglProgram = JSWebGLProgram::GetInstance(program);
    webglShader = JSWebGLShader::GetInstance(shader);
    if (!shader || !program) {
        JS_ReportError(cx, "Invalid argument(s)");
        return false;
    }

    GL_CALL(this, AttachShader(webglProgram->id(), webglShader->id()));

    return true;
}

bool JSWebGLRenderingContext::JS_bindAttribLocation(JSContext *cx,
                                                    JS::CallArgs &args)
{
    GLuint vertex;
    const char *cname;
    JSWebGLProgram *webglProgram;

    JS::RootedObject program(cx);
    JS::RootedString name(cx);
    if (!JS_ConvertArguments(cx, args, "ouS", program.address(), &vertex,
                             name.address())) {
        return false;
    }

    webglProgram = JSWebGLProgram::GetInstance(program);
    if (!webglProgram) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    cname = JS_EncodeString(cx, name);

    GL_CALL(this, BindAttribLocation(webglProgram->id(), vertex, cname));

    JS_free(cx, (void *)cname);

    return true;
}

bool JSWebGLRenderingContext::JS_bindBuffer(JSContext *cx, JS::CallArgs &args)
{
    GLenum target;
    JSWebGLBuffer *webglBuffer;

    JS::RootedObject buffer(cx);
    JS::RootedObject thisobj(cx, this->getJSObject());

    if (!JS_ConvertArguments(cx, args, "uo", &target, buffer.address())) {
        return false;
    }

    if (buffer == NULL) {
        JSWebGLBuffer::UnbindFrom(cx, thisobj, WebGLResource::kBuffer, target);
        GL_CALL(this, BindBuffer(target, 0));

        return true;
    }

    webglBuffer = JSWebGLBuffer::GetInstance(buffer);
    if (!webglBuffer) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    webglBuffer->bindTo(target);

    GL_CALL(this, BindBuffer(target, webglBuffer->id()));

    return true;
}

bool JSWebGLRenderingContext::JS_bindFramebuffer(JSContext *cx,
                                                 JS::CallArgs &args)
{
    GLenum target;
    JSWebGLFramebuffer *webglBuffer;

    JS::RootedObject buffer(cx);
    JS::RootedObject thisobj(cx, this->getJSObject());

    if (!JS_ConvertArguments(cx, args, "uo", &target, buffer.address())) {
        return false;
    }

    if (buffer == NULL) {
        /*
           Bind to the default framebuffer
       */
        uint32_t fbo = this->getFrameBufferID();
        GL_CALL(this, BindFramebuffer(target, fbo));

        JSWebGLFramebuffer::UnbindFrom(cx, thisobj, WebGLResource::kFramebuffer,
                                       target);

        return true;
    }

    webglBuffer = JSWebGLFramebuffer::GetInstance(buffer);
    if (!webglBuffer) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    webglBuffer->bindTo(target);

    GL_CALL(this, BindFramebuffer(target, webglBuffer->id()));

    return true;
}

bool JSWebGLRenderingContext::JS_bindRenderbuffer(JSContext *cx,
                                                  JS::CallArgs &args)
{
    GLenum target;
    JSWebGLRenderbuffer *webglBuffer;

    JS::RootedObject buffer(cx);
    JS::RootedObject thisobj(cx, this->getJSObject());

    if (!JS_ConvertArguments(cx, args, "uo", &target, buffer.address())) {
        return false;
    }

    if (buffer == NULL) {
        GL_CALL(this, BindRenderbuffer(target, 0));
        JSWebGLRenderbuffer::UnbindFrom(cx, thisobj,
                                        WebGLResource::kRenderbuffer, target);
        return true;
    }

    webglBuffer = JSWebGLRenderbuffer::GetInstance(buffer);
    if (!webglBuffer) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    webglBuffer->bindTo(target);

    GL_CALL(this, BindRenderbuffer(target, webglBuffer->id()));

    return true;
}

bool JSWebGLRenderingContext::JS_bindTexture(JSContext *cx, JS::CallArgs &args)
{
    GLenum target;
    JSWebGLTexture *webglTexture;

    JS::RootedObject texture(cx);
    JS::RootedObject thisobj(cx, this->getJSObject());

    if (!JS_ConvertArguments(cx, args, "uo", &target, texture.address())) {
        return false;
    }

    if (texture == NULL) {
        GL_CALL(this, BindTexture(target, 0));
        JSWebGLTexture::UnbindFrom(cx, thisobj, WebGLResource::kTexture,
                                   target);
        return true;
    }

    webglTexture = JSWebGLTexture::GetInstance(texture);
    if (!webglTexture) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    webglTexture->bindTo(target);

    GL_CALL(this, BindTexture(target, webglTexture->id()));

    return true;
}

bool JSWebGLRenderingContext::JS_copyTexImage2D(JSContext *cx,
                                                JS::CallArgs &args)
{
    GLenum target;
    GLint level;
    GLenum internalFormat;
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;
    GLint border;

    if (!JS_ConvertArguments(cx, args, "uiuiiiii", &target, &level,
                             &internalFormat, &x, &y, &width, &height,
                             &border)) {
        return false;
    }

    /*
        GL_CALL(this, CopyTexImage2D(target, level, internalFormat,
            x, y, width, height, border));
    */

    return true;
}

bool JSWebGLRenderingContext::JS_copyTexSubImage2D(JSContext *cx,
                                                   JS::CallArgs &args)
{
    GLenum target;
    GLint level;
    GLint xoffset;
    GLint yoffset;
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;

    if (!JS_ConvertArguments(cx, args, "uiiiiiii", &target, &level, &xoffset,
                             &yoffset, &x, &y, &width, &height)) {
        return false;
    }

    GL_CALL(this, CopyTexSubImage2D(target, level, xoffset, yoffset, x, y,
                                    width, height));

    return true;
}

bool JSWebGLRenderingContext::JS_blendEquation(JSContext *cx,
                                               JS::CallArgs &args)
{
    GLuint mode;

    if (!JS_ConvertArguments(cx, args, "u", &mode)) {
        return false;
    }

    GL_CALL(this, BlendEquation(mode));

    return true;
}

bool JSWebGLRenderingContext::JS_blendEquationSeparate(JSContext *cx,
                                                       JS::CallArgs &args)
{
    GLuint modeRGB;
    GLenum modeAlpha;

    if (!JS_ConvertArguments(cx, args, "uu", &modeRGB, &modeAlpha)) {
        return false;
    }

    GL_CALL(this, BlendEquationSeparate(modeRGB, modeAlpha));

    return true;
}

bool JSWebGLRenderingContext::JS_blendFunc(JSContext *cx, JS::CallArgs &args)
{
    GLenum sfactor;
    GLuint dfactor;

    if (!JS_ConvertArguments(cx, args, "uu", &sfactor, &dfactor)) {
        return false;
    }

    GL_CALL(this, BlendFunc(sfactor, dfactor));

    return true;
}

bool JSWebGLRenderingContext::JS_blendFuncSeparate(JSContext *cx,
                                                   JS::CallArgs &args)
{
    GLuint srcRGB;
    GLuint dstRGB;
    GLenum srcAlpha;
    GLenum dstAlpha;

    if (!JS_ConvertArguments(cx, args, "uuuu", &srcRGB, &dstRGB, &srcAlpha,
                             &dstAlpha)) {
        return false;
    }

    GL_CALL(this, BlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha));

    return true;
}


bool JSWebGLRenderingContext::JS_bufferData(JSContext *cx, JS::CallArgs &args)
{
    GLenum target;
    GLenum usage;

    JS::RootedObject array(cx);
    if (!JS_ConvertArguments(cx, args, "uou", &target, array.address(),
                             &usage)) {
        return false;
    }

    if (array == NULL || !JS_IsTypedArrayObject(array)) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    GL_CALL(this, BufferData(target, JS_GetArrayBufferViewByteLength(array),
                             JS_GetArrayBufferViewData(array), usage));

    return true;
}

bool JSWebGLRenderingContext::JS_bufferSubData(JSContext *cx,
                                               JS::CallArgs &args)
{
    GLenum target;
    GLint offset;

    JS::RootedObject array(cx);
    if (!JS_ConvertArguments(cx, args, "uuo", &target, &offset,
                             array.address())) {
        return false;
    }

    if (array == NULL || !JS_IsTypedArrayObject(array)
        || !JS_IsArrayBufferViewObject(array)) {

        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    GL_CALL(this, BufferSubData(target, offset,
                                JS_GetArrayBufferViewByteLength(array),
                                JS_GetArrayBufferViewData(array)));

    return true;
}

bool JSWebGLRenderingContext::JS_clear(JSContext *cx, JS::CallArgs &args)
{
    GLbitfield bits;

    if (!JS_ConvertArguments(cx, args, "i", &bits)) {
        return false;
    }

    GL_CALL(this, Clear(bits | GL_DEPTH_BUFFER_BIT));

    return true;
}

bool JSWebGLRenderingContext::JS_clearColor(JSContext *cx, JS::CallArgs &args)
{
    double r;
    double g;
    double b;
    double a;

    if (!JS_ConvertArguments(cx, args, "dddd", &r, &g, &b, &a)) {
        return false;
    }

    GL_CALL(this, ClearColor(r, g, b, a));

    return true;
}

bool JSWebGLRenderingContext::JS_clearDepth(JSContext *cx, JS::CallArgs &args)
{
    double clampd;

    if (!JS_ConvertArguments(cx, args, "d", &clampd)) {
        return false;
    }

    GL_CALL(this, ClearDepth(clampd));

    return true;
}

bool JSWebGLRenderingContext::JS_clearStencil(JSContext *cx, JS::CallArgs &args)
{
    GLint s;

    if (!JS_ConvertArguments(cx, args, "i", &s)) {
        return false;
    }

    GL_CALL(this, ClearStencil(s));

    return true;
}

bool JSWebGLRenderingContext::JS_colorMask(JSContext *cx, JS::CallArgs &args)
{
    bool red   = true;
    bool green = true;
    bool blue  = true;
    bool alpha = true;

    GLboolean gred   = GL_TRUE;
    GLboolean ggreen = GL_TRUE;
    GLboolean gblue  = GL_TRUE;
    GLboolean galpha = GL_TRUE;

    if (!JS_ConvertArguments(cx, args, "/bbbb", &red, &green, &blue, &alpha)) {
        return false;
    }

    if (!red) gred = GL_FALSE;
    if (!green) ggreen = GL_FALSE;
    if (!blue) gblue = GL_FALSE;
    if (!alpha) galpha = GL_FALSE;

    GL_CALL(this, ColorMask(gred, ggreen, gblue, galpha));

    return true;
}

bool JSWebGLRenderingContext::JS_compileShader(JSContext *cx,
                                               JS::CallArgs &args)
{
    JSWebGLShader *webglShader;
    const char *shaderStr;
    GLint shaderLen;
    GLenum err;

    JS::RootedObject shader(cx);
    if (!JS_ConvertArguments(cx, args, "o", shader.address())) {
        return false;
    }

    webglShader = JSWebGLShader::GetInstance(shader);
    if (!webglShader) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    if (!(shaderStr = CanvasContext::ProcessShader(
              webglShader->getShaderSource(), webglShader->getShaderType()))) {
        JS_ReportError(cx, "Failed to process the shader");
        return false;
    }

    shaderLen = strlen(shaderStr);

    GL_CALL(this, ShaderSource(webglShader->id(), 1, &shaderStr, &shaderLen));
    GL_CALL_RET(this, GetError(), err);
    if (err != 0) {
        JS_ReportError(cx, "Failed to source the shader glError = %d", err);
        return false;
    }

    GL_CALL(this, CompileShader(webglShader->id()));

    webglShader->freeShaderSource();
    free((char *)shaderStr);

    return true;
}

bool JSWebGLRenderingContext::JS_createBuffer(JSContext *cx, JS::CallArgs &args)
{
    GLuint buffer;

    GL_CALL(this, GenBuffers(1, &buffer));

    JSWebGLBuffer *webglBuffer = new JSWebGLBuffer(buffer, this);
    JS::RootedObject ret(cx, JSWebGLBuffer::CreateObject(cx, webglBuffer));

    args.rval().setObjectOrNull(ret);

    return true;
}

bool JSWebGLRenderingContext::JS_createFramebuffer(JSContext *cx,
                                                   JS::CallArgs &args)
{
    GLuint buffer;

    GL_CALL(this, GenFramebuffers(1, &buffer));

    JSWebGLFramebuffer *webglBuffer = new JSWebGLFramebuffer(buffer, this);
    JS::RootedObject ret(cx, JSWebGLFramebuffer::CreateObject(cx, webglBuffer));

    args.rval().setObjectOrNull(ret);

    return true;
}

bool JSWebGLRenderingContext::JS_createRenderbuffer(JSContext *cx,
                                                    JS::CallArgs &args)
{
    GLuint buffer;

    GL_CALL(this, GenRenderbuffers(1, &buffer));

    JSWebGLRenderbuffer *webglBuffer = new JSWebGLRenderbuffer(buffer, this);
    JS::RootedObject ret(cx,
                         JSWebGLRenderbuffer::CreateObject(cx, webglBuffer));

    args.rval().setObjectOrNull(ret);

    return true;
}

bool JSWebGLRenderingContext::JS_createProgram(JSContext *cx,
                                               JS::CallArgs &args)
{
    GLuint program;

    GL_CALL_RET(this, CreateProgram(), program);

    JSWebGLProgram *webglProgram = new JSWebGLProgram(program, this);
    JS::RootedObject ret(cx, JSWebGLProgram::CreateObject(cx, webglProgram));

    args.rval().setObjectOrNull(ret);

    return true;
}

bool JSWebGLRenderingContext::JS_createShader(JSContext *cx, JS::CallArgs &args)
{
    GLenum type;
    GLuint cshader;

    if (!JS_ConvertArguments(cx, args, "u", &type)) {
        return false;
    }

    if (type != NGL_VERTEX_SHADER && type != NGL_FRAGMENT_SHADER) {
        JS_ReportError(cx, "Invalid shader type");
        return false;
    }

    GL_CALL_RET(this, CreateShader(type), cshader);

    JSWebGLShader *webglShader = new JSWebGLShader(type, cshader, this);
    JS::RootedObject ret(cx, JSWebGLShader::CreateObject(cx, webglShader));

    args.rval().setObjectOrNull(ret);

    return true;
}

bool JSWebGLRenderingContext::JS_createTexture(JSContext *cx,
                                               JS::CallArgs &args)
{
    GLuint texture;

    GL_CALL(this, GenTextures(1, &texture));

    JSWebGLTexture *webglTexture = new JSWebGLTexture(texture, this);
    JS::RootedObject ret(cx, JSWebGLTexture::CreateObject(cx, webglTexture));

    args.rval().setObjectOrNull(ret);

    return true;
}

bool JSWebGLRenderingContext::JS_cullFace(JSContext *cx, JS::CallArgs &args)
{
    GLuint mode;

    if (!JS_ConvertArguments(cx, args, "u", &mode)) {
        return false;
    }

    GL_CALL(this, CullFace(mode));

    return true;
}

NGL_JS_FN_DELETE_X(deleteBuffer, Buffer)
NGL_JS_FN_DELETE_X(deleteFramebuffer, Framebuffer)
NGL_JS_FN_DELETE_X(deleteProgram, Program)
NGL_JS_FN_DELETE_X(deleteRenderbuffer, Renderbuffer)
NGL_JS_FN_DELETE_X(deleteTexture, Texture)
NGL_JS_FN_DELETE_X(deleteShader, Shader)

bool JSWebGLRenderingContext::JS_depthFunc(JSContext *cx, JS::CallArgs &args)
{
    GLuint func;

    if (!JS_ConvertArguments(cx, args, "u", &func)) {
        return false;
    }

    GL_CALL(this, DepthFunc(func));

    return true;
}

bool JSWebGLRenderingContext::JS_depthMask(JSContext *cx, JS::CallArgs &args)
{
    bool flag;

    if (!JS_ConvertArguments(cx, args, "b", &flag)) {
        return false;
    }

    GL_CALL(this, DepthMask((GLboolean)flag));

    return true;
}

bool JSWebGLRenderingContext::JS_depthRange(JSContext *cx, JS::CallArgs &args)
{
    double zNear;
    double zFar;

    if (!JS_ConvertArguments(cx, args, "dd", &zNear, &zFar)) {
        return false;
    }

    // GL_CALL(this, DepthRange(zNear, zFar));

    return true;
}

bool JSWebGLRenderingContext::JS_detachShader(JSContext *cx, JS::CallArgs &args)
{
    JSWebGLProgram *webglProgram;
    JSWebGLShader *webglShader;

    JS::RootedObject program(cx);
    JS::RootedObject shader(cx);
    if (!JS_ConvertArguments(cx, args, "oo", program.address(),
                             shader.address())) {
        return false;
    }

    webglProgram = JSWebGLProgram::GetInstance(program);
    webglShader  = JSWebGLShader::GetInstance(shader);

    // FIXME : Should we free the c++ object ?
    // FIXME : glDetachShader not implemented in skia
    // GL_CALL(this, DetachShader(webglProgram->id(), webglShader->id()));

    return true;
}

bool JSWebGLRenderingContext::JS_disable(JSContext *cx, JS::CallArgs &args)
{
    GLenum cap;

    if (!JS_ConvertArguments(cx, args, "u", &cap)) {
        return false;
    }

    GL_CALL(this, Disable(cap));

    return true;
}

bool JSWebGLRenderingContext::JS_disableVertexAttribArray(JSContext *cx,
                                                          JS::CallArgs &args)
{
    GLuint attr;

    if (!JS_ConvertArguments(cx, args, "u", &attr)) {
        return false;
    }

    GL_CALL(this, DisableVertexAttribArray(attr));

    return true;
}

bool JSWebGLRenderingContext::JS_drawArrays(JSContext *cx, JS::CallArgs &args)
{
    GLenum mode;
    GLint first;
    GLsizei count;

    if (!JS_ConvertArguments(cx, args, "uii", &mode, &first, &count)) {
        return false;
    }

    GL_CALL(this, DrawArrays(mode, first, count));

    return true;
}

bool JSWebGLRenderingContext::JS_drawElements(JSContext *cx, JS::CallArgs &args)
{
    GLenum mode;
    GLsizei count;
    GLenum type;
    GLint offset;

    if (!JS_ConvertArguments(cx, args, "uiui", &mode, &count, &type, &offset)) {
        return false;
    }

    if (offset + count < offset || offset + count < count) {
        JS_ReportError(cx, "Overflow in drawElements");
        return false;
    }

    GL_CALL(this, DrawElements(mode, count, type, (void *)(intptr_t)offset));

    return true;
}

bool JSWebGLRenderingContext::JS_enable(JSContext *cx, JS::CallArgs &args)
{
    GLuint bits;

    if (!JS_ConvertArguments(cx, args, "u", &bits)) {
        return false;
    }

    GL_CALL(this, Enable(bits));

    return true;
}

bool JSWebGLRenderingContext::JS_enableVertexAttribArray(JSContext *cx,
                                                         JS::CallArgs &args)
{
    GLuint attr;

    if (!JS_ConvertArguments(cx, args, "u", &attr)) {
        return false;
    }

    GL_CALL(this, EnableVertexAttribArray(attr));

    return true;
}

bool JSWebGLRenderingContext::JS_finish(JSContext *cx, JS::CallArgs &args)
{
    GL_CALL(this, Finish());
    return true;
}

bool JSWebGLRenderingContext::JS_flush(JSContext *cx, JS::CallArgs &args)
{
    GL_CALL(this, Flush());
    return true;
}

bool JSWebGLRenderingContext::JS_getUniformLocation(JSContext *cx,
                                                    JS::CallArgs &args)
{
    GLint location;
    JS::RootedValue proto(cx);
    const char *cname;
    JSWebGLProgram *webglProgram;

    JS::RootedString name(cx);
    JS::RootedObject program(cx);

    if (!JS_ConvertArguments(cx, args, "oS", program.address(),
                             name.address())) {
        return false;
    }

    webglProgram = JSWebGLProgram::GetInstance(program);
    if (!webglProgram) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    cname = JS_EncodeString(cx, name);

    GL_CALL_RET(this, GetUniformLocation(webglProgram->id(), cname), location);

    if (location < 0) {
        args.rval().setNull();
    } else {
        JS::RootedObject ret(cx, JSWebGLUniformLocation::CreateObject(
                                     cx, new JSWebGLUniformLocation(location)));

        args.rval().setObjectOrNull(ret);
    }

    JS_free(cx, (void *)cname);

    return true;
}

bool JSWebGLRenderingContext::JS_getShaderPrecisionFormat(JSContext *cx,
                                                          JS::CallArgs &args)
{
#define SET_PROP(prop, val) JS_SetProperty(cx, obj, prop, val)
    GLenum shaderType, precisionType;
    GLint crange[2];
    GLint cprecision;

    if (!JS_ConvertArguments(cx, args, "uu", &shaderType, &precisionType)) {
        return false;
    }

    JS::RootedObject obj(cx, JSWebGLShaderPrecisionFormat::CreateObject(
                                 cx, new JSWebGLShaderPrecisionFormat()));

    // Since getShaderPrecisionFormat is not available everywhere...
    // (Taken from mozilla GLContext.h)
    crange[0]  = 24;
    crange[1]  = 24;
    cprecision = 0;
    switch (precisionType) {
        case NGL_LOW_FLOAT:
        case NGL_MEDIUM_FLOAT:
        case NGL_HIGH_FLOAT:
            // Assume IEEE 754 precision
            crange[0]  = 127;
            crange[1]  = 127;
            cprecision = 23;
            break;
        case NGL_LOW_INT:
        case NGL_MEDIUM_INT:
        case NGL_HIGH_INT:
            // Some (most) hardware only supports single-precision
            // floating-point
            // numbers,
            // which can accurately represent integers up to +/-16777216
            crange[0]  = 24;
            crange[1]  = 24;
            cprecision = 0;
            break;
        default:
            JS_ReportError(cx, "Invalid precision specified");
            return false;
    }

    JS::RootedValue rangeMin(cx, INT_TO_JSVAL(crange[0]));
    JS::RootedValue rangeMax(cx, INT_TO_JSVAL(crange[1]));
    JS::RootedValue precision(cx, INT_TO_JSVAL(cprecision));

    SET_PROP("rangeMin", rangeMin);
    SET_PROP("rangeMax", rangeMax);
    SET_PROP("precision", precision);

    args.rval().setObjectOrNull(obj);

    return true;
#undef SET_PROP
}

bool JSWebGLRenderingContext::JS_framebufferRenderbuffer(JSContext *cx,
                                                         JS::CallArgs &args)
{
    GLenum target, attachement, renderbuffertarget;
    JS::RootedObject renderbuffer(cx);
    JSWebGLRenderbuffer *webglRenderbuffer;

    if (!JS_ConvertArguments(cx, args, "uuuo", &target, &attachement,
                             &renderbuffertarget, renderbuffer.address())) {
        return false;
    }

    webglRenderbuffer = JSWebGLRenderbuffer::GetInstance(renderbuffer, cx);
    if (!webglRenderbuffer) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    GL_CALL(this,
            FramebufferRenderbuffer(target, attachement, renderbuffertarget,
                                    webglRenderbuffer->id()));

    return true;
}

bool JSWebGLRenderingContext::JS_framebufferTexture2D(JSContext *cx,
                                                      JS::CallArgs &args)
{
    GLenum target, attachement, textarget;
    uintptr_t level;
    JSWebGLTexture *webglTexture;

    JS::RootedObject texture(cx);
    if (!JS_ConvertArguments(cx, args, "uuuoi", &target, &attachement,
                             &textarget, texture.address(), &level)) {
        return false;
    }

    webglTexture = JSWebGLTexture::GetInstance(texture);
    if (!webglTexture) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    GL_CALL(this, FramebufferTexture2D(target, attachement, textarget,
                                       webglTexture->id(), level));

    GLenum status;
    GL_CALL_RET(this, CheckFramebufferStatus(GL_FRAMEBUFFER), status);

    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            JS_ReportError(cx, "FBO unsupported");
            return false;
        default:
            JS_ReportError(cx, "FBO fatal error wat %d\n", status);
            return false;
    }

    return true;
}

bool JSWebGLRenderingContext::JS_frontFace(JSContext *cx, JS::CallArgs &args)
{
    GLuint mode;

    if (!JS_ConvertArguments(cx, args, "u", &mode)) {
        return false;
    }

    GL_CALL(this, FrontFace(mode));

    return true;
}

bool JSWebGLRenderingContext::JS_generateMipmap(JSContext *cx,
                                                JS::CallArgs &args)
{
    GLenum target;

    if (!JS_ConvertArguments(cx, args, "u", &target)) {
        return false;
    }

    GL_CALL(this, GenerateMipmap(target));

    return true;
}

bool JSWebGLRenderingContext::JS_getActiveAttrib(JSContext *cx,
                                                 JS::CallArgs &args)
{
    JSWebGLProgram *webglProgram;
    char name[2048];
    unsigned int type = 0;
    unsigned int index;
    int size = 0;

    JS::RootedObject program(cx);
    if (!JS_ConvertArguments(cx, args, "ou", program.address(), &index)) {
        return false;
    }

    webglProgram = JSWebGLProgram::GetInstance(program);
    if (!webglProgram) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    // FIXME : Missing GetActiveAttrib in skia interface
    //
    // Temporary workaround to make sure we are on the correct
    // drawing context before calling a GL function
    int err;
    GL_CALL_RET(this, GetError(), err);

    int len;
    glGetActiveAttrib(webglProgram->id(), index, 2048, &len, &size, &type,
                      name);

    GL_CALL_RET(this, GetError(), err);
    if (err != 0) {
        args.rval().setNull();
        return true;
    }


    JSWebGLActiveInfo *webglActiveInfo = new JSWebGLActiveInfo();
    JS::RootedObject obj(cx,
                         webglActiveInfo->createObject(cx, size, type, name));

    args.rval().setObjectOrNull(obj);

    return true;
}

bool JSWebGLRenderingContext::JS_getActiveUniform(JSContext *cx,
                                                  JS::CallArgs &args)
{
    JSWebGLProgram *webglProgram;
    GLuint index;

    char name[2048];
    GLsizei length;
    GLint size;
    GLenum type;

    JS::RootedObject program(cx);
    if (!JS_ConvertArguments(cx, args, "ou", program.address(), &index)) {
        return false;
    }

    webglProgram = JSWebGLProgram::GetInstance(program);

    // FIXME : Missing getActiveUniform in skia interface
    //
    // Temporary workaround to make sure we are on the correct
    // drawing context before calling a GL function
    int err;
    GL_CALL_RET(this, GetError(), err);

    glGetActiveUniform(webglProgram->id(), index, 2048, &length, &size, &type,
                       name);
    GL_CALL_RET(this, GetError(), err);
    if (err != 0) {
        args.rval().setNull();
        return true;
    }

    JSWebGLActiveInfo *webglActiveInfo = new JSWebGLActiveInfo();
    JS::RootedObject obj(cx,
                         webglActiveInfo->createObject(cx, size, type, name));

    args.rval().setObjectOrNull(obj);

    return true;
}

bool JSWebGLRenderingContext::JS_getAttribLocation(JSContext *cx,
                                                   JS::CallArgs &args)
{
    GLint location;
    JSWebGLProgram *webglProgram;
    const char *cattr;

    JS::RootedString attr(cx);
    JS::RootedObject program(cx);
    if (!JS_ConvertArguments(cx, args, "oS", program.address(),
                             attr.address())) {
        return false;
    }

    webglProgram = JSWebGLProgram::GetInstance(program);
    if (!program) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    cattr = JS_EncodeString(cx, attr);

    GL_CALL_RET(this, GetAttribLocation(webglProgram->id(), cattr), location);

    JS_free(cx, (void *)cattr);

    args.rval().setInt32(location);

    return true;
}

bool JSWebGLRenderingContext::JS_getParameter(JSContext *cx, JS::CallArgs &args)
{
    GLenum name;
    JS::RootedValue value(cx);

    if (!JS_ConvertArguments(cx, args, "u", &name)) {
        return false;
    }

    switch (name) {
        // String
        case NGL_VENDOR:
        case NGL_RENDERER:
        case NGL_VERSION:
        case NGL_SHADING_LANGUAGE_VERSION: {
            const GLubyte *cstr;
            GL_CALL_RET(this, GetString(name), cstr);
            JS::RootedString str(cx, JS_NewStringCopyZ(cx, (const char *)cstr));
            value.setString(str);

            break;
        }
        case NGL_CULL_FACE_MODE:
        case NGL_FRONT_FACE:
        case NGL_ACTIVE_TEXTURE:
        case NGL_STENCIL_FUNC:
        case NGL_STENCIL_FAIL:
        case NGL_STENCIL_PASS_DEPTH_FAIL:
        case NGL_STENCIL_PASS_DEPTH_PASS:
        case NGL_STENCIL_BACK_FUNC:
        case NGL_STENCIL_BACK_FAIL:
        case NGL_STENCIL_BACK_PASS_DEPTH_FAIL:
        case NGL_STENCIL_BACK_PASS_DEPTH_PASS:
        case NGL_DEPTH_FUNC:
        case NGL_BLEND_SRC_RGB:
        case NGL_BLEND_SRC_ALPHA:
        case NGL_BLEND_DST_RGB:
        case NGL_BLEND_DST_ALPHA:
        case NGL_BLEND_EQUATION_RGB:
        case NGL_BLEND_EQUATION_ALPHA:
        case NGL_GENERATE_MIPMAP_HINT: {
            GLint i = 0;
            GL_CALL(this, GetIntegerv(name, &i));
            value.setNumber(uint32_t(i));
            break;
        }
        // int
        case NGL_STENCIL_CLEAR_VALUE:
        case NGL_STENCIL_REF:
        case NGL_STENCIL_BACK_REF:
        case NGL_UNPACK_ALIGNMENT:
        case NGL_PACK_ALIGNMENT:
        case NGL_SUBPIXEL_BITS:
        case NGL_MAX_TEXTURE_SIZE:
        case NGL_MAX_CUBE_MAP_TEXTURE_SIZE:
        case NGL_SAMPLE_BUFFERS:
        case NGL_SAMPLES:
        case NGL_MAX_VERTEX_ATTRIBS:
        case NGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
        case NGL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
        case NGL_MAX_TEXTURE_IMAGE_UNITS:
        case NGL_MAX_RENDERBUFFER_SIZE:
        case NGL_RED_BITS:
        case NGL_GREEN_BITS:
        case NGL_BLUE_BITS:
        case NGL_ALPHA_BITS:
        case NGL_DEPTH_BITS:
        case NGL_STENCIL_BITS:
        case NGL_MAX_VERTEX_UNIFORM_VECTORS:
        case NGL_MAX_FRAGMENT_UNIFORM_VECTORS:
        case NGL_MAX_VARYING_VECTORS:
            // case NGL_NUM_COMPRESSED_TEXTURE_FORMATS:
            {
                GLint i = 0;
                GL_CALL(this, GetIntegerv(name, &i));
                value.setInt32(i);
                break;
            }
#if 0
        case NGL_FRAGMENT_SHADER_DERIVATIVE_HINT:
            if (IsExtensionEnabled(OES_standard_derivatives)) {
                GLint i = 0;
                gl->fGetIntegerv(pname, &i);
                return JS::Int32Value(i);
            }
            else {
                ErrorInvalidEnum("getParameter: parameter", pname);
                return JS::NullValue();
            }
            value.setNull();
            break;
#endif
        case NGL_COMPRESSED_TEXTURE_FORMATS: {
            GLint length;
            GLint *textures;
            uint32_t *data;

            GL_CALL(this,
                    GetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &length));
            JS::RootedObject obj(cx, JS_NewUint32Array(cx, length));
            textures = (GLint *)malloc(sizeof(GLint) * length);

            if (!obj || !textures) {
                if (textures != NULL) {
                    free(textures);
                }
                JS_ReportOutOfMemory(cx);
                return false;
            }

            GL_CALL(this, GetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, textures));

            data = JS_GetUint32ArrayData(obj);
            memcpy(data, textures, length * sizeof(GLint));
            free(textures);

            value.setObjectOrNull(obj);
            break;
        }

        // unsigned int
        case NGL_STENCIL_BACK_VALUE_MASK:
        case NGL_STENCIL_BACK_WRITEMASK:
        case NGL_STENCIL_VALUE_MASK:
        case NGL_STENCIL_WRITEMASK: {
            GLint i = 0; // the GL api (glGetIntegerv) only does signed ints
            GL_CALL(this, GetIntegerv(name, &i));
            GLuint i_unsigned(i); // this is where -1 becomes 2^32-1
            double i_double(
                i_unsigned); // pass as FP value to allow large values
                             // such as 2^32-1.
            value.setDouble(i_double);
            break;
        }

// float
#if 0
        case NGL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
            if (IsExtensionEnabled(EXT_texture_filter_anisotropic)) {
                GLfloat f = 0.f;
                gl->fGetFloatv(pname, &f);
                return JS::DoubleValue(f);
            } else {
                ErrorInvalidEnumInfo("getParameter: parameter", pname);
                return JS::NullValue();
            }
            value.setNull();
            break;
#endif
        case NGL_DEPTH_CLEAR_VALUE:
        case NGL_LINE_WIDTH:
        case NGL_POLYGON_OFFSET_FACTOR:
        case NGL_POLYGON_OFFSET_UNITS:
        case NGL_SAMPLE_COVERAGE_VALUE: {
            GLfloat f = 0.f;
            GL_CALL(this, GetFloatv(name, &f));
            value.setDouble(f);
            break;
        }

        // bool
        case NGL_BLEND:
        case NGL_DEPTH_TEST:
        case NGL_STENCIL_TEST:
        case NGL_CULL_FACE:
        case NGL_DITHER:
        case NGL_POLYGON_OFFSET_FILL:
        case NGL_SCISSOR_TEST:
        case NGL_SAMPLE_COVERAGE_INVERT:
        case NGL_DEPTH_WRITEMASK: {
            GLboolean b = 0;
            GL_CALL(this, GetBooleanv(name, &b));
            value.setBoolean(b);
            break;
        }

        // bool, WebGL-specific
        case NGL_UNPACK_FLIP_Y_WEBGL:
            value.setBoolean(
                this->hasFlag(Canvas3DContext::kUNPACK_FLIP_Y_WEBGL_Flag));
            break;
        case NGL_UNPACK_PREMULTIPLY_ALPHA_WEBGL:
            value.setBoolean(this->hasFlag(
                Canvas3DContext::kUNPACK_PREMULTIPLY_ALPHA_WEBGL_Flag));
            break;

        // uint, WebGL-specific
        case NGL_UNPACK_COLORSPACE_CONVERSION_WEBGL:
            value.setInt32(GL_NONE);
            break;

        // Complex values
        case NGL_DEPTH_RANGE:              // 2 floats
        case NGL_ALIASED_POINT_SIZE_RANGE: // 2 floats
        case NGL_ALIASED_LINE_WIDTH_RANGE: // 2 floats
        {
            GLfloat fv[2] = { 0 };
            JS::RootedObject obj(cx, JS_NewFloat32Array(cx, 2));
            float *data;

            if (!obj) {
                JS_ReportOutOfMemory(cx);
                return false;
            }

            GL_CALL(this, GetFloatv(name, fv));

            data = JS_GetFloat32ArrayData(obj);
            memcpy(data, fv, 2 * sizeof(float));
            value.setObjectOrNull(obj);
            break;
        }

        case NGL_COLOR_CLEAR_VALUE: // 4 floats
        case NGL_BLEND_COLOR:       // 4 floats
        {
            GLfloat fv[4] = { 0 };
            JS::RootedObject obj(cx, JS_NewFloat32Array(cx, 4));
            float *data;

            if (!obj) {
                JS_ReportOutOfMemory(cx);
                return false;
            }

            GL_CALL(this, GetFloatv(name, fv));

            data = JS_GetFloat32ArrayData(obj);
            memcpy(data, fv, 4 * sizeof(GLfloat));
            value.setObjectOrNull(obj);
            break;
        }

        case NGL_MAX_VIEWPORT_DIMS: // 2 ints
        {
            GLint iv[2] = { 0 };
            JS::RootedObject obj(cx, JS_NewInt32Array(cx, 2));
            int32_t *data;

            if (!obj) {
                JS_ReportOutOfMemory(cx);
                return false;
            }

            GL_CALL(this, GetIntegerv(name, iv));

            data = JS_GetInt32ArrayData(obj);
            memcpy(data, iv, 2 * sizeof(GLint));
            value.setObjectOrNull(obj);
            break;
        }

        case NGL_SCISSOR_BOX: // 4 ints
        case NGL_VIEWPORT:    // 4 ints
        {
            GLint iv[4] = { 0 };
            JS::RootedObject obj(cx, JS_NewInt32Array(cx, 4));
            int32_t *data;

            if (!obj) {
                JS_ReportOutOfMemory(cx);
                return false;
            }

            GL_CALL(this, GetIntegerv(name, iv));

            data = JS_GetInt32ArrayData(obj);
            memcpy(data, iv, 4 * sizeof(GLint));
            value.setObjectOrNull(obj);
            break;
        }

        case NGL_COLOR_WRITEMASK: // 4 bools
        {
            GLboolean gl_bv[4] = { 0 };

            GL_CALL(this, GetBooleanv(name, gl_bv));
            JS::RootedObject obj(cx, JS_NewArrayObject(cx, 4));
            if (!obj.get()) {
                JS_ReportOutOfMemory(cx);
                return false;
            }
            size_t i;
            for (i = 0; i < 4; i++) {
                JS::RootedId id(cx, INT_TO_JSID(i));
                JS::RootedValue bVal(cx, BOOLEAN_TO_JSVAL((bool)gl_bv[i]));
                JS_SetPropertyById(cx, obj, id, bVal);
            }
            value.setObjectOrNull(obj);
            break;
        }

        // TODO
        case NGL_ARRAY_BUFFER_BINDING:
        case NGL_ELEMENT_ARRAY_BUFFER_BINDING:
        case NGL_RENDERBUFFER_BINDING:
        case NGL_FRAMEBUFFER_BINDING:
        case NGL_CURRENT_PROGRAM:
        case NGL_TEXTURE_BINDING_2D:
        case NGL_TEXTURE_BINDING_CUBE_MAP:
            value.setNull();
            break;

        default:
            JS_ReportError(cx, "getParameter invalue value");
            return false;
    }

    args.rval().set(value);

    return true;
}

bool JSWebGLRenderingContext::JS_getProgramParameter(JSContext *cx,
                                                     JS::CallArgs &args)
{
    GLenum param = 0;
    GLint status;
    JSWebGLProgram *webglProgram;

    JS::RootedObject program(cx);
    if (!JS_ConvertArguments(cx, args, "ou", program.address(), &param)) {
        return false;
    }

    webglProgram = JSWebGLProgram::GetInstance(program);
    if (!program) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    GL_CALL(this, GetProgramiv(webglProgram->id(), param, &status));

    switch (param) {
        case GL_DELETE_STATUS:
        case GL_LINK_STATUS:
        case GL_VALIDATE_STATUS:
            args.rval().setBoolean((bool)(GLboolean)status);
            break;
        default:
            args.rval().setInt32(status);
            break;
    }

    return true;
}

bool JSWebGLRenderingContext::JS_getProgramInfoLog(JSContext *cx,
                                                   JS::CallArgs &args)
{
    GLsizei max;
    GLsizei length;
    JSWebGLProgram *webglProgram;
    char *clog;

    JS::RootedObject program(cx);
    if (!JS_ConvertArguments(cx, args, "o", program.address())) {
        return false;
    }

    webglProgram = JSWebGLProgram::GetInstance(program);
    if (!webglProgram) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    GL_CALL(this, GetProgramiv(webglProgram->id(), GL_INFO_LOG_LENGTH, &max));

    clog = (char *)malloc(max);
    GL_CALL(this, GetProgramInfoLog(webglProgram->id(), max, &length, clog));
    JS::RootedString log(cx, JS_NewStringCopyN(cx, clog, length));
    free(clog);

    args.rval().setString(log);

    return true;
}

bool JSWebGLRenderingContext::JS_getShaderParameter(JSContext *cx,
                                                    JS::CallArgs &args)
{
    GLenum pname;
    GLint param;
    JSWebGLShader *webglShader;

    JS::RootedObject shader(cx);
    if (!JS_ConvertArguments(cx, args, "ou", shader.address(), &pname)) {
        return false;
    }

    webglShader = JSWebGLShader::GetInstance(shader);
    if (!webglShader) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    GL_CALL(this, GetShaderiv(webglShader->id(), pname, &param));

    args.rval().setInt32(param);

    return true;
}

bool JSWebGLRenderingContext::JS_getShaderInfoLog(JSContext *cx,
                                                  JS::CallArgs &args)
{
    GLsizei length;
    GLsizei max;
    JSWebGLShader *webglShader;
    char *clog;

    JS::RootedObject shader(cx);
    if (!JS_ConvertArguments(cx, args, "o", shader.address())) {
        return false;
    }

    webglShader = JSWebGLShader::GetInstance(shader);
    if (!webglShader) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    GL_CALL(this, GetShaderiv(webglShader->id(), GL_INFO_LOG_LENGTH, &max));

    clog = (char *)malloc(max);
    GL_CALL(this, GetShaderInfoLog(webglShader->id(), max, &length, clog));
    JS::RootedString log(cx, JS_NewStringCopyN(cx, clog, length));
    free(clog);

    args.rval().setString(log);

    return true;
}

bool JSWebGLRenderingContext::JS_lineWidth(JSContext *cx, JS::CallArgs &args)
{
    double width;

    if (!JS_ConvertArguments(cx, args, "d", &width)) {
        return false;
    }

    GL_CALL(this, LineWidth((GLfloat)width));

    return true;
}

bool JSWebGLRenderingContext::JS_linkProgram(JSContext *cx, JS::CallArgs &args)
{
    JSWebGLProgram *webglProgram;

    JS::RootedObject program(cx);
    if (!JS_ConvertArguments(cx, args, "o", program.address())) {
        return false;
    }

    webglProgram = JSWebGLProgram::GetInstance(program);
    if (!webglProgram) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    GL_CALL(this, LinkProgram(webglProgram->id()));

    return true;
}

bool JSWebGLRenderingContext::JS_pixelStorei(JSContext *cx, JS::CallArgs &args)
{
    GLuint param;
    GLint value;

    if (!JS_ConvertArguments(cx, args, "ui", &param, &value)) {
        return false;
    }

    switch (param) {
        case NGL_UNPACK_FLIP_Y_WEBGL: {
            if (value) {
                this->addFlag(Canvas3DContext::kUNPACK_FLIP_Y_WEBGL_Flag);
            } else {
                this->removeFlag(Canvas3DContext::kUNPACK_FLIP_Y_WEBGL_Flag);
            }
            break;
        }
        case NGL_UNPACK_PREMULTIPLY_ALPHA_WEBGL: {
            if (value) {
                this->addFlag(
                    Canvas3DContext::kUNPACK_PREMULTIPLY_ALPHA_WEBGL_Flag);
            } else {
                this->removeFlag(
                    Canvas3DContext::kUNPACK_PREMULTIPLY_ALPHA_WEBGL_Flag);
            }
            break;
        }
        case NGL_UNPACK_COLORSPACE_CONVERSION_WEBGL:
            JS_ReportError(cx, "Not implemented");
            return false;
            break;
        default:
            GL_CALL(this, PixelStorei(param, value));
            break;
    }

    return true;
}

bool JSWebGLRenderingContext::JS_renderbufferStorage(JSContext *cx,
                                                     JS::CallArgs &args)
{
    GLenum target, internalFormat;
    GLsizei width, height;

    if (!JS_ConvertArguments(cx, args, "uuii", &target, &internalFormat, &width,
                             &height)) {
        return false;
    }

    GL_CALL(this, RenderbufferStorage(target, internalFormat, width, height));

    return true;
}

bool JSWebGLRenderingContext::JS_scissor(JSContext *cx, JS::CallArgs &args)
{
    GLint x, y;
    GLsizei width, height;

    if (!JS_ConvertArguments(cx, args, "iiii", &x, &y, &width, &height)) {
        return false;
    }

    GL_CALL(this, Scissor(x, y, width, height));

    return true;
}

bool JSWebGLRenderingContext::JS_shaderSource(JSContext *cx, JS::CallArgs &args)
{
    JSWebGLShader *webglShader;

    JS::RootedObject shader(cx);
    JS::RootedString source(cx);
    if (!JS_ConvertArguments(cx, args, "oS", shader.address(),
                             source.address())) {
        return false;
    }

    webglShader = JSWebGLShader::GetInstance(shader);
    if (!webglShader) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    webglShader->setShaderSource(source);

    return true;
}

bool JSWebGLRenderingContext::JS_texImage2D(JSContext *cx, JS::CallArgs &args)
{
    GLenum target;
    GLint level;
    GLint internalFormat;
    GLenum format;
    GLenum type;
    int width, height;

    if (args.length() == 9) {
        GLint border;
        void *pixels = NULL;

        JS::RootedObject array(cx);
        if (!JS_ConvertArguments(cx, args, "uiiiiiuuo", &target, &level,
                                 &internalFormat, &width, &height, &border,
                                 &format, &type, array.address())) {
            return false;
        }

        if (array != NULL && JS_IsTypedArrayObject(array)) {
            pixels = JS_GetArrayBufferViewData(array);
        } else {
            JS_ReportError(cx, "Invalid array (not a typed array)");
            return false;
        }

        GL_CALL(this, TexImage2D(target, level, internalFormat, width, height,
                                 border, format, type, pixels));
    } else {
        unsigned char *pixels;
        JS::RootedObject image(cx);

        if (!JS_ConvertArguments(cx, args, "uiiuuo", &target, &level,
                                 &internalFormat, &format, &type,
                                 image.address())
            || image == NULL) {
            JS_ReportError(cx, "texImage2D() invalid arguments");
            return false;
        }

        if (JSImage::InstanceOf(image)) {
            JSImage *nimg;

            nimg = JSImage::GetInstance(image);
            if (!nimg || !nimg->getImage()) {
                JS_ReportError(cx,
                               !nimg ? "Invalid Image object"
                                     : "No Image data (is the image loaded?)");
                return false;
            }

            width  = nimg->getImage()->getWidth();
            height = nimg->getImage()->getHeight();

            // Image are always decoded to RGBA
            format         = NGL_RGBA;
            internalFormat = NGL_RGBA;

            pixels
                = (unsigned char *)malloc(nimg->getImage()->m_Image->getSize());

            if (!Image::ConvertToRGBA(
                    nimg->getImage(), pixels,
                    this->hasFlag(Canvas3DContext::kUNPACK_FLIP_Y_WEBGL_Flag),
                    this->hasFlag(Canvas3DContext::
                                      kUNPACK_PREMULTIPLY_ALPHA_WEBGL_Flag))) {

                JS_ReportError(cx, "Failed to read image data");
                return false;
            }
        } else if (image && JSCanvas::InstanceOf(image)) {
            JSCanvas *jsCanvas = JSCanvas::GetInstance(image);
            CanvasHandler *handler
                = static_cast<CanvasHandler *>(jsCanvas->getHandler());
            Canvas2DContext *ctx
                = static_cast<Canvas2DContext *>(handler->getContext());

            width  = handler->getWidth();
            height = handler->getHeight();

            // Canvas are always RGBA
            format         = NGL_RGBA;
            internalFormat = NGL_RGBA;

            pixels = (unsigned char *)malloc(width * height * 4);

            ctx->getSurface()->readPixels(0, 0, width, height, pixels);
        } else {
            JS_ReportError(cx, "Unsupported or invalid image data");
            return false;
        }

        GL_CALL(this, TexImage2D(target, level, internalFormat, width, height,
                                 0, format, type, pixels));

        free(pixels);
    }

    return true;
}


bool JSWebGLRenderingContext::JS_texParameteri(JSContext *cx,
                                               JS::CallArgs &args)
{
    GLuint target;
    GLuint pname;
    GLuint param;

    if (!JS_ConvertArguments(cx, args, "uuu", &target, &pname, &param)) {
        return false;
    }

    GL_CALL(this, TexParameteri(target, pname, param));

    return true;
}

// {{{ Helpers for uniformXXX, uniformMatrixXXX, vertexAttribXXX
bool NGL_uniformxf(Canvas3DContext *glctx,
                   JSContext *cx,
                   JS::CallArgs &args,
                   int nb)
{
    double x;
    double y;
    double z;
    double w;
    JSWebGLUniformLocation *webglLocation;
    JS::RootedObject location(cx);

    if (args.length() == 0 || !args[0].isObject()) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    location = args[0].toObjectOrNull();
    if (!location) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    webglLocation
        = JSWebGLUniformLocation::GetInstance(args[0].toObjectOrNull());
    if (!webglLocation) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    if (nb > 0) JS::ToNumber(cx, args[1], &x);
    if (nb > 1) JS::ToNumber(cx, args[2], &y);
    if (nb > 2) JS::ToNumber(cx, args[3], &z);
    if (nb > 3) JS::ToNumber(cx, args[4], &w);

    switch (nb) {
        case 1:
            GL_CALL(glctx, Uniform1f(webglLocation->get(), (GLfloat)x));
            break;
        case 2:
            GL_CALL(glctx,
                    Uniform2f(webglLocation->get(), (GLfloat)x, (GLfloat)y));
            break;
        case 3:
            GL_CALL(glctx, Uniform3f(webglLocation->get(), (GLfloat)x,
                                     (GLfloat)y, (GLfloat)z));
            break;
        case 4:
            GL_CALL(glctx, Uniform4f(webglLocation->get(), (GLfloat)x,
                                     (GLfloat)y, (GLfloat)z, (GLfloat)w));
            break;
    }

    return true;
}

bool NGL_uniformxfv(Canvas3DContext *glctx,
                    JSContext *cx,
                    JS::CallArgs &args,
                    int nb)
{
    JSWebGLUniformLocation *webglLocation;
    GLsizei length;
    GLfloat *carray;

    JS::RootedObject array(cx);
    JS::RootedObject location(cx);
    if (!JS_ConvertArguments(cx, args, "oo", location.address(),
                             array.address())) {
        return false;
    }

    webglLocation = JSWebGLUniformLocation::GetInstance(location);
    if (!webglLocation) {
        JS_ReportError(cx, "Invalid argument");
        return false;
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

    switch (nb) {
        case 1:
            GL_CALL(glctx, Uniform1fv(webglLocation->get(), length, carray));
            break;
        case 2:
            GL_CALL(glctx,
                    Uniform2fv(webglLocation->get(), length / 2, carray));
            break;
        case 3:
            GL_CALL(glctx,
                    Uniform3fv(webglLocation->get(), length / 3, carray));
            break;
        case 4:
            GL_CALL(glctx,
                    Uniform4fv(webglLocation->get(), length / 4, carray));
            break;
    }

    return true;
}

bool NGL_uniformxi(Canvas3DContext *glctx,
                   JSContext *cx,
                   JS::CallArgs &args,
                   int nb)
{
    GLint x;
    GLint y;
    GLint z;
    GLint w;
    JSWebGLUniformLocation *webglLocation;
    JS::RootedObject location(cx);

    if (args.length() == 0 || !args[0].isObject() || nb < 1) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    location = args[0].toObjectOrNull();
    if (!location) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    webglLocation = JSWebGLUniformLocation::GetInstance(location);
    if (!webglLocation) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    JS::ToInt32(cx, args[1], &x);
    if (nb > 1) JS::ToInt32(cx, args[2], &y);
    if (nb > 2) JS::ToInt32(cx, args[3], &z);
    if (nb > 3) JS::ToInt32(cx, args[4], &w);

    switch (nb) {
        case 1:
            GL_CALL(glctx, Uniform1i(webglLocation->get(), x));
            break;
        case 2:
            GL_CALL(glctx, Uniform2i(webglLocation->get(), x, y));
            break;
        case 3:
            GL_CALL(glctx, Uniform3i(webglLocation->get(), x, y, z));
            break;
        case 4:
            GL_CALL(glctx, Uniform4i(webglLocation->get(), x, y, z, w));
            break;
    }

    return true;
}

bool NGL_uniformxiv(Canvas3DContext *glctx,
                    JSContext *cx,
                    JS::CallArgs &args,
                    int nb)
{
    GLsizei length;
    GLint *carray;
    JSWebGLUniformLocation *webglLocation;

    JS::RootedObject array(cx);
    JS::RootedObject location(cx);
    if (!JS_ConvertArguments(cx, args, "oo", location.address(),
                             array.address())) {
        return false;
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

    webglLocation = JSWebGLUniformLocation::GetInstance(location);
    if (!webglLocation) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    if (nb == 1) {
        GL_CALL(glctx, Uniform1iv(webglLocation->get(), length, carray));
    } else if (nb == 2) {
        GL_CALL(glctx, Uniform2iv(webglLocation->get(), length / 2, carray));
    } else if (nb == 3) {
        GL_CALL(glctx, Uniform3iv(webglLocation->get(), length / 3, carray));
    } else if (nb == 4) {
        GL_CALL(glctx, Uniform4iv(webglLocation->get(), length / 4, carray));
    }

    return true;
}

bool NGL_uniformMatrixxfv(Canvas3DContext *glctx,
                          JSContext *cx,
                          JS::CallArgs &args,
                          int nb)
{
    GLint length;
    GLfloat *carray;
    bool transpose;
    JSWebGLUniformLocation *webglLocation;

    JS::RootedObject array(cx);
    JS::RootedObject location(cx);
    if (!JS_ConvertArguments(cx, args, "obo", location.address(), &transpose,
                             array.address())) {
        return false;
    }

    webglLocation = JSWebGLUniformLocation::GetInstance(location);
    if (!webglLocation) {
        JS_ReportError(cx, "Invalid argument");
        return false;
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

    switch (nb) {
        case 2:
            GL_CALL(glctx, UniformMatrix2fv(webglLocation->get(), length / 4,
                                            (GLboolean)transpose, carray));
            break;
        case 3:
            GL_CALL(glctx, UniformMatrix3fv(webglLocation->get(), length / 8,
                                            (GLboolean)transpose, carray));
            break;
        case 4:
            GL_CALL(glctx, UniformMatrix4fv(webglLocation->get(), length / 16,
                                            (GLboolean)transpose, carray));
            break;
    }

    return true;
}

bool NGL_vertexAttribxf(Canvas3DContext *glctx,
                        JSContext *cx,
                        JS::CallArgs &args,
                        int nb)
{
    GLuint index;
    double v0;
    double v1;
    double v2;
    double v3;

    JS::ToUint32(cx, args[0], &index);
    if (nb > 0) JS::ToNumber(cx, args[1], &v0);
    if (nb > 1) JS::ToNumber(cx, args[2], &v1);
    if (nb > 2) JS::ToNumber(cx, args[3], &v2);
    if (nb > 3) JS::ToNumber(cx, args[4], &v3);

    switch (nb) {
        case 1:
            GL_CALL(glctx, VertexAttrib1f(index, (GLfloat)v0));
            break;
        case 2:
            GL_CALL(glctx, VertexAttrib2f(index, (GLfloat)v0, (GLfloat)v1));
            break;
        case 3:
            GL_CALL(glctx, VertexAttrib3f(index, (GLfloat)v0, (GLfloat)v1,
                                          (GLfloat)v2));
            break;
        case 4:
            GL_CALL(glctx, VertexAttrib4f(index, (GLfloat)v0, (GLfloat)v1,
                                          (GLfloat)v2, (GLfloat)v3));
            break;
    }

    return true;
}

bool NGL_vertexAttribxfv(Canvas3DContext *glctx,
                         JSContext *cx,
                         JS::CallArgs &args,
                         int nb)
{
    GLuint index;
    GLfloat *carray;

    if (!args[1].isObject()) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    if (!args[0].isNumber()) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    JS::ToUint32(cx, args[0], &index);
    JS::RootedObject array(cx, args[1].toObjectOrNull());

    if (JS_IsFloat32Array(array)) {
        carray = (GLfloat *)JS_GetFloat32ArrayData(array);
    } else if (JS_IsArrayObject(cx, array)) {
        JS::RootedObject tmp(cx, JS_NewFloat32ArrayFromArray(cx, array));
        carray = (GLfloat *)JS_GetFloat32ArrayData(tmp);
    } else {
        JS_ReportError(cx, "Array is not a Float32 array");
        return false;
    }

    switch (nb) {
        case 1:
            GL_CALL(glctx, VertexAttrib1fv(index, carray));
            break;
        case 2:
            GL_CALL(glctx, VertexAttrib2fv(index, carray));
            break;
        case 3:
            GL_CALL(glctx, VertexAttrib3fv(index, carray));
            break;
        case 4:
            GL_CALL(glctx, VertexAttrib4fv(index, carray));
            break;
    }

    return true;
}
// }}}

bool JSWebGLRenderingContext::JS_uniform1f(JSContext *cx, JS::CallArgs &args)
{
    return NGL_uniformxf(this, cx, args, 1);
}

bool JSWebGLRenderingContext::JS_uniform1fv(JSContext *cx, JS::CallArgs &args)
{
    return NGL_uniformxfv(this, cx, args, 1);
}

bool JSWebGLRenderingContext::JS_uniform1i(JSContext *cx, JS::CallArgs &args)
{
    return NGL_uniformxi(this, cx, args, 1);
}

bool JSWebGLRenderingContext::JS_uniform1iv(JSContext *cx, JS::CallArgs &args)
{
    return NGL_uniformxiv(this, cx, args, 1);
}

bool JSWebGLRenderingContext::JS_uniform2f(JSContext *cx, JS::CallArgs &args)
{
    return NGL_uniformxf(this, cx, args, 2);
}

bool JSWebGLRenderingContext::JS_uniform2fv(JSContext *cx, JS::CallArgs &args)
{
    return NGL_uniformxfv(this, cx, args, 2);
}

bool JSWebGLRenderingContext::JS_uniform2i(JSContext *cx, JS::CallArgs &args)
{
    return NGL_uniformxi(this, cx, args, 2);
}

bool JSWebGLRenderingContext::JS_uniform2iv(JSContext *cx, JS::CallArgs &args)
{
    return NGL_uniformxiv(this, cx, args, 2);
}

bool JSWebGLRenderingContext::JS_uniform3f(JSContext *cx, JS::CallArgs &args)
{
    return NGL_uniformxf(this, cx, args, 3);
}

bool JSWebGLRenderingContext::JS_uniform3fv(JSContext *cx, JS::CallArgs &args)
{
    return NGL_uniformxfv(this, cx, args, 3);
}

bool JSWebGLRenderingContext::JS_uniform3i(JSContext *cx, JS::CallArgs &args)
{
    return NGL_uniformxi(this, cx, args, 3);
}

bool JSWebGLRenderingContext::JS_uniform3iv(JSContext *cx, JS::CallArgs &args)
{
    return NGL_uniformxiv(this, cx, args, 3);
}

bool JSWebGLRenderingContext::JS_uniform4f(JSContext *cx, JS::CallArgs &args)
{
    return NGL_uniformxf(this, cx, args, 4);
}

bool JSWebGLRenderingContext::JS_uniform4fv(JSContext *cx, JS::CallArgs &args)
{
    return NGL_uniformxfv(this, cx, args, 4);
}

bool JSWebGLRenderingContext::JS_uniform4i(JSContext *cx, JS::CallArgs &args)
{
    return NGL_uniformxi(this, cx, args, 4);
}

bool JSWebGLRenderingContext::JS_uniform4iv(JSContext *cx, JS::CallArgs &args)
{
    return NGL_uniformxiv(this, cx, args, 4);
}

bool JSWebGLRenderingContext::JS_uniformMatrix2fv(JSContext *cx,
                                                  JS::CallArgs &args)
{
    return NGL_uniformMatrixxfv(this, cx, args, 2);
}

bool JSWebGLRenderingContext::JS_uniformMatrix3fv(JSContext *cx,
                                                  JS::CallArgs &args)
{
    return NGL_uniformMatrixxfv(this, cx, args, 3);
}

bool JSWebGLRenderingContext::JS_uniformMatrix4fv(JSContext *cx,
                                                  JS::CallArgs &args)
{
    return NGL_uniformMatrixxfv(this, cx, args, 4);
}

bool JSWebGLRenderingContext::JS_vertexAttrib1f(JSContext *cx,
                                                JS::CallArgs &args)
{
    return NGL_vertexAttribxf(this, cx, args, 1);
}

bool JSWebGLRenderingContext::JS_vertexAttrib1fv(JSContext *cx,
                                                 JS::CallArgs &args)
{
    return NGL_vertexAttribxfv(this, cx, args, 1);
}

bool JSWebGLRenderingContext::JS_vertexAttrib2f(JSContext *cx,
                                                JS::CallArgs &args)
{
    return NGL_vertexAttribxf(this, cx, args, 2);
}

bool JSWebGLRenderingContext::JS_vertexAttrib2fv(JSContext *cx,
                                                 JS::CallArgs &args)
{
    return NGL_vertexAttribxfv(this, cx, args, 2);
}

bool JSWebGLRenderingContext::JS_vertexAttrib3f(JSContext *cx,
                                                JS::CallArgs &args)
{
    return NGL_vertexAttribxf(this, cx, args, 3);
}

bool JSWebGLRenderingContext::JS_vertexAttrib3fv(JSContext *cx,
                                                 JS::CallArgs &args)
{
    return NGL_vertexAttribxfv(this, cx, args, 3);
}

bool JSWebGLRenderingContext::JS_vertexAttrib4f(JSContext *cx,
                                                JS::CallArgs &args)
{
    return NGL_vertexAttribxf(this, cx, args, 4);
}

bool JSWebGLRenderingContext::JS_vertexAttrib4fv(JSContext *cx,
                                                 JS::CallArgs &args)
{
    return NGL_vertexAttribxfv(this, cx, args, 4);
}

bool JSWebGLRenderingContext::JS_vertexAttribPointer(JSContext *cx,
                                                     JS::CallArgs &args)
{
    GLuint attr;
    GLint size;
    GLenum type;
    GLsizei stride;
    GLint offset;
    bool normalized;

    if (!JS_ConvertArguments(cx, args, "uiubii", &attr, &size, &type,
                             &normalized, &stride, &offset)) {
        return false;
    }

    if (offset + size < offset || offset + size < size) {
        JS_ReportError(cx, "Overflow in vertexAttribPointer");
        return false;
    }

    GL_CALL(this, VertexAttribPointer(attr, size, type, (GLboolean)normalized,
                                      stride, (void *)(intptr_t)offset));

    return true;
}

bool JSWebGLRenderingContext::JS_useProgram(JSContext *cx, JS::CallArgs &args)
{
    JSWebGLProgram *webglProgram;

    JS::RootedObject program(cx);
    JS::RootedObject thisobj(cx, this->getJSObject());

    if (!JS_ConvertArguments(cx, args, "o", program.address())) {
        return false;
    }

    if (program == NULL) {
        JS::RootedObject thisobj(cx, this->getJSObject());
        JSWebGLProgram::Unbind(thisobj, WebGLResource::kProgram);
        GL_CALL(this, UseProgram(0));
        return true;
    }

    webglProgram = JSWebGLProgram::GetInstance(program);
    if (!webglProgram) {
        JS_ReportError(cx, "Invalid argument");
        return false;
    }

    webglProgram->bind();

    GL_CALL(this, UseProgram(webglProgram->id()));

    return true;
}

bool JSWebGLRenderingContext::JS_viewport(JSContext *cx, JS::CallArgs &args)
{
    GLint x, y, w, h;

    float ratio = SystemInterface::GetInstance()->backingStorePixelRatio();

    if (!JS_ConvertArguments(cx, args, "iiii", &x, &y, &w, &h)) {
        return false;
    }

    GL_CALL(this, Viewport(x * ratio, y * ratio, w * ratio, h * ratio));

    return true;
}

bool JSWebGLRenderingContext::JS_getError(JSContext *cx, JS::CallArgs &args)
{
    GLenum err;

    GL_CALL_RET(this, GetError(), err);

    args.rval().setNumber(err);

    return true;
}

bool JSWebGLRenderingContext::JS_swapBuffer(JSContext *cx, JS::CallArgs &args)
{
#if 0
    Context::GetObject(cx)->getUI()->swapGLBuffer();

    return true;
#endif
    return true;
}

void JSWebGLRenderingContext::RegisterObject(JSContext *cx)
{
    JS::RootedObject ctor(cx);
    JS::RootedObject obj(
        cx, JSWebGLRenderingContext::ExposeClass<0>(
                cx, "WebGLRenderingContext",
                JSCLASS_HAS_RESERVED_SLOTS(WebGLResource::kResources_end)));

    if (!obj || !(ctor = JS_GetConstructor(cx, obj))) {
        // FIXME : Handle failure ? Throw exception ?
        return;
    }

    JS_DefineConstDoubles(cx, obj, JSWebGLRenderingContext::ListConstDoubles());
    JS_DefineConstDoubles(cx, ctor,
                          JSWebGLRenderingContext::ListConstDoubles());
}

void JSWebGLRenderingContext::RegisterAllObjects(JSContext *cx)
{
    JSWebGLRenderingContext::RegisterObject(cx);
    JSWebGLBuffer::RegisterObject(cx);
    JSWebGLFramebuffer::RegisterObject(cx);
    JSWebGLProgram::RegisterObject(cx);
    JSWebGLRenderbuffer::RegisterObject(cx);
    JSWebGLShader::RegisterObject(cx);
    JSWebGLTexture::RegisterObject(cx);
    JSWebGLUniformLocation::RegisterObject(cx);
    JSWebGLShaderPrecisionFormat::RegisterObject(cx);
    JSWebGLActiveInfo::RegisterObject(cx);
}
// }}}

} // namespace Nidium
} // namespace Binding
