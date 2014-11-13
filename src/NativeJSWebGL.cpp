#include "NativeJSWebGL.h"
#include "NativeJSImage.h"
#include "NativeSkImage.h"
#include "NativeSkia.h"
#include "NativeCanvas2DContext.h"
#include "GLSLANG/ShaderLang.h"
#include <SkCanvas.h>
#include <NativeContext.h>
#include <NativeUIInterface.h>
#include <NativeMacros.h>

#define NATIVE_GL_GETTER(obj) ((class NativeCanvasWebGLContext*)JS_GetPrivate(obj))

//#define GL_CALL(fn) fn
#define GL_CALL(fn) fn; { GLint err = glGetError(); if (err != 0) NLOG("err = %d / call = %s\n", err, #fn);}

#define D_NGL_JS_FN(func_name) static JSBool func_name(JSContext *cx, unsigned int argc, jsval *vp);

#define NGL_JS_FN(func_name) static JSBool func_name(JSContext *cx, unsigned int argc, jsval *vp)

#define NATIVE_GL_OBJECT_EXPOSE_NOT_INST(name) \
    void NativeJS ## name::registerObject(JSContext *cx) \
    { \
        JS_DefineObject(cx, JS_GetGlobalObject(cx), #name, \
            &name ## _class , NULL, 0); \
    }

#define MAKE_GL_CURRENT(cx, vp) \
    //NativeCanvasWebGLContext *ngl = NATIVE_GL_GETTER(JS_THIS_OBJECT(cx, vp)); \
    //ngl->MakeGLCurrent();

static JSClass NativeGL_class = {
    "NativeGL", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

JSClass WebGLRenderingContext_class = {
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
static JSClass WebGLShaderPrecisionFormat_class = {
    "WebGLShaderPrecisionFormat", JSCLASS_HAS_PRIVATE,
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
            GL_CALL(glUniform1f(clocation, (GLfloat)x));
            break;
        case 2:
            GL_CALL(glUniform2f(clocation, (GLfloat)x, (GLfloat)y));
            break;
        case 3:
            GL_CALL(glUniform3f(clocation, (GLfloat)x, (GLfloat)y, (GLfloat)z));
            break;
        case 4:
            GL_CALL(glUniform4f(clocation, (GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)w));
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
            GL_CALL(glUniform1fv(clocation, length, carray));
            break;
        case 2:
            GL_CALL(glUniform2fv(clocation, length/2, carray));
            break;
        case 3:
            GL_CALL(glUniform3fv(clocation, length/3, carray));
            break;
        case 4:
            GL_CALL(glUniform4fv(clocation, length/4, carray));
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
            GL_CALL(glUniform1i(clocation, x));
            break;
        case 2:
            GL_CALL(glUniform2i(clocation, x, y));
            break;
        case 3:
            GL_CALL(glUniform3i(clocation, x, y, z));
            break;
        case 4:
            GL_CALL(glUniform4i(clocation, x, y, z, w));
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
        GL_CALL(glUniform1iv(clocation, length, carray));
    } else if (nb == 2) {
        GL_CALL(glUniform2iv(clocation, length/2, carray));
    } else if (nb == 3) {
        GL_CALL(glUniform3iv(clocation, length/3, carray));
    } else if (nb == 4) {
        GL_CALL(glUniform4iv(clocation, length/4, carray));
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
            GL_CALL(glUniformMatrix2fv(clocation, length/4, (GLboolean)transpose, carray));
            break;
        case 3:
            GL_CALL(glUniformMatrix3fv(clocation, length/8, (GLboolean)transpose, carray));
            break;
        case 4:
            GL_CALL(glUniformMatrix4fv(clocation, length/16, (GLboolean)transpose, carray));
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
            GL_CALL(glVertexAttrib1f(index, (GLfloat)v0));
            break;
        case 2:
            GL_CALL(glVertexAttrib2f(index, (GLfloat)v0, (GLfloat)v1));
            break;
        case 3:
            GL_CALL(glVertexAttrib3f(index, (GLfloat)v0, (GLfloat)v1, (GLfloat)v2));
            break;
        case 4:
            GL_CALL(glVertexAttrib4f(index, (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, (GLfloat)v3));
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
        return false;
    }

    switch (nb) {
        case 1:
            GL_CALL(glVertexAttrib1fv(index, carray));
            break;
        case 2:
            GL_CALL(glVertexAttrib2fv(index, carray));
            break;
        case 3:
            GL_CALL(glVertexAttrib3fv(index, carray));
            break;
        case 4:
            GL_CALL(glVertexAttrib4fv(index, carray));
            break;
    }

    return JS_TRUE;
}

#if 0
NativeCanvasWebGLContext::NativeCanvasWebGLContext(JSContext *cx, NGLContextAttributes *attributes, int width, int height) 
    : unpackFlipY(false), unpackPremultiplyAlpha(false)
{
    //int texType = attributes->m_Antialias ? GL_EXT_framebuffer_multisample : GL_TEXTURE_2D;
    // TODO : Test for GL version if > 3.2 use GL_TEXTURE_2D_MULTISAMPLE else GL_EXT_framebuffer_multisample
    int texType = GL_TEXTURE_2D; // OpenGL 2.1 doesn't support multisampling
    this->jscx = cx;

    NativeUIInterface *ui = NativeContext::getNativeClass(cx)->getUI();

    m_GLContext = new NativeGLContext(ui);
    NLOG("Current : %d", this->MakeGLCurrent());

    GL_CALL(glViewport(0, 0, width, height));
    GL_CALL(glEnable(GL_TEXTURE_2D));
    GL_CALL(glEnable(GL_MULTISAMPLE));

    //GL_CALL(glShadeModel(GL_SMOOTH));
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glDepthFunc(GL_LEQUAL));
    //GL_CALL(glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST));
    GL_CALL(glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST));


    //GL_CALL(glEnable(GL_FRAGMENT_PRECISION_HIGH));

    GL_CALL(glGenTextures(1, &m_tex));
    GL_CALL(glBindTexture(texType, m_tex));

    GL_CALL(glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    /* Allocate memory for the new texture */
    GL_CALL(glTexImage2D(
            texType,
            0,
            GL_RGBA,
            width, height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            NULL
    ));

    GL_CALL(glBindTexture(texType, 0));

    /* Generate the FBO */
    GL_CALL(glGenFramebuffers(1, &m_fbo));
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));

    /* Set the FBO backing store using the new texture */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        texType, m_tex, 0);

    GLenum status;
    status = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));

    switch(status) {
        case GL_FRAMEBUFFER_COMPLETE:
            printf("OH YEAH\n");
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            printf("fbo unsupported\n");
            exit(2);
            return;
        default:
            printf("fbo fatal error wat %d\n", status);
            exit(2);
            return;
    }

    m_width = width;
    m_height = height;

    // OpenGL context is ready, setup JS
    jsval proto;
    JSObject *webGLContext;
    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLRenderingContext", &proto);
	webGLContext = JS_NewObject(cx, &WebGLRenderingContext_class, JSVAL_TO_OBJECT(proto), NULL);

    if (webGLContext == NULL) {
        JS_ReportError(cx, "Failed to create WebGLRenderingContext");
        // TODO
        exit(2);
    } 

    JS_SetPrivate(webGLContext, static_cast<void *>(this));
    this->jsobj = webGLContext;

    // Compatibility OpenGL/WebGL
    // XXX : Is this belongs here ?
    GL_CALL(glEnable(GL_VERTEX_PROGRAM_POINT_SIZE));
    GL_CALL(glEnable(GL_POINT_SPRITE));
    GL_CALL(glEnableVertexAttribArray(0));

    //GL_ARB_point
    //JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(webGLContext));

}

void NativeCanvasWebGLContext::composeWith(NativeCanvas2DContext *layer,
    double left, double top, double opacity,
    double zoom, const NativeRect *rclip)
{
    this->MakeGLCurrent();
    GL_CALL(glFinish());
    GLenum err;
    layer->MakeGLCurrent();

    NativeSkia *skia = layer->getSurface();
    skia->canvas->flush();

    GL_CALL(glUseProgram(0));

    layer->drawTexIDToFBO(m_tex, m_width, m_height, left, top, layer->getMainFBO());
    layer->resetGLContext();
}

NativeCanvasWebGLContext::~NativeCanvasWebGLContext() 
{
    // TODO
    /*
    //Delete resources
    GL_CALL(glDeleteRenderbuffersEXT(1, &color_rb));
    GL_CALL(glDeleteRenderbuffersEXT(1, &depth_rb));
    //Bind 0, which means render to back buffer, as a result, fb is unbound
    GL_CALL(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
    GL_CALL(glDeleteFramebuffersEXT(1, &fb));
    */

}
#endif

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
    JS_FS("isContextLost", WebGLRenderingContext_isContextLost, 0, 0),
    JS_FS("getExtension", WebGLRenderingContext_getExtension, 1, 0),
    JS_FS("activeTexture", WebGLRenderingContext_activeTexture, 1, 0),
    JS_FS("attachShader", WebGLRenderingContext_attachShader, 2, 0),
    JS_FS("bindAttribLocation", WebGLRenderingContext_bindAttribLocation, 3, 0),
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
    JS_FS("getShaderPrecisionFormat", WebGLRenderingContext_getShaderPrecisionFormat, 2, 0),
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
    JS_FS("swapBuffer", WebGLRenderingContext_swapBuffer, 0, 0),
    JS_FS_END
};

