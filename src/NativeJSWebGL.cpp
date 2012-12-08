#include "NativeJSWebGL.h"
#include "NativeJSImage.h"
#include "NativeSkImage.h"

#define GL_GLEXT_PROTOTYPES
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>

typedef union {
    GLuint buffer;
    void *dummy;
} GLuint_s;

#define NGL_UNPACK_FLIP_Y_WEBGL             0x9240
#define NGL_UNPACK_PREMULTIPLY_ALPHA_WEBGL  0x9241
#define NGL_CONTEXT_LOST_WEBGL              0x9242
#define NGL_UNPACK_COLORSPACE_CONVERSION_WEBGL 0x9243
#define NGL_BROWSER_DEFAULT_WEBGL           0x9244

#define NATIVE_GL_GETTER(obj) ((class NativeJSNativeGL *)JS_GetPrivate(obj))

#define D_NGL_JS_FN(func_name) static JSBool func_name(JSContext *cx, unsigned int argc, jsval *vp);

#define NGL_JS_FN(func_name) static JSBool func_name(JSContext *cx, unsigned int argc, jsval *vp)

#define NATIVE_GL_OBJECT_EXPOSE_NOT_INST(name) \
    void NativeJS ## name::registerObject(JSContext *cx) \
    { \
        JS_DefineObject(cx, JS_GetGlobalObject(cx), #name, \
            &name ## _class , NULL, 0); \
    }

static JSClass NativeGL_class = {
    "NativeGL", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};
static JSClass WebGLRenderingContext_class = {
    "WebGLRenderingContext", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};
static JSClass WebGLObject_class = {
    "WebGLObject", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};
static JSClass WebGLBuffer_class = {
    "WebGLBuffer", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};
static JSClass WebGLFrameBuffer_class = {
    "WebGLFrameBuffer", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};
static JSClass WebGLProgram_class = {
    "WebGLProgram", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};
static JSClass WebGLRenderbuffer_class = {
    "WebGLRenderbuffer", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};
static JSClass WebGLShader_class = {
    "WebGLShader", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};
static JSClass WebGLTexture_class = {
    "WebGLTexture", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};
static JSClass WebGLUniformLocation_class = {
    "WebGLUniformLocation", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSBool NativeGL_uniform_x_f(JSContext *cx, unsigned int argc, jsval *vp, int nb) 
{
    uintptr_t clocation;
    JSObject *location;
    jsval *argv = JS_ARGV(cx, vp);
    double x;
    double y;
    double z;
    double w;

    JS_ValueToObject(cx, argv[0], &location);
    clocation = (uintptr_t)JS_GetInstancePrivate(cx, location, &WebGLUniformLocation_class, JS_ARGV(cx, vp));
    
    if (nb > 0) JS_ValueToNumber(cx, argv[1], &x);
    if (nb > 1) JS_ValueToNumber(cx, argv[2], &y);
    if (nb > 2) JS_ValueToNumber(cx, argv[3], &z);
    if (nb > 3) JS_ValueToNumber(cx, argv[4], &w);

    switch (nb) {
        case 1:
            glUniform1f(clocation, (GLfloat)x);
            break;
        case 2:
            glUniform2f(clocation, (GLfloat)x, (GLfloat)y);
            break;
        case 3:
            glUniform3f(clocation, (GLfloat)x, (GLfloat)y, (GLfloat)z);
            break;
        case 4:
            glUniform4f(clocation, (GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w);
            break;
    }

    return JS_TRUE;
} 

static JSBool NativeGL_uniform_x_fv(JSContext *cx, unsigned int argc, jsval *vp, int nb) 
{
    intptr_t clocation;
    GLsizei length;
    GLfloat *carray;
    JSObject *array;
    JSObject *location;

    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oo", &location, &array)) {
        return JS_TRUE;
    }

    clocation = (intptr_t)JS_GetInstancePrivate(cx, location, &WebGLUniformLocation_class, JS_ARGV(cx, vp));

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
        return JS_TRUE;
    }

    switch (nb) {
        case 1:
            glUniform1fv(clocation, length, carray);
            break;
        case 2:
            glUniform2fv(clocation, length/2, carray);
            break;
        case 3:
            glUniform3fv(clocation, length/3, carray);
            break;
        case 4:
            glUniform4fv(clocation, length/4, carray);
            break;
    }

    return JS_TRUE;
} 

static JSBool NativeGL_uniform_x_i(JSContext *cx, unsigned int argc, jsval *vp, int nb) 
{
    uintptr_t clocation;
    GLint x;
    GLint y;
    GLint z;
    GLint w;
    JSObject *location;
    jsval *argv = JS_ARGV(cx, vp);

    JS_ValueToObject(cx, argv[0], &location);
    clocation = (uintptr_t)JS_GetInstancePrivate(cx, location, &WebGLUniformLocation_class, JS_ARGV(cx, vp));

    if (nb > 0) JS_ValueToInt32(cx, argv[1], &x);
    if (nb > 1) JS_ValueToInt32(cx, argv[2], &y);
    if (nb > 2) JS_ValueToInt32(cx, argv[3], &z);
    if (nb > 3) JS_ValueToInt32(cx, argv[4], &w);

    switch (nb) {
        case 1:
            glUniform1i(clocation, x);
            break;
        case 2:
            glUniform2i(clocation, x, y);
            break;
        case 3:
            glUniform3i(clocation, x, y, z);
            break;
        case 4:
            glUniform4i(clocation, x, y, z, w);
            break;
    }
    
    return JS_TRUE;
} 

static JSBool NativeGL_uniform_x_iv(JSContext *cx, unsigned int argc, jsval *vp, int nb) 
{
    uintptr_t clocation;
    GLsizei length;
    GLint *carray;
    JSObject *tmp;
    JSObject *array;
    JSObject *location;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oo", &location, &array)) {
        return JS_TRUE;
    }

    clocation = (uintptr_t)JS_GetInstancePrivate(cx, location, &WebGLUniformLocation_class, JS_ARGV(cx, vp));

    if (JS_IsInt32Array(array)) {
        carray = (GLint *)JS_GetInt32ArrayData(array);
        length = (GLsizei)JS_GetTypedArrayLength(array);
    } else if (JS_IsArrayObject(cx, array)) {
        tmp = JS_NewInt32ArrayFromArray(cx, array); 
        carray = (GLint *)JS_GetInt32ArrayData(tmp);
        length = (GLsizei)JS_GetTypedArrayLength(tmp);
    } else {
        JS_ReportError(cx, "Array is not a Int32 array");
        return JS_TRUE;
    }

    if (nb == 1) {
        glUniform1iv(clocation, length, carray);
    } else if (nb == 2) {
        glUniform2iv(clocation, length/2, carray);
    } else if (nb == 3) {
        glUniform3iv(clocation, length/3, carray);
    } else if (nb == 4) {
        glUniform4iv(clocation, length/4, carray);
    }
    
    return JS_TRUE;
} 

static JSBool NativeGL_uniformMatrix_x_fv(JSContext *cx, unsigned int argc, jsval *vp, int nb) 
{
    uintptr_t clocation;
    GLint length;
    GLfloat *carray;
    JSBool transpose;
    JSObject *tmp;
    JSObject *array;
    JSObject *location;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "obo", &location, &transpose, &array)) {
        return JS_TRUE;
    }

    clocation = (uintptr_t)JS_GetInstancePrivate(cx, location, &WebGLUniformLocation_class, JS_ARGV(cx, vp));
    if (JS_IsFloat32Array(array)) {
        carray = (GLfloat *)JS_GetFloat32ArrayData(array);
        length = (GLsizei)JS_GetTypedArrayLength(array);
    } else if (JS_IsArrayObject(cx, array)) {
        tmp = JS_NewFloat32ArrayFromArray(cx, array); 
        carray = (GLfloat *)JS_GetFloat32ArrayData(tmp);
        length = (GLsizei)JS_GetTypedArrayLength(tmp);
    } else {
        JS_ReportError(cx, "Array is not a Float32 array");
        return JS_TRUE;
    }
    
    switch (nb) {
        case 2:
            glUniformMatrix2fv(clocation, length/4, (GLboolean)transpose, carray);
            break;
        case 3:
            glUniformMatrix3fv(clocation, length/8, (GLboolean)transpose, carray);
            break;
        case 4:
            glUniformMatrix4fv(clocation, length/16, (GLboolean)transpose, carray);
            break;
    }
    
    return JS_TRUE;
}

