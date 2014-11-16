#include "NativeJSWebGL.h"
#include "NativeJSImage.h"
#include "NativeSkImage.h"
#include "NativeSkia.h"
#include "NativeCanvas2DContext.h"
#include "NativeCanvas3DContext.h"
#include "NativeJSCanvas.h"
#include "NativeGLState.h"
#include "GLSLANG/ShaderLang.h"
#include <SkCanvas.h>
#include <NativeContext.h>
#include <NativeUIInterface.h>
#include <NativeMacros.h>
#include <NativeSystemInterface.h>


#define GL_GLEXT_PROTOTYPES
#if __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

extern JSClass Canvas_class;

#define NATIVE_GL_GETTER(obj) ((class NativeCanvasWebGLContext*)JS_GetPrivate(obj))

#define GL_CALL(IFACE, FN)\
    NATIVE_GL_CALL(IFACE, FN); \
    { GLint err = glGetError(); if (err != 0) NLOG("err = %d / call = %s\n", err, #FN); }

#define GL_CALL_RET(IFACE, FN, RET)\
    NATIVE_GL_CALL_RET(IFACE, FN, RET); \
    { GLint err = glGetError(); if (err != 0) NLOG("err = %d / call = %s\n", err, #FN); }

#define D_NGL_JS_FN(func_name) static JSBool func_name(JSContext *cx, unsigned int argc, jsval *vp);

#define NGL_JS_FN(func_name) static JSBool func_name(JSContext *cx, unsigned int argc, jsval *vp) {\
    JSNATIVE_PROLOGUE_CLASS(NativeCanvas3DContext, &WebGLRenderingContext_class);

#define NATIVE_GL_OBJECT_EXPOSE_NOT_INST(name) \
    void NativeJS ## name::registerObject(JSContext *cx) \
    { \
        JS_DefineObject(cx, JS_GetGlobalObject(cx), #name, \
            &name ## _class , NULL, 0); \
    }

class WebGLResource {
public:
    enum ResourceType {
        kTexture,
        kProgram,
        kShader,
        kBuffer,
        kFrameBuffer,
        kRenderBuffer,
        kVertexArray,
        kResources_end
    };

    WebGLResource(uint32_t id, ResourceType type,
        NativeCanvas3DContext *ctx, JSObject *jsobj) :
        m_GlIdentifier(id), m_GLctx(ctx), m_Type(type), m_JSObj(jsobj){};

    ~WebGLResource() {
        switch(m_Type) {
            case kProgram:
                GL_CALL(m_GLctx, DeleteProgram(m_GlIdentifier));
                break;
            case kShader:
                GL_CALL(m_GLctx, DeleteShader(m_GlIdentifier));
                break;
            case kTexture:
                GL_CALL(m_GLctx, DeleteTextures(1, &m_GlIdentifier));
                break;
            case kBuffer:
                GL_CALL(m_GLctx, DeleteBuffers(1, &m_GlIdentifier));
                break;
            case kVertexArray:
                GL_CALL(m_GLctx, DeleteVertexArraysAPPLE(1, &m_GlIdentifier));
                break;
            case kFrameBuffer:
                GL_CALL(m_GLctx, DeleteFramebuffers(1, &m_GlIdentifier));
                break;
            case kRenderBuffer:
                GL_CALL(m_GLctx, DeleteRenderbuffers(1, &m_GlIdentifier));
                break;
            default:
                break;
        }
    }

    uint32_t id() const {
        return m_GlIdentifier;
    }

    JSObject *jsobj() const {
        return m_JSObj;
    }

    uint32_t m_GlIdentifier;
    NativeCanvas3DContext *m_GLctx;
    ResourceType m_Type;
    JSObject *m_JSObj;
};

static void Buffer_Finalize(JSFreeOp *fop, JSObject *obj);
static void WebGLRenderingContext_Finalize(JSFreeOp *fop, JSObject *obj);

JSClass WebGLRenderingContext_class = {
    "WebGLRenderingContext",
    JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(WebGLResource::kResources_end),
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, WebGLRenderingContext_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass WebGLBuffer_class = {
    "WebGLBuffer", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Buffer_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};
static JSClass WebGLFrameBuffer_class = {
    "WebGLFrameBuffer", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Buffer_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};
static JSClass WebGLProgram_class = {
    "WebGLProgram", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Buffer_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};
static JSClass WebGLRenderbuffer_class = {
    "WebGLRenderbuffer", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Buffer_Finalize,
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
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Buffer_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};
static JSClass WebGLUniformLocation_class = {
    "WebGLUniformLocation", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};
static JSClass WebGLShaderPrecisionFormat_class = {
    "WebGLShaderPrecisionFormat", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static void Buffer_Finalize(JSFreeOp *fop, JSObject *obj)
{
    WebGLResource *res = (WebGLResource *)JS_GetPrivate(obj);

    if (res) {
        delete res;
    }
}

static void WebGLRenderingContext_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeCanvas3DContext *ctx = (NativeCanvas3DContext *)JS_GetPrivate(obj);
    if (ctx) {
        delete ctx;
    }
}

bool NGL_uniformxf(NativeCanvas3DContext *glctx, JSContext *cx, unsigned int argc, jsval *vp, int nb) {
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
            GL_CALL(glctx, Uniform1f(clocation, (GLfloat)x));
            break;
        case 2:
            GL_CALL(glctx, Uniform2f(clocation, (GLfloat)x, (GLfloat)y));
            break;
        case 3:
            GL_CALL(glctx, Uniform3f(clocation, (GLfloat)x, (GLfloat)y, (GLfloat)z));
            break;
        case 4:
            GL_CALL(glctx, Uniform4f(clocation, (GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w));
            break;
    }

    return true;
} 

bool NGL_uniformxfv(NativeCanvas3DContext *glctx, JSContext *cx, unsigned int argc, jsval *vp, int nb) 
{
    intptr_t clocation;
    GLsizei length;
    GLfloat *carray;
    JSObject *array;
    JSObject *location;

    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oo", &location, &array)) {
        return false;
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
        return false;
    }

    switch (nb) {
        case 1:
            GL_CALL(glctx, Uniform1fv(clocation, length, carray));
            break;
        case 2:
            GL_CALL(glctx, Uniform2fv(clocation, length/2, carray));
            break;
        case 3:
            GL_CALL(glctx, Uniform3fv(clocation, length/3, carray));
            break;
        case 4:
            GL_CALL(glctx, Uniform4fv(clocation, length/4, carray));
            break;
    }

    return true;
} 

bool NGL_uniformxi(NativeCanvas3DContext *glctx, JSContext *cx, unsigned int argc, jsval *vp, int nb) 
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
            GL_CALL(glctx, Uniform1i(clocation, x));
            break;
        case 2:
            GL_CALL(glctx, Uniform2i(clocation, x, y));
            break;
        case 3:
            GL_CALL(glctx, Uniform3i(clocation, x, y, z));
            break;
        case 4:
            GL_CALL(glctx, Uniform4i(clocation, x, y, z, w));
            break;
    }
    
    return true;
} 

bool NGL_uniformxiv(NativeCanvas3DContext *glctx, JSContext *cx, unsigned int argc, jsval *vp, int nb) 
{
    uintptr_t clocation;
    GLsizei length;
    GLint *carray;
    JSObject *tmp;
    JSObject *array;
    JSObject *location;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oo", &location, &array)) {
        return false;
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
        return false;
    }

    if (nb == 1) {
        GL_CALL(glctx, Uniform1iv(clocation, length, carray));
    } else if (nb == 2) {
        GL_CALL(glctx, Uniform2iv(clocation, length/2, carray));
    } else if (nb == 3) {
        GL_CALL(glctx, Uniform3iv(clocation, length/3, carray));
    } else if (nb == 4) {
        GL_CALL(glctx, Uniform4iv(clocation, length/4, carray));
    }
    
    return true;
} 

bool NGL_uniformMatrixxfv(NativeCanvas3DContext *glctx, JSContext *cx, unsigned int argc, jsval *vp, int nb) 
{
    uintptr_t clocation;
    GLint length;
    GLfloat *carray;
    JSBool transpose;
    JSObject *tmp;
    JSObject *array;
    JSObject *location;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "obo", &location, &transpose, &array)) {
        return false;
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
        return false;
    }
    
    switch (nb) {
        case 2:
            GL_CALL(glctx, UniformMatrix2fv(clocation, length/4, (GLboolean)transpose, carray));
            break;
        case 3:
            GL_CALL(glctx, UniformMatrix3fv(clocation, length/8, (GLboolean)transpose, carray));
            break;
        case 4:
            GL_CALL(glctx, UniformMatrix4fv(clocation, length/16, (GLboolean)transpose, carray));
            break;
    }
    
    return true;
}

bool NGL_vertexAttribxf(NativeCanvas3DContext *glctx, JSContext *cx, unsigned int argc, jsval *vp, int nb) 
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
            GL_CALL(glctx, VertexAttrib1f(index, (GLfloat)v0));
            break;
        case 2:
            GL_CALL(glctx, VertexAttrib2f(index, (GLfloat)v0, (GLfloat)v1));
            break;
        case 3:
            GL_CALL(glctx, VertexAttrib3f(index, (GLfloat)v0, (GLfloat)v1, (GLfloat)v2));
            break;
        case 4:
            GL_CALL(glctx, VertexAttrib4f(index, (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, (GLfloat)v3));
            break;
    }

    return true;
}

bool NGL_vertexAttribxfv(NativeCanvas3DContext *glctx, JSContext *cx, unsigned int argc, jsval *vp, int nb) 
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