JSConstDoubleSpec WebGLRenderingContext_const [] = {
    //{NGL_ES_VERSION_2_0, "ES_VERSION_2_0", 0, {0,0,0}},
    {NGL_DEPTH_BUFFER_BIT, "DEPTH_BUFFER_BIT", 0, {0,0,0}},
    {NGL_STENCIL_BUFFER_BIT, "STENCIL_BUFFER_BIT", 0, {0,0,0}},
    {NGL_COLOR_BUFFER_BIT, "COLOR_BUFFER_BIT", 0, {0,0,0}},
    {NGL_POINTS, "POINTS", 0, {0,0,0}},
    {NGL_LINES, "LINES", 0, {0,0,0}},
    {NGL_LINE_LOOP, "LINE_LOOP", 0, {0,0,0}},
    {NGL_LINE_STRIP, "LINE_STRIP", 0, {0,0,0}},
    {NGL_TRIANGLES, "TRIANGLES", 0, {0,0,0}},
    {NGL_TRIANGLE_STRIP, "TRIANGLE_STRIP", 0, {0,0,0}},
    {NGL_TRIANGLE_FAN, "TRIANGLE_FAN", 0, {0,0,0}},
    {NGL_ZERO, "ZERO", 0, {0,0,0}},
    {NGL_ONE, "ONE", 0, {0,0,0}},
    {NGL_SRC_COLOR, "SRC_COLOR", 0, {0,0,0}},
    {NGL_ONE_MINUS_SRC_COLOR, "ONE_MINUS_SRC_COLOR", 0, {0,0,0}},
    {NGL_SRC_ALPHA, "SRC_ALPHA", 0, {0,0,0}},
    {NGL_ONE_MINUS_SRC_ALPHA, "ONE_MINUS_SRC_ALPHA", 0, {0,0,0}},
    {NGL_DST_ALPHA, "DST_ALPHA", 0, {0,0,0}},
    {NGL_ONE_MINUS_DST_ALPHA, "ONE_MINUS_DST_ALPHA", 0, {0,0,0}},
    {NGL_DST_COLOR, "DST_COLOR", 0, {0,0,0}},
    {NGL_ONE_MINUS_DST_COLOR, "ONE_MINUS_DST_COLOR", 0, {0,0,0}},
    {NGL_SRC_ALPHA_SATURATE, "SRC_ALPHA_SATURATE", 0, {0,0,0}},
    {NGL_FUNC_ADD, "FUNC_ADD", 0, {0,0,0}},
    {NGL_BLEND_EQUATION, "BLEND_EQUATION", 0, {0,0,0}},
    {NGL_BLEND_EQUATION_RGB, "BLEND_EQUATION_RGB", 0, {0,0,0}},
    {NGL_BLEND_EQUATION_ALPHA, "BLEND_EQUATION_ALPHA", 0, {0,0,0}},
    {NGL_FUNC_SUBTRACT, "FUNC_SUBTRACT", 0, {0,0,0}},
    {NGL_FUNC_REVERSE_SUBTRACT, "FUNC_REVERSE_SUBTRACT", 0, {0,0,0}},
    {NGL_BLEND_DST_RGB, "BLEND_DST_RGB", 0, {0,0,0}},
    {NGL_BLEND_SRC_RGB, "BLEND_SRC_RGB", 0, {0,0,0}},
    {NGL_BLEND_DST_ALPHA, "BLEND_DST_ALPHA", 0, {0,0,0}},
    {NGL_BLEND_SRC_ALPHA, "BLEND_SRC_ALPHA", 0, {0,0,0}},
    {NGL_CONSTANT_COLOR, "CONSTANT_COLOR", 0, {0,0,0}},
    {NGL_ONE_MINUS_CONSTANT_COLOR, "ONE_MINUS_CONSTANT_COLOR", 0, {0,0,0}},
    {NGL_CONSTANT_ALPHA, "CONSTANT_ALPHA", 0, {0,0,0}},
    {NGL_ONE_MINUS_CONSTANT_ALPHA, "ONE_MINUS_CONSTANT_ALPHA", 0, {0,0,0}},
    {NGL_BLEND_COLOR, "BLEND_COLOR", 0, {0,0,0}},
    {NGL_ARRAY_BUFFER, "ARRAY_BUFFER", 0, {0,0,0}},
    {NGL_ELEMENT_ARRAY_BUFFER, "ELEMENT_ARRAY_BUFFER", 0, {0,0,0}},
    {NGL_ARRAY_BUFFER_BINDING, "ARRAY_BUFFER_BINDING", 0, {0,0,0}},
    {NGL_ELEMENT_ARRAY_BUFFER_BINDING, "ELEMENT_ARRAY_BUFFER_BINDING", 0, {0,0,0}},
    {NGL_STREAM_DRAW, "STREAM_DRAW", 0, {0,0,0}},
    {NGL_STATIC_DRAW, "STATIC_DRAW", 0, {0,0,0}},
    {NGL_DYNAMIC_DRAW, "DYNAMIC_DRAW", 0, {0,0,0}},
    {NGL_BUFFER_SIZE, "BUFFER_SIZE", 0, {0,0,0}},
    {NGL_BUFFER_USAGE, "BUFFER_USAGE", 0, {0,0,0}},
    {NGL_CURRENT_VERTEX_ATTRIB, "CURRENT_VERTEX_ATTRIB", 0, {0,0,0}},
    {NGL_FRONT, "FRONT", 0, {0,0,0}},
    {NGL_BACK, "BACK", 0, {0,0,0}},
    {NGL_FRONT_AND_BACK, "FRONT_AND_BACK", 0, {0,0,0}},
    {NGL_TEXTURE_2D, "TEXTURE_2D", 0, {0,0,0}},
    {NGL_CULL_FACE, "CULL_FACE", 0, {0,0,0}},
    {NGL_BLEND, "BLEND", 0, {0,0,0}},
    {NGL_DITHER, "DITHER", 0, {0,0,0}},
    {NGL_STENCIL_TEST, "STENCIL_TEST", 0, {0,0,0}},
    {NGL_DEPTH_TEST, "DEPTH_TEST", 0, {0,0,0}},
    {NGL_SCISSOR_TEST, "SCISSOR_TEST", 0, {0,0,0}},
    {NGL_POLYGON_OFFSET_FILL, "POLYGON_OFFSET_FILL", 0, {0,0,0}},
    {NGL_SAMPLE_ALPHA_TO_COVERAGE, "SAMPLE_ALPHA_TO_COVERAGE", 0, {0,0,0}},
    {NGL_SAMPLE_COVERAGE, "SAMPLE_COVERAGE", 0, {0,0,0}},
    {NGL_NO_ERROR, "NO_ERROR", 0, {0,0,0}},
    {NGL_INVALID_ENUM, "INVALID_ENUM", 0, {0,0,0}},
    {NGL_INVALID_VALUE, "INVALID_VALUE", 0, {0,0,0}},
    {NGL_INVALID_OPERATION, "INVALID_OPERATION", 0, {0,0,0}},
    {NGL_OUT_OF_MEMORY, "OUT_OF_MEMORY", 0, {0,0,0}},
    {NGL_CW, "CW", 0, {0,0,0}},
    {NGL_CCW, "CCW", 0, {0,0,0}},
    {NGL_LINE_WIDTH, "LINE_WIDTH", 0, {0,0,0}},
    {NGL_ALIASED_POINT_SIZE_RANGE, "ALIASED_POINT_SIZE_RANGE", 0, {0,0,0}},
    {NGL_ALIASED_LINE_WIDTH_RANGE, "ALIASED_LINE_WIDTH_RANGE", 0, {0,0,0}},
    {NGL_CULL_FACE_MODE, "CULL_FACE_MODE", 0, {0,0,0}},
    {NGL_FRONT_FACE, "FRONT_FACE", 0, {0,0,0}},
    {NGL_DEPTH_RANGE, "DEPTH_RANGE", 0, {0,0,0}},
    {NGL_DEPTH_WRITEMASK, "DEPTH_WRITEMASK", 0, {0,0,0}},
    {NGL_DEPTH_CLEAR_VALUE, "DEPTH_CLEAR_VALUE", 0, {0,0,0}},
    {NGL_DEPTH_FUNC, "DEPTH_FUNC", 0, {0,0,0}},
    {NGL_STENCIL_CLEAR_VALUE, "STENCIL_CLEAR_VALUE", 0, {0,0,0}},
    {NGL_STENCIL_FUNC, "STENCIL_FUNC", 0, {0,0,0}},
    {NGL_STENCIL_FAIL, "STENCIL_FAIL", 0, {0,0,0}},
    {NGL_STENCIL_PASS_DEPTH_FAIL, "STENCIL_PASS_DEPTH_FAIL", 0, {0,0,0}},
    {NGL_STENCIL_PASS_DEPTH_PASS, "STENCIL_PASS_DEPTH_PASS", 0, {0,0,0}},
    {NGL_STENCIL_REF, "STENCIL_REF", 0, {0,0,0}},
    {NGL_STENCIL_VALUE_MASK, "STENCIL_VALUE_MASK", 0, {0,0,0}},
    {NGL_STENCIL_WRITEMASK, "STENCIL_WRITEMASK", 0, {0,0,0}},
    {NGL_STENCIL_BACK_FUNC, "STENCIL_BACK_FUNC", 0, {0,0,0}},
    {NGL_STENCIL_BACK_FAIL, "STENCIL_BACK_FAIL", 0, {0,0,0}},
    {NGL_STENCIL_BACK_PASS_DEPTH_FAIL, "STENCIL_BACK_PASS_DEPTH_FAIL", 0, {0,0,0}},
    {NGL_STENCIL_BACK_PASS_DEPTH_PASS, "STENCIL_BACK_PASS_DEPTH_PASS", 0, {0,0,0}},
    {NGL_STENCIL_BACK_REF, "STENCIL_BACK_REF", 0, {0,0,0}},
    {NGL_STENCIL_BACK_VALUE_MASK, "STENCIL_BACK_VALUE_MASK", 0, {0,0,0}},
    {NGL_STENCIL_BACK_WRITEMASK, "STENCIL_BACK_WRITEMASK", 0, {0,0,0}},
    {NGL_VIEWPORT, "VIEWPORT", 0, {0,0,0}},
    {NGL_SCISSOR_BOX, "SCISSOR_BOX", 0, {0,0,0}},
    {NGL_COLOR_CLEAR_VALUE, "COLOR_CLEAR_VALUE", 0, {0,0,0}},
    {NGL_COLOR_WRITEMASK, "COLOR_WRITEMASK", 0, {0,0,0}},
    {NGL_UNPACK_ALIGNMENT, "UNPACK_ALIGNMENT", 0, {0,0,0}},
    {NGL_PACK_ALIGNMENT, "PACK_ALIGNMENT", 0, {0,0,0}},
    {NGL_MAX_TEXTURE_SIZE, "MAX_TEXTURE_SIZE", 0, {0,0,0}},
    {NGL_MAX_VIEWPORT_DIMS, "MAX_VIEWPORT_DIMS", 0, {0,0,0}},
    {NGL_SUBPIXEL_BITS, "SUBPIXEL_BITS", 0, {0,0,0}},
    {NGL_RED_BITS, "RED_BITS", 0, {0,0,0}},
    {NGL_GREEN_BITS, "GREEN_BITS", 0, {0,0,0}},
    {NGL_BLUE_BITS, "BLUE_BITS", 0, {0,0,0}},
    {NGL_ALPHA_BITS, "ALPHA_BITS", 0, {0,0,0}},
    {NGL_DEPTH_BITS, "DEPTH_BITS", 0, {0,0,0}},
    {NGL_STENCIL_BITS, "STENCIL_BITS", 0, {0,0,0}},
    {NGL_POLYGON_OFFSET_UNITS, "POLYGON_OFFSET_UNITS", 0, {0,0,0}},
    {NGL_POLYGON_OFFSET_FACTOR, "POLYGON_OFFSET_FACTOR", 0, {0,0,0}},
    {NGL_TEXTURE_BINDING_2D, "TEXTURE_BINDING_2D", 0, {0,0,0}},
    {NGL_SAMPLE_BUFFERS, "SAMPLE_BUFFERS", 0, {0,0,0}},
    {NGL_SAMPLES, "SAMPLES", 0, {0,0,0}},
    {NGL_SAMPLE_COVERAGE_VALUE, "SAMPLE_COVERAGE_VALUE", 0, {0,0,0}},
    {NGL_SAMPLE_COVERAGE_INVERT, "SAMPLE_COVERAGE_INVERT", 0, {0,0,0}},
    //{NGL_NUM_COMPRESSED_TEXTURE_FORMATS, "NUM_COMPRESSED_TEXTURE_FORMATS", 0, {0,0,0}},
    {NGL_COMPRESSED_TEXTURE_FORMATS, "COMPRESSED_TEXTURE_FORMATS", 0, {0,0,0}},
    {NGL_DONT_CARE, "DONT_CARE", 0, {0,0,0}},
    {NGL_FASTEST, "FASTEST", 0, {0,0,0}},
    {NGL_NICEST, "NICEST", 0, {0,0,0}},
    {NGL_GENERATE_MIPMAP_HINT, "GENERATE_MIPMAP_HINT", 0, {0,0,0}},
    {NGL_BYTE, "BYTE", 0, {0,0,0}},
    {NGL_UNSIGNED_BYTE, "UNSIGNED_BYTE", 0, {0,0,0}},
    {NGL_SHORT, "SHORT", 0, {0,0,0}},
    {NGL_UNSIGNED_SHORT, "UNSIGNED_SHORT", 0, {0,0,0}},
    {NGL_INT, "INT", 0, {0,0,0}},
    {NGL_UNSIGNED_INT, "UNSIGNED_INT", 0, {0,0,0}},
    {NGL_FLOAT, "FLOAT", 0, {0,0,0}},
    //{NGL_FIXED, "FIXED", 0, {0,0,0}},
    {NGL_DEPTH_COMPONENT, "DEPTH_COMPONENT", 0, {0,0,0}},
    {NGL_ALPHA, "ALPHA", 0, {0,0,0}},
    {NGL_RGB, "RGB", 0, {0,0,0}},
    {NGL_RGBA, "RGBA", 0, {0,0,0}},
    {NGL_LUMINANCE, "LUMINANCE", 0, {0,0,0}},
    {NGL_LUMINANCE_ALPHA, "LUMINANCE_ALPHA", 0, {0,0,0}},
    {NGL_UNSIGNED_SHORT_4_4_4_4, "UNSIGNED_SHORT_4_4_4_4", 0, {0,0,0}},
    {NGL_UNSIGNED_SHORT_5_5_5_1, "UNSIGNED_SHORT_5_5_5_1", 0, {0,0,0}},
    {NGL_UNSIGNED_SHORT_5_6_5, "UNSIGNED_SHORT_5_6_5", 0, {0,0,0}},
    {NGL_FRAGMENT_SHADER, "FRAGMENT_SHADER", 0, {0,0,0}},
    {NGL_VERTEX_SHADER, "VERTEX_SHADER", 0, {0,0,0}},
    {NGL_MAX_VERTEX_ATTRIBS, "MAX_VERTEX_ATTRIBS", 0, {0,0,0}},
    {NGL_MAX_VERTEX_UNIFORM_VECTORS, "MAX_VERTEX_UNIFORM_VECTORS", 0, {0,0,0}},
    {NGL_MAX_VARYING_VECTORS, "MAX_VARYING_VECTORS", 0, {0,0,0}},
    {NGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, "MAX_COMBINED_TEXTURE_IMAGE_UNITS", 0, {0,0,0}},
    {NGL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, "MAX_VERTEX_TEXTURE_IMAGE_UNITS", 0, {0,0,0}},
    {NGL_MAX_TEXTURE_IMAGE_UNITS, "MAX_TEXTURE_IMAGE_UNITS", 0, {0,0,0}},
    {NGL_MAX_FRAGMENT_UNIFORM_VECTORS, "MAX_FRAGMENT_UNIFORM_VECTORS", 0, {0,0,0}},
    {NGL_SHADER_TYPE, "SHADER_TYPE", 0, {0,0,0}},
    {NGL_DELETE_STATUS, "DELETE_STATUS", 0, {0,0,0}},
    {NGL_LINK_STATUS, "LINK_STATUS", 0, {0,0,0}},
    {NGL_VALIDATE_STATUS, "VALIDATE_STATUS", 0, {0,0,0}},
    {NGL_ATTACHED_SHADERS, "ATTACHED_SHADERS", 0, {0,0,0}},
    {NGL_ACTIVE_UNIFORMS, "ACTIVE_UNIFORMS", 0, {0,0,0}},
    //{NGL_ACTIVE_UNIFORM_MAX_LENGTH, "ACTIVE_UNIFORM_MAX_LENGTH", 0, {0,0,0}},
    {NGL_ACTIVE_ATTRIBUTES, "ACTIVE_ATTRIBUTES", 0, {0,0,0}},
    //{NGL_ACTIVE_ATTRIBUTE_MAX_LENGTH, "ACTIVE_ATTRIBUTE_MAX_LENGTH", 0, {0,0,0}},
    {NGL_SHADING_LANGUAGE_VERSION, "SHADING_LANGUAGE_VERSION", 0, {0,0,0}},
    {NGL_CURRENT_PROGRAM, "CURRENT_PROGRAM", 0, {0,0,0}},
    {NGL_NEVER, "NEVER", 0, {0,0,0}},
    {NGL_LESS, "LESS", 0, {0,0,0}},
    {NGL_EQUAL, "EQUAL", 0, {0,0,0}},
    {NGL_LEQUAL, "LEQUAL", 0, {0,0,0}},
    {NGL_GREATER, "GREATER", 0, {0,0,0}},
    {NGL_NOTEQUAL, "NOTEQUAL", 0, {0,0,0}},
    {NGL_GEQUAL, "GEQUAL", 0, {0,0,0}},
    {NGL_ALWAYS, "ALWAYS", 0, {0,0,0}},
    {NGL_KEEP, "KEEP", 0, {0,0,0}},
    {NGL_REPLACE, "REPLACE", 0, {0,0,0}},
    {NGL_INCR, "INCR", 0, {0,0,0}},
    {NGL_DECR, "DECR", 0, {0,0,0}},
    {NGL_INVERT, "INVERT", 0, {0,0,0}},
    {NGL_INCR_WRAP, "INCR_WRAP", 0, {0,0,0}},
    {NGL_DECR_WRAP, "DECR_WRAP", 0, {0,0,0}},
    {NGL_VENDOR, "VENDOR", 0, {0,0,0}},
    {NGL_RENDERER, "RENDERER", 0, {0,0,0}},
    {NGL_VERSION, "VERSION", 0, {0,0,0}},
    //{NGL_EXTENSIONS, "EXTENSIONS", 0, {0,0,0}},
    {NGL_NEAREST, "NEAREST", 0, {0,0,0}},
    {NGL_LINEAR, "LINEAR", 0, {0,0,0}},
    {NGL_NEAREST_MIPMAP_NEAREST, "NEAREST_MIPMAP_NEAREST", 0, {0,0,0}},
    {NGL_LINEAR_MIPMAP_NEAREST, "LINEAR_MIPMAP_NEAREST", 0, {0,0,0}},
    {NGL_NEAREST_MIPMAP_LINEAR, "NEAREST_MIPMAP_LINEAR", 0, {0,0,0}},
    {NGL_LINEAR_MIPMAP_LINEAR, "LINEAR_MIPMAP_LINEAR", 0, {0,0,0}},
    {NGL_TEXTURE_MAG_FILTER, "TEXTURE_MAG_FILTER", 0, {0,0,0}},
    {NGL_TEXTURE_MIN_FILTER, "TEXTURE_MIN_FILTER", 0, {0,0,0}},
    {NGL_TEXTURE_WRAP_S, "TEXTURE_WRAP_S", 0, {0,0,0}},
    {NGL_TEXTURE_WRAP_T, "TEXTURE_WRAP_T", 0, {0,0,0}},
    {NGL_TEXTURE, "TEXTURE", 0, {0,0,0}},
    {NGL_TEXTURE_CUBE_MAP, "TEXTURE_CUBE_MAP", 0, {0,0,0}},
    {NGL_TEXTURE_BINDING_CUBE_MAP, "TEXTURE_BINDING_CUBE_MAP", 0, {0,0,0}},
    {NGL_TEXTURE_CUBE_MAP_POSITIVE_X, "TEXTURE_CUBE_MAP_POSITIVE_X", 0, {0,0,0}},
    {NGL_TEXTURE_CUBE_MAP_NEGATIVE_X, "TEXTURE_CUBE_MAP_NEGATIVE_X", 0, {0,0,0}},
    {NGL_TEXTURE_CUBE_MAP_POSITIVE_Y, "TEXTURE_CUBE_MAP_POSITIVE_Y", 0, {0,0,0}},
    {NGL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "TEXTURE_CUBE_MAP_NEGATIVE_Y", 0, {0,0,0}},
    {NGL_TEXTURE_CUBE_MAP_POSITIVE_Z, "TEXTURE_CUBE_MAP_POSITIVE_Z", 0, {0,0,0}},
    {NGL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "TEXTURE_CUBE_MAP_NEGATIVE_Z", 0, {0,0,0}},
    {NGL_MAX_CUBE_MAP_TEXTURE_SIZE, "MAX_CUBE_MAP_TEXTURE_SIZE", 0, {0,0,0}},
    {NGL_TEXTURE0, "TEXTURE0", 0, {0,0,0}},
    {NGL_TEXTURE1, "TEXTURE1", 0, {0,0,0}},
    {NGL_TEXTURE2, "TEXTURE2", 0, {0,0,0}},
    {NGL_TEXTURE3, "TEXTURE3", 0, {0,0,0}},
    {NGL_TEXTURE4, "TEXTURE4", 0, {0,0,0}},
    {NGL_TEXTURE5, "TEXTURE5", 0, {0,0,0}},
    {NGL_TEXTURE6, "TEXTURE6", 0, {0,0,0}},
    {NGL_TEXTURE7, "TEXTURE7", 0, {0,0,0}},
    {NGL_TEXTURE8, "TEXTURE8", 0, {0,0,0}},
    {NGL_TEXTURE9, "TEXTURE9", 0, {0,0,0}},
    {NGL_TEXTURE10, "TEXTURE10", 0, {0,0,0}},
    {NGL_TEXTURE11, "TEXTURE11", 0, {0,0,0}},
    {NGL_TEXTURE12, "TEXTURE12", 0, {0,0,0}},
    {NGL_TEXTURE13, "TEXTURE13", 0, {0,0,0}},
    {NGL_TEXTURE14, "TEXTURE14", 0, {0,0,0}},
    {NGL_TEXTURE15, "TEXTURE15", 0, {0,0,0}},
    {NGL_TEXTURE16, "TEXTURE16", 0, {0,0,0}},
    {NGL_TEXTURE17, "TEXTURE17", 0, {0,0,0}},
    {NGL_TEXTURE18, "TEXTURE18", 0, {0,0,0}},
    {NGL_TEXTURE19, "TEXTURE19", 0, {0,0,0}},
    {NGL_TEXTURE20, "TEXTURE20", 0, {0,0,0}},
    {NGL_TEXTURE21, "TEXTURE21", 0, {0,0,0}},
    {NGL_TEXTURE22, "TEXTURE22", 0, {0,0,0}},
    {NGL_TEXTURE23, "TEXTURE23", 0, {0,0,0}},
    {NGL_TEXTURE24, "TEXTURE24", 0, {0,0,0}},
    {NGL_TEXTURE25, "TEXTURE25", 0, {0,0,0}},
    {NGL_TEXTURE26, "TEXTURE26", 0, {0,0,0}},
    {NGL_TEXTURE27, "TEXTURE27", 0, {0,0,0}},
    {NGL_TEXTURE28, "TEXTURE28", 0, {0,0,0}},
    {NGL_TEXTURE29, "TEXTURE29", 0, {0,0,0}},
    {NGL_TEXTURE30, "TEXTURE30", 0, {0,0,0}},
    {NGL_TEXTURE31, "TEXTURE31", 0, {0,0,0}},
    {NGL_ACTIVE_TEXTURE, "ACTIVE_TEXTURE", 0, {0,0,0}},
    {NGL_REPEAT, "REPEAT", 0, {0,0,0}},
    {NGL_CLAMP_TO_EDGE, "CLAMP_TO_EDGE", 0, {0,0,0}},
    {NGL_MIRRORED_REPEAT, "MIRRORED_REPEAT", 0, {0,0,0}},
    {NGL_FLOAT_VEC2, "FLOAT_VEC2", 0, {0,0,0}},
    {NGL_FLOAT_VEC3, "FLOAT_VEC3", 0, {0,0,0}},
    {NGL_FLOAT_VEC4, "FLOAT_VEC4", 0, {0,0,0}},
    {NGL_INT_VEC2, "INT_VEC2", 0, {0,0,0}},
    {NGL_INT_VEC3, "INT_VEC3", 0, {0,0,0}},
    {NGL_INT_VEC4, "INT_VEC4", 0, {0,0,0}},
    {NGL_BOOL, "BOOL", 0, {0,0,0}},
    {NGL_BOOL_VEC2, "BOOL_VEC2", 0, {0,0,0}},
    {NGL_BOOL_VEC3, "BOOL_VEC3", 0, {0,0,0}},
    {NGL_BOOL_VEC4, "BOOL_VEC4", 0, {0,0,0}},
    {NGL_FLOAT_MAT2, "FLOAT_MAT2", 0, {0,0,0}},
    {NGL_FLOAT_MAT3, "FLOAT_MAT3", 0, {0,0,0}},
    {NGL_FLOAT_MAT4, "FLOAT_MAT4", 0, {0,0,0}},
    {NGL_SAMPLER_2D, "SAMPLER_2D", 0, {0,0,0}},
    {NGL_SAMPLER_CUBE, "SAMPLER_CUBE", 0, {0,0,0}},
    {NGL_VERTEX_ATTRIB_ARRAY_ENABLED, "VERTEX_ATTRIB_ARRAY_ENABLED", 0, {0,0,0}},
    {NGL_VERTEX_ATTRIB_ARRAY_SIZE, "VERTEX_ATTRIB_ARRAY_SIZE", 0, {0,0,0}},
    {NGL_VERTEX_ATTRIB_ARRAY_STRIDE, "VERTEX_ATTRIB_ARRAY_STRIDE", 0, {0,0,0}},
    {NGL_VERTEX_ATTRIB_ARRAY_TYPE, "VERTEX_ATTRIB_ARRAY_TYPE", 0, {0,0,0}},
    {NGL_VERTEX_ATTRIB_ARRAY_NORMALIZED, "VERTEX_ATTRIB_ARRAY_NORMALIZED", 0, {0,0,0}},
    {NGL_VERTEX_ATTRIB_ARRAY_POINTER, "VERTEX_ATTRIB_ARRAY_POINTER", 0, {0,0,0}},
    {NGL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, "VERTEX_ATTRIB_ARRAY_BUFFER_BINDING", 0, {0,0,0}},
    //{NGL_IMPLEMENTATION_COLOR_READ_TYPE, "IMPLEMENTATION_COLOR_READ_TYPE", 0, {0,0,0}},
    //{NGL_IMPLEMENTATION_COLOR_READ_FORMAT, "IMPLEMENTATION_COLOR_READ_FORMAT", 0, {0,0,0}},
    {NGL_COMPILE_STATUS, "COMPILE_STATUS", 0, {0,0,0}},
    //{NGL_INFO_LOG_LENGTH, "INFO_LOG_LENGTH", 0, {0,0,0}},
    //{NGL_SHADER_SOURCE_LENGTH, "SHADER_SOURCE_LENGTH", 0, {0,0,0}},
    //{NGL_SHADER_COMPILER, "SHADER_COMPILER", 0, {0,0,0}},
    {NGL_LOW_FLOAT, "LOW_FLOAT", 0, {0,0,0}},
    {NGL_MEDIUM_FLOAT, "MEDIUM_FLOAT", 0, {0,0,0}},
    {NGL_HIGH_FLOAT, "HIGH_FLOAT", 0, {0,0,0}},
    {NGL_LOW_INT, "LOW_INT", 0, {0,0,0}},
    {NGL_MEDIUM_INT, "MEDIUM_INT", 0, {0,0,0}},
    {NGL_HIGH_INT, "HIGH_INT", 0, {0,0,0}},
    {NGL_FRAMEBUFFER, "FRAMEBUFFER", 0, {0,0,0}},
    {NGL_RENDERBUFFER, "RENDERBUFFER", 0, {0,0,0}},
    {NGL_RGBA4, "RGBA4", 0, {0,0,0}},
    {NGL_RGB5_A1, "RGB5_A1", 0, {0,0,0}},
    {NGL_RGB565, "RGB565", 0, {0,0,0}},
    {NGL_DEPTH_COMPONENT16, "DEPTH_COMPONENT16", 0, {0,0,0}},
    {NGL_STENCIL_INDEX, "STENCIL_INDEX", 0, {0,0,0}},
    {NGL_STENCIL_INDEX8, "STENCIL_INDEX8", 0, {0,0,0}},
    {NGL_DEPTH_STENCIL, "DEPTH_STENCIL", 0, {0,0,0}},
    {NGL_RENDERBUFFER_WIDTH, "RENDERBUFFER_WIDTH", 0, {0,0,0}},
    {NGL_RENDERBUFFER_HEIGHT, "RENDERBUFFER_HEIGHT", 0, {0,0,0}},
    {NGL_RENDERBUFFER_INTERNAL_FORMAT, "RENDERBUFFER_INTERNAL_FORMAT", 0, {0,0,0}},
    {NGL_RENDERBUFFER_RED_SIZE, "RENDERBUFFER_RED_SIZE", 0, {0,0,0}},
    {NGL_RENDERBUFFER_GREEN_SIZE, "RENDERBUFFER_GREEN_SIZE", 0, {0,0,0}},
    {NGL_RENDERBUFFER_BLUE_SIZE, "RENDERBUFFER_BLUE_SIZE", 0, {0,0,0}},
    {NGL_RENDERBUFFER_ALPHA_SIZE, "RENDERBUFFER_ALPHA_SIZE", 0, {0,0,0}},
    {NGL_RENDERBUFFER_DEPTH_SIZE, "RENDERBUFFER_DEPTH_SIZE", 0, {0,0,0}},
    {NGL_RENDERBUFFER_STENCIL_SIZE, "RENDERBUFFER_STENCIL_SIZE", 0, {0,0,0}},
    {NGL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, "FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE", 0, {0,0,0}},
    {NGL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, "FRAMEBUFFER_ATTACHMENT_OBJECT_NAME", 0, {0,0,0}},
    {NGL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, "FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL", 0, {0,0,0}},
    {NGL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE, "FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE", 0, {0,0,0}},
    {NGL_COLOR_ATTACHMENT0, "COLOR_ATTACHMENT0", 0, {0,0,0}},
    {NGL_DEPTH_ATTACHMENT, "DEPTH_ATTACHMENT", 0, {0,0,0}},
    {NGL_STENCIL_ATTACHMENT, "STENCIL_ATTACHMENT", 0, {0,0,0}},
    {NGL_DEPTH_STENCIL_ATTACHMENT, "DEPTH_STENCIL_ATTACHMENT", 0, {0,0,0}},
    {NGL_NONE, "NONE", 0, {0,0,0}},
    {NGL_FRAMEBUFFER_COMPLETE, "FRAMEBUFFER_COMPLETE", 0, {0,0,0}},
    {NGL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT, "FRAMEBUFFER_INCOMPLETE_ATTACHMENT", 0, {0,0,0}},
    {NGL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT, "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT", 0, {0,0,0}},
    //{NGL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT, "FRAMEBUFFER_INCOMPLETE_DIMENSIONS", 0, {0,0,0}},
    {NGL_FRAMEBUFFER_UNSUPPORTED, "FRAMEBUFFER_UNSUPPORTED", 0, {0,0,0}},
    {NGL_FRAMEBUFFER_BINDING, "FRAMEBUFFER_BINDING", 0, {0,0,0}},
    {NGL_RENDERBUFFER_BINDING, "RENDERBUFFER_BINDING", 0, {0,0,0}},
    {NGL_MAX_RENDERBUFFER_SIZE, "MAX_RENDERBUFFER_SIZE", 0, {0,0,0}},
    {NGL_INVALID_FRAMEBUFFER_OPERATION, "INVALID_FRAMEBUFFER_OPERATION", 0, {0,0,0}},
    {NGL_UNPACK_FLIP_Y_WEBGL, "UNPACK_FLIP_Y_WEBGL", 0, {0,0,0}},
    {NGL_UNPACK_PREMULTIPLY_ALPHA_WEBGL, "UNPACK_PREMULTIPLY_ALPHA_WEBGL", 0, {0,0,0}},
    {NGL_CONTEXT_LOST_WEBGL, "CONTEXT_LOST_WEBGL", 0, {0,0,0}},
    {NGL_UNPACK_COLORSPACE_CONVERSION_WEBGL, "UNPACK_COLORSPACE_CONVERSION_WEBGL", 0, {0,0,0}},
    {NGL_BROWSER_DEFAULT_WEBGL, "BROWSER_DEFAULT_WEBGL", 0, {0,0,0}},
    {0, NULL, 0, {0,0,0}}
};