static JSBool NativeGL_vertexAttrib_x_f(JSContext *cx, unsigned int argc, jsval *vp, int nb) 
{
    GLuint index;
    jsval *argv = JS_ARGV(cx, vp);
    double v0;
    double v1;
    double v2;
    double v3;

    JS_ValueToECMAUint32(cx, argv[0], &index);

    if (nb > 0) JS_ValueToNumber(cx, argv[1], &v0);
    if (nb > 1) JS_ValueToNumber(cx, argv[2], &v1);
    if (nb > 2) JS_ValueToNumber(cx, argv[3], &v2);
    if (nb > 3) JS_ValueToNumber(cx, argv[4], &v3);

    switch (nb) {
        case 1:
            glVertexAttrib1f(index, (GLfloat)v0);
            break;
        case 2:
            glVertexAttrib2f(index, (GLfloat)v0, (GLfloat)v1);
            break;
        case 3:
            glVertexAttrib3f(index, (GLfloat)v0, (GLfloat)v1, (GLfloat)v2);
            break;
        case 4:
            glVertexAttrib4f(index, (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, (GLfloat)v3);
            break;
    }

    return JS_TRUE;
}

static JSBool NativeGL_vertexAttrib_x_fv(JSContext *cx, unsigned int argc, jsval *vp, int nb) 
{
    GLuint index;
    GLfloat *carray;
    jsval *argv = JS_ARGV(cx, vp);
    JSObject *tmp;
    JSObject *array;

    JS_ValueToECMAUint32(cx, argv[0], &index);
    JS_ValueToObject(cx, argv[1], &array);

    if (JS_IsFloat32Array(array)) {
        carray = (GLfloat *)JS_GetFloat32ArrayData(array);
    } else if (JS_IsArrayObject(cx, array)) {
        tmp = JS_NewFloat32ArrayFromArray(cx, array); 
        carray = (GLfloat *)JS_GetFloat32ArrayData(tmp);
    } else {
        JS_ReportError(cx, "Array is not a Float32 array");
        return JS_TRUE;
    }

    switch (nb) {
        case 1:
            glVertexAttrib1fv(index, carray);
            break;
        case 2:
            glVertexAttrib2fv(index, carray);
            break;
        case 3:
            glVertexAttrib3fv(index, carray);
            break;
        case 4:
            glVertexAttrib4fv(index, carray);
            break;
    }

    return JS_TRUE;
}

D_NGL_JS_FN(WebGLRenderingContext_isContextLost)
D_NGL_JS_FN(WebGLRenderingContext_getExtension)
D_NGL_JS_FN(WebGLRenderingContext_activeTexture)
D_NGL_JS_FN(WebGLRenderingContext_attachShader)
D_NGL_JS_FN(WebGLRenderingContext_bindBuffer)
D_NGL_JS_FN(WebGLRenderingContext_bindRenderbuffer)
D_NGL_JS_FN(WebGLRenderingContext_bindFramebuffer)
D_NGL_JS_FN(WebGLRenderingContext_bindTexture)
D_NGL_JS_FN(WebGLRenderingContext_blendEquation)
D_NGL_JS_FN(WebGLRenderingContext_blendEquationSeparate)
D_NGL_JS_FN(WebGLRenderingContext_blendFunc)
D_NGL_JS_FN(WebGLRenderingContext_blendFuncSeparate)
D_NGL_JS_FN(WebGLRenderingContext_bufferData)
D_NGL_JS_FN(WebGLRenderingContext_clear)
D_NGL_JS_FN(WebGLRenderingContext_clearColor)
D_NGL_JS_FN(WebGLRenderingContext_clearDepth)
D_NGL_JS_FN(WebGLRenderingContext_clearStencil)
D_NGL_JS_FN(WebGLRenderingContext_compileShader)
D_NGL_JS_FN(WebGLRenderingContext_createBuffer)
D_NGL_JS_FN(WebGLRenderingContext_createFramebuffer)
D_NGL_JS_FN(WebGLRenderingContext_createRenderbuffer)
D_NGL_JS_FN(WebGLRenderingContext_createProgram)
D_NGL_JS_FN(WebGLRenderingContext_createShader)
D_NGL_JS_FN(WebGLRenderingContext_createTexture)
D_NGL_JS_FN(WebGLRenderingContext_cullFace)
D_NGL_JS_FN(WebGLRenderingContext_deleteShader)
D_NGL_JS_FN(WebGLRenderingContext_depthFunc)
D_NGL_JS_FN(WebGLRenderingContext_depthMask)
D_NGL_JS_FN(WebGLRenderingContext_disable)
D_NGL_JS_FN(WebGLRenderingContext_disableVertexAttribArray)
D_NGL_JS_FN(WebGLRenderingContext_drawArrays)
D_NGL_JS_FN(WebGLRenderingContext_drawElements)
D_NGL_JS_FN(WebGLRenderingContext_enable)
D_NGL_JS_FN(WebGLRenderingContext_enableVertexAttribArray)
D_NGL_JS_FN(WebGLRenderingContext_getUniformLocation)
D_NGL_JS_FN(WebGLRenderingContext_framebufferRenderbuffer)
D_NGL_JS_FN(WebGLRenderingContext_framebufferTexture2D)
D_NGL_JS_FN(WebGLRenderingContext_frontFace)
D_NGL_JS_FN(WebGLRenderingContext_generateMipmap)
D_NGL_JS_FN(WebGLRenderingContext_getAttribLocation)
D_NGL_JS_FN(WebGLRenderingContext_getParameter)
D_NGL_JS_FN(WebGLRenderingContext_getProgramParameter)
D_NGL_JS_FN(WebGLRenderingContext_getProgramInfoLog)
D_NGL_JS_FN(WebGLRenderingContext_getShaderParameter)
D_NGL_JS_FN(WebGLRenderingContext_getShaderInfoLog)
D_NGL_JS_FN(WebGLRenderingContext_lineWidth)
D_NGL_JS_FN(WebGLRenderingContext_linkProgram)
D_NGL_JS_FN(WebGLRenderingContext_pixelStorei)
D_NGL_JS_FN(WebGLRenderingContext_renderbufferStorage)
D_NGL_JS_FN(WebGLRenderingContext_shaderSource)
D_NGL_JS_FN(WebGLRenderingContext_texImage2D)
D_NGL_JS_FN(WebGLRenderingContext_texParameteri)
D_NGL_JS_FN(WebGLRenderingContext_uniform1f)
D_NGL_JS_FN(WebGLRenderingContext_uniform1fv)
D_NGL_JS_FN(WebGLRenderingContext_uniform1i)
D_NGL_JS_FN(WebGLRenderingContext_uniform1iv)
D_NGL_JS_FN(WebGLRenderingContext_uniform2f)
D_NGL_JS_FN(WebGLRenderingContext_uniform2fv)
D_NGL_JS_FN(WebGLRenderingContext_uniform2i)
D_NGL_JS_FN(WebGLRenderingContext_uniform2iv)
D_NGL_JS_FN(WebGLRenderingContext_uniform3f)
D_NGL_JS_FN(WebGLRenderingContext_uniform3fv)
D_NGL_JS_FN(WebGLRenderingContext_uniform3i)
D_NGL_JS_FN(WebGLRenderingContext_uniform3iv)
D_NGL_JS_FN(WebGLRenderingContext_uniform4f)
D_NGL_JS_FN(WebGLRenderingContext_uniform4fv)
D_NGL_JS_FN(WebGLRenderingContext_uniform4i)
D_NGL_JS_FN(WebGLRenderingContext_uniform4iv)
D_NGL_JS_FN(WebGLRenderingContext_uniformMatrix2fv)
D_NGL_JS_FN(WebGLRenderingContext_uniformMatrix3fv)
D_NGL_JS_FN(WebGLRenderingContext_uniformMatrix4fv)
D_NGL_JS_FN(WebGLRenderingContext_vertexAttrib1f);
D_NGL_JS_FN(WebGLRenderingContext_vertexAttrib1fv);
D_NGL_JS_FN(WebGLRenderingContext_vertexAttrib2f);
D_NGL_JS_FN(WebGLRenderingContext_vertexAttrib2fv);
D_NGL_JS_FN(WebGLRenderingContext_vertexAttrib3f);
D_NGL_JS_FN(WebGLRenderingContext_vertexAttrib3fv);
D_NGL_JS_FN(WebGLRenderingContext_vertexAttrib4f);
D_NGL_JS_FN(WebGLRenderingContext_vertexAttrib4fv);
D_NGL_JS_FN(WebGLRenderingContext_vertexAttribPointer);
D_NGL_JS_FN(WebGLRenderingContext_viewport)
D_NGL_JS_FN(WebGLRenderingContext_useProgram)
D_NGL_JS_FN(WebGLRenderingContext_getError)


static JSFunctionSpec WebGLRenderingContext_funcs [] = {
    JS_FS("isContextLost", WebGLRenderingContext_isContextLost, 0, 0),
    JS_FS("getExtension", WebGLRenderingContext_getExtension, 1, 0),
    JS_FS("activeTexture", WebGLRenderingContext_activeTexture, 1, 0),
    JS_FS("attachShader", WebGLRenderingContext_attachShader, 2, 0),
    JS_FS("bindBuffer", WebGLRenderingContext_bindBuffer, 2, 0),
    JS_FS("bindFramebuffer", WebGLRenderingContext_bindFramebuffer, 2, 0),
    JS_FS("bindRenderbuffer", WebGLRenderingContext_bindRenderbuffer, 2, 0),
    JS_FS("bindTexture", WebGLRenderingContext_bindTexture, 2, 0),
    JS_FS("blendEquation", WebGLRenderingContext_blendEquation, 1, 0),
    JS_FS("blendEquationSeparate", WebGLRenderingContext_blendEquationSeparate, 2, 0),
    JS_FS("blendFunc", WebGLRenderingContext_blendFunc, 2, 0),
    JS_FS("blendFuncSeparate", WebGLRenderingContext_blendFuncSeparate, 4, 0),
    JS_FS("bufferData", WebGLRenderingContext_bufferData, 3, 0),
    JS_FS("clear", WebGLRenderingContext_clear, 1, 0),
    JS_FS("clearColor", WebGLRenderingContext_clearColor, 4, 0),
    JS_FS("clearDepth", WebGLRenderingContext_clearDepth, 1, 0),
    JS_FS("clearStencil", WebGLRenderingContext_clearStencil, 1, 0),
    JS_FS("compileShader", WebGLRenderingContext_compileShader, 1, 0),
    JS_FS("texImage2D", WebGLRenderingContext_texImage2D, 6, 0),
    JS_FS("createBuffer", WebGLRenderingContext_createBuffer, 0, 0),
    JS_FS("createFramebuffer", WebGLRenderingContext_createFramebuffer, 0, 0),
    JS_FS("createRenderbuffer", WebGLRenderingContext_createRenderbuffer, 0, 0),
    JS_FS("createProgram", WebGLRenderingContext_createProgram, 0, 0),
    JS_FS("createShader", WebGLRenderingContext_createShader, 1, 0),
    JS_FS("createTexture", WebGLRenderingContext_createTexture, 0, 0),
    JS_FS("cullFace", WebGLRenderingContext_cullFace, 1, 0),
    JS_FS("deleteShader", WebGLRenderingContext_deleteShader, 1, 0),
    JS_FS("depthFunc", WebGLRenderingContext_depthFunc, 1, 0),
    JS_FS("depthMask", WebGLRenderingContext_depthMask, 1, 0),
    JS_FS("disable", WebGLRenderingContext_disable, 1, 0),
    JS_FS("disableVertexAttribArray", WebGLRenderingContext_disableVertexAttribArray, 1, 0),
    JS_FS("drawArrays", WebGLRenderingContext_drawArrays, 3, 0),
    JS_FS("drawElements", WebGLRenderingContext_drawElements, 4, 0),
    JS_FS("enable", WebGLRenderingContext_enable, 1, 0),
    JS_FS("enableVertexAttribArray", WebGLRenderingContext_enableVertexAttribArray, 1, 0),
    JS_FS("framebufferRenderbuffer", WebGLRenderingContext_framebufferRenderbuffer, 4, 0),
    JS_FS("framebufferTexture2D", WebGLRenderingContext_framebufferTexture2D, 5, 0),
    JS_FS("frontFace", WebGLRenderingContext_frontFace, 1, 0),
    JS_FS("generateMipmap", WebGLRenderingContext_generateMipmap, 1, 0),
    JS_FS("getAttribLocation", WebGLRenderingContext_getAttribLocation, 2, 0),
    JS_FS("getParameter", WebGLRenderingContext_getParameter, 1, 0),
    JS_FS("getProgramParameter", WebGLRenderingContext_getProgramParameter, 2, 0),
    JS_FS("getProgramInfoLog", WebGLRenderingContext_getProgramInfoLog, 1, 0),
    JS_FS("getShaderParameter", WebGLRenderingContext_getShaderParameter, 2, 0),
    JS_FS("getShaderInfoLog", WebGLRenderingContext_getShaderInfoLog, 1, 0),
    JS_FS("getUniformLocation", WebGLRenderingContext_getUniformLocation, 2, 0),
    JS_FS("lineWidth", WebGLRenderingContext_lineWidth, 1, 0),
    JS_FS("linkProgram", WebGLRenderingContext_linkProgram, 1, 0),
    JS_FS("pixelStorei", WebGLRenderingContext_pixelStorei, 2, 0),
    JS_FS("renderbufferStorage", WebGLRenderingContext_renderbufferStorage, 4, 0),
    JS_FS("shaderSource", WebGLRenderingContext_shaderSource, 2, 0),
    JS_FS("texParameteri", WebGLRenderingContext_texParameteri, 3, 0),
    JS_FS("uniform1f", WebGLRenderingContext_uniform1f, 2, 0),
    JS_FS("uniform1fv", WebGLRenderingContext_uniform1fv, 2, 0),
    JS_FS("uniform1i", WebGLRenderingContext_uniform1i, 2, 0),
    JS_FS("uniform1iv", WebGLRenderingContext_uniform1iv, 2, 0),
    JS_FS("uniform2f", WebGLRenderingContext_uniform2f, 2, 0),
    JS_FS("uniform2fv", WebGLRenderingContext_uniform2fv, 2, 0),
    JS_FS("uniform2i", WebGLRenderingContext_uniform2i, 2, 0),
    JS_FS("uniform2iv", WebGLRenderingContext_uniform2iv, 2, 0),
    JS_FS("uniform3f", WebGLRenderingContext_uniform3f, 2, 0),
    JS_FS("uniform3fv", WebGLRenderingContext_uniform3fv, 2, 0),
    JS_FS("uniform3i", WebGLRenderingContext_uniform3i, 2, 0),
    JS_FS("uniform3iv", WebGLRenderingContext_uniform3iv, 2, 0),
    JS_FS("uniform4f", WebGLRenderingContext_uniform4f, 2, 0),
    JS_FS("uniform4fv", WebGLRenderingContext_uniform4fv, 2, 0),
    JS_FS("uniform4i", WebGLRenderingContext_uniform4i, 2, 0),
    JS_FS("uniform4iv", WebGLRenderingContext_uniform4iv, 2, 0),
    JS_FS("uniformMatrix2fv", WebGLRenderingContext_uniformMatrix2fv, 3, 0),
    JS_FS("uniformMatrix3fv", WebGLRenderingContext_uniformMatrix3fv, 3, 0),
    JS_FS("uniformMatrix4fv", WebGLRenderingContext_uniformMatrix4fv, 3, 0),
    JS_FS("vertexAttrib1f", WebGLRenderingContext_vertexAttrib1f, 2, 0),
    JS_FS("vertexAttrib1fv", WebGLRenderingContext_vertexAttrib1fv, 2, 0),
    JS_FS("vertexAttrib2f", WebGLRenderingContext_vertexAttrib2f, 2, 0),
    JS_FS("vertexAttrib2fv", WebGLRenderingContext_vertexAttrib2fv, 2, 0),
    JS_FS("vertexAttrib3f", WebGLRenderingContext_vertexAttrib3f, 2, 0),
    JS_FS("vertexAttrib3fv", WebGLRenderingContext_vertexAttrib3fv, 2, 0),
    JS_FS("vertexAttrib4f", WebGLRenderingContext_vertexAttrib4f, 2, 0),
    JS_FS("vertexAttrib4fv", WebGLRenderingContext_vertexAttrib4fv, 2, 0),
    JS_FS("vertexAttribPointer", WebGLRenderingContext_vertexAttribPointer, 6, 0),
    JS_FS("viewport", WebGLRenderingContext_viewport, 4, 0),
    JS_FS("useProgram", WebGLRenderingContext_useProgram, 1, 0),
    JS_FS("getError", WebGLRenderingContext_getError, 0, 0),
    JS_FS_END
};

static JSConstDoubleSpec WebGLRenderingContext_const [] = {
    /*{GL_ES_VERSION_2_0, "ES_VERSION_2_0", 0, {0,0,0}},*/
    {GL_DEPTH_BUFFER_BIT, "DEPTH_BUFFER_BIT", 0, {0,0,0}},
    {GL_STENCIL_BUFFER_BIT, "STENCIL_BUFFER_BIT", 0, {0,0,0}},
    {GL_COLOR_BUFFER_BIT, "COLOR_BUFFER_BIT", 0, {0,0,0}},
    {GL_POINTS, "POINTS", 0, {0,0,0}},
    {GL_LINES, "LINES", 0, {0,0,0}},
    {GL_LINE_LOOP, "LINE_LOOP", 0, {0,0,0}},
    {GL_LINE_STRIP, "LINE_STRIP", 0, {0,0,0}},
    {GL_TRIANGLES, "TRIANGLES", 0, {0,0,0}},
    {GL_TRIANGLE_STRIP, "TRIANGLE_STRIP", 0, {0,0,0}},
    {GL_TRIANGLE_FAN, "TRIANGLE_FAN", 0, {0,0,0}},
    {GL_ZERO, "ZERO", 0, {0,0,0}},
    {GL_ONE, "ONE", 0, {0,0,0}},
    {GL_SRC_COLOR, "SRC_COLOR", 0, {0,0,0}},
    {GL_ONE_MINUS_SRC_COLOR, "ONE_MINUS_SRC_COLOR", 0, {0,0,0}},
    {GL_SRC_ALPHA, "SRC_ALPHA", 0, {0,0,0}},
    {GL_ONE_MINUS_SRC_ALPHA, "ONE_MINUS_SRC_ALPHA", 0, {0,0,0}},
    {GL_DST_ALPHA, "DST_ALPHA", 0, {0,0,0}},
    {GL_ONE_MINUS_DST_ALPHA, "ONE_MINUS_DST_ALPHA", 0, {0,0,0}},
    {GL_DST_COLOR, "DST_COLOR", 0, {0,0,0}},
    {GL_ONE_MINUS_DST_COLOR, "ONE_MINUS_DST_COLOR", 0, {0,0,0}},
    {GL_SRC_ALPHA_SATURATE, "SRC_ALPHA_SATURATE", 0, {0,0,0}},
    {GL_FUNC_ADD, "FUNC_ADD", 0, {0,0,0}},
    {GL_BLEND_EQUATION, "BLEND_EQUATION", 0, {0,0,0}},
    {GL_BLEND_EQUATION_RGB, "BLEND_EQUATION_RGB", 0, {0,0,0}},
    {GL_BLEND_EQUATION_ALPHA, "BLEND_EQUATION_ALPHA", 0, {0,0,0}},
    {GL_FUNC_SUBTRACT, "FUNC_SUBTRACT", 0, {0,0,0}},
    {GL_FUNC_REVERSE_SUBTRACT, "FUNC_REVERSE_SUBTRACT", 0, {0,0,0}},
    {GL_BLEND_DST_RGB, "BLEND_DST_RGB", 0, {0,0,0}},
    {GL_BLEND_SRC_RGB, "BLEND_SRC_RGB", 0, {0,0,0}},
    {GL_BLEND_DST_ALPHA, "BLEND_DST_ALPHA", 0, {0,0,0}},
    {GL_BLEND_SRC_ALPHA, "BLEND_SRC_ALPHA", 0, {0,0,0}},
    {GL_CONSTANT_COLOR, "CONSTANT_COLOR", 0, {0,0,0}},
    {GL_ONE_MINUS_CONSTANT_COLOR, "ONE_MINUS_CONSTANT_COLOR", 0, {0,0,0}},
    {GL_CONSTANT_ALPHA, "CONSTANT_ALPHA", 0, {0,0,0}},
    {GL_ONE_MINUS_CONSTANT_ALPHA, "ONE_MINUS_CONSTANT_ALPHA", 0, {0,0,0}},
    {GL_BLEND_COLOR, "BLEND_COLOR", 0, {0,0,0}},
    {GL_ARRAY_BUFFER, "ARRAY_BUFFER", 0, {0,0,0}},
    {GL_ELEMENT_ARRAY_BUFFER, "ELEMENT_ARRAY_BUFFER", 0, {0,0,0}},
    {GL_ARRAY_BUFFER_BINDING, "ARRAY_BUFFER_BINDING", 0, {0,0,0}},
    {GL_ELEMENT_ARRAY_BUFFER_BINDING, "ELEMENT_ARRAY_BUFFER_BINDING", 0, {0,0,0}},
    {GL_STREAM_DRAW, "STREAM_DRAW", 0, {0,0,0}},
    {GL_STATIC_DRAW, "STATIC_DRAW", 0, {0,0,0}},
    {GL_DYNAMIC_DRAW, "DYNAMIC_DRAW", 0, {0,0,0}},
    {GL_BUFFER_SIZE, "BUFFER_SIZE", 0, {0,0,0}},
    {GL_BUFFER_USAGE, "BUFFER_USAGE", 0, {0,0,0}},
    {GL_CURRENT_VERTEX_ATTRIB, "CURRENT_VERTEX_ATTRIB", 0, {0,0,0}},
    {GL_FRONT, "FRONT", 0, {0,0,0}},
    {GL_BACK, "BACK", 0, {0,0,0}},
    {GL_FRONT_AND_BACK, "FRONT_AND_BACK", 0, {0,0,0}},
    {GL_TEXTURE_2D, "TEXTURE_2D", 0, {0,0,0}},
    {GL_CULL_FACE, "CULL_FACE", 0, {0,0,0}},
    {GL_BLEND, "BLEND", 0, {0,0,0}},
    {GL_DITHER, "DITHER", 0, {0,0,0}},
    {GL_STENCIL_TEST, "STENCIL_TEST", 0, {0,0,0}},
    {GL_DEPTH_TEST, "DEPTH_TEST", 0, {0,0,0}},
    {GL_SCISSOR_TEST, "SCISSOR_TEST", 0, {0,0,0}},
    {GL_POLYGON_OFFSET_FILL, "POLYGON_OFFSET_FILL", 0, {0,0,0}},
    {GL_SAMPLE_ALPHA_TO_COVERAGE, "SAMPLE_ALPHA_TO_COVERAGE", 0, {0,0,0}},
    {GL_SAMPLE_COVERAGE, "SAMPLE_COVERAGE", 0, {0,0,0}},
    {GL_NO_ERROR, "NO_ERROR", 0, {0,0,0}},
    {GL_INVALID_ENUM, "INVALID_ENUM", 0, {0,0,0}},
    {GL_INVALID_VALUE, "INVALID_VALUE", 0, {0,0,0}},
    {GL_INVALID_OPERATION, "INVALID_OPERATION", 0, {0,0,0}},
    {GL_OUT_OF_MEMORY, "OUT_OF_MEMORY", 0, {0,0,0}},
    {GL_CW, "CW", 0, {0,0,0}},
    {GL_CCW, "CCW", 0, {0,0,0}},
    {GL_LINE_WIDTH, "LINE_WIDTH", 0, {0,0,0}},
    //{GL_ALIASED_POINT_SIZE_RANGE, "ALIASED_POINT_SIZE_RANGE", 0, {0,0,0}},
    //{GL_ALIASED_LINE_WIDTH_RANGE, "ALIASED_LINE_WIDTH_RANGE", 0, {0,0,0}},
    {GL_CULL_FACE_MODE, "CULL_FACE_MODE", 0, {0,0,0}},
    {GL_FRONT_FACE, "FRONT_FACE", 0, {0,0,0}},
    {GL_DEPTH_RANGE, "DEPTH_RANGE", 0, {0,0,0}},
    {GL_DEPTH_WRITEMASK, "DEPTH_WRITEMASK", 0, {0,0,0}},
    {GL_DEPTH_CLEAR_VALUE, "DEPTH_CLEAR_VALUE", 0, {0,0,0}},
    {GL_DEPTH_FUNC, "DEPTH_FUNC", 0, {0,0,0}},
    {GL_STENCIL_CLEAR_VALUE, "STENCIL_CLEAR_VALUE", 0, {0,0,0}},
    {GL_STENCIL_FUNC, "STENCIL_FUNC", 0, {0,0,0}},
    {GL_STENCIL_FAIL, "STENCIL_FAIL", 0, {0,0,0}},
    {GL_STENCIL_PASS_DEPTH_FAIL, "STENCIL_PASS_DEPTH_FAIL", 0, {0,0,0}},
    {GL_STENCIL_PASS_DEPTH_PASS, "STENCIL_PASS_DEPTH_PASS", 0, {0,0,0}},
    {GL_STENCIL_REF, "STENCIL_REF", 0, {0,0,0}},
    {GL_STENCIL_VALUE_MASK, "STENCIL_VALUE_MASK", 0, {0,0,0}},
    {GL_STENCIL_WRITEMASK, "STENCIL_WRITEMASK", 0, {0,0,0}},
    {GL_STENCIL_BACK_FUNC, "STENCIL_BACK_FUNC", 0, {0,0,0}},
    {GL_STENCIL_BACK_FAIL, "STENCIL_BACK_FAIL", 0, {0,0,0}},
    {GL_STENCIL_BACK_PASS_DEPTH_FAIL, "STENCIL_BACK_PASS_DEPTH_FAIL", 0, {0,0,0}},
    {GL_STENCIL_BACK_PASS_DEPTH_PASS, "STENCIL_BACK_PASS_DEPTH_PASS", 0, {0,0,0}},
    {GL_STENCIL_BACK_REF, "STENCIL_BACK_REF", 0, {0,0,0}},
    {GL_STENCIL_BACK_VALUE_MASK, "STENCIL_BACK_VALUE_MASK", 0, {0,0,0}},
    {GL_STENCIL_BACK_WRITEMASK, "STENCIL_BACK_WRITEMASK", 0, {0,0,0}},
    {GL_VIEWPORT, "VIEWPORT", 0, {0,0,0}},
    {GL_SCISSOR_BOX, "SCISSOR_BOX", 0, {0,0,0}},
    {GL_COLOR_CLEAR_VALUE, "COLOR_CLEAR_VALUE", 0, {0,0,0}},
    {GL_COLOR_WRITEMASK, "COLOR_WRITEMASK", 0, {0,0,0}},
    {GL_UNPACK_ALIGNMENT, "UNPACK_ALIGNMENT", 0, {0,0,0}},
    {GL_PACK_ALIGNMENT, "PACK_ALIGNMENT", 0, {0,0,0}},
    {GL_MAX_TEXTURE_SIZE, "MAX_TEXTURE_SIZE", 0, {0,0,0}},
    {GL_MAX_VIEWPORT_DIMS, "MAX_VIEWPORT_DIMS", 0, {0,0,0}},
    {GL_SUBPIXEL_BITS, "SUBPIXEL_BITS", 0, {0,0,0}},
    //{GL_RED_BITS, "RED_BITS", 0, {0,0,0}},
    //{GL_GREEN_BITS, "GREEN_BITS", 0, {0,0,0}},
    //{GL_BLUE_BITS, "BLUE_BITS", 0, {0,0,0}},
    //{GL_ALPHA_BITS, "ALPHA_BITS", 0, {0,0,0}},
    //{GL_DEPTH_BITS, "DEPTH_BITS", 0, {0,0,0}},
    //{GL_STENCIL_BITS, "STENCIL_BITS", 0, {0,0,0}},
    {GL_POLYGON_OFFSET_UNITS, "POLYGON_OFFSET_UNITS", 0, {0,0,0}},
    {GL_POLYGON_OFFSET_FACTOR, "POLYGON_OFFSET_FACTOR", 0, {0,0,0}},
    {GL_TEXTURE_BINDING_2D, "TEXTURE_BINDING_2D", 0, {0,0,0}},
    {GL_SAMPLE_BUFFERS, "SAMPLE_BUFFERS", 0, {0,0,0}},
    {GL_SAMPLES, "SAMPLES", 0, {0,0,0}},
    {GL_SAMPLE_COVERAGE_VALUE, "SAMPLE_COVERAGE_VALUE", 0, {0,0,0}},
    {GL_SAMPLE_COVERAGE_INVERT, "SAMPLE_COVERAGE_INVERT", 0, {0,0,0}},
    {GL_NUM_COMPRESSED_TEXTURE_FORMATS, "NUM_COMPRESSED_TEXTURE_FORMATS", 0, {0,0,0}},
    {GL_COMPRESSED_TEXTURE_FORMATS, "COMPRESSED_TEXTURE_FORMATS", 0, {0,0,0}},
    {GL_DONT_CARE, "DONT_CARE", 0, {0,0,0}},
    {GL_FASTEST, "FASTEST", 0, {0,0,0}},
    {GL_NICEST, "NICEST", 0, {0,0,0}},
    //{GL_GENERATE_MIPMAP_HINT, "GENERATE_MIPMAP_HINT", 0, {0,0,0}},
    {GL_BYTE, "BYTE", 0, {0,0,0}},
    {GL_UNSIGNED_BYTE, "UNSIGNED_BYTE", 0, {0,0,0}},
    {GL_SHORT, "SHORT", 0, {0,0,0}},
    {GL_UNSIGNED_SHORT, "UNSIGNED_SHORT", 0, {0,0,0}},
    {GL_INT, "INT", 0, {0,0,0}},
    {GL_UNSIGNED_INT, "UNSIGNED_INT", 0, {0,0,0}},
    {GL_FLOAT, "FLOAT", 0, {0,0,0}},
    //{GL_FIXED, "FIXED", 0, {0,0,0}},
    {GL_DEPTH_COMPONENT, "DEPTH_COMPONENT", 0, {0,0,0}},
    {GL_ALPHA, "ALPHA", 0, {0,0,0}},
    {GL_RGB, "RGB", 0, {0,0,0}},
    {GL_RGBA, "RGBA", 0, {0,0,0}},
    //{GL_LUMINANCE, "LUMINANCE", 0, {0,0,0}},
    //{GL_LUMINANCE_ALPHA, "LUMINANCE_ALPHA", 0, {0,0,0}},
    {GL_UNSIGNED_SHORT_4_4_4_4, "UNSIGNED_SHORT_4_4_4_4", 0, {0,0,0}},
    {GL_UNSIGNED_SHORT_5_5_5_1, "UNSIGNED_SHORT_5_5_5_1", 0, {0,0,0}},
    {GL_UNSIGNED_SHORT_5_6_5, "UNSIGNED_SHORT_5_6_5", 0, {0,0,0}},
    {GL_FRAGMENT_SHADER, "FRAGMENT_SHADER", 0, {0,0,0}},
    {GL_VERTEX_SHADER, "VERTEX_SHADER", 0, {0,0,0}},
    {GL_MAX_VERTEX_ATTRIBS, "MAX_VERTEX_ATTRIBS", 0, {0,0,0}},
    //{GL_MAX_VERTEX_UNIFORM_VECTORS, "MAX_VERTEX_UNIFORM_VECTORS", 0, {0,0,0}},
    //{GL_MAX_VARYING_VECTORS, "MAX_VARYING_VECTORS", 0, {0,0,0}},
    {GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, "MAX_COMBINED_TEXTURE_IMAGE_UNITS", 0, {0,0,0}},
    {GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, "MAX_VERTEX_TEXTURE_IMAGE_UNITS", 0, {0,0,0}},
    {GL_MAX_TEXTURE_IMAGE_UNITS, "MAX_TEXTURE_IMAGE_UNITS", 0, {0,0,0}},
    //{GL_MAX_FRAGMENT_UNIFORM_VECTORS, "MAX_FRAGMENT_UNIFORM_VECTORS", 0, {0,0,0}},
    {GL_SHADER_TYPE, "SHADER_TYPE", 0, {0,0,0}},
    {GL_DELETE_STATUS, "DELETE_STATUS", 0, {0,0,0}},
    {GL_LINK_STATUS, "LINK_STATUS", 0, {0,0,0}},
    {GL_VALIDATE_STATUS, "VALIDATE_STATUS", 0, {0,0,0}},
    {GL_ATTACHED_SHADERS, "ATTACHED_SHADERS", 0, {0,0,0}},
    {GL_ACTIVE_UNIFORMS, "ACTIVE_UNIFORMS", 0, {0,0,0}},
    {GL_ACTIVE_UNIFORM_MAX_LENGTH, "ACTIVE_UNIFORM_MAX_LENGTH", 0, {0,0,0}},
    {GL_ACTIVE_ATTRIBUTES, "ACTIVE_ATTRIBUTES", 0, {0,0,0}},
    {GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, "ACTIVE_ATTRIBUTE_MAX_LENGTH", 0, {0,0,0}},
    {GL_SHADING_LANGUAGE_VERSION, "SHADING_LANGUAGE_VERSION", 0, {0,0,0}},
    {GL_CURRENT_PROGRAM, "CURRENT_PROGRAM", 0, {0,0,0}},
    {GL_NEVER, "NEVER", 0, {0,0,0}},
    {GL_LESS, "LESS", 0, {0,0,0}},
    {GL_EQUAL, "EQUAL", 0, {0,0,0}},
    {GL_LEQUAL, "LEQUAL", 0, {0,0,0}},
    {GL_GREATER, "GREATER", 0, {0,0,0}},
    {GL_NOTEQUAL, "NOTEQUAL", 0, {0,0,0}},
    {GL_GEQUAL, "GEQUAL", 0, {0,0,0}},
    {GL_ALWAYS, "ALWAYS", 0, {0,0,0}},
    {GL_KEEP, "KEEP", 0, {0,0,0}},
    {GL_REPLACE, "REPLACE", 0, {0,0,0}},
    {GL_INCR, "INCR", 0, {0,0,0}},
    {GL_DECR, "DECR", 0, {0,0,0}},
    {GL_INVERT, "INVERT", 0, {0,0,0}},
    {GL_INCR_WRAP, "INCR_WRAP", 0, {0,0,0}},
    {GL_DECR_WRAP, "DECR_WRAP", 0, {0,0,0}},
    {GL_VENDOR, "VENDOR", 0, {0,0,0}},
    {GL_RENDERER, "RENDERER", 0, {0,0,0}},
    {GL_VERSION, "VERSION", 0, {0,0,0}},
    {GL_EXTENSIONS, "EXTENSIONS", 0, {0,0,0}},
    {GL_NEAREST, "NEAREST", 0, {0,0,0}},
    {GL_LINEAR, "LINEAR", 0, {0,0,0}},
    {GL_NEAREST_MIPMAP_NEAREST, "NEAREST_MIPMAP_NEAREST", 0, {0,0,0}},
    {GL_LINEAR_MIPMAP_NEAREST, "LINEAR_MIPMAP_NEAREST", 0, {0,0,0}},
    {GL_NEAREST_MIPMAP_LINEAR, "NEAREST_MIPMAP_LINEAR", 0, {0,0,0}},
    {GL_LINEAR_MIPMAP_LINEAR, "LINEAR_MIPMAP_LINEAR", 0, {0,0,0}},
    {GL_TEXTURE_MAG_FILTER, "TEXTURE_MAG_FILTER", 0, {0,0,0}},
    {GL_TEXTURE_MIN_FILTER, "TEXTURE_MIN_FILTER", 0, {0,0,0}},
    {GL_TEXTURE_WRAP_S, "TEXTURE_WRAP_S", 0, {0,0,0}},
    {GL_TEXTURE_WRAP_T, "TEXTURE_WRAP_T", 0, {0,0,0}},
    {GL_TEXTURE, "TEXTURE", 0, {0,0,0}},
    {GL_TEXTURE_CUBE_MAP, "TEXTURE_CUBE_MAP", 0, {0,0,0}},
    {GL_TEXTURE_BINDING_CUBE_MAP, "TEXTURE_BINDING_CUBE_MAP", 0, {0,0,0}},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_X, "TEXTURE_CUBE_MAP_POSITIVE_X", 0, {0,0,0}},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "TEXTURE_CUBE_MAP_NEGATIVE_X", 0, {0,0,0}},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "TEXTURE_CUBE_MAP_POSITIVE_Y", 0, {0,0,0}},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "TEXTURE_CUBE_MAP_NEGATIVE_Y", 0, {0,0,0}},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "TEXTURE_CUBE_MAP_POSITIVE_Z", 0, {0,0,0}},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "TEXTURE_CUBE_MAP_NEGATIVE_Z", 0, {0,0,0}},
    {GL_MAX_CUBE_MAP_TEXTURE_SIZE, "MAX_CUBE_MAP_TEXTURE_SIZE", 0, {0,0,0}},
    {GL_TEXTURE0, "TEXTURE0", 0, {0,0,0}},
    {GL_TEXTURE1, "TEXTURE1", 0, {0,0,0}},
    {GL_TEXTURE2, "TEXTURE2", 0, {0,0,0}},
    {GL_TEXTURE3, "TEXTURE3", 0, {0,0,0}},
    {GL_TEXTURE4, "TEXTURE4", 0, {0,0,0}},
    {GL_TEXTURE5, "TEXTURE5", 0, {0,0,0}},
    {GL_TEXTURE6, "TEXTURE6", 0, {0,0,0}},
    {GL_TEXTURE7, "TEXTURE7", 0, {0,0,0}},
    {GL_TEXTURE8, "TEXTURE8", 0, {0,0,0}},
    {GL_TEXTURE9, "TEXTURE9", 0, {0,0,0}},
    {GL_TEXTURE10, "TEXTURE10", 0, {0,0,0}},
    {GL_TEXTURE11, "TEXTURE11", 0, {0,0,0}},
    {GL_TEXTURE12, "TEXTURE12", 0, {0,0,0}},
    {GL_TEXTURE13, "TEXTURE13", 0, {0,0,0}},
    {GL_TEXTURE14, "TEXTURE14", 0, {0,0,0}},
    {GL_TEXTURE15, "TEXTURE15", 0, {0,0,0}},
    {GL_TEXTURE16, "TEXTURE16", 0, {0,0,0}},
    {GL_TEXTURE17, "TEXTURE17", 0, {0,0,0}},
    {GL_TEXTURE18, "TEXTURE18", 0, {0,0,0}},
    {GL_TEXTURE19, "TEXTURE19", 0, {0,0,0}},
    {GL_TEXTURE20, "TEXTURE20", 0, {0,0,0}},
    {GL_TEXTURE21, "TEXTURE21", 0, {0,0,0}},
    {GL_TEXTURE22, "TEXTURE22", 0, {0,0,0}},
    {GL_TEXTURE23, "TEXTURE23", 0, {0,0,0}},
    {GL_TEXTURE24, "TEXTURE24", 0, {0,0,0}},
    {GL_TEXTURE25, "TEXTURE25", 0, {0,0,0}},
    {GL_TEXTURE26, "TEXTURE26", 0, {0,0,0}},
    {GL_TEXTURE27, "TEXTURE27", 0, {0,0,0}},
    {GL_TEXTURE28, "TEXTURE28", 0, {0,0,0}},
    {GL_TEXTURE29, "TEXTURE29", 0, {0,0,0}},
    {GL_TEXTURE30, "TEXTURE30", 0, {0,0,0}},
    {GL_TEXTURE31, "TEXTURE31", 0, {0,0,0}},
    {GL_ACTIVE_TEXTURE, "ACTIVE_TEXTURE", 0, {0,0,0}},
    {GL_REPEAT, "REPEAT", 0, {0,0,0}},
    {GL_CLAMP_TO_EDGE, "CLAMP_TO_EDGE", 0, {0,0,0}},
    {GL_MIRRORED_REPEAT, "MIRRORED_REPEAT", 0, {0,0,0}},
    {GL_FLOAT_VEC2, "FLOAT_VEC2", 0, {0,0,0}},
    {GL_FLOAT_VEC3, "FLOAT_VEC3", 0, {0,0,0}},
    {GL_FLOAT_VEC4, "FLOAT_VEC4", 0, {0,0,0}},
    {GL_INT_VEC2, "INT_VEC2", 0, {0,0,0}},
    {GL_INT_VEC3, "INT_VEC3", 0, {0,0,0}},
    {GL_INT_VEC4, "INT_VEC4", 0, {0,0,0}},
    {GL_BOOL, "BOOL", 0, {0,0,0}},
    {GL_BOOL_VEC2, "BOOL_VEC2", 0, {0,0,0}},
    {GL_BOOL_VEC3, "BOOL_VEC3", 0, {0,0,0}},
    {GL_BOOL_VEC4, "BOOL_VEC4", 0, {0,0,0}},
    {GL_FLOAT_MAT2, "FLOAT_MAT2", 0, {0,0,0}},
    {GL_FLOAT_MAT3, "FLOAT_MAT3", 0, {0,0,0}},
    {GL_FLOAT_MAT4, "FLOAT_MAT4", 0, {0,0,0}},
    {GL_SAMPLER_2D, "SAMPLER_2D", 0, {0,0,0}},
    {GL_SAMPLER_CUBE, "SAMPLER_CUBE", 0, {0,0,0}},
    {GL_VERTEX_ATTRIB_ARRAY_ENABLED, "VERTEX_ATTRIB_ARRAY_ENABLED", 0, {0,0,0}},
    {GL_VERTEX_ATTRIB_ARRAY_SIZE, "VERTEX_ATTRIB_ARRAY_SIZE", 0, {0,0,0}},
    {GL_VERTEX_ATTRIB_ARRAY_STRIDE, "VERTEX_ATTRIB_ARRAY_STRIDE", 0, {0,0,0}},
    {GL_VERTEX_ATTRIB_ARRAY_TYPE, "VERTEX_ATTRIB_ARRAY_TYPE", 0, {0,0,0}},
    {GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, "VERTEX_ATTRIB_ARRAY_NORMALIZED", 0, {0,0,0}},
    {GL_VERTEX_ATTRIB_ARRAY_POINTER, "VERTEX_ATTRIB_ARRAY_POINTER", 0, {0,0,0}},
    {GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, "VERTEX_ATTRIB_ARRAY_BUFFER_BINDING", 0, {0,0,0}},
    //{GL_IMPLEMENTATION_COLOR_READ_TYPE, "IMPLEMENTATION_COLOR_READ_TYPE", 0, {0,0,0}},
    //{GL_IMPLEMENTATION_COLOR_READ_FORMAT, "IMPLEMENTATION_COLOR_READ_FORMAT", 0, {0,0,0}},
    {GL_COMPILE_STATUS, "COMPILE_STATUS", 0, {0,0,0}},
    {GL_INFO_LOG_LENGTH, "INFO_LOG_LENGTH", 0, {0,0,0}},
    {GL_SHADER_SOURCE_LENGTH, "SHADER_SOURCE_LENGTH", 0, {0,0,0}},
    //{GL_SHADER_COMPILER, "SHADER_COMPILER", 0, {0,0,0}},
    /*{GL_SHADER_BINARY_FORMATS, "SHADER_BINARY_FORMATS", 0, {0,0,0}},*/
    //{GL_NUM_SHADER_BINARY_FORMATS, "NUM_SHADER_BINARY_FORMATS", 0, {0,0,0}},
    //{GL_LOW_FLOAT, "LOW_FLOAT", 0, {0,0,0}},
    //{GL_MEDIUM_FLOAT, "MEDIUM_FLOAT", 0, {0,0,0}},
    //{GL_HIGH_FLOAT, "HIGH_FLOAT", 0, {0,0,0}},
    //{GL_LOW_INT, "LOW_INT", 0, {0,0,0}},
    //{GL_MEDIUM_INT, "MEDIUM_INT", 0, {0,0,0}},
    //{GL_HIGH_INT, "HIGH_INT", 0, {0,0,0}},
    {GL_FRAMEBUFFER, "FRAMEBUFFER", 0, {0,0,0}},
    {GL_RENDERBUFFER, "RENDERBUFFER", 0, {0,0,0}},
    /*{GL_RGBA4, "RGBA4", 0, {0,0,0}},
    {GL_RGB5_A1, "RGB5_A1", 0, {0,0,0}},
    {GL_RGB565, "RGB565", 0, {0,0,0}},*/
    {GL_DEPTH_COMPONENT16, "DEPTH_COMPONENT16", 0, {0,0,0}},
    {GL_STENCIL_INDEX, "STENCIL_INDEX", 0, {0,0,0}},
    {GL_STENCIL_INDEX8, "STENCIL_INDEX8", 0, {0,0,0}},
    {GL_RENDERBUFFER_WIDTH, "RENDERBUFFER_WIDTH", 0, {0,0,0}},
    {GL_RENDERBUFFER_HEIGHT, "RENDERBUFFER_HEIGHT", 0, {0,0,0}},
    {GL_RENDERBUFFER_INTERNAL_FORMAT, "RENDERBUFFER_INTERNAL_FORMAT", 0, {0,0,0}},
    {GL_RENDERBUFFER_RED_SIZE, "RENDERBUFFER_RED_SIZE", 0, {0,0,0}},
    {GL_RENDERBUFFER_GREEN_SIZE, "RENDERBUFFER_GREEN_SIZE", 0, {0,0,0}},
    {GL_RENDERBUFFER_BLUE_SIZE, "RENDERBUFFER_BLUE_SIZE", 0, {0,0,0}},
    {GL_RENDERBUFFER_ALPHA_SIZE, "RENDERBUFFER_ALPHA_SIZE", 0, {0,0,0}},
    {GL_RENDERBUFFER_DEPTH_SIZE, "RENDERBUFFER_DEPTH_SIZE", 0, {0,0,0}},
    {GL_RENDERBUFFER_STENCIL_SIZE, "RENDERBUFFER_STENCIL_SIZE", 0, {0,0,0}},
    {GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, "FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE", 0, {0,0,0}},
    {GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, "FRAMEBUFFER_ATTACHMENT_OBJECT_NAME", 0, {0,0,0}},
    {GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, "FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL", 0, {0,0,0}},
    {GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE, "FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE", 0, {0,0,0}},
    {GL_COLOR_ATTACHMENT0, "COLOR_ATTACHMENT0", 0, {0,0,0}},
    {GL_DEPTH_ATTACHMENT, "DEPTH_ATTACHMENT", 0, {0,0,0}},
    {GL_STENCIL_ATTACHMENT, "STENCIL_ATTACHMENT", 0, {0,0,0}},
    {GL_NONE, "NONE", 0, {0,0,0}},
    {GL_FRAMEBUFFER_COMPLETE, "FRAMEBUFFER_COMPLETE", 0, {0,0,0}},
    {GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT, "FRAMEBUFFER_INCOMPLETE_ATTACHMENT", 0, {0,0,0}},
    {GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT, "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT", 0, {0,0,0}},
    //{GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT, "FRAMEBUFFER_INCOMPLETE_DIMENSIONS", 0, {0,0,0}},
    {GL_FRAMEBUFFER_UNSUPPORTED, "FRAMEBUFFER_UNSUPPORTED", 0, {0,0,0}},
    {GL_FRAMEBUFFER_BINDING, "FRAMEBUFFER_BINDING", 0, {0,0,0}},
    {GL_RENDERBUFFER_BINDING, "RENDERBUFFER_BINDING", 0, {0,0,0}},
    {GL_MAX_RENDERBUFFER_SIZE, "MAX_RENDERBUFFER_SIZE", 0, {0,0,0}},
    {GL_INVALID_FRAMEBUFFER_OPERATION, "INVALID_FRAMEBUFFER_OPERATION", 0, {0,0,0}},
    {NGL_UNPACK_FLIP_Y_WEBGL, "UNPACK_FLIP_Y_WEBGL", 0, {0,0,0}},
    {NGL_UNPACK_PREMULTIPLY_ALPHA_WEBGL, "UNPACK_PREMULTIPLY_ALPHA_WEBGL", 0, {0,0,0}},
    {NGL_CONTEXT_LOST_WEBGL, "CONTEXT_LOST_WEBGL", 0, {0,0,0}},
    {NGL_UNPACK_COLORSPACE_CONVERSION_WEBGL, "UNPACK_COLORSPACE_CONVERSION_WEBGL", 0, {0,0,0}},
    {NGL_BROWSER_DEFAULT_WEBGL, "BROWSER_DEFAULT_WEBGL", 0, {0,0,0}},
    {0, NULL, 0, {0,0,0}}
};