D_NGL_JS_FN(WebGLRenderingContext_isContextLost)
D_NGL_JS_FN(WebGLRenderingContext_getExtension)
D_NGL_JS_FN(WebGLRenderingContext_activeTexture)
D_NGL_JS_FN(WebGLRenderingContext_attachShader)
D_NGL_JS_FN(WebGLRenderingContext_bindAttribLocation)
D_NGL_JS_FN(WebGLRenderingContext_bindBuffer)
D_NGL_JS_FN(WebGLRenderingContext_bindRenderbuffer)
D_NGL_JS_FN(WebGLRenderingContext_bindFramebuffer)
D_NGL_JS_FN(WebGLRenderingContext_bindTexture)
D_NGL_JS_FN(WebGLRenderingContext_blendEquation)
D_NGL_JS_FN(WebGLRenderingContext_blendEquationSeparate)
D_NGL_JS_FN(WebGLRenderingContext_blendFunc)
D_NGL_JS_FN(WebGLRenderingContext_blendFuncSeparate)
D_NGL_JS_FN(WebGLRenderingContext_bufferData)
D_NGL_JS_FN(WebGLRenderingContext_bufferSubData)
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
D_NGL_JS_FN(WebGLRenderingContext_getShaderPrecisionFormat)
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
D_NGL_JS_FN(WebGLRenderingContext_swapBuffer)


static JSFunctionSpec WebGLRenderingContext_funcs [] = {
    JS_FS("isContextLost", WebGLRenderingContext_isContextLost, 0, JSPROP_ENUMERATE),
    JS_FS("getExtension", WebGLRenderingContext_getExtension, 1, JSPROP_ENUMERATE),
    JS_FS("activeTexture", WebGLRenderingContext_activeTexture, 1, JSPROP_ENUMERATE),
    JS_FS("attachShader", WebGLRenderingContext_attachShader, 2, JSPROP_ENUMERATE),
    JS_FS("bindAttribLocation", WebGLRenderingContext_bindAttribLocation, 3, JSPROP_ENUMERATE),
    JS_FS("bindBuffer", WebGLRenderingContext_bindBuffer, 2, JSPROP_ENUMERATE),
    JS_FS("bindFramebuffer", WebGLRenderingContext_bindFramebuffer, 2, JSPROP_ENUMERATE),
    JS_FS("bindRenderbuffer", WebGLRenderingContext_bindRenderbuffer, 2, JSPROP_ENUMERATE),
    JS_FS("bindTexture", WebGLRenderingContext_bindTexture, 2, JSPROP_ENUMERATE),
    JS_FS("blendEquation", WebGLRenderingContext_blendEquation, 1, JSPROP_ENUMERATE),
    JS_FS("blendEquationSeparate", WebGLRenderingContext_blendEquationSeparate, 2, JSPROP_ENUMERATE),
    JS_FS("blendFunc", WebGLRenderingContext_blendFunc, 2, JSPROP_ENUMERATE),
    JS_FS("blendFuncSeparate", WebGLRenderingContext_blendFuncSeparate, 4, JSPROP_ENUMERATE),
    JS_FS("bufferData", WebGLRenderingContext_bufferData, 3, JSPROP_ENUMERATE),
    JS_FS("bufferSubData", WebGLRenderingContext_bufferSubData, 3, JSPROP_ENUMERATE),
    JS_FS("clear", WebGLRenderingContext_clear, 1, JSPROP_ENUMERATE),
    JS_FS("clearColor", WebGLRenderingContext_clearColor, 4, JSPROP_ENUMERATE),
    JS_FS("clearDepth", WebGLRenderingContext_clearDepth, 1, JSPROP_ENUMERATE),
    JS_FS("clearStencil", WebGLRenderingContext_clearStencil, 1, JSPROP_ENUMERATE),
    JS_FS("compileShader", WebGLRenderingContext_compileShader, 1, JSPROP_ENUMERATE),
    JS_FS("texImage2D", WebGLRenderingContext_texImage2D, 6, JSPROP_ENUMERATE),
    JS_FS("createBuffer", WebGLRenderingContext_createBuffer, 0, JSPROP_ENUMERATE),
    JS_FS("createFramebuffer", WebGLRenderingContext_createFramebuffer, 0, JSPROP_ENUMERATE),
    JS_FS("createRenderbuffer", WebGLRenderingContext_createRenderbuffer, 0, JSPROP_ENUMERATE),
    JS_FS("createProgram", WebGLRenderingContext_createProgram, 0, JSPROP_ENUMERATE),
    JS_FS("createShader", WebGLRenderingContext_createShader, 1, JSPROP_ENUMERATE),
    JS_FS("createTexture", WebGLRenderingContext_createTexture, 0, JSPROP_ENUMERATE),
    JS_FS("cullFace", WebGLRenderingContext_cullFace, 1, JSPROP_ENUMERATE),
    JS_FS("deleteShader", WebGLRenderingContext_deleteShader, 1, JSPROP_ENUMERATE),
    JS_FS("depthFunc", WebGLRenderingContext_depthFunc, 1, JSPROP_ENUMERATE),
    JS_FS("depthMask", WebGLRenderingContext_depthMask, 1, JSPROP_ENUMERATE),
    JS_FS("disable", WebGLRenderingContext_disable, 1, JSPROP_ENUMERATE),
    JS_FS("disableVertexAttribArray", WebGLRenderingContext_disableVertexAttribArray, 1, JSPROP_ENUMERATE),
    JS_FS("drawArrays", WebGLRenderingContext_drawArrays, 3, JSPROP_ENUMERATE),
    JS_FS("drawElements", WebGLRenderingContext_drawElements, 4, JSPROP_ENUMERATE),
    JS_FS("enable", WebGLRenderingContext_enable, 1, JSPROP_ENUMERATE),
    JS_FS("enableVertexAttribArray", WebGLRenderingContext_enableVertexAttribArray, 1, JSPROP_ENUMERATE),
    JS_FS("framebufferRenderbuffer", WebGLRenderingContext_framebufferRenderbuffer, 4, JSPROP_ENUMERATE),
    JS_FS("framebufferTexture2D", WebGLRenderingContext_framebufferTexture2D, 5, JSPROP_ENUMERATE),
    JS_FS("frontFace", WebGLRenderingContext_frontFace, 1, JSPROP_ENUMERATE),
    JS_FS("generateMipmap", WebGLRenderingContext_generateMipmap, 1, JSPROP_ENUMERATE),
    JS_FS("getAttribLocation", WebGLRenderingContext_getAttribLocation, 2, JSPROP_ENUMERATE),
    JS_FS("getParameter", WebGLRenderingContext_getParameter, 1, JSPROP_ENUMERATE),
    JS_FS("getProgramParameter", WebGLRenderingContext_getProgramParameter, 2, JSPROP_ENUMERATE),
    JS_FS("getProgramInfoLog", WebGLRenderingContext_getProgramInfoLog, 1, JSPROP_ENUMERATE),
    JS_FS("getShaderParameter", WebGLRenderingContext_getShaderParameter, 2, JSPROP_ENUMERATE),
    JS_FS("getShaderInfoLog", WebGLRenderingContext_getShaderInfoLog, 1, JSPROP_ENUMERATE),
    JS_FS("getUniformLocation", WebGLRenderingContext_getUniformLocation, 2, JSPROP_ENUMERATE),
    JS_FS("getShaderPrecisionFormat", WebGLRenderingContext_getShaderPrecisionFormat, 2, JSPROP_ENUMERATE),
    JS_FS("lineWidth", WebGLRenderingContext_lineWidth, 1, JSPROP_ENUMERATE),
    JS_FS("linkProgram", WebGLRenderingContext_linkProgram, 1, JSPROP_ENUMERATE),
    JS_FS("pixelStorei", WebGLRenderingContext_pixelStorei, 2, JSPROP_ENUMERATE),
    JS_FS("renderbufferStorage", WebGLRenderingContext_renderbufferStorage, 4, JSPROP_ENUMERATE),
    JS_FS("shaderSource", WebGLRenderingContext_shaderSource, 2, JSPROP_ENUMERATE),
    JS_FS("texParameteri", WebGLRenderingContext_texParameteri, 3, JSPROP_ENUMERATE),
    JS_FS("uniform1f", WebGLRenderingContext_uniform1f, 2, JSPROP_ENUMERATE),
    JS_FS("uniform1fv", WebGLRenderingContext_uniform1fv, 2, JSPROP_ENUMERATE),
    JS_FS("uniform1i", WebGLRenderingContext_uniform1i, 2, JSPROP_ENUMERATE),
    JS_FS("uniform1iv", WebGLRenderingContext_uniform1iv, 2, JSPROP_ENUMERATE),
    JS_FS("uniform2f", WebGLRenderingContext_uniform2f, 2, JSPROP_ENUMERATE),
    JS_FS("uniform2fv", WebGLRenderingContext_uniform2fv, 2, JSPROP_ENUMERATE),
    JS_FS("uniform2i", WebGLRenderingContext_uniform2i, 2, JSPROP_ENUMERATE),
    JS_FS("uniform2iv", WebGLRenderingContext_uniform2iv, 2, JSPROP_ENUMERATE),
    JS_FS("uniform3f", WebGLRenderingContext_uniform3f, 2, JSPROP_ENUMERATE),
    JS_FS("uniform3fv", WebGLRenderingContext_uniform3fv, 2, JSPROP_ENUMERATE),
    JS_FS("uniform3i", WebGLRenderingContext_uniform3i, 2, JSPROP_ENUMERATE),
    JS_FS("uniform3iv", WebGLRenderingContext_uniform3iv, 2, JSPROP_ENUMERATE),
    JS_FS("uniform4f", WebGLRenderingContext_uniform4f, 2, JSPROP_ENUMERATE),
    JS_FS("uniform4fv", WebGLRenderingContext_uniform4fv, 2, JSPROP_ENUMERATE),
    JS_FS("uniform4i", WebGLRenderingContext_uniform4i, 2, JSPROP_ENUMERATE),
    JS_FS("uniform4iv", WebGLRenderingContext_uniform4iv, 2, JSPROP_ENUMERATE),
    JS_FS("uniformMatrix2fv", WebGLRenderingContext_uniformMatrix2fv, 3, JSPROP_ENUMERATE),
    JS_FS("uniformMatrix3fv", WebGLRenderingContext_uniformMatrix3fv, 3, JSPROP_ENUMERATE),
    JS_FS("uniformMatrix4fv", WebGLRenderingContext_uniformMatrix4fv, 3, JSPROP_ENUMERATE),
    JS_FS("vertexAttrib1f", WebGLRenderingContext_vertexAttrib1f, 2, JSPROP_ENUMERATE),
    JS_FS("vertexAttrib1fv", WebGLRenderingContext_vertexAttrib1fv, 2, JSPROP_ENUMERATE),
    JS_FS("vertexAttrib2f", WebGLRenderingContext_vertexAttrib2f, 2, JSPROP_ENUMERATE),
    JS_FS("vertexAttrib2fv", WebGLRenderingContext_vertexAttrib2fv, 2, JSPROP_ENUMERATE),
    JS_FS("vertexAttrib3f", WebGLRenderingContext_vertexAttrib3f, 2, JSPROP_ENUMERATE),
    JS_FS("vertexAttrib3fv", WebGLRenderingContext_vertexAttrib3fv, 2, JSPROP_ENUMERATE),
    JS_FS("vertexAttrib4f", WebGLRenderingContext_vertexAttrib4f, 2, JSPROP_ENUMERATE),
    JS_FS("vertexAttrib4fv", WebGLRenderingContext_vertexAttrib4fv, 2, JSPROP_ENUMERATE),
    JS_FS("vertexAttribPointer", WebGLRenderingContext_vertexAttribPointer, 6, JSPROP_ENUMERATE),
    JS_FS("viewport", WebGLRenderingContext_viewport, 4, JSPROP_ENUMERATE),
    JS_FS("useProgram", WebGLRenderingContext_useProgram, 1, JSPROP_ENUMERATE),
    JS_FS("getError", WebGLRenderingContext_getError, 0, JSPROP_ENUMERATE),
    JS_FS("swapBuffer", WebGLRenderingContext_swapBuffer, 0, JSPROP_ENUMERATE),
    JS_FS_END
};