NGL_JS_FN(WebGLRenderingContext_isContextLost) 
{
	MAKE_GL_CURRENT(cx, vp);
    // TODO
    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(false));
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_getExtension) 
{
	MAKE_GL_CURRENT(cx, vp);
    JS_SET_RVAL(cx, vp, JSVAL_NULL);
    return JS_TRUE;
}


NGL_JS_FN(WebGLRenderingContext_activeTexture) 
{
	MAKE_GL_CURRENT(cx, vp);
	GLuint texture;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &texture)) {
        return false;
    }

    GL_CALL(glActiveTexture(texture));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_attachShader)  
{
	MAKE_GL_CURRENT(cx, vp);
    uintptr_t cprogram;
    NGLShader *cshader;
    JSObject *program;
    JSObject *shader;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oo", &program, &shader)) {
        return false;
    }

    cprogram = (uintptr_t)JS_GetInstancePrivate(cx, program, &WebGLProgram_class, JS_ARGV(cx, vp));
    cshader = (NGLShader *)JS_GetInstancePrivate(cx, shader, &WebGLShader_class, JS_ARGV(cx, vp));

    GL_CALL(glAttachShader(cprogram, cshader->shader));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_bindAttribLocation)  
{
	MAKE_GL_CURRENT(cx, vp);
    uintptr_t cprogram;
    GLuint vertex;
    const char *cname;
    JSObject *program;
    JSString *name;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ouS", &program, &vertex, &name)) {
        return false;
    }

    cprogram = (uintptr_t)JS_GetInstancePrivate(cx, program, &WebGLProgram_class, JS_ARGV(cx, vp));
    cname = JS_EncodeString(cx, name);

    GL_CALL(glBindAttribLocation(cprogram, vertex, cname));

    JS_free(cx, (void *)cname);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_bindBuffer) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLenum target;
    uintptr_t cbuffer;
    JSObject *buffer;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uo", &target, &buffer)) {
        return false;
    }

	if (buffer == NULL) {
        printf("nullbuffer\n");
        return JS_TRUE;
    }

    cbuffer = (uintptr_t)JS_GetInstancePrivate(cx, buffer, &WebGLBuffer_class, JS_ARGV(cx, vp));

    GL_CALL(glBindBuffer(target, cbuffer));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_bindFramebuffer) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLenum target;
    uintptr_t cbuffer;
    JSObject *buffer;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uo", &target, &buffer)) {
        return false;
    }

	if (buffer == NULL) return JS_TRUE;

    cbuffer = (uintptr_t)JS_GetInstancePrivate(cx, buffer, &WebGLFrameBuffer_class, JS_ARGV(cx, vp));

    GL_CALL(glBindFramebuffer(target, cbuffer));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_bindRenderbuffer) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLenum target;
    uintptr_t cbuffer;
    JSObject *buffer;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uo", &target, &buffer)) {
        return false;
    }

	if (buffer == NULL) return JS_TRUE;

    cbuffer = (uintptr_t)JS_GetInstancePrivate(cx, buffer, &WebGLRenderbuffer_class, JS_ARGV(cx, vp));

    GL_CALL(glBindRenderbuffer(target, cbuffer));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_bindTexture) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLenum target;
    uintptr_t ctexture;
    JSObject *texture;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uo", &target, &texture)) {
        return false;
    }

	// No argv passed, as texture starting at 0
    ctexture = (uintptr_t)JS_GetInstancePrivate(cx, texture, &WebGLTexture_class, NULL);
    GL_CALL(glBindTexture(target, ctexture));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_blendEquation) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLuint mode;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &mode)) {
        return false;
    }
    
    GL_CALL(glBlendEquation(mode));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_blendEquationSeparate)     
{
	MAKE_GL_CURRENT(cx, vp);
    GLuint modeRGB;
    GLenum modeAlpha;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uu", &modeRGB, &modeAlpha)) {
        return false;
    }
    
    GL_CALL(glBlendEquationSeparate(modeRGB, modeAlpha));

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_blendFunc) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLenum sfactor;
    GLuint dfactor;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uu", &sfactor, &dfactor)) {
        return false;
    }
    
    GL_CALL(glBlendFunc(sfactor, dfactor));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_blendFuncSeparate) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLuint srcRGB;
    GLuint dstRGB;
    GLenum srcAlpha;
    GLenum dstAlpha;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuuu", &srcRGB, &dstRGB, &srcAlpha, &dstAlpha)) {
        return false;
    }
    
    GL_CALL(glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha));

    return JS_TRUE;
}