NGL_JS_FN(WebGLRenderingContext_isContextLost) 
{
    // TODO
    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(false));
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_getExtension) 
{
    JS_SET_RVAL(cx, vp, JSVAL_NULL);
    return JS_TRUE;
}


NGL_JS_FN(WebGLRenderingContext_activeTexture) 
{
	GLuint texture;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &texture)) {
        return JS_TRUE;
    }

    glActiveTexture(texture);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_attachShader)  
{
    uintptr_t cprogram;
    uintptr_t cshader;
    JSObject *program;
    JSObject *shader;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oo", &program, &shader)) {
        return JS_TRUE;
    }

    cprogram = (uintptr_t)JS_GetInstancePrivate(cx, program, &WebGLProgram_class, JS_ARGV(cx, vp));
    cshader = (uintptr_t)JS_GetInstancePrivate(cx, shader, &WebGLShader_class, JS_ARGV(cx, vp));

    glAttachShader(cprogram, cshader);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_bindBuffer) 
{
    GLenum target;
    uintptr_t cbuffer;
    JSObject *buffer;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uo", &target, &buffer)) {
        return JS_TRUE;
    }

	if (buffer == NULL) {
        printf("nullbuffer\n");
        return JS_TRUE;
    }

    cbuffer = (uintptr_t)JS_GetInstancePrivate(cx, buffer, &WebGLBuffer_class, JS_ARGV(cx, vp));

    glBindBuffer(target, cbuffer);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_bindFramebuffer) 
{
    GLenum target;
    uintptr_t cbuffer;
    JSObject *buffer;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uo", &target, &buffer)) {
        return JS_TRUE;
    }

	if (buffer == NULL) return JS_TRUE;

    cbuffer = (uintptr_t)JS_GetInstancePrivate(cx, buffer, &WebGLFrameBuffer_class, JS_ARGV(cx, vp));

    glBindFramebuffer(target, cbuffer);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_bindRenderbuffer) 
{
    GLenum target;
    uintptr_t cbuffer;
    JSObject *buffer;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uo", &target, &buffer)) {
        return JS_TRUE;
    }

	if (buffer == NULL) return JS_TRUE;

    cbuffer = (uintptr_t)JS_GetInstancePrivate(cx, buffer, &WebGLRenderbuffer_class, JS_ARGV(cx, vp));

    glBindRenderbuffer(target, cbuffer);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_bindTexture) 
{
    GLenum target;
    uintptr_t ctexture;
    JSObject *texture;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uo", &target, &texture)) {
        return JS_TRUE;
    }

	// No argv passed, as texture starting at 0
    ctexture = (uintptr_t)JS_GetInstancePrivate(cx, texture, &WebGLTexture_class, NULL);
    glBindTexture(target, ctexture);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_blendEquation) 
{
    GLuint mode;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &mode)) {
        return JS_TRUE;
    }
    
    glBlendEquation(mode);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_blendEquationSeparate)     
{
    GLuint modeRGB;
    GLenum modeAlpha;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uu", &modeRGB, &modeAlpha)) {
        return JS_TRUE;
    }
    
    glBlendEquationSeparate(modeRGB, modeAlpha);

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_blendFunc) 
{
    GLenum sfactor;
    GLuint dfactor;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uu", &sfactor, &dfactor)) {
        return JS_TRUE;
    }
    
    glBlendFunc(sfactor, dfactor);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_blendFuncSeparate) 
{
    GLuint srcRGB;
    GLuint dstRGB;
    GLenum srcAlpha;
    GLenum dstAlpha;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuuu", &srcRGB, &dstRGB, &srcAlpha, &dstAlpha)) {
        return JS_TRUE;
    }
    
    glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);

    return JS_TRUE;
}