JSConstDoubleSpec WebGLRenderingContext_const [] = {
    //{NGL_ES_VERSION_2_0, "ES_VERSION_2_0", 0, {0,0,0}},
    {NGL_DEPTH_BUFFER_BIT, "DEPTH_BUFFER_BIT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_BUFFER_BIT, "STENCIL_BUFFER_BIT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_COLOR_BUFFER_BIT, "COLOR_BUFFER_BIT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_POINTS, "POINTS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_LINES, "LINES", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_LINE_LOOP, "LINE_LOOP", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_LINE_STRIP, "LINE_STRIP", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TRIANGLES, "TRIANGLES", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TRIANGLE_STRIP, "TRIANGLE_STRIP", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TRIANGLE_FAN, "TRIANGLE_FAN", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ZERO, "ZERO", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ONE, "ONE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SRC_COLOR, "SRC_COLOR", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ONE_MINUS_SRC_COLOR, "ONE_MINUS_SRC_COLOR", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SRC_ALPHA, "SRC_ALPHA", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ONE_MINUS_SRC_ALPHA, "ONE_MINUS_SRC_ALPHA", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DST_ALPHA, "DST_ALPHA", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ONE_MINUS_DST_ALPHA, "ONE_MINUS_DST_ALPHA", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DST_COLOR, "DST_COLOR", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ONE_MINUS_DST_COLOR, "ONE_MINUS_DST_COLOR", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SRC_ALPHA_SATURATE, "SRC_ALPHA_SATURATE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FUNC_ADD, "FUNC_ADD", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BLEND_EQUATION, "BLEND_EQUATION", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BLEND_EQUATION_RGB, "BLEND_EQUATION_RGB", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BLEND_EQUATION_ALPHA, "BLEND_EQUATION_ALPHA", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FUNC_SUBTRACT, "FUNC_SUBTRACT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FUNC_REVERSE_SUBTRACT, "FUNC_REVERSE_SUBTRACT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BLEND_DST_RGB, "BLEND_DST_RGB", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BLEND_SRC_RGB, "BLEND_SRC_RGB", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BLEND_DST_ALPHA, "BLEND_DST_ALPHA", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BLEND_SRC_ALPHA, "BLEND_SRC_ALPHA", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_CONSTANT_COLOR, "CONSTANT_COLOR", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ONE_MINUS_CONSTANT_COLOR, "ONE_MINUS_CONSTANT_COLOR", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_CONSTANT_ALPHA, "CONSTANT_ALPHA", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ONE_MINUS_CONSTANT_ALPHA, "ONE_MINUS_CONSTANT_ALPHA", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BLEND_COLOR, "BLEND_COLOR", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ARRAY_BUFFER, "ARRAY_BUFFER", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ELEMENT_ARRAY_BUFFER, "ELEMENT_ARRAY_BUFFER", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ARRAY_BUFFER_BINDING, "ARRAY_BUFFER_BINDING", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ELEMENT_ARRAY_BUFFER_BINDING, "ELEMENT_ARRAY_BUFFER_BINDING", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STREAM_DRAW, "STREAM_DRAW", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STATIC_DRAW, "STATIC_DRAW", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DYNAMIC_DRAW, "DYNAMIC_DRAW", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BUFFER_SIZE, "BUFFER_SIZE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BUFFER_USAGE, "BUFFER_USAGE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_CURRENT_VERTEX_ATTRIB, "CURRENT_VERTEX_ATTRIB", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FRONT, "FRONT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BACK, "BACK", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FRONT_AND_BACK, "FRONT_AND_BACK", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE_2D, "TEXTURE_2D", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_CULL_FACE, "CULL_FACE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BLEND, "BLEND", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DITHER, "DITHER", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_TEST, "STENCIL_TEST", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DEPTH_TEST, "DEPTH_TEST", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SCISSOR_TEST, "SCISSOR_TEST", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_POLYGON_OFFSET_FILL, "POLYGON_OFFSET_FILL", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SAMPLE_ALPHA_TO_COVERAGE, "SAMPLE_ALPHA_TO_COVERAGE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SAMPLE_COVERAGE, "SAMPLE_COVERAGE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_NO_ERROR, "NO_ERROR", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_INVALID_ENUM, "INVALID_ENUM", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_INVALID_VALUE, "INVALID_VALUE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_INVALID_OPERATION, "INVALID_OPERATION", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_OUT_OF_MEMORY, "OUT_OF_MEMORY", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_CW, "CW", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_CCW, "CCW", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_LINE_WIDTH, "LINE_WIDTH", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ALIASED_POINT_SIZE_RANGE, "ALIASED_POINT_SIZE_RANGE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ALIASED_LINE_WIDTH_RANGE, "ALIASED_LINE_WIDTH_RANGE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_CULL_FACE_MODE, "CULL_FACE_MODE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FRONT_FACE, "FRONT_FACE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DEPTH_RANGE, "DEPTH_RANGE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DEPTH_WRITEMASK, "DEPTH_WRITEMASK", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DEPTH_CLEAR_VALUE, "DEPTH_CLEAR_VALUE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DEPTH_FUNC, "DEPTH_FUNC", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_CLEAR_VALUE, "STENCIL_CLEAR_VALUE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_FUNC, "STENCIL_FUNC", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_FAIL, "STENCIL_FAIL", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_PASS_DEPTH_FAIL, "STENCIL_PASS_DEPTH_FAIL", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_PASS_DEPTH_PASS, "STENCIL_PASS_DEPTH_PASS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_REF, "STENCIL_REF", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_VALUE_MASK, "STENCIL_VALUE_MASK", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_WRITEMASK, "STENCIL_WRITEMASK", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_BACK_FUNC, "STENCIL_BACK_FUNC", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_BACK_FAIL, "STENCIL_BACK_FAIL", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_BACK_PASS_DEPTH_FAIL, "STENCIL_BACK_PASS_DEPTH_FAIL", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_BACK_PASS_DEPTH_PASS, "STENCIL_BACK_PASS_DEPTH_PASS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_BACK_REF, "STENCIL_BACK_REF", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_BACK_VALUE_MASK, "STENCIL_BACK_VALUE_MASK", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_BACK_WRITEMASK, "STENCIL_BACK_WRITEMASK", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_VIEWPORT, "VIEWPORT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SCISSOR_BOX, "SCISSOR_BOX", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_COLOR_CLEAR_VALUE, "COLOR_CLEAR_VALUE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_COLOR_WRITEMASK, "COLOR_WRITEMASK", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_UNPACK_ALIGNMENT, "UNPACK_ALIGNMENT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_PACK_ALIGNMENT, "PACK_ALIGNMENT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_MAX_TEXTURE_SIZE, "MAX_TEXTURE_SIZE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_MAX_VIEWPORT_DIMS, "MAX_VIEWPORT_DIMS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SUBPIXEL_BITS, "SUBPIXEL_BITS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RED_BITS, "RED_BITS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_GREEN_BITS, "GREEN_BITS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BLUE_BITS, "BLUE_BITS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ALPHA_BITS, "ALPHA_BITS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DEPTH_BITS, "DEPTH_BITS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_BITS, "STENCIL_BITS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_POLYGON_OFFSET_UNITS, "POLYGON_OFFSET_UNITS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_POLYGON_OFFSET_FACTOR, "POLYGON_OFFSET_FACTOR", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE_BINDING_2D, "TEXTURE_BINDING_2D", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SAMPLE_BUFFERS, "SAMPLE_BUFFERS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SAMPLES, "SAMPLES", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SAMPLE_COVERAGE_VALUE, "SAMPLE_COVERAGE_VALUE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SAMPLE_COVERAGE_INVERT, "SAMPLE_COVERAGE_INVERT", JSPROP_ENUMERATE, {0,0,0}},
    //{NGL_NUM_COMPRESSED_TEXTURE_FORMATS, "NUM_COMPRESSED_TEXTURE_FORMATS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_COMPRESSED_TEXTURE_FORMATS, "COMPRESSED_TEXTURE_FORMATS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DONT_CARE, "DONT_CARE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FASTEST, "FASTEST", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_NICEST, "NICEST", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_GENERATE_MIPMAP_HINT, "GENERATE_MIPMAP_HINT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BYTE, "BYTE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_UNSIGNED_BYTE, "UNSIGNED_BYTE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SHORT, "SHORT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_UNSIGNED_SHORT, "UNSIGNED_SHORT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_INT, "INT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_UNSIGNED_INT, "UNSIGNED_INT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FLOAT, "FLOAT", JSPROP_ENUMERATE, {0,0,0}},
    //{NGL_FIXED, "FIXED", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DEPTH_COMPONENT, "DEPTH_COMPONENT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ALPHA, "ALPHA", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RGB, "RGB", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RGBA, "RGBA", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_LUMINANCE, "LUMINANCE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_LUMINANCE_ALPHA, "LUMINANCE_ALPHA", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_UNSIGNED_SHORT_4_4_4_4, "UNSIGNED_SHORT_4_4_4_4", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_UNSIGNED_SHORT_5_5_5_1, "UNSIGNED_SHORT_5_5_5_1", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_UNSIGNED_SHORT_5_6_5, "UNSIGNED_SHORT_5_6_5", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FRAGMENT_SHADER, "FRAGMENT_SHADER", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_VERTEX_SHADER, "VERTEX_SHADER", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_MAX_VERTEX_ATTRIBS, "MAX_VERTEX_ATTRIBS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_MAX_VERTEX_UNIFORM_VECTORS, "MAX_VERTEX_UNIFORM_VECTORS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_MAX_VARYING_VECTORS, "MAX_VARYING_VECTORS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, "MAX_COMBINED_TEXTURE_IMAGE_UNITS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, "MAX_VERTEX_TEXTURE_IMAGE_UNITS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_MAX_TEXTURE_IMAGE_UNITS, "MAX_TEXTURE_IMAGE_UNITS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_MAX_FRAGMENT_UNIFORM_VECTORS, "MAX_FRAGMENT_UNIFORM_VECTORS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SHADER_TYPE, "SHADER_TYPE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DELETE_STATUS, "DELETE_STATUS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_LINK_STATUS, "LINK_STATUS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_VALIDATE_STATUS, "VALIDATE_STATUS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ATTACHED_SHADERS, "ATTACHED_SHADERS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ACTIVE_UNIFORMS, "ACTIVE_UNIFORMS", JSPROP_ENUMERATE, {0,0,0}},
    //{NGL_ACTIVE_UNIFORM_MAX_LENGTH, "ACTIVE_UNIFORM_MAX_LENGTH", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ACTIVE_ATTRIBUTES, "ACTIVE_ATTRIBUTES", JSPROP_ENUMERATE, {0,0,0}},
    //{NGL_ACTIVE_ATTRIBUTE_MAX_LENGTH, "ACTIVE_ATTRIBUTE_MAX_LENGTH", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SHADING_LANGUAGE_VERSION, "SHADING_LANGUAGE_VERSION", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_CURRENT_PROGRAM, "CURRENT_PROGRAM", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_NEVER, "NEVER", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_LESS, "LESS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_EQUAL, "EQUAL", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_LEQUAL, "LEQUAL", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_GREATER, "GREATER", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_NOTEQUAL, "NOTEQUAL", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_GEQUAL, "GEQUAL", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ALWAYS, "ALWAYS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_KEEP, "KEEP", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_REPLACE, "REPLACE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_INCR, "INCR", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DECR, "DECR", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_INVERT, "INVERT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_INCR_WRAP, "INCR_WRAP", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DECR_WRAP, "DECR_WRAP", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_VENDOR, "VENDOR", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RENDERER, "RENDERER", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_VERSION, "VERSION", JSPROP_ENUMERATE, {0,0,0}},
    //{NGL_EXTENSIONS, "EXTENSIONS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_NEAREST, "NEAREST", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_LINEAR, "LINEAR", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_NEAREST_MIPMAP_NEAREST, "NEAREST_MIPMAP_NEAREST", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_LINEAR_MIPMAP_NEAREST, "LINEAR_MIPMAP_NEAREST", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_NEAREST_MIPMAP_LINEAR, "NEAREST_MIPMAP_LINEAR", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_LINEAR_MIPMAP_LINEAR, "LINEAR_MIPMAP_LINEAR", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE_MAG_FILTER, "TEXTURE_MAG_FILTER", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE_MIN_FILTER, "TEXTURE_MIN_FILTER", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE_WRAP_S, "TEXTURE_WRAP_S", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE_WRAP_T, "TEXTURE_WRAP_T", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE, "TEXTURE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE_CUBE_MAP, "TEXTURE_CUBE_MAP", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE_BINDING_CUBE_MAP, "TEXTURE_BINDING_CUBE_MAP", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE_CUBE_MAP_POSITIVE_X, "TEXTURE_CUBE_MAP_POSITIVE_X", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE_CUBE_MAP_NEGATIVE_X, "TEXTURE_CUBE_MAP_NEGATIVE_X", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE_CUBE_MAP_POSITIVE_Y, "TEXTURE_CUBE_MAP_POSITIVE_Y", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "TEXTURE_CUBE_MAP_NEGATIVE_Y", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE_CUBE_MAP_POSITIVE_Z, "TEXTURE_CUBE_MAP_POSITIVE_Z", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "TEXTURE_CUBE_MAP_NEGATIVE_Z", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_MAX_CUBE_MAP_TEXTURE_SIZE, "MAX_CUBE_MAP_TEXTURE_SIZE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE0, "TEXTURE0", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE1, "TEXTURE1", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE2, "TEXTURE2", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE3, "TEXTURE3", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE4, "TEXTURE4", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE5, "TEXTURE5", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE6, "TEXTURE6", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE7, "TEXTURE7", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE8, "TEXTURE8", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE9, "TEXTURE9", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE10, "TEXTURE10", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE11, "TEXTURE11", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE12, "TEXTURE12", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE13, "TEXTURE13", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE14, "TEXTURE14", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE15, "TEXTURE15", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE16, "TEXTURE16", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE17, "TEXTURE17", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE18, "TEXTURE18", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE19, "TEXTURE19", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE20, "TEXTURE20", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE21, "TEXTURE21", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE22, "TEXTURE22", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE23, "TEXTURE23", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE24, "TEXTURE24", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE25, "TEXTURE25", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE26, "TEXTURE26", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE27, "TEXTURE27", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE28, "TEXTURE28", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE29, "TEXTURE29", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE30, "TEXTURE30", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_TEXTURE31, "TEXTURE31", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_ACTIVE_TEXTURE, "ACTIVE_TEXTURE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_REPEAT, "REPEAT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_CLAMP_TO_EDGE, "CLAMP_TO_EDGE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_MIRRORED_REPEAT, "MIRRORED_REPEAT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FLOAT_VEC2, "FLOAT_VEC2", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FLOAT_VEC3, "FLOAT_VEC3", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FLOAT_VEC4, "FLOAT_VEC4", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_INT_VEC2, "INT_VEC2", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_INT_VEC3, "INT_VEC3", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_INT_VEC4, "INT_VEC4", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BOOL, "BOOL", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BOOL_VEC2, "BOOL_VEC2", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BOOL_VEC3, "BOOL_VEC3", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BOOL_VEC4, "BOOL_VEC4", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FLOAT_MAT2, "FLOAT_MAT2", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FLOAT_MAT3, "FLOAT_MAT3", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FLOAT_MAT4, "FLOAT_MAT4", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SAMPLER_2D, "SAMPLER_2D", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_SAMPLER_CUBE, "SAMPLER_CUBE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_VERTEX_ATTRIB_ARRAY_ENABLED, "VERTEX_ATTRIB_ARRAY_ENABLED", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_VERTEX_ATTRIB_ARRAY_SIZE, "VERTEX_ATTRIB_ARRAY_SIZE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_VERTEX_ATTRIB_ARRAY_STRIDE, "VERTEX_ATTRIB_ARRAY_STRIDE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_VERTEX_ATTRIB_ARRAY_TYPE, "VERTEX_ATTRIB_ARRAY_TYPE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_VERTEX_ATTRIB_ARRAY_NORMALIZED, "VERTEX_ATTRIB_ARRAY_NORMALIZED", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_VERTEX_ATTRIB_ARRAY_POINTER, "VERTEX_ATTRIB_ARRAY_POINTER", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, "VERTEX_ATTRIB_ARRAY_BUFFER_BINDING", JSPROP_ENUMERATE, {0,0,0}},
    //{NGL_IMPLEMENTATION_COLOR_READ_TYPE, "IMPLEMENTATION_COLOR_READ_TYPE", JSPROP_ENUMERATE, {0,0,0}},
    //{NGL_IMPLEMENTATION_COLOR_READ_FORMAT, "IMPLEMENTATION_COLOR_READ_FORMAT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_COMPILE_STATUS, "COMPILE_STATUS", JSPROP_ENUMERATE, {0,0,0}},
    //{NGL_INFO_LOG_LENGTH, "INFO_LOG_LENGTH", JSPROP_ENUMERATE, {0,0,0}},
    //{NGL_SHADER_SOURCE_LENGTH, "SHADER_SOURCE_LENGTH", JSPROP_ENUMERATE, {0,0,0}},
    //{NGL_SHADER_COMPILER, "SHADER_COMPILER", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_LOW_FLOAT, "LOW_FLOAT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_MEDIUM_FLOAT, "MEDIUM_FLOAT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_HIGH_FLOAT, "HIGH_FLOAT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_LOW_INT, "LOW_INT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_MEDIUM_INT, "MEDIUM_INT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_HIGH_INT, "HIGH_INT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FRAMEBUFFER, "FRAMEBUFFER", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RENDERBUFFER, "RENDERBUFFER", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RGBA4, "RGBA4", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RGB5_A1, "RGB5_A1", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RGB565, "RGB565", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DEPTH_COMPONENT16, "DEPTH_COMPONENT16", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_INDEX, "STENCIL_INDEX", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_INDEX8, "STENCIL_INDEX8", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DEPTH_STENCIL, "DEPTH_STENCIL", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RENDERBUFFER_WIDTH, "RENDERBUFFER_WIDTH", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RENDERBUFFER_HEIGHT, "RENDERBUFFER_HEIGHT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RENDERBUFFER_INTERNAL_FORMAT, "RENDERBUFFER_INTERNAL_FORMAT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RENDERBUFFER_RED_SIZE, "RENDERBUFFER_RED_SIZE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RENDERBUFFER_GREEN_SIZE, "RENDERBUFFER_GREEN_SIZE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RENDERBUFFER_BLUE_SIZE, "RENDERBUFFER_BLUE_SIZE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RENDERBUFFER_ALPHA_SIZE, "RENDERBUFFER_ALPHA_SIZE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RENDERBUFFER_DEPTH_SIZE, "RENDERBUFFER_DEPTH_SIZE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RENDERBUFFER_STENCIL_SIZE, "RENDERBUFFER_STENCIL_SIZE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, "FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, "FRAMEBUFFER_ATTACHMENT_OBJECT_NAME", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, "FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE, "FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_COLOR_ATTACHMENT0, "COLOR_ATTACHMENT0", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DEPTH_ATTACHMENT, "DEPTH_ATTACHMENT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_STENCIL_ATTACHMENT, "STENCIL_ATTACHMENT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_DEPTH_STENCIL_ATTACHMENT, "DEPTH_STENCIL_ATTACHMENT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_NONE, "NONE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FRAMEBUFFER_COMPLETE, "FRAMEBUFFER_COMPLETE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT, "FRAMEBUFFER_INCOMPLETE_ATTACHMENT", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT, "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT", JSPROP_ENUMERATE, {0,0,0}},
    //{NGL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT, "FRAMEBUFFER_INCOMPLETE_DIMENSIONS", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FRAMEBUFFER_UNSUPPORTED, "FRAMEBUFFER_UNSUPPORTED", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_FRAMEBUFFER_BINDING, "FRAMEBUFFER_BINDING", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_RENDERBUFFER_BINDING, "RENDERBUFFER_BINDING", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_MAX_RENDERBUFFER_SIZE, "MAX_RENDERBUFFER_SIZE", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_INVALID_FRAMEBUFFER_OPERATION, "INVALID_FRAMEBUFFER_OPERATION", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_UNPACK_FLIP_Y_WEBGL, "UNPACK_FLIP_Y_WEBGL", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_UNPACK_PREMULTIPLY_ALPHA_WEBGL, "UNPACK_PREMULTIPLY_ALPHA_WEBGL", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_CONTEXT_LOST_WEBGL, "CONTEXT_LOST_WEBGL", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_UNPACK_COLORSPACE_CONVERSION_WEBGL, "UNPACK_COLORSPACE_CONVERSION_WEBGL", JSPROP_ENUMERATE, {0,0,0}},
    {NGL_BROWSER_DEFAULT_WEBGL, "BROWSER_DEFAULT_WEBGL", JSPROP_ENUMERATE, {0,0,0}},
    {0, NULL, 0, {0,0,0}}
};