NGL_JS_FN(WebGLRenderingContext_bufferData) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLenum target;
    JSObject *array;
    GLenum usage;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uou", &target, &array, &usage)) {
        return false;
    }

	if (array == NULL || !JS_IsTypedArrayObject(array)) {
        JS_ReportError(cx, "Invalid value");
		return false;
	}

    GL_CALL(glBufferData(target, JS_GetArrayBufferViewByteLength(array), JS_GetArrayBufferViewData(array), usage));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_clear) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLbitfield bits;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "i", &bits)) {
        return false;
    }
    
    GL_CALL(glClear(bits | GL_DEPTH_BUFFER_BIT));

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_clearColor) 
{
	MAKE_GL_CURRENT(cx, vp);
    double r;
    double g;
    double b;
    double a;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddd", &r, &g, &b, &a)) {
        return false;
    }
    
    GL_CALL(glClearColor(r, g, b, a));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_clearDepth) 
{
	MAKE_GL_CURRENT(cx, vp);
    double clampd;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "d", &clampd)) {
        return false;
    }
    GL_CALL(glClearDepth(clampd));

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_clearStencil) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLint s;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "i", &s)) {
        return false;
    }

    GL_CALL(glClearStencil(s));

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_compileShader) 
{
	MAKE_GL_CURRENT(cx, vp);
    NGLShader *cshader;
    JSObject *shader;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &shader)) {
        return false;
    }

    cshader = (NGLShader *)JS_GetInstancePrivate(cx, shader, &WebGLShader_class, JS_ARGV(cx, vp));

    ShBuiltInResources resources; 
    ShHandle compiler = 0;
    size_t len = 0;
    char *str;

    ShInitBuiltInResources(&resources);

    // TODO use real values (see third-party/mozilla-central/content/canvas/src/WebGLContextValidate.cpp )
    resources.MaxVertexAttribs = 8*4;
    resources.MaxVertexUniformVectors = 128*4;
    resources.MaxVaryingVectors = 8*4;
    resources.MaxVertexTextureImageUnits = 4*4;
    resources.MaxCombinedTextureImageUnits = 8*4;
    resources.MaxTextureImageUnits = 8*4;
    resources.MaxFragmentUniformVectors = 16*4;
    resources.MaxDrawBuffers = 1;

    resources.OES_standard_derivatives = 1;
    resources.FragmentPrecisionHigh = 1;

    compiler = ShConstructCompiler((ShShaderType)cshader->type, SH_WEBGL_SPEC, SH_GLSL_OUTPUT, &resources);
    // XXX : Might be interesting to use SH_VALIDATE
    if (!ShCompile(compiler, &cshader->source, 1, SH_OBJECT_CODE|SH_ATTRIBUTES_UNIFORMS|SH_ENFORCE_PACKING_RESTRICTIONS|SH_MAP_LONG_VARIABLE_NAMES)) {
        size_t bufferLen;
        ShGetInfo(compiler, SH_INFO_LOG_LENGTH, &bufferLen);
        char *buffer = (char*) malloc(bufferLen * sizeof(char));
        ShGetInfoLog(compiler, buffer);

        JS_ReportError(cx, "Failed to translate shader to GLSL. %s", buffer);
        free(buffer);

        return false;
    }

    ShGetInfo(compiler, SH_OBJECT_CODE_LENGTH, &len);
    str = (char *)malloc(len * sizeof(char));
    ShGetObjectCode(compiler, str);
    const char *foo = (const char *)str;

    GLint shaderLen = len;
    GL_CALL(glShaderSource(cshader->shader, 1, &foo, &shaderLen));

    GL_CALL(glCompileShader(cshader->shader));

    ShDestruct(compiler);
    JS_free(cx, (void *)cshader->source);
    free(str);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_createBuffer) 
{
	MAKE_GL_CURRENT(cx, vp);
    intptr_t buffer;
    jsval proto;
    JSObject *ret;
    
    GL_CALL(glGenBuffers(1, (GLuint *)&buffer));

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLBuffer", &proto);
    ret = JS_NewObject(cx, &WebGLBuffer_class, JSVAL_TO_OBJECT(proto), NULL);
    JS_SetPrivate(ret, (void *)buffer);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_createFramebuffer) 
{
	MAKE_GL_CURRENT(cx, vp);
    intptr_t buffer;
    jsval proto;
    JSObject *ret;
    
    GL_CALL(glGenFramebuffers(1, (GLuint *)&buffer));

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLFrameBuffer", &proto);
    ret = JS_NewObject(cx, &WebGLFrameBuffer_class, JSVAL_TO_OBJECT(proto), NULL);
    JS_SetPrivate(ret, (void *)buffer);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_createRenderbuffer) 
{
	MAKE_GL_CURRENT(cx, vp);
    intptr_t buffer;
    jsval proto;
    JSObject *ret;
    
    GL_CALL(glGenRenderbuffers(1, (GLuint *)&buffer));

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLRenderbuffer", &proto);
    ret = JS_NewObject(cx, &WebGLRenderbuffer_class, JSVAL_TO_OBJECT(proto), NULL);
    JS_SetPrivate(ret, (void *)buffer);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_createProgram) 
{
	MAKE_GL_CURRENT(cx, vp);
    intptr_t program = GL_CALL(glCreateProgram());
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
	MAKE_GL_CURRENT(cx, vp);
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

    cshader = GL_CALL(glCreateShader(type));
    shader = new NGLShader(type, cshader);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLShader", &proto);
    ret = JS_NewObject(cx, &WebGLShader_class, JSVAL_TO_OBJECT(proto), NULL);

    JS_SetPrivate(ret, (void *)shader);
    
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_createTexture) 
{
	MAKE_GL_CURRENT(cx, vp);
    uintptr_t texture;
	JSObject *ret;
    jsval proto;

    GL_CALL(glGenTextures(1, (GLuint *)&texture));

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "WebGLTexture", &proto);
    ret = JS_NewObject(cx, &WebGLTexture_class, JSVAL_TO_OBJECT(proto), NULL);
    JS_SetPrivate(ret, (void *)texture);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}