NGL_JS_FN(WebGLRenderingContext_bufferData) 
{
    GLenum target;
    JSObject *array;
    GLenum usage;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uou", &target, &array, &usage)) {
        return JS_TRUE;
    }

	if (array == NULL || !JS_IsTypedArrayObject(array)) {
        JS_ReportError(cx, "Invalid value");
		return JS_TRUE;
	}

    glBufferData(target, JS_GetArrayBufferViewByteLength(array), JS_GetArrayBufferViewData(array), usage);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_clear) 
{
    GLbitfield bits;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "i", &bits)) {
        return JS_TRUE;
    }
    
    glClear(bits);

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_clearColor) 
{
    double r;
    double g;
    double b;
    double a;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddd", &r, &g, &b, &a)) {
        return JS_TRUE;
    }
    
    glClearColor(r, g, b, a);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_clearDepth) 
{
    double clampd;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "d", &clampd)) {
        return JS_TRUE;
    }
    glClearDepth(clampd);

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_clearStencil) 
{
    GLint s;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "i", &s)) {
        return JS_TRUE;
    }

    glClearStencil(s);

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_compileShader) 
{
    uintptr_t cshader;
    JSObject *shader;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &shader)) {
        return JS_TRUE;
    }

    cshader = (uintptr_t)JS_GetInstancePrivate(cx, shader, &WebGLShader_class, JS_ARGV(cx, vp));
    glCompileShader(cshader);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_createBuffer) 
{
    intptr_t buffer;
    jsval proto;
    JSObject *ret;
    
    glGenBuffers(1, (GLuint *)&buffer);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLBuffer", &proto);
    ret = JS_NewObject(cx, &WebGLBuffer_class, JSVAL_TO_OBJECT(proto), NULL);
    JS_SetPrivate(ret, (void *)buffer);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_createFramebuffer) 
{
    intptr_t buffer;
    jsval proto;
    JSObject *ret;
    
    glGenFramebuffers(1, (GLuint *)&buffer);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLFrameBuffer", &proto);
    ret = JS_NewObject(cx, &WebGLFrameBuffer_class, JSVAL_TO_OBJECT(proto), NULL);
    JS_SetPrivate(ret, (void *)buffer);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_createRenderbuffer) 
{
    intptr_t buffer;
    jsval proto;
    JSObject *ret;
    
    glGenRenderbuffers(1, (GLuint *)&buffer);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLRenderbuffer", &proto);
    ret = JS_NewObject(cx, &WebGLRenderbuffer_class, JSVAL_TO_OBJECT(proto), NULL);
    JS_SetPrivate(ret, (void *)buffer);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_createProgram) 
{
    intptr_t program = glCreateProgram();
    jsval proto;
    JSObject *ret;
    
    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLProgram", &proto);
    ret = JS_NewObject(cx, &WebGLProgram_class, JSVAL_TO_OBJECT(proto), NULL);
    JS_SetPrivate(ret, (void *)program);
    
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_createShader)     
{
    GLenum type;
    uintptr_t shader;
    jsval proto;
    JSObject *ret;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &type)) {
        return JS_TRUE;
    }

    shader = glCreateShader(type);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLShader", &proto);
    ret = JS_NewObject(cx, &WebGLShader_class, JSVAL_TO_OBJECT(proto), NULL);
    JS_SetPrivate(ret, (void *)shader);
    
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_createTexture) 
{
    uintptr_t texture;
	JSObject *ret;
    jsval proto;

    glGenTextures(1, (GLuint *)&texture);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLTexture", &proto);
    ret = JS_NewObject(cx, &WebGLTexture_class, JSVAL_TO_OBJECT(proto), NULL);
    JS_SetPrivate(ret, (void *)texture);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}