NGL_JS_FN(WebGLRenderingContext_isContextLost)
//{
    // TODO
    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(false));
    return true;
}

NGL_JS_FN(WebGLRenderingContext_getExtension)
//{
    JS_SET_RVAL(cx, vp, JSVAL_NULL);
    return true;
}


NGL_JS_FN(WebGLRenderingContext_activeTexture)
//{
	GLuint texture;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &texture)) {
        return false;
    }

    GL_CALL(CppObj, ActiveTexture(texture));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_attachShader) 
//{
    NGLShader *cshader;
    JSObject *program;
    JSObject *shader;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oo", &program, &shader)) {
        return false;
    }

    WebGLResource *res = (WebGLResource *)JS_GetInstancePrivate(cx, program,
        &WebGLProgram_class, JS_ARGV(cx, vp));

    cshader = (NGLShader *)JS_GetInstancePrivate(cx, shader,
        &WebGLShader_class, JS_ARGV(cx, vp));

    GL_CALL(CppObj, AttachShader(res->id(), cshader->shader));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_bindAttribLocation) 
//{
    GLuint vertex;
    const char *cname;
    JSObject *program;
    JSString *name;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ouS", &program, &vertex, &name)) {
        return false;
    }

    WebGLResource *res = (WebGLResource *)JS_GetInstancePrivate(cx, program,
        &WebGLProgram_class, JS_ARGV(cx, vp));

    cname = JS_EncodeString(cx, name);

    GL_CALL(CppObj, BindAttribLocation(res->id(), vertex, cname));

    JS_free(cx, (void *)cname);
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_bindBuffer)
//{
    GLenum target;
    JSObject *buffer;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uo", &target, &buffer)) {
        return false;
    }

    if (buffer == NULL) {
        JS_SetReservedSlot(thisobj, WebGLResource::kBuffer, JSVAL_NULL);
        GL_CALL(CppObj, BindBuffer(target, 0));

        return true;
    }

    WebGLResource *res = (WebGLResource *)JS_GetInstancePrivate(cx, buffer,
        &WebGLBuffer_class, JS_ARGV(cx, vp));

    JS_SetReservedSlot(thisobj, WebGLResource::kBuffer,
        OBJECT_TO_JSVAL(res->jsobj()));

    GL_CALL(CppObj, BindBuffer(target, res->id()));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_bindFramebuffer)