NGL_JS_FN(WebGLRenderingContext_cullFace) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLuint mode;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &mode)) {
        return false;
    }
    
    GL_CALL(glCullFace(mode));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_deleteShader) 
{
	MAKE_GL_CURRENT(cx, vp);
    JSObject *shader;
    NGLShader *cshader;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &shader)) {
        return false;
    }

    cshader = (NGLShader *)JS_GetInstancePrivate(cx, shader, &WebGLShader_class, JS_ARGV(cx, vp));
    GL_CALL(glDeleteShader(cshader->shader));
    delete cshader;
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_depthFunc) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLuint func;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &func)) {
        return false;
    }
    
    GL_CALL(glDepthFunc(func));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_depthMask) 
{
	MAKE_GL_CURRENT(cx, vp);
    JSBool flag;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "b", &flag)) {
        return false;
    }

    GL_CALL(glDepthMask((GLboolean)flag));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_disable) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLenum cap;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &cap)) {
        return false;
    }
    
    GL_CALL(glDisable(cap));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_disableVertexAttribArray) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLuint attr;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &attr)) {
        return false;
    }
    
    GL_CALL(glDisableVertexAttribArray(attr));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_drawArrays) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLenum mode;
    GLint first;
    GLsizei count;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uii", &mode, &first, &count)) {
        return false;
    }
    
    GL_CALL(glDrawArrays(mode, first, count));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_drawElements) 
{
	MAKE_GL_CURRENT(cx, vp);
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
    
    printf("in glDrawElements %d\n", offset);
    GL_CALL(glDrawElements(mode, count, type, (void *)(intptr_t)offset));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_enable) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLuint bits;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &bits)) {
        return false;
    }
    
    GL_CALL(glEnable(bits));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_enableVertexAttribArray) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLuint attr;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &attr)) {
        return false;
    }
    
    GL_CALL(glEnableVertexAttribArray(attr));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_getUniformLocation) 
{
	MAKE_GL_CURRENT(cx, vp);
    uintptr_t cprogram;
    intptr_t location;
    JSString *name;
    JSObject *program;
    JSObject *ret;
    jsval proto;
    const char *cname;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oS", &program, &name)) {
        return false;
    }

    cprogram = (uintptr_t)JS_GetInstancePrivate(cx, program, &WebGLProgram_class, JS_ARGV(cx, vp));
    cname = JS_EncodeString(cx, name);
 
    location = GL_CALL(glGetUniformLocation(cprogram, cname));

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