NGL_JS_FN(WebGLRenderingContext_cullFace) 
{
    GLuint mode;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &mode)) {
        return JS_TRUE;
    }
    
    glCullFace(mode);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_deleteShader) 
{
    JSObject *shader;
    uintptr_t cshader;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &shader)) {
        return JS_TRUE;
    }

    cshader = (uintptr_t)JS_GetInstancePrivate(cx, shader, &WebGLShader_class, JS_ARGV(cx, vp));
    glDeleteShader(cshader);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_depthFunc) 
{
    GLuint func;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &func)) {
        return JS_TRUE;
    }
    
    glDepthFunc(func);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_depthMask) 
{
    JSBool flag;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "b", &flag)) {
        return JS_TRUE;
    }

    glDepthMask((GLboolean)flag);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_disable) 
{
    GLenum cap;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &cap)) {
        return JS_TRUE;
    }
    
    glDisable(cap);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_disableVertexAttribArray) 
{
    GLuint attr;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &attr)) {
        return JS_TRUE;
    }
    
    glDisableVertexAttribArray(attr);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_drawArrays) 
{
    GLenum mode;
    GLint first;
    GLsizei count;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uii", &mode, &first, &count)) {
        return JS_TRUE;
    }
    
    glDrawArrays(mode, first, count);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_drawElements) 
{
    GLenum mode;
    GLsizei count;
    GLenum type;
    GLint offset;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uiui", &mode, &count, &type, &offset)) {
        return JS_TRUE;
    }

    if (offset+count < offset || offset+count < count) {
        JS_ReportError(cx, "Overflow in drawElements");
        return JS_TRUE;
    }
    
    glDrawElements(mode, count, type, (void *)offset);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_enable) 
{
    GLuint bits;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &bits)) {
        return JS_TRUE;
    }
    
    glEnable(bits);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_enableVertexAttribArray) 
{
    GLuint attr;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &attr)) {
        return JS_TRUE;
    }
    
    glEnableVertexAttribArray(attr);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_getUniformLocation) 
{
    uintptr_t cprogram;
    intptr_t location;
    JSString *name;
    JSObject *program;
    JSObject *ret;
    jsval proto;
    const char *cname;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oS", &program, &name)) {
        return JS_TRUE;
    }

    cprogram = (uintptr_t)JS_GetInstancePrivate(cx, program, &WebGLProgram_class, JS_ARGV(cx, vp));
    cname = JS_EncodeString(cx, name);
 
    location = glGetUniformLocation(cprogram, cname);

    if (location < 0 ) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
    } else {
        JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLUniformLocation", &proto);
        ret = JS_NewObject(cx, &WebGLUniformLocation_class, JSVAL_TO_OBJECT(proto), NULL);
        JS_SetPrivate(ret, (void *)location);
        
        JS_free(cx, (void *)cname);
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));
    }
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_framebufferRenderbuffer) 
{
	GLenum target, attachement, renderbuffertarget;
	uintptr_t crenderbuffer;
	JSObject *renderbuffer;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuuoi", &target, &attachement, &renderbuffertarget, &renderbuffer)) {
        return JS_TRUE;
    }

    crenderbuffer = (uintptr_t)JS_GetInstancePrivate(cx, renderbuffer, &WebGLRenderbuffer_class, JS_ARGV(cx, vp));

	glFramebufferRenderbuffer(target, attachement, renderbuffertarget, crenderbuffer);

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_framebufferTexture2D) 
{
    GLenum target, attachement, textarget;
	uintptr_t ctexture, level;
    JSObject *texture;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuuoi", &target, &attachement, &textarget, &texture, &level)) {
        return JS_TRUE;
    }

	// No argv passed, as texture starting at 0
    ctexture = (uintptr_t)JS_GetInstancePrivate(cx, texture, &WebGLTexture_class, NULL);

	glFramebufferTexture2D(target, attachement, textarget, ctexture, level);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_frontFace) 
{
    GLuint mode;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &mode)) {
        return JS_TRUE;
    }
    
    glFrontFace(mode);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_generateMipmap) 
{
    GLenum target;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &target)) {
        return JS_TRUE;
    }
    
    glGenerateMipmap(target);
    
    return JS_TRUE;
}