//{
    GLenum target;
    JSObject *buffer;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uo", &target, &buffer)) {
        return false;
    }

	if (buffer == NULL) {
        /*
            Bind to the default framebuffer
        */
        uint32_t fbo = CppObj->getFrameBufferID();
        GL_CALL(CppObj, BindFramebuffer(target, fbo));

        JS_SetReservedSlot(thisobj, WebGLResource::kFrameBuffer, JSVAL_NULL);

        return true;
    }

    WebGLResource *res = (WebGLResource *)JS_GetInstancePrivate(cx,
        buffer, &WebGLFrameBuffer_class, JS_ARGV(cx, vp));

    JS_SetReservedSlot(thisobj, WebGLResource::kFrameBuffer,
        OBJECT_TO_JSVAL(res->jsobj()));

    GL_CALL(CppObj, BindFramebuffer(target, res->id()));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_bindRenderbuffer)
//{
    GLenum target;
    JSObject *buffer;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uo", &target, &buffer)) {
        return false;
    }

	if (buffer == NULL) {
        GL_CALL(CppObj, BindRenderbuffer(target, 0));
        JS_SetReservedSlot(thisobj, WebGLResource::kRenderBuffer, JSVAL_NULL);
        return true;
    }

    WebGLResource *res = (WebGLResource *)JS_GetInstancePrivate(cx, buffer,
        &WebGLRenderbuffer_class, JS_ARGV(cx, vp));

    JS_SetReservedSlot(thisobj, WebGLResource::kRenderBuffer,
        OBJECT_TO_JSVAL(res->jsobj()));

    GL_CALL(CppObj, BindRenderbuffer(target, res->id()));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_bindTexture)
//{
    GLenum target;
    JSObject *texture;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uo", &target, 
        &texture)) {
        return false;
    }

    if (texture == NULL) {
        GL_CALL(CppObj, BindTexture(target, 0));
        JS_SetReservedSlot(thisobj, WebGLResource::kTexture, JSVAL_NULL);        
        return true;
    }

	// No argv passed, as texture starting at 0
    WebGLResource *res = (WebGLResource *)JS_GetInstancePrivate(cx,
        texture, &WebGLTexture_class, NULL);

    JS_SetReservedSlot(thisobj, WebGLResource::kTexture,
        OBJECT_TO_JSVAL(res->jsobj()));

    GL_CALL(CppObj, BindTexture(target, res->id()));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_blendEquation)
//{
    GLuint mode;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &mode)) {
        return false;
    }
    
    GL_CALL(CppObj, BlendEquation(mode));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_blendEquationSeparate)    
//{
    GLuint modeRGB;
    GLenum modeAlpha;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uu", &modeRGB, &modeAlpha)) {
        return false;
    }
    
    GL_CALL(CppObj, BlendEquationSeparate(modeRGB, modeAlpha));

    return true;
}

NGL_JS_FN(WebGLRenderingContext_blendFunc)
//{
    GLenum sfactor;
    GLuint dfactor;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uu", &sfactor, &dfactor)) {
        return false;
    }
    
    GL_CALL(CppObj, BlendFunc(sfactor, dfactor));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_blendFuncSeparate)
//{
    GLuint srcRGB;
    GLuint dstRGB;
    GLenum srcAlpha;
    GLenum dstAlpha;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuuu",
        &srcRGB, &dstRGB, &srcAlpha, &dstAlpha)) {
        return false;
    }
    
    GL_CALL(CppObj, BlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha));

    return true;
}


NGL_JS_FN(WebGLRenderingContext_bufferData)
//{
    GLenum target;
    JSObject *array;
    GLenum usage;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uou",
        &target, &array, &usage)) {
        return false;
    }

	if (array == NULL || !JS_IsTypedArrayObject(array)) {
        JS_ReportError(cx, "Invalid value");
		return false;
	}

    GL_CALL(CppObj, BufferData(target, JS_GetArrayBufferViewByteLength(array),
        JS_GetArrayBufferViewData(array), usage));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_bufferSubData)
//{
    GLenum target;
    GLint offset;
    JSObject *array;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuo",
        &target, &offset, &array)) {
        return false;
    }

	if (array == NULL || !JS_IsTypedArrayObject(array) ||
        !JS_IsArrayBufferViewObject(array)) {

        JS_ReportError(cx, "Invalid value");
		return false;
	}

    GL_CALL(CppObj, BufferSubData(target, offset,
        JS_GetArrayBufferViewByteLength(array), JS_GetArrayBufferViewData(array)));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_clear)
//{
    GLbitfield bits;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "i", &bits)) {
        return false;
    }
    
    GL_CALL(CppObj, Clear(bits | GL_DEPTH_BUFFER_BIT));

    return true;
}

NGL_JS_FN(WebGLRenderingContext_clearColor)
//{
    double r;
    double g;
    double b;
    double a;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddd", &r, &g, &b, &a)) {
        return false;
    }

    GL_CALL(CppObj, ClearColor(r, g, b, a));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_clearDepth)
//{
    double clampd;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "d", &clampd)) {
        return false;
    }
    GL_CALL(CppObj, ClearDepth(clampd));

    return true;
}

NGL_JS_FN(WebGLRenderingContext_clearStencil)
//{
    GLint s;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "i", &s)) {
        return false;
    }

    GL_CALL(CppObj, ClearStencil(s));

    return true;
}