NGL_JS_FN(WebGLRenderingContext_getShaderPrecisionFormat) 
{
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

    return JS_TRUE;
#undef SET_PROP
}

NGL_JS_FN(WebGLRenderingContext_framebufferRenderbuffer) 
{
	MAKE_GL_CURRENT(cx, vp);
	GLenum target, attachement, renderbuffertarget;
	uintptr_t crenderbuffer;
	JSObject *renderbuffer;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuuo", &target, &attachement, &renderbuffertarget, &renderbuffer)) {
        return false;
    }

    crenderbuffer = (uintptr_t)JS_GetInstancePrivate(cx, renderbuffer, &WebGLRenderbuffer_class, JS_ARGV(cx, vp));

	GL_CALL(glFramebufferRenderbuffer(target, attachement, renderbuffertarget, crenderbuffer));

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_framebufferTexture2D) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLenum target, attachement, textarget;
	uintptr_t ctexture, level;
    JSObject *texture;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuuoi", &target, &attachement, &textarget, &texture, &level)) {
        return false;
    }

	// No argv passed, as texture starting at 0
    ctexture = (uintptr_t)JS_GetInstancePrivate(cx, texture, &WebGLTexture_class, NULL);

	GL_CALL(glFramebufferTexture2D(target, attachement, textarget, ctexture, level));

    GLenum status;
    status = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));

    switch(status) {
        case GL_FRAMEBUFFER_COMPLETE:
            printf("OH YEAH\n");
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            printf("fbo unsupported\n");
            exit(2);
            return false;
        default:
            printf("fbo fatal error wat %d\n", status);
            exit(2);
            return false;
    }
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_frontFace) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLuint mode;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &mode)) {
        return false;
    }
    
    GL_CALL(glFrontFace(mode));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_generateMipmap) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLenum target;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &target)) {
        return false;
    }
    
    GL_CALL(glGenerateMipmap(target));
    
    return JS_TRUE;
}