NGL_JS_FN(WebGLRenderingContext_getAttribLocation) 
{
    uintptr_t cprogram;
    GLint location;
    JSString *attr;
    JSObject *program;
    const char *cattr;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oS", &program, &attr)) {
        return JS_TRUE;
    }
    
    cprogram = (uintptr_t)JS_GetInstancePrivate(cx, program, &WebGLProgram_class, JS_ARGV(cx, vp));
    cattr = JS_EncodeString(cx, attr);

    location = glGetAttribLocation(cprogram, cattr);

    JS_free(cx, (void *)cattr);
    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(location));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_getParameter) 
{
    GLenum name;
    JS::Value value;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &name)) {
        return JS_TRUE;
    }

    switch (name) {
        // String
        case GL_VENDOR:
        case GL_RENDERER:
        case GL_VERSION:
        case GL_SHADING_LANGUAGE_VERSION:
        {
            const char *cstr = (const char *)glGetString(name);
            JSString *str = JS_NewStringCopyZ(cx, cstr);

            value.setString(str);

            break;
        }
        case GL_CULL_FACE_MODE:
        case GL_FRONT_FACE:
        case GL_ACTIVE_TEXTURE:
        case GL_STENCIL_FUNC:
        case GL_STENCIL_FAIL:
        case GL_STENCIL_PASS_DEPTH_FAIL:
        case GL_STENCIL_PASS_DEPTH_PASS:
        case GL_STENCIL_BACK_FUNC:
        case GL_STENCIL_BACK_FAIL:
        case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
        case GL_STENCIL_BACK_PASS_DEPTH_PASS:
        case GL_DEPTH_FUNC:
        case GL_BLEND_SRC_RGB:
        case GL_BLEND_SRC_ALPHA:
        case GL_BLEND_DST_RGB:
        case GL_BLEND_DST_ALPHA:
        case GL_BLEND_EQUATION_RGB:
        case GL_BLEND_EQUATION_ALPHA:
        //case GL_GENERATE_MIPMAP_HINT:
        {
            GLint i = 0;
            glGetIntegerv(name, &i);
            value.setNumber(uint32_t(i));
            break;
        }
        // int
        case GL_STENCIL_CLEAR_VALUE:
        case GL_STENCIL_REF:
        case GL_STENCIL_BACK_REF:
        case GL_UNPACK_ALIGNMENT:
        case GL_PACK_ALIGNMENT:
        case GL_SUBPIXEL_BITS:
        case GL_MAX_TEXTURE_SIZE:
        case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
        case GL_SAMPLE_BUFFERS:
        case GL_SAMPLES:
        case GL_MAX_VERTEX_ATTRIBS:
        case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
        case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
        case GL_MAX_TEXTURE_IMAGE_UNITS:
        case GL_MAX_RENDERBUFFER_SIZE:
        /*case GL_RED_BITS:
        case GL_GREEN_BITS:
        case GL_BLUE_BITS:
        case GL_ALPHA_BITS:
        case GL_DEPTH_BITS:
        case GL_STENCIL_BITS:
        case GL_MAX_VERTEX_UNIFORM_VECTORS:
        case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
        case GL_MAX_VARYING_VECTORS:*/
        case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
        {
            GLint i = 0;
            glGetIntegerv(name, &i);
            value.setInt32(i);
            break;
        }
        case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
            #if 0
            if (IsExtensionEnabled(OES_standard_derivatives)) {
                GLint i = 0;
                gl->fGetIntegerv(pname, &i);
                return JS::Int32Value(i);
            }
            else {
                ErrorInvalidEnum("getParameter: parameter", pname);
                return JS::NullValue();
            }
            #endif
            value.setNull();
            break;
        case GL_COMPRESSED_TEXTURE_FORMATS:
        {
            GLint length;
            GLint *textures;
            uint32_t *data;
            JSObject *obj;

            glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &length);
            obj = JS_NewUint32Array(cx, length);
            textures = (GLint *)malloc(sizeof(GLint) * length);

            if (!obj || !textures) {
                if (textures != NULL) {
                    free(textures);
                }
                JS_ReportError(cx, "OOM");
                return JS_TRUE;
            }

            glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, textures);

            data = JS_GetUint32ArrayData(obj);
            memcpy(data, textures, length * sizeof(GLint));
            free(textures);

            value.setObjectOrNull(obj);
            break;
        }

        // unsigned int
        case GL_STENCIL_BACK_VALUE_MASK:
        case GL_STENCIL_BACK_WRITEMASK:
        case GL_STENCIL_VALUE_MASK:
        case GL_STENCIL_WRITEMASK:
        {
            GLint i = 0; // the GL api (glGetIntegerv) only does signed ints
            glGetIntegerv(name, &i);
            GLuint i_unsigned(i); // this is where -1 becomes 2^32-1
            double i_double(i_unsigned); // pass as FP value to allow large values such as 2^32-1.
            value.setDouble(i_double);
            break;
        }

        // float
        case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
            #if 0
            if (IsExtensionEnabled(EXT_texture_filter_anisotropic)) {
                GLfloat f = 0.f;
                gl->fGetFloatv(pname, &f);
                return JS::DoubleValue(f);
            } else {
                ErrorInvalidEnumInfo("getParameter: parameter", pname);
                return JS::NullValue();
            }
            #endif
            value.setNull();
            break;
        case GL_DEPTH_CLEAR_VALUE:
        case GL_LINE_WIDTH:
        case GL_POLYGON_OFFSET_FACTOR:
        case GL_POLYGON_OFFSET_UNITS:
        case GL_SAMPLE_COVERAGE_VALUE:
        {
            GLfloat f = 0.f;
            glGetFloatv(name, &f);
            value.setDouble(f);
            break;
        }

        // bool
        case GL_BLEND:
        case GL_DEPTH_TEST:
        case GL_STENCIL_TEST:
        case GL_CULL_FACE:
        case GL_DITHER:
        case GL_POLYGON_OFFSET_FILL:
        case GL_SCISSOR_TEST:
        case GL_SAMPLE_COVERAGE_INVERT:
        case GL_DEPTH_WRITEMASK:
        {
            GLboolean b = 0;
            glGetBooleanv(name, &b);
            value.setBoolean(b);
            break;
        }

        // bool, WebGL-specific
        case NGL_UNPACK_FLIP_Y_WEBGL:
        case NGL_UNPACK_PREMULTIPLY_ALPHA_WEBGL:
            value.setBoolean(false);
        break;

        // uint, WebGL-specific
        case NGL_UNPACK_COLORSPACE_CONVERSION_WEBGL:
            value.setInt32(GL_NONE);
        break;

        // Complex values
        case GL_DEPTH_RANGE: // 2 floats
        //case GL_ALIASED_POINT_SIZE_RANGE: // 2 floats
        case GL_ALIASED_LINE_WIDTH_RANGE: // 2 floats
        {
            GLfloat fv[2] = { 0 };
            JSObject *obj = JS_NewFloat32Array(cx, 2);
            float *data;

            if (!obj) {
                JS_ReportError(cx, "OOM");
                return JS_TRUE;
            }

            glGetFloatv(name, fv);

            data = JS_GetFloat32ArrayData(obj);
            memcpy(data, fv, 2 * sizeof(float));
            value.setObjectOrNull(obj);
            break;
        }
        
        case GL_COLOR_CLEAR_VALUE: // 4 floats
        case GL_BLEND_COLOR: // 4 floats
        {
            GLfloat fv[4] = { 0 };
            JSObject *obj = JS_NewFloat32Array(cx, 4);
            float *data;

            if (!obj) {
                JS_ReportError(cx, "OOM");
                return JS_TRUE;
            }

            glGetFloatv(name, fv);

            data = JS_GetFloat32ArrayData(obj);
            memcpy(data, fv, 4 * sizeof(GLfloat));
            value.setObjectOrNull(obj);
            break;
        }

        case GL_MAX_VIEWPORT_DIMS: // 2 ints
        {
            GLint iv[2] = { 0 };
            JSObject *obj = JS_NewInt32Array(cx, 2);
            int32_t *data;

            if (!obj) {
                JS_ReportError(cx, "OOM");
                return JS_TRUE;
            }

            glGetIntegerv(name, iv);

            data = JS_GetInt32ArrayData(obj);
            memcpy(data, iv, 2 * sizeof(GLint));
            value.setObjectOrNull(obj);
            break;
        }

        case GL_SCISSOR_BOX: // 4 ints
        case GL_VIEWPORT: // 4 ints
        {
            GLint iv[4] = { 0 };
            JSObject *obj = JS_NewInt32Array(cx, 4);
            int32_t *data;

            if (!obj) {
                JS_ReportError(cx, "OOM");
                return JS_TRUE;
            }

            glGetIntegerv(name, iv);

            data = JS_GetInt32ArrayData(obj);
            memcpy(data, iv, 4 * sizeof(GLint));
            value.setObjectOrNull(obj);
            break;
        }

        case GL_COLOR_WRITEMASK: // 4 bools
        {
            GLboolean gl_bv[4] = { 0 };

            glGetBooleanv(name, gl_bv);

            JS::Value vals[4] = { JS::BooleanValue(bool(gl_bv[0])),
                                  JS::BooleanValue(bool(gl_bv[1])),
                                  JS::BooleanValue(bool(gl_bv[2])),
                                  JS::BooleanValue(bool(gl_bv[3])) };

            JSObject* obj = JS_NewArrayObject(cx, 4, vals);

            if (!obj) {
                JS_ReportError(cx, "OOM");
                return JS_TRUE;
            }


            value.setObjectOrNull(obj);
            break;
        }

        // TODO 
        case GL_ARRAY_BUFFER_BINDING:
        case GL_ELEMENT_ARRAY_BUFFER_BINDING:
        case GL_RENDERBUFFER_BINDING:
        case GL_FRAMEBUFFER_BINDING:
        case GL_CURRENT_PROGRAM:
        case GL_TEXTURE_BINDING_2D:
        case GL_TEXTURE_BINDING_CUBE_MAP:
            value.setNull();
        break;

        default:
            JS_ReportError(cx, "getParameter invalue value");
            return JS_TRUE;
    }

    JS_SET_RVAL(cx, vp, value);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_getProgramParameter)  
{
    uintptr_t cprogram;
    GLenum param;
    GLint status;
    JSObject *program;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ou", &program, &param)) {
        return JS_TRUE;
    }

    cprogram = (uintptr_t)JS_GetInstancePrivate(cx, program, &WebGLProgram_class, JS_ARGV(cx, vp));

    glGetProgramiv(cprogram, param, &status);
    
    switch (param) {
        case GL_DELETE_STATUS:
        case GL_LINK_STATUS:
        case GL_VALIDATE_STATUS:
            JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL((GLboolean)status));
            break;
        default:
            JS_SET_RVAL(cx, vp, INT_TO_JSVAL(status));
            break;
    }
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_getProgramInfoLog) 
{
    uintptr_t cprogram;
    GLsizei max;
    GLsizei length;
    JSString *log;
    JSObject *program;
    char *clog;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &program)) {
        return JS_TRUE;
    }

    cprogram = (uintptr_t)JS_GetInstancePrivate(cx, program, &WebGLProgram_class, JS_ARGV(cx, vp));
    
    glGetProgramiv(cprogram, GL_INFO_LOG_LENGTH, &max);

    clog = (char *)malloc(max);
    glGetProgramInfoLog(cprogram, max, &length, clog);
    log = JS_NewStringCopyN(cx, clog, length);
    free(clog);

    JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(log));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_getShaderParameter) 
{
    uintptr_t cshader;
    GLenum pname;
    GLint param;
    JSObject *shader;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ou", &shader, &pname)) {
        return JS_TRUE;
    }

    cshader = (uintptr_t)JS_GetInstancePrivate(cx, shader, &WebGLShader_class, JS_ARGV(cx, vp));

    glGetShaderiv(cshader, pname, &param);

    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(param));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_getShaderInfoLog) 
{
    uintptr_t cshader;
    GLsizei length;
    GLsizei max;
    JSString *log;
    JSObject *shader;
    char *clog;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &shader)) {
        return JS_TRUE;
    }
    cshader = (uintptr_t)JS_GetInstancePrivate(cx, shader, &WebGLShader_class, JS_ARGV(cx, vp));
    
    glGetShaderiv(cshader, GL_INFO_LOG_LENGTH, &max);

    clog = (char *)malloc(max);
    glGetShaderInfoLog(cshader, max, &length, clog);
    log = JS_NewStringCopyN(cx, clog, length);
    free(clog);

    JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(log));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_lineWidth) 
{
    GLfloat width;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "d", &width)) {
        return JS_TRUE;
    }

    glLineWidth(width);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_linkProgram) 
{
    uintptr_t cprogram;
    JSObject *program;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &program)) {
        return JS_TRUE;
    }
    
    cprogram = (uintptr_t)JS_GetInstancePrivate(cx, program, &WebGLProgram_class, JS_ARGV(cx, vp));

    glLinkProgram(cprogram);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_pixelStorei)  
{
    GLuint param;
    GLint value;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ui", &param, &value)) {
        return JS_TRUE;
    }

    switch (param) {
        case NGL_UNPACK_FLIP_Y_WEBGL:
        {
            NativeJSNativeGL *ngl = NATIVE_GL_GETTER(JS_THIS_OBJECT(cx, vp));
            ngl->unpackFlipY = (bool)value;
            break;
        }
        case NGL_UNPACK_PREMULTIPLY_ALPHA_WEBGL:
        {
            NativeJSNativeGL *ngl = NATIVE_GL_GETTER(JS_THIS_OBJECT(cx, vp));
            ngl->unpackPremultiplyAlpha = (bool)value;
            break;
        }
        case NGL_UNPACK_COLORSPACE_CONVERSION_WEBGL:
            JS_ReportError(cx, "Not implemented");
        break;
        default: 
            glPixelStorei(param, value);
        break;
    }
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_renderbufferStorage)  
{
    GLenum target, internalFormat;
	GLsizei width, height;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuii", &target, &internalFormat, &width, &height)) {
        return JS_TRUE;
    }

	glRenderbufferStorage(target, internalFormat, width, height);

	return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_shaderSource) 
{
    uintptr_t cshader;
    GLsizei length;
    JSString *source;
    JSObject *shader;
    const char *csource;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oS", &shader, &source)) {
        return JS_TRUE;
    }

    cshader = (uintptr_t)JS_GetInstancePrivate(cx, shader, &WebGLShader_class, JS_ARGV(cx, vp));
    csource = JS_EncodeString(cx, source);
    length = JS_GetStringLength(source);
    
    glShaderSource(cshader, 1, &csource, &length);

    JS_free(cx, (void *)csource);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_texImage2D) 
{
    GLenum target;
    GLint level;
    GLint internalFormat;
    GLenum format;
    GLenum type;
    JSObject *image;
    int width, height;
	NativeJSImage *nimg;
    void *pixels = NULL;
    NativeJSNativeGL *ngl;
    unsigned char *rgbaPixels;

    ngl = NATIVE_GL_GETTER(JS_THIS_OBJECT(cx, vp));
    
    if (argc == 9) {
        GLint border;
        JSObject *array;

        if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uiiiiiuuo", &target, &level, &internalFormat, &width, &height, &border, &format, &type, &array)) {
            return JS_TRUE;
        }

        if (array != NULL && JS_IsTypedArrayObject(array)) {
            pixels = JS_GetArrayBufferViewData(array);
        } 
        
        glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
    } else {
        if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uiiuuo", &target, &level, &internalFormat, &format, &type, &image)) {
            return JS_TRUE;
        }

        // XXX : UNSAFE use JS_GetInstancePrivate Image_class 
        //if ((nimg = static_cast<NativeJSImage *>(JS_GetInstancePrivate(cx, image, &Image_class, JS_ARGV(cx, vp)))) == NULL) {
        if ((nimg = static_cast<NativeJSImage *>(JS_GetPrivate(image))) == NULL) {
            JS_ReportError(cx, "Invalid image object");
            return JS_TRUE;
        }

        width = nimg->img->getWidth();
        height = nimg->img->getHeight();
        
        rgbaPixels = (unsigned char*)malloc(nimg->img->img.getSize());

        if (!NativeSkImage::ConvertToRGBA(nimg->img, rgbaPixels, ngl->unpackFlipY, 
                ngl->unpackPremultiplyAlpha)) {
            JS_ReportError(cx, "Failed to read image data");
            return JS_TRUE;
        }

        glTexImage2D(target, level, internalFormat, width, height, 0, format, type, rgbaPixels);

        free(rgbaPixels);
    }

    return JS_TRUE;
}