NGL_JS_FN(WebGLRenderingContext_compileShader)
//{
    NGLShader *cshader;
    JSObject *shader;
    char *shaderStr;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &shader)) {
        return false;
    }

    cshader = (NGLShader *)JS_GetInstancePrivate(cx, shader,
        &WebGLShader_class, JS_ARGV(cx, vp));

    if (!(shaderStr = NativeCanvasContext::processShader(
            cshader->source, (NativeCanvasContext::shaderType)cshader->type))) {
        return false;
    }

    GLint shaderLen = strlen(shaderStr);
    GL_CALL(CppObj, ShaderSource(cshader->shader, 1, &shaderStr, &shaderLen));

    GL_CALL(CppObj, CompileShader(cshader->shader));

    JS_free(cx, (void *)cshader->source);
    free(shaderStr);
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_createBuffer)
//{
    intptr_t buffer;
    jsval proto;
    JSObject *ret;
    
    GL_CALL(CppObj, GenBuffers(1, (GLuint *)&buffer));

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLBuffer", &proto);
    ret = JS_NewObject(cx, &WebGLBuffer_class, JSVAL_TO_OBJECT(proto), NULL);

    JS_SetPrivate(ret, new WebGLResource(buffer,
        WebGLResource::kBuffer, CppObj, ret));

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return true;
}

NGL_JS_FN(WebGLRenderingContext_createFramebuffer)
//{
    uintptr_t buffer;
    jsval proto;
    JSObject *ret;
    
    GL_CALL(CppObj, GenFramebuffers(1, (GLuint *)&buffer));

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLFrameBuffer", &proto);
    ret = JS_NewObject(cx, &WebGLFrameBuffer_class, JSVAL_TO_OBJECT(proto), NULL);

    JS_SetPrivate(ret, new WebGLResource(buffer,
        WebGLResource::kFrameBuffer, CppObj, ret));

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return true;
}

NGL_JS_FN(WebGLRenderingContext_createRenderbuffer)
//{
    intptr_t buffer;
    jsval proto;
    JSObject *ret;
    
    GL_CALL(CppObj, GenRenderbuffers(1, (GLuint *)&buffer));

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLRenderbuffer", &proto);
    ret = JS_NewObject(cx, &WebGLRenderbuffer_class, JSVAL_TO_OBJECT(proto), NULL);

    JS_SetPrivate(ret, new WebGLResource(buffer,
        WebGLResource::kRenderBuffer, CppObj, ret));

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return true;
}

NGL_JS_FN(WebGLRenderingContext_createProgram)
//{
    GLuint program;
    jsval proto;
    JSObject *ret;

    GL_CALL_RET(CppObj, CreateProgram(), program);
    
    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLProgram", &proto);
    ret = JS_NewObject(cx, &WebGLProgram_class, JSVAL_TO_OBJECT(proto), NULL);

    JS_SetPrivate(ret, new WebGLResource(program,
        WebGLResource::kProgram, CppObj, ret));
    
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_createShader)    
//{
    GLenum type;
    NGLShader *shader;
    GLuint cshader;
    jsval proto;
    JSObject *ret;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &type)) {
        return false;
    }

    if (type != NGL_VERTEX_SHADER && type != NGL_FRAGMENT_SHADER) {
        JS_ReportError(cx, "Invalid shader type");
        return false;
    }

    GL_CALL_RET(CppObj, CreateShader(type), cshader);
    shader = new NGLShader(type, cshader);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLShader", &proto);
    ret = JS_NewObject(cx, &WebGLShader_class, JSVAL_TO_OBJECT(proto), NULL);

    JS_SetPrivate(ret, (void *)shader);
    
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_createTexture)
//{
    uintptr_t texture;
	JSObject *ret;
    jsval proto;

    GL_CALL(CppObj, GenTextures(1, (GLuint *)&texture));

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLTexture", &proto);

    ret = JS_NewObject(cx, &WebGLTexture_class, JSVAL_TO_OBJECT(proto), NULL);

    JS_SetPrivate(ret, new WebGLResource(texture,
        WebGLResource::kTexture, CppObj, ret));

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return true;
}


NGL_JS_FN(WebGLRenderingContext_cullFace)
//{
    GLuint mode;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &mode)) {
        return false;
    }
    
    GL_CALL(CppObj, CullFace(mode));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_deleteShader)
//{
    JSObject *shader;
    NGLShader *cshader;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &shader)) {
        return false;
    }

    cshader = (NGLShader *)JS_GetInstancePrivate(cx, shader, &WebGLShader_class, JS_ARGV(cx, vp));
    GL_CALL(CppObj, DeleteShader(cshader->shader));
    delete cshader;
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_depthFunc)
//{
    GLuint func;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &func)) {
        return false;
    }
    
    GL_CALL(CppObj, DepthFunc(func));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_depthMask)
//{
    JSBool flag;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "b", &flag)) {
        return false;
    }

    GL_CALL(CppObj, DepthMask((GLboolean)flag));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_disable)
//{
    GLenum cap;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &cap)) {
        return false;
    }
    
    GL_CALL(CppObj, Disable(cap));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_disableVertexAttribArray)
//{
    GLuint attr;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &attr)) {
        return false;
    }
    
    GL_CALL(CppObj, DisableVertexAttribArray(attr));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_drawArrays)
//{
    GLenum mode;
    GLint first;
    GLsizei count;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uii", &mode, &first, &count)) {
        return false;
    }
    
    GL_CALL(CppObj, DrawArrays(mode, first, count));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_drawElements)
//{
    GLenum mode;
    GLsizei count;
    GLenum type;
    GLint offset;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uiui", &mode, &count, &type, &offset)) {
        return false;
    }

    if (offset+count < offset || offset+count < count) {
        JS_ReportError(cx, "Overflow in drawElements");
        return false;
    }

    GL_CALL(CppObj, DrawElements(mode, count, type, (void *)(intptr_t)offset));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_enable)
//{
    GLuint bits;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &bits)) {
        return false;
    }
    
    GL_CALL(CppObj, Enable(bits));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_enableVertexAttribArray)
//{
    GLuint attr;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &attr)) {
        return false;
    }

    GL_CALL(CppObj, EnableVertexAttribArray(attr));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_getUniformLocation)
//{
    GLint location;
    JSString *name;
    JSObject *program;
    JSObject *ret;
    jsval proto;
    const char *cname;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oS", &program, &name)) {
        return false;
    }

    WebGLResource *res = (WebGLResource *)JS_GetInstancePrivate(cx, program,
        &WebGLProgram_class, JS_ARGV(cx, vp));

    cname = JS_EncodeString(cx, name);
 
    GL_CALL_RET(CppObj, GetUniformLocation(res->id(), cname), location);

    if (location < 0 ) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
    } else {
        JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLUniformLocation", &proto);
        ret = JS_NewObject(cx, &WebGLUniformLocation_class, JSVAL_TO_OBJECT(proto), NULL);
        JS_SetPrivate(ret, (void *)(uintptr_t)location);
        
        JS_free(cx, (void *)cname);
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));
    }
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_getShaderPrecisionFormat)
//{
#define SET_PROP(prop, val) JS_SetProperty(cx, obj, prop, val)
    GLenum shaderType, precisionType;
    GLint crange[2];
    GLint cprecision;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uu", &shaderType, &precisionType)) {
        return false;
    }

    jsval proto, rangeMin, rangeMax, precision;
    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLShaderPrecisionFormat", &proto);
	JSObject *obj = JS_NewObject(cx, &WebGLShaderPrecisionFormat_class, JSVAL_TO_OBJECT(proto), NULL);

    // Since getShaderPrecisionFormat is not available everywhere...
    // (Taken from mozilla GLContext.h)

    switch (precisionType) {
        case NGL_LOW_FLOAT:
        case NGL_MEDIUM_FLOAT:
        case NGL_HIGH_FLOAT:
            // Assume IEEE 754 precision
            crange[0] = 127;
            crange[1] = 127;
            cprecision = 23;
            break;
        case NGL_LOW_INT:
        case NGL_MEDIUM_INT:
        case NGL_HIGH_INT:
            // Some (most) hardware only supports single-precision floating-point numbers,
            // which can accurately represent integers up to +/-16777216
            crange[0] = 24;
            crange[1] = 24;
            cprecision = 0;
            break;
    }

    rangeMin = INT_TO_JSVAL(crange[0]);
    rangeMax = INT_TO_JSVAL(crange[1]);
    precision = INT_TO_JSVAL(cprecision);

    SET_PROP("rangeMin", &rangeMin);
    SET_PROP("rangeMax", &rangeMax);
    SET_PROP("precision", &precision);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));

    return true;
#undef SET_PROP
}

NGL_JS_FN(WebGLRenderingContext_framebufferRenderbuffer)
//{
	GLenum target, attachement, renderbuffertarget;
	JSObject *renderbuffer;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuuo", &target,
        &attachement, &renderbuffertarget, &renderbuffer)) {
        return false;
    }

    WebGLResource *res = (WebGLResource *)JS_GetInstancePrivate(cx, renderbuffer,
        &WebGLRenderbuffer_class, JS_ARGV(cx, vp));

	GL_CALL(CppObj, FramebufferRenderbuffer(target, attachement,
        renderbuffertarget, res->id()));

    return true;
}

NGL_JS_FN(WebGLRenderingContext_framebufferTexture2D)
//{
    GLenum target, attachement, textarget;
	uintptr_t level;
    JSObject *texture;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuuoi",
        &target, &attachement, &textarget, &texture, &level)) {
        return false;
    }

	// No argv passed, as texture starting at 0
    WebGLResource *res = (WebGLResource *)JS_GetInstancePrivate(cx, texture,
        &WebGLTexture_class, NULL);

	GL_CALL(CppObj, FramebufferTexture2D(target, attachement,
        textarget, res->id(), level));

    GLenum status;
    GL_CALL_RET(CppObj, CheckFramebufferStatus(GL_FRAMEBUFFER), status);

    switch(status) {
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

NGL_JS_FN(WebGLRenderingContext_frontFace)
//{
    GLuint mode;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &mode)) {
        return false;
    }
    
    GL_CALL(CppObj, FrontFace(mode));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_generateMipmap)
//{
    GLenum target;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &target)) {
        return false;
    }
    
    GL_CALL(CppObj, GenerateMipmap(target));
    
    return true;
}