NGL_JS_FN(WebGLRenderingContext_getAttribLocation) 
{
	MAKE_GL_CURRENT(cx, vp);
    uintptr_t cprogram;
    GLint location;
    JSString *attr;
    JSObject *program;
    const char *cattr;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oS", &program, &attr)) {
        return false;
    }
    
    cprogram = (uintptr_t)JS_GetInstancePrivate(cx, program, &WebGLProgram_class, JS_ARGV(cx, vp));
    cattr = JS_EncodeString(cx, attr);

    location = GL_CALL(glGetAttribLocation(cprogram, cattr));

    JS_free(cx, (void *)cattr);
    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(location));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_getParameter) 
{
	MAKE_GL_CURRENT(cx, vp);
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
            const char *cstr = (const char *)GL_CALL(glGetString(name));
            JSString *str = JS_NewStringCopyZ(cx, cstr);

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
            GL_CALL(glGetIntegerv(name, &i));
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
            GL_CALL(glGetIntegerv(name, &i));
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

            GL_CALL(glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &length));
            obj = JS_NewUint32Array(cx, length);
            textures = (GLint *)malloc(sizeof(GLint) * length);

            if (!obj || !textures) {
                if (textures != NULL) {
                    free(textures);
                }
                JS_ReportOutOfMemory(cx);
                return false;
            }

            GL_CALL(glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, textures));

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
            GL_CALL(glGetIntegerv(name, &i));
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
            GL_CALL(glGetFloatv(name, &f));
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
            GL_CALL(glGetBooleanv(name, &b));
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

            GL_CALL(glGetFloatv(name, fv));

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

            GL_CALL(glGetFloatv(name, fv));

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

            GL_CALL(glGetIntegerv(name, iv));

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

            GL_CALL(glGetIntegerv(name, iv));

            data = JS_GetInt32ArrayData(obj);
            memcpy(data, iv, 4 * sizeof(GLint));
            value.setObjectOrNull(obj);
            break;
        }

        case NGL_COLOR_WRITEMASK: // 4 bools
        {
            GLboolean gl_bv[4] = { 0 };

            GL_CALL(glGetBooleanv(name, gl_bv));

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
            return JS_TRUE;
    }

    JS_SET_RVAL(cx, vp, value);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_getProgramParameter)  
{
	MAKE_GL_CURRENT(cx, vp);
    uintptr_t cprogram;
    GLenum param;
    GLint status;
    JSObject *program;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ou", &program, &param)) {
        return false;
    }

    cprogram = (uintptr_t)JS_GetInstancePrivate(cx, program, &WebGLProgram_class, JS_ARGV(cx, vp));

    GL_CALL(glGetProgramiv(cprogram, param, &status));
    
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
	MAKE_GL_CURRENT(cx, vp);
    uintptr_t cprogram;
    GLsizei max;
    GLsizei length;
    JSString *log;
    JSObject *program;
    char *clog;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &program)) {
        return false;
    }

    cprogram = (uintptr_t)JS_GetInstancePrivate(cx, program, &WebGLProgram_class, JS_ARGV(cx, vp));
    
    GL_CALL(glGetProgramiv(cprogram, GL_INFO_LOG_LENGTH, &max));

    clog = (char *)malloc(max);
    GL_CALL(glGetProgramInfoLog(cprogram, max, &length, clog));
    log = JS_NewStringCopyN(cx, clog, length);
    free(clog);

    JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(log));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_getShaderParameter) 
{
	MAKE_GL_CURRENT(cx, vp);
    NGLShader *cshader;
    GLenum pname;
    GLint param;
    JSObject *shader;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ou", &shader, &pname)) {
        return false;
    }

    cshader = (NGLShader *)JS_GetInstancePrivate(cx, shader, &WebGLShader_class, JS_ARGV(cx, vp));

    GL_CALL(glGetShaderiv(cshader->shader, pname, &param));

    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(param));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_getShaderInfoLog) 
{
	MAKE_GL_CURRENT(cx, vp);
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
    
    GL_CALL(glGetShaderiv(cshader->shader, GL_INFO_LOG_LENGTH, &max));

    clog = (char *)malloc(max);
    GL_CALL(glGetShaderInfoLog(cshader->shader, max, &length, clog));
    log = JS_NewStringCopyN(cx, clog, length);
    free(clog);

    JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(log));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_lineWidth) 
{
	MAKE_GL_CURRENT(cx, vp);
    //GLfloat width;
    double width;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "d", &width)) {
        return false;
    }

    GL_CALL(glLineWidth((GLfloat)width));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_linkProgram) 
{
	MAKE_GL_CURRENT(cx, vp);
    uintptr_t cprogram;
    JSObject *program;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &program)) {
        return false;
    }
    
    cprogram = (uintptr_t)JS_GetInstancePrivate(cx, program, &WebGLProgram_class, JS_ARGV(cx, vp));

    GL_CALL(glLinkProgram(cprogram));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_pixelStorei)  
{
	MAKE_GL_CURRENT(cx, vp);
    GLuint param;
    GLint value;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ui", &param, &value)) {
        return false;
    }

    switch (param) {
        case NGL_UNPACK_FLIP_Y_WEBGL:
        {
            NativeCanvasWebGLContext *ngl = NATIVE_GL_GETTER(JS_THIS_OBJECT(cx, vp));
            ngl->unpackFlipY = (bool)value;
            break;
        }
        case NGL_UNPACK_PREMULTIPLY_ALPHA_WEBGL:
        {
            NativeCanvasWebGLContext *ngl = NATIVE_GL_GETTER(JS_THIS_OBJECT(cx, vp));
            ngl->unpackPremultiplyAlpha = (bool)value;
            break;
        }
        case NGL_UNPACK_COLORSPACE_CONVERSION_WEBGL:
            JS_ReportError(cx, "Not implemented");
            return false;
        break;
        default: 
            GL_CALL(glPixelStorei(param, value));
        break;
    }
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_renderbufferStorage)  
{
	MAKE_GL_CURRENT(cx, vp);
    GLenum target, internalFormat;
	GLsizei width, height;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuii", &target, &internalFormat, &width, &height)) {
        return false;
    }

	GL_CALL(glRenderbufferStorage(target, internalFormat, width, height));

	return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_shaderSource) 
{
	MAKE_GL_CURRENT(cx, vp);
    NGLShader *cshader;
    GLsizei length;
    JSString *source;
    JSObject *shader;
    const char *csource;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oS", &shader, &source)) {
        return false;
    }

    cshader = (NGLShader *)JS_GetInstancePrivate(cx, shader, &WebGLShader_class, JS_ARGV(cx, vp));
    csource = JS_EncodeString(cx, source);
    length = JS_GetStringLength(source);

    cshader->source = csource;
    cshader->length = length;
    
    //GL_CALL(glShaderSource(cshader->shader, 1, &csource, &length));

    //JS_free(cx, (void *)csource);
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_texImage2D) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLenum target;
    GLint level;
    GLint internalFormat;
    GLenum format;
    GLenum type;
    JSObject *image;
    int width, height;
	NativeJSImage *nimg;
    void *pixels = NULL;
    NativeCanvasWebGLContext *ngl;
    unsigned char *rgbaPixels;

    ngl = NATIVE_GL_GETTER(JS_THIS_OBJECT(cx, vp));
    
    if (argc == 9) {
        GLint border;
        JSObject *array;

        if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uiiiiiuuo", &target, &level, &internalFormat, &width, &height, &border, &format, &type, &array)) {
            return false;
        }

        if (array != NULL && JS_IsTypedArrayObject(array)) {
            pixels = JS_GetArrayBufferViewData(array);
        } 
        
        GL_CALL(glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels));
    } else {
        if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uiiuuo", &target, &level, &internalFormat, &format, &type, &image)) {
            return false;
        }

        // XXX : UNSAFE use JS_GetInstancePrivate Image_class 
        //if ((nimg = static_cast<NativeJSImage *>(JS_GetInstancePrivate(cx, image, &Image_class, JS_ARGV(cx, vp)))) == NULL) {
        if ((nimg = static_cast<NativeJSImage *>(JS_GetPrivate(image))) == NULL) {
            JS_ReportError(cx, "Invalid image object");
            return false;
        }

        width = nimg->img->getWidth();
        height = nimg->img->getHeight();
        
        rgbaPixels = (unsigned char*)malloc(nimg->img->img->getSize());

        if (!NativeSkImage::ConvertToRGBA(nimg->img, rgbaPixels, ngl->unpackFlipY, 
                ngl->unpackPremultiplyAlpha)) {
            JS_ReportError(cx, "Failed to read image data");
            return false;
        }

        GL_CALL(glTexImage2D(target, level, internalFormat, width, height, 0, format, type, rgbaPixels));

        free(rgbaPixels);
    }

    return JS_TRUE;
}