NGL_JS_FN(WebGLRenderingContext_texParameteri) 
{
    GLuint target;
    GLuint pname;
    GLuint param;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuu", &target, &pname, &param)) {
        return JS_TRUE;
    }
    
    glTexParameteri(target, pname, param);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_uniform1f)  
{
    return NativeGL_uniform_x_f(cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_uniform1fv) 
{
    return NativeGL_uniform_x_fv(cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_uniform1i) 
{
    return NativeGL_uniform_x_i(cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_uniform1iv) 
{
    return NativeGL_uniform_x_iv(cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_uniform2f)  
{
    return NativeGL_uniform_x_f(cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_uniform2fv) 
{
    return NativeGL_uniform_x_fv(cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_uniform2i) 
{
    return NativeGL_uniform_x_i(cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_uniform2iv) 
{
    return NativeGL_uniform_x_iv(cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_uniform3f)  
{
    return NativeGL_uniform_x_f(cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_uniform3fv) 
{
    return NativeGL_uniform_x_fv(cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_uniform3i) 
{
    return NativeGL_uniform_x_i(cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_uniform3iv) 
{
    return NativeGL_uniform_x_iv(cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_uniform4f)  
{
    return NativeGL_uniform_x_f(cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_uniform4fv) 
{
    return NativeGL_uniform_x_fv(cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_uniform4i) 
{
    return NativeGL_uniform_x_i(cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_uniform4iv) 
{
    return NativeGL_uniform_x_iv(cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_uniformMatrix2fv) 
{
    return NativeGL_uniformMatrix_x_fv(cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_uniformMatrix3fv) 
{
    return NativeGL_uniformMatrix_x_fv(cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_uniformMatrix4fv) 
{
    return NativeGL_uniformMatrix_x_fv(cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib1f) 
{
    return NativeGL_vertexAttrib_x_f(cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib1fv) 
{
    return NativeGL_vertexAttrib_x_fv(cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib2f) 
{
    return NativeGL_vertexAttrib_x_f(cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib2fv) 
{
    return NativeGL_vertexAttrib_x_fv(cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib3f) 
{
    return NativeGL_vertexAttrib_x_f(cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib3fv) 
{
    return NativeGL_vertexAttrib_x_fv(cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib4f) 
{
    return NativeGL_vertexAttrib_x_f(cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib4fv) 
{
    return NativeGL_vertexAttrib_x_fv(cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttribPointer) 
{
    GLuint attr;
    GLint size;
    GLenum type;
    GLsizei stride;
    GLint offset;
    JSBool normalized;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uiubii", &attr, &size, &type, &normalized, &stride, &offset)) {
        return JS_TRUE;
    }

    if (offset+size < offset || offset+size < size) {
        JS_ReportError(cx, "Overflow in vertexAttribPointer");
        return JS_TRUE;
    }

    glVertexAttribPointer(attr, size, type, (GLboolean)normalized, stride, (void *)offset);

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_useProgram)  
{
    uintptr_t cprogram;
    JSObject *program;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &program)) {
        return JS_TRUE;
    }
    
    cprogram = (uintptr_t)JS_GetInstancePrivate(cx, program, &WebGLProgram_class, JS_ARGV(cx, vp));

    glUseProgram(cprogram);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_viewport)  
{
    GLint x, y, w, h;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "iiii", &x, &y, &w, &h)) {
        return JS_TRUE;
    }

    glViewport(x, y, w, h);

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_getError) 
{
    JS_SET_RVAL(cx, vp, UINT_TO_JSVAL(glGetError()));
    return JS_TRUE;
}

static JSBool native_NativeGL_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    jsval proto;
    JSObject *webGLContext;
    NativeJSNativeGL *ngl = new NativeJSNativeGL();
    
    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLRenderingContext", &proto);
	webGLContext = JS_NewObject(cx, &WebGLRenderingContext_class, JSVAL_TO_OBJECT(proto), NULL);

    if (webGLContext == NULL) {
        JS_ReportError(cx, "Failed to create WebGLRenderingContext");
        return JS_TRUE;
    } 

    JS_SetPrivate(webGLContext, static_cast<void *>(ngl));
    ngl->jsobj = webGLContext;

    // Compatibility OpenGL/WebGL
    // XXX : Is this belongs here ?
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    //glEnable(GL_POINT_SPRITE);
    //GL_ARB_point
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(webGLContext));

    return JS_TRUE;
}

NATIVE_OBJECT_EXPOSE(NativeGL);
void NativeJSWebGLRenderingContext::registerObject(JSContext *cx) {
    JSObject *proto;

    proto = JS_NewObject(cx, NULL, NULL, NULL);
    JS_DefineConstDoubles(cx, proto, WebGLRenderingContext_const);
    JS_DefineFunctions(cx, proto, WebGLRenderingContext_funcs);
    
    JS_DefineObject(cx, JS_GetGlobalObject(cx),
                    "WebGLRenderingContext", &WebGLRenderingContext_class,
                    proto, 0);
}

NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLObject);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLBuffer);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLFrameBuffer);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLProgram);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLRenderbuffer);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLShader);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLTexture);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLUniformLocation);