NGL_JS_FN(WebGLRenderingContext_getAttribLocation)
//{
    GLint location;
    JSString *attr;
    JSObject *program;
    const char *cattr;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oS", &program, &attr)) {
        return false;
    }
    
    WebGLResource *res = (WebGLResource *)JS_GetInstancePrivate(cx, program,
        &WebGLProgram_class, JS_ARGV(cx, vp));
    cattr = JS_EncodeString(cx, attr);

    GL_CALL_RET(CppObj, GetAttribLocation(res->id(), cattr), location);

    JS_free(cx, (void *)cattr);
    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(location));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_getParameter)
//{
    GLenum name;
    JS::Value value;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &name)) {
        return false;
    }

    switch (name) {
        // String
        case NGL_VENDOR:
        case NGL_RENDERER:
        case NGL_VERSION:
        case NGL_SHADING_LANGUAGE_VERSION:
        {
            const GLubyte *cstr;
            GL_CALL_RET(CppObj, GetString(name), cstr);
            JSString *str = JS_NewStringCopyZ(cx, (const char *)cstr);

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
        case NGL_GENERATE_MIPMAP_HINT:
        {
            GLint i = 0;
            GL_CALL(CppObj, GetIntegerv(name, &i));
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
        //case NGL_NUM_COMPRESSED_TEXTURE_FORMATS:
        {
            GLint i = 0;
            GL_CALL(CppObj, GetIntegerv(name, &i));
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
        case NGL_COMPRESSED_TEXTURE_FORMATS:
        {
            GLint length;
            GLint *textures;
            uint32_t *data;
            JSObject *obj;

            GL_CALL(CppObj, GetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &length));
            obj = JS_NewUint32Array(cx, length);
            textures = (GLint *)malloc(sizeof(GLint) * length);

            if (!obj || !textures) {
                if (textures != NULL) {
                    free(textures);
                }
                JS_ReportOutOfMemory(cx);
                return false;
            }

            GL_CALL(CppObj, GetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, textures));

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
        case NGL_STENCIL_WRITEMASK:
        {
            GLint i = 0; // the GL api (glGetIntegerv) only does signed ints
            GL_CALL(CppObj, GetIntegerv(name, &i));
            GLuint i_unsigned(i); // this is where -1 becomes 2^32-1
            double i_double(i_unsigned); // pass as FP value to allow large values such as 2^32-1.
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
        case NGL_SAMPLE_COVERAGE_VALUE:
        {
            GLfloat f = 0.f;
            GL_CALL(CppObj, GetFloatv(name, &f));
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
        case NGL_DEPTH_WRITEMASK:
        {
            GLboolean b = 0;
            GL_CALL(CppObj, GetBooleanv(name, &b));
            value.setBoolean(b);
            break;
        }

        // bool, WebGL-specific
        case NGL_UNPACK_FLIP_Y_WEBGL:
            value.setBoolean(CppObj->hasFlag(NativeCanvas3DContext::kUNPACK_FLIP_Y_WEBGL_Flag));
            break;
        case NGL_UNPACK_PREMULTIPLY_ALPHA_WEBGL:
            value.setBoolean(CppObj->hasFlag(NativeCanvas3DContext::kUNPACK_PREMULTIPLY_ALPHA_WEBGL_Flag));
            break;

        // uint, WebGL-specific
        case NGL_UNPACK_COLORSPACE_CONVERSION_WEBGL:
            value.setInt32(GL_NONE);
        break;

        // Complex values
        case NGL_DEPTH_RANGE: // 2 floats
        case NGL_ALIASED_POINT_SIZE_RANGE: // 2 floats
        case NGL_ALIASED_LINE_WIDTH_RANGE: // 2 floats
        {
            GLfloat fv[2] = { 0 };
            JSObject *obj = JS_NewFloat32Array(cx, 2);
            float *data;

            if (!obj) {
                JS_ReportOutOfMemory(cx);
                return false;
            }

            GL_CALL(CppObj, GetFloatv(name, fv));

            data = JS_GetFloat32ArrayData(obj);
            memcpy(data, fv, 2 * sizeof(float));
            value.setObjectOrNull(obj);
            break;
        }
        
        case NGL_COLOR_CLEAR_VALUE: // 4 floats
        case NGL_BLEND_COLOR: // 4 floats
        {
            GLfloat fv[4] = { 0 };
            JSObject *obj = JS_NewFloat32Array(cx, 4);
            float *data;

            if (!obj) {
                JS_ReportOutOfMemory(cx);
                return false;
            }

            GL_CALL(CppObj, GetFloatv(name, fv));

            data = JS_GetFloat32ArrayData(obj);
            memcpy(data, fv, 4 * sizeof(GLfloat));
            value.setObjectOrNull(obj);
            break;
        }

        case NGL_MAX_VIEWPORT_DIMS: // 2 ints
        {
            GLint iv[2] = { 0 };
            JSObject *obj = JS_NewInt32Array(cx, 2);
            int32_t *data;

            if (!obj) {
                JS_ReportOutOfMemory(cx);
                return false;
            }

            GL_CALL(CppObj, GetIntegerv(name, iv));

            data = JS_GetInt32ArrayData(obj);
            memcpy(data, iv, 2 * sizeof(GLint));
            value.setObjectOrNull(obj);
            break;
        }

        case NGL_SCISSOR_BOX: // 4 ints
        case NGL_VIEWPORT: // 4 ints
        {
            GLint iv[4] = { 0 };
            JSObject *obj = JS_NewInt32Array(cx, 4);
            int32_t *data;

            if (!obj) {
                JS_ReportOutOfMemory(cx);
                return false;
            }

            GL_CALL(CppObj, GetIntegerv(name, iv));

            data = JS_GetInt32ArrayData(obj);
            memcpy(data, iv, 4 * sizeof(GLint));
            value.setObjectOrNull(obj);
            break;
        }

        case NGL_COLOR_WRITEMASK: // 4 bools
        {
            GLboolean gl_bv[4] = { 0 };

            GL_CALL(CppObj, GetBooleanv(name, gl_bv));

            JS::Value vals[4] = { JS::BooleanValue(bool(gl_bv[0])),
                                  JS::BooleanValue(bool(gl_bv[1])),
                                  JS::BooleanValue(bool(gl_bv[2])),
                                  JS::BooleanValue(bool(gl_bv[3])) };

            JSObject* obj = JS_NewArrayObject(cx, 4, vals);

            if (!obj) {
                JS_ReportOutOfMemory(cx);
                return false;
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
            return true;
    }

    JS_SET_RVAL(cx, vp, value);
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_getProgramParameter) 
//{
    GLenum param;
    GLint status;
    JSObject *program;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ou",
        &program, &param)) {
        return false;
    }

    WebGLResource *res = (WebGLResource *)JS_GetInstancePrivate(cx, program,
        &WebGLProgram_class, JS_ARGV(cx, vp));

    GL_CALL(CppObj, GetProgramiv(res->id(), param, &status));
    
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
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_getProgramInfoLog)
//{
    GLsizei max;
    GLsizei length;
    JSString *log;
    JSObject *program;
    char *clog;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &program)) {
        return false;
    }

    WebGLResource *res = (WebGLResource *)JS_GetInstancePrivate(cx, program,
        &WebGLProgram_class, JS_ARGV(cx, vp));
    
    GL_CALL(CppObj, GetProgramiv(res->id(), GL_INFO_LOG_LENGTH, &max));

    clog = (char *)malloc(max);
    GL_CALL(CppObj, GetProgramInfoLog(res->id(), max, &length, clog));
    log = JS_NewStringCopyN(cx, clog, length);
    free(clog);

    JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(log));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_getShaderParameter)
//{
    NGLShader *cshader;
    GLenum pname;
    GLint param;
    JSObject *shader;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ou", &shader, &pname)) {
        return false;
    }

    cshader = (NGLShader *)JS_GetInstancePrivate(cx, shader, &WebGLShader_class, JS_ARGV(cx, vp));

    GL_CALL(CppObj, GetShaderiv(cshader->shader, pname, &param));

    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(param));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_getShaderInfoLog)
//{
    NGLShader *cshader;
    GLsizei length;
    GLsizei max;
    JSString *log;
    JSObject *shader;
    char *clog;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &shader)) {
        return false;
    }
    cshader = (NGLShader *)JS_GetInstancePrivate(cx, shader, &WebGLShader_class, JS_ARGV(cx, vp));
    
    GL_CALL(CppObj, GetShaderiv(cshader->shader, GL_INFO_LOG_LENGTH, &max));

    clog = (char *)malloc(max);
    GL_CALL(CppObj, GetShaderInfoLog(cshader->shader, max, &length, clog));
    log = JS_NewStringCopyN(cx, clog, length);
    free(clog);

    JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(log));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_lineWidth)
//{
    //GLfloat width;
    double width;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "d", &width)) {
        return false;
    }

    GL_CALL(CppObj, LineWidth((GLfloat)width));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_linkProgram)
//{
    JSObject *program;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &program)) {
        return false;
    }
    
    WebGLResource *res = (WebGLResource *)JS_GetInstancePrivate(cx, program,
        &WebGLProgram_class, JS_ARGV(cx, vp));

    GL_CALL(CppObj, LinkProgram(res->id()));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_pixelStorei) 
//{
    GLuint param;
    GLint value;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ui", &param, &value)) {
        return false;
    }

    switch (param) {
        case NGL_UNPACK_FLIP_Y_WEBGL:
        {
            if (value) {
                CppObj->addFlag(NativeCanvas3DContext::kUNPACK_FLIP_Y_WEBGL_Flag);
            } else {
                CppObj->removeFlag(NativeCanvas3DContext::kUNPACK_FLIP_Y_WEBGL_Flag);
            }
            break;
        }
        case NGL_UNPACK_PREMULTIPLY_ALPHA_WEBGL:
        {
            if (value) {
                CppObj->addFlag(NativeCanvas3DContext::kUNPACK_PREMULTIPLY_ALPHA_WEBGL_Flag);
            } else {
                CppObj->removeFlag(NativeCanvas3DContext::kUNPACK_PREMULTIPLY_ALPHA_WEBGL_Flag);
            }
            break;
        }
        case NGL_UNPACK_COLORSPACE_CONVERSION_WEBGL:
            JS_ReportError(cx, "Not implemented");
            return false;
        break;
        default: 
            GL_CALL(CppObj, PixelStorei(param, value));
        break;
    }
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_renderbufferStorage) 
//{
    GLenum target, internalFormat;
	GLsizei width, height;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuii",
        &target, &internalFormat, &width, &height)) {
        return false;
    }

	GL_CALL(CppObj, RenderbufferStorage(target, internalFormat, width, height));

	return true;
}