NGL_JS_FN(WebGLRenderingContext_texParameteri) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLuint target;
    GLuint pname;
    GLuint param;
    
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuu", &target, &pname, &param)) {
        return false;
    }
    
    GL_CALL(glTexParameteri(target, pname, param));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_uniform1f)  
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniform_x_f(cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_uniform1fv) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniform_x_fv(cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_uniform1i) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniform_x_i(cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_uniform1iv) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniform_x_iv(cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_uniform2f)  
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniform_x_f(cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_uniform2fv) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniform_x_fv(cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_uniform2i) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniform_x_i(cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_uniform2iv) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniform_x_iv(cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_uniform3f)  
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniform_x_f(cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_uniform3fv) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniform_x_fv(cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_uniform3i) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniform_x_i(cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_uniform3iv) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniform_x_iv(cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_uniform4f)  
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniform_x_f(cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_uniform4fv) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniform_x_fv(cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_uniform4i) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniform_x_i(cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_uniform4iv) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniform_x_iv(cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_uniformMatrix2fv) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniformMatrix_x_fv(cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_uniformMatrix3fv) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniformMatrix_x_fv(cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_uniformMatrix4fv) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_uniformMatrix_x_fv(cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib1f) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_vertexAttrib_x_f(cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib1fv) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_vertexAttrib_x_fv(cx, argc, vp, 1);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib2f) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_vertexAttrib_x_f(cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib2fv) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_vertexAttrib_x_fv(cx, argc, vp, 2);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib3f) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_vertexAttrib_x_f(cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib3fv) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_vertexAttrib_x_fv(cx, argc, vp, 3);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib4f) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_vertexAttrib_x_f(cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttrib4fv) 
{
	MAKE_GL_CURRENT(cx, vp);
    return NativeGL_vertexAttrib_x_fv(cx, argc, vp, 4);
}

NGL_JS_FN(WebGLRenderingContext_vertexAttribPointer) 
{
	MAKE_GL_CURRENT(cx, vp);
    GLuint attr;
    GLint size;
    GLenum type;
    GLsizei stride;
    GLint offset;
    JSBool normalized;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uiubii", &attr, &size, &type, &normalized, &stride, &offset)) {
        return false;
    }

    if (offset+size < offset || offset+size < size) {
        JS_ReportError(cx, "Overflow in vertexAttribPointer");
        return false;
    }

    GL_CALL(glVertexAttribPointer(attr, size, type, (GLboolean)normalized, stride, (void *)(intptr_t)offset));

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_useProgram)  
{
	MAKE_GL_CURRENT(cx, vp);
    uintptr_t cprogram;
    JSObject *program;

    if (JSVAL_IS_INT(JS_ARGV(cx, vp)[0])) {
        int prog = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);
        printf("program=%d\n", prog);
        GL_CALL(glUseProgram(prog));
        return JS_TRUE;
    }

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &program)) {
        return false;
    }
    
    cprogram = (uintptr_t)JS_GetInstancePrivate(cx, program, &WebGLProgram_class, JS_ARGV(cx, vp));

    GL_CALL(glUseProgram(cprogram));
    
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_viewport)  
{
	MAKE_GL_CURRENT(cx, vp);
    GLint x, y, w, h;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "iiii", &x, &y, &w, &h)) {
        return false;
    }

    GL_CALL(glViewport(x, y, w, h));

    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_getError) 
{
	MAKE_GL_CURRENT(cx, vp);
    JS_SET_RVAL(cx, vp, UINT_TO_JSVAL(glGetError()));
    return JS_TRUE;
}

NGL_JS_FN(WebGLRenderingContext_swapBuffer) 
{
#if 0
	MAKE_GL_CURRENT(cx, vp);
    NativeContext::getNativeClass(cx)->getUI()->swapGLBuffer();
    return JS_TRUE;
#endif
    return JS_TRUE;
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
    GL_CALL(glEnable(GL_VERTEX_PROGRAM_POINT_SIZE));
    //GL_CALL(glEnable(GL_POINT_SPRITE));
    //GL_ARB_point
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(webGLContext));

    return JS_TRUE;
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
    obj = JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, &WebGLRenderingContext_class,
                NativeJSWebGLRenderingContext_constructor,
                0, NULL, WebGLRenderingContext_funcs, NULL, NULL);

    if (!obj || !(ctor = JS_GetConstructor(cx, obj))) {
        // TODO : Handle failure. Throw exception ? 
    }

    JS_DefineConstDoubles(cx, ctor, WebGLRenderingContext_const);
}

NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLObject);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLBuffer);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLFrameBuffer);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLProgram);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLRenderbuffer);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLShader);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLTexture);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLUniformLocation);
NATIVE_GL_OBJECT_EXPOSE_NOT_INST(WebGLShaderPrecisionFormat);