NGL_JS_FN(WebGLRenderingContext_shaderSource)
//{
    NGLShader *cshader;
    GLsizei length;
    JSString *source;
    JSObject *shader;
    const char *csource;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oS", &shader, &source)) {
        return false;
    }

    cshader = (NGLShader *)JS_GetInstancePrivate(cx, shader,
        &WebGLShader_class, JS_ARGV(cx, vp));
    csource = JS_EncodeString(cx, source);
    length = JS_GetStringLength(source);

    cshader->source = csource;
    cshader->length = length;
    
    //GL_CALL(CppObj, ShaderSource(cshader->shader, 1, &csource, &length));

    //JS_free(cx, (void *)csource);
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_texImage2D)
//{
    GLenum target;
    GLint level;
    GLint internalFormat;
    GLenum format;
    GLenum type;
    JSObject *image;
    int width, height;

    if (argc == 9) {
        GLint border;
        JSObject *array;
        void *pixels = NULL;

        if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uiiiiiuuo", &target, &level, &internalFormat, &width, &height, &border, &format, &type, &array)) {
            return false;
        }

        if (array != NULL && JS_IsTypedArrayObject(array)) {
            pixels = JS_GetArrayBufferViewData(array);
        } 
        
        GL_CALL(CppObj, TexImage2D(target, level, internalFormat, width, height, border, format, type, pixels));
    } else {
        unsigned char *pixels;

        if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uiiuuo", &target, &level, &internalFormat, &format, &type, &image) || image == NULL) {
            JS_ReportError(cx, "texImage2D() invalid arguments");
            return false;
        }

        if (!OBJECT_TO_JSVAL(image).isObject()) {
            JS_ReportError(cx, "Invalid image object");
            return false;
        }

        if (NativeJSImage::JSObjectIs(cx, image)) {
	        NativeJSImage *nimg;

            nimg = static_cast<NativeJSImage *>(JS_GetPrivate(image));

            width = nimg->img->getWidth();
            height = nimg->img->getHeight();
            
            pixels = (unsigned char*)malloc(nimg->img->img->getSize());

           if (!NativeSkImage::ConvertToRGBA(nimg->img, pixels,
                    CppObj->hasFlag(NativeCanvas3DContext::kUNPACK_FLIP_Y_WEBGL_Flag), 
                    CppObj->hasFlag(NativeCanvas3DContext::kUNPACK_PREMULTIPLY_ALPHA_WEBGL_Flag))) {
            
                JS_ReportError(cx, "Failed to read image data");
                return false;
            }
        } else if (JS_InstanceOf(cx, image, &Canvas_class, NULL)) {
            NativeCanvasHandler *handler = static_cast<NativeCanvasHandler *>
                (static_cast<NativeJSCanvas*>(JS_GetPrivate(image))->getHandler());
            NativeCanvas2DContext *ctx = static_cast<NativeCanvas2DContext *>(handler->getContext());

            width = handler->getWidth();
            height = handler->getHeight();

            pixels = (unsigned char*)malloc(width * height * 4);

            ctx->getSurface()->readPixels(0, 0, width, height, pixels);
        } else {
            JS_ReportError(cx, "Unsupported or invalid image data");
            return false;
        }

        GL_CALL(CppObj, TexImage2D(target, level, internalFormat, width, height, 0, format, type, pixels));

        free(pixels);
    }

    return true;
}


NGL_JS_FN(WebGLRenderingContext_texParameteri)
//{
    GLuint target;
    GLuint pname;
    GLuint param;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuu", &target, &pname, &param)) {
        return false;
    }
    
    GL_CALL(CppObj, TexParameteri(target, pname, param));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_uniform1f) 
//{
    return NGL_uniformxf(CppObj, cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_uniform1fv)
//{
    return NGL_uniformxfv(CppObj, cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_uniform1i)
//{
    return NGL_uniformxi(CppObj, cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_uniform1iv)
//{
    return NGL_uniformxiv(CppObj, cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_uniform2f) 
//{
    return NGL_uniformxf(CppObj, cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_uniform2fv)
//{
    return NGL_uniformxfv(CppObj, cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_uniform2i)
//{
    return NGL_uniformxi(CppObj, cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_uniform2iv)
//{
    return NGL_uniformxiv(CppObj, cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_uniform3f) 
//{
    return NGL_uniformxf(CppObj, cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_uniform3fv)
//{
    return NGL_uniformxfv(CppObj, cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_uniform3i)
//{
    return NGL_uniformxi(CppObj, cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_uniform3iv)
//{
    return NGL_uniformxiv(CppObj, cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_uniform4f) 
//{
    return NGL_uniformxf(CppObj, cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_uniform4fv)
//{
    return NGL_uniformxfv(CppObj, cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_uniform4i)
//{
    return NGL_uniformxi(CppObj, cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_uniform4iv)
//{
    return NGL_uniformxiv(CppObj, cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_uniformMatrix2fv)
//{
    return NGL_uniformMatrixxfv(CppObj, cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_uniformMatrix3fv)
//{
    return NGL_uniformMatrixxfv(CppObj, cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_uniformMatrix4fv)
//{
    return NGL_uniformMatrixxfv(CppObj, cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib1f)
//{
    return NGL_vertexAttribxf(CppObj, cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib1fv)
//{
    return NGL_vertexAttribxfv(CppObj, cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib2f)
//{
    return NGL_vertexAttribxf(CppObj, cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib2fv)
//{
    return NGL_vertexAttribxfv(CppObj, cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib3f)
//{
    return NGL_vertexAttribxf(CppObj, cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib3fv)
//{
    return NGL_vertexAttribxfv(CppObj, cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib4f)
//{
    return NGL_vertexAttribxf(CppObj, cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib4fv)
//{
    return NGL_vertexAttribxfv(CppObj, cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttribPointer)
//{
    GLuint attr;
    GLint size;
    GLenum type;
    GLsizei stride;
    GLint offset;
    JSBool normalized;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uiubii", &attr,
        &size, &type, &normalized, &stride, &offset)) {
        return false;
    }

    if (offset+size < offset || offset+size < size) {
        JS_ReportError(cx, "Overflow in vertexAttribPointer");
        return false;
    }

    GL_CALL(CppObj, VertexAttribPointer(attr, size, type,
        (GLboolean)normalized, stride, (void *)(intptr_t)offset));

    return true;
}

NGL_JS_FN(WebGLRenderingContext_useProgram) 
//{
    JSObject *program;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &program)) {
        return false;
    }

    if (program == NULL) {
        JS_SetReservedSlot(thisobj, WebGLResource::kProgram, JSVAL_NULL);
        GL_CALL(CppObj, UseProgram(0));
        return true;
    }

    WebGLResource *res = (WebGLResource *)JS_GetInstancePrivate(cx, program,
        &WebGLProgram_class, JS_ARGV(cx, vp));

    JS_SetReservedSlot(thisobj, WebGLResource::kProgram,
        OBJECT_TO_JSVAL(res->jsobj()));

    GL_CALL(CppObj, UseProgram(res->id()));
    
    return true;
}

NGL_JS_FN(WebGLRenderingContext_viewport) 
//{
    GLint x, y, w, h;

    float ratio = NativeSystemInterface::getInstance()->backingStorePixelRatio();

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "iiii", &x, &y, &w, &h)) {
        return false;
    }

    GL_CALL(CppObj, Viewport(x*ratio, y*ratio, w*ratio, h*ratio));

    return true;
}

NGL_JS_FN(WebGLRenderingContext_getError)
//{
    JS_SET_RVAL(cx, vp, UINT_TO_JSVAL(glGetError()));
    return true;
}

NGL_JS_FN(WebGLRenderingContext_swapBuffer)
//{
#if 0
    NativeContext::getNativeClass(cx)->getUI()->swapGLBuffer();
    return true;
#endif
    return true;
}

static JSBool native_NativeGL_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    return true;
#if 0
    jsval proto;
    JSObject *webGLContext;
    NativeCanvasWebGLContext *ngl = new NativeCanvasWebGLContext(cx);
    
    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLRenderingContext", &proto);
	webGLContext = JS_NewObject(cx, &WebGLRenderingContext_class, JSVAL_TO_OBJECT(proto), NULL);

    if (webGLContext == NULL) {
        JS_ReportError(cx, "Failed to create WebGLRenderingContext");
        return false;
    } 

    JS_SetPrivate(webGLContext, static_cast<void *>(ngl));
    ngl->jsobj = webGLContext;

    // Compatibility OpenGL/WebGL
    // XXX : Is this belongs here ?
    GL_CALL(CppObj, Enable(GL_VERTEX_PROGRAM_POINT_SIZE));
    //GL_CALL(CppObj, Enable(GL_POINT_SPRITE));
    //GL_ARB_point
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(webGLContext));

    return true;
#endif
}

static JSBool NativeJSWebGLRenderingContext_constructor(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JS_ReportError(cx, "Illegal constructor");
    return false;
}

void NativeJSWebGLRenderingContext::registerObject(JSContext *cx) {
    JSObject *obj;
    JSObject *ctor;
    obj = JS_InitClass(cx, JS_GetGlobalObject(cx), NULL,
                &WebGLRenderingContext_class,
                NativeJSWebGLRenderingContext_constructor,
                0, NULL, WebGLRenderingContext_funcs, NULL, NULL);

    if (!obj || !(ctor = JS_GetConstructor(cx, obj))) {
        // TODO : Handle failure. Throw exception ? 
        return;
    }

    JS_DefineConstDoubles(cx, ctor, WebGLRenderingContext_const);
}

NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLBuffer);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLFrameBuffer);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLProgram);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLRenderbuffer);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLShader);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLTexture);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLUniformLocation);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLShaderPrecisionFormat);
