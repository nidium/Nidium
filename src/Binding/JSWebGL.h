/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jswebgl_h__
#define binding_jswebgl_h__

#include "Binding/ClassMapper.h"

#include "Graphics/GLHeader.h"

#include "Graphics/CanvasContext.h"
#include "Graphics/Canvas3DContext.h"
#include "Graphics/CanvasHandler.h"
#include "Graphics/GLContext.h"
#include "WebGL_Types.h"

namespace Nidium {
namespace Binding {

// {{{ WebGL Classes declaration
// {{{ JSWebGLRenderingContext
class JSWebGLRenderingContext : public ClassMapper<JSWebGLRenderingContext>,
                                public Graphics::Canvas3DContext
{
public:
    JSWebGLRenderingContext(Graphics::CanvasHandler *handler,
                            struct JSContext *cx,
                            int width,
                            int height,
                            Interface::UIInterface *ui) :
        Graphics::Canvas3DContext(handler, cx, width, height, ui) {}
    static void RegisterObject(JSContext *cx);
    static void RegisterAllObjects(JSContext *cx);
	static JSFunctionSpec *ListMethods();
	static JSConstDoubleSpec *ListConstDoubles();
    JSObject *getJSInstance() override {
        return this->getJSObject();
    }

protected:
	NIDIUM_DECL_JSCALL(isContextLost);
	NIDIUM_DECL_JSCALL(getExtension);
	NIDIUM_DECL_JSCALL(activeTexture);
	NIDIUM_DECL_JSCALL(attachShader);
	NIDIUM_DECL_JSCALL(bindAttribLocation);
	NIDIUM_DECL_JSCALL(bindBuffer);
	NIDIUM_DECL_JSCALL(bindRenderbuffer);
	NIDIUM_DECL_JSCALL(bindFramebuffer);
	NIDIUM_DECL_JSCALL(bindTexture);
	NIDIUM_DECL_JSCALL(copyTexImage2D);
	NIDIUM_DECL_JSCALL(copyTexSubImage2D);
	NIDIUM_DECL_JSCALL(blendEquation);
	NIDIUM_DECL_JSCALL(blendEquationSeparate);
	NIDIUM_DECL_JSCALL(blendFunc);
	NIDIUM_DECL_JSCALL(blendFuncSeparate);
	NIDIUM_DECL_JSCALL(bufferData);
	NIDIUM_DECL_JSCALL(bufferSubData);
	NIDIUM_DECL_JSCALL(clear);
	NIDIUM_DECL_JSCALL(clearColor);
	NIDIUM_DECL_JSCALL(clearDepth);
	NIDIUM_DECL_JSCALL(clearStencil);
	NIDIUM_DECL_JSCALL(colorMask);
	NIDIUM_DECL_JSCALL(compileShader);
	NIDIUM_DECL_JSCALL(createBuffer);
	NIDIUM_DECL_JSCALL(createFramebuffer);
	NIDIUM_DECL_JSCALL(createRenderbuffer);
	NIDIUM_DECL_JSCALL(createProgram);
	NIDIUM_DECL_JSCALL(createShader);
	NIDIUM_DECL_JSCALL(createTexture);
	NIDIUM_DECL_JSCALL(cullFace);
	NIDIUM_DECL_JSCALL(deleteBuffer);
	NIDIUM_DECL_JSCALL(deleteFramebuffer);
	NIDIUM_DECL_JSCALL(deleteProgram);
	NIDIUM_DECL_JSCALL(deleteRenderbuffer);
	NIDIUM_DECL_JSCALL(deleteShader);
	NIDIUM_DECL_JSCALL(deleteTexture);
	NIDIUM_DECL_JSCALL(depthFunc);
	NIDIUM_DECL_JSCALL(depthMask);
	NIDIUM_DECL_JSCALL(depthRange);
	NIDIUM_DECL_JSCALL(detachShader);
	NIDIUM_DECL_JSCALL(disable);
	NIDIUM_DECL_JSCALL(disableVertexAttribArray);
	NIDIUM_DECL_JSCALL(drawArrays);
	NIDIUM_DECL_JSCALL(drawElements);
	NIDIUM_DECL_JSCALL(enable);
	NIDIUM_DECL_JSCALL(enableVertexAttribArray);
	NIDIUM_DECL_JSCALL(finish);
	NIDIUM_DECL_JSCALL(flush);
	NIDIUM_DECL_JSCALL(getUniformLocation);
	NIDIUM_DECL_JSCALL(getShaderPrecisionFormat);
	NIDIUM_DECL_JSCALL(framebufferRenderbuffer);
	NIDIUM_DECL_JSCALL(framebufferTexture2D);
	NIDIUM_DECL_JSCALL(frontFace);
	NIDIUM_DECL_JSCALL(generateMipmap);
	NIDIUM_DECL_JSCALL(getActiveAttrib);
	NIDIUM_DECL_JSCALL(getActiveUniform);
	NIDIUM_DECL_JSCALL(getAttribLocation);
	NIDIUM_DECL_JSCALL(getParameter);
	NIDIUM_DECL_JSCALL(getProgramParameter);
	NIDIUM_DECL_JSCALL(getProgramInfoLog);
	NIDIUM_DECL_JSCALL(getShaderParameter);
	NIDIUM_DECL_JSCALL(getShaderInfoLog);
	NIDIUM_DECL_JSCALL(lineWidth);
	NIDIUM_DECL_JSCALL(linkProgram);
	NIDIUM_DECL_JSCALL(pixelStorei);
	NIDIUM_DECL_JSCALL(renderbufferStorage);
	NIDIUM_DECL_JSCALL(scissor);
	NIDIUM_DECL_JSCALL(shaderSource);
	NIDIUM_DECL_JSCALL(texImage2D);
	NIDIUM_DECL_JSCALL(texParameteri);
	NIDIUM_DECL_JSCALL(uniform1f);
	NIDIUM_DECL_JSCALL(uniform1fv);
	NIDIUM_DECL_JSCALL(uniform1i);
	NIDIUM_DECL_JSCALL(uniform1iv);
	NIDIUM_DECL_JSCALL(uniform2f);
	NIDIUM_DECL_JSCALL(uniform2fv);
	NIDIUM_DECL_JSCALL(uniform2i);
	NIDIUM_DECL_JSCALL(uniform2iv);
	NIDIUM_DECL_JSCALL(uniform3f);
	NIDIUM_DECL_JSCALL(uniform3fv);
	NIDIUM_DECL_JSCALL(uniform3i);
	NIDIUM_DECL_JSCALL(uniform3iv);
	NIDIUM_DECL_JSCALL(uniform4f);
	NIDIUM_DECL_JSCALL(uniform4fv);
	NIDIUM_DECL_JSCALL(uniform4i);
	NIDIUM_DECL_JSCALL(uniform4iv);
	NIDIUM_DECL_JSCALL(uniformMatrix2fv);
	NIDIUM_DECL_JSCALL(uniformMatrix3fv);
	NIDIUM_DECL_JSCALL(uniformMatrix4fv);
	NIDIUM_DECL_JSCALL(vertexAttrib1f);
	NIDIUM_DECL_JSCALL(vertexAttrib1fv);
	NIDIUM_DECL_JSCALL(vertexAttrib2f);
	NIDIUM_DECL_JSCALL(vertexAttrib2fv);
	NIDIUM_DECL_JSCALL(vertexAttrib3f);
	NIDIUM_DECL_JSCALL(vertexAttrib3fv);
	NIDIUM_DECL_JSCALL(vertexAttrib4f);
	NIDIUM_DECL_JSCALL(vertexAttrib4fv);
	NIDIUM_DECL_JSCALL(vertexAttribPointer);
	NIDIUM_DECL_JSCALL(viewport);
	NIDIUM_DECL_JSCALL(useProgram);
	NIDIUM_DECL_JSCALL(getError);
	NIDIUM_DECL_JSCALL(swapBuffer);
};
// }}}
// {{{ WebGLResource
class WebGLResource
{
public:
    enum ResourceType
    {
        kTexture,
        kProgram,
        kShader,
        kBuffer,
        kFramebuffer,
        kRenderbuffer,
        kVertexArray,
        kResources_end
    };

    struct ShaderData
    {
        unsigned int type;
        const char *source;
    };

    WebGLResource(GLuint id,
                  ResourceType type,
                  JSWebGLRenderingContext *webglCtx)
        : m_GlIdentifier(id), m_JSCx(webglCtx->getJSContext()),
          m_WebGLCtx(webglCtx), m_Type(type) {};

    ~WebGLResource();

    virtual JSObject *getJSObject() = 0;

    GLuint id() const
    {
        return m_GlIdentifier;
    }

    void bind();
    void bindTo(GLenum target);
    void unbind();
    void static Unbind(JS::HandleObject JSGLCtx, ResourceType type);
    void static UnbindFrom(JSContext *cx,
                           JS::HandleObject JSGLCtx,
                           ResourceType type,
                           GLenum target);

    GLuint m_GlIdentifier;
    JSContext *m_JSCx;
    JSWebGLRenderingContext *m_WebGLCtx;
    ResourceType m_Type;

    GLenum m_Target = NGL_NONE;
    bool m_IsBound  = false;
};
// }}}
#define NIDIUM_WEBGL_DECL_CLASS(NAME)                   \
    class JS##NAME : public ClassMapper<JS##NAME>   \
    {                                               \
    public:                                         \
        static void RegisterObject(JSContext *cx) { \
            JS##NAME::ExposeClass<0>(cx, #NAME);    \
        }                                           \
    };

#define NIDIUM_WEBGL_DECL_RESOURCE_CLASS(NAME, TYPE)                   \
    class JS##NAME : public ClassMapper<JS##NAME>, public WebGLResource   \
    {                                               \
    public:                                         \
        JS##NAME(GLuint id, JSWebGLRenderingContext *webglCtx) : \
            WebGLResource(id, TYPE, webglCtx) {} \
        static void RegisterObject(JSContext *cx) { \
            JS##NAME::ExposeClass<0>(cx, #NAME);    \
        }                                           \
        virtual JSObject *getJSObject() override { \
            return ClassMapper<JS##NAME>::getJSObject(); \
        } \
    };

NIDIUM_WEBGL_DECL_RESOURCE_CLASS(WebGLBuffer, WebGLResource::kBuffer)
NIDIUM_WEBGL_DECL_RESOURCE_CLASS(WebGLFramebuffer, WebGLResource::kFramebuffer)
NIDIUM_WEBGL_DECL_RESOURCE_CLASS(WebGLProgram, WebGLResource::kProgram)
NIDIUM_WEBGL_DECL_RESOURCE_CLASS(WebGLRenderbuffer, WebGLResource::kRenderbuffer)
NIDIUM_WEBGL_DECL_RESOURCE_CLASS(WebGLTexture, WebGLResource::kTexture)
// {{{ JSWebGLShader
class JSWebGLShader : public ClassMapper<JSWebGLShader>,
                      public WebGLResource
{
public:
    JSWebGLShader(GLenum type, GLuint id, JSWebGLRenderingContext *webglCtx) :
        WebGLResource(id, WebGLResource::kShader, webglCtx),
        m_ShaderData({type, NULL }) {}

    virtual ~JSWebGLShader();

    static void RegisterObject(JSContext *cx)
    {
        JSWebGLShader::ExposeClass<0>(cx, "WebGLShader");
    }

    void setShaderSource(JS::HandleString str);
    void freeShaderSource();

    const char *getShaderSource()
    {
        return m_ShaderData.source;
    }

    Graphics::CanvasContext::shaderType getShaderType()
    {
        return static_cast<Graphics::CanvasContext::shaderType>(m_ShaderData.type);
    }

    virtual JSObject *getJSObject() override {
        return ClassMapper<JSWebGLShader>::getJSObject();
    }
private:
    ShaderData m_ShaderData;
};
// }}}

//NIDIUM_WEBGL_DECL_CLASS(WebGLObject)
// {{{ JSWebGLActiveInfo
class JSWebGLActiveInfo : public ClassMapper<JSWebGLActiveInfo>
{
public:
    JSObject *createObject(JSContext *cx, GLint csize, GLenum ctype, const char *cname);
    static void RegisterObject(JSContext *cx) {
        JSWebGLActiveInfo::ExposeClass<0>(cx, "WebGLActiveInfo");
    }
};
// }}}
// {{{ JSWebGLUniformLocation
class JSWebGLUniformLocation : public ClassMapper<JSWebGLUniformLocation>
{
public:
    JSWebGLUniformLocation(GLint location) :
        m_Location(location) {};

    GLint get() {
        return m_Location;
    }

    static void RegisterObject(JSContext *cx) {
        JSWebGLUniformLocation::ExposeClass<0>(cx, "WebGLUniformLocation");
    }
private:
    GLint m_Location;
};
// }}}
NIDIUM_WEBGL_DECL_CLASS(WebGLShaderPrecisionFormat)

#undef NIDIUM_GL_NEW_CLASS
// }}}

} // namespace Binding
} // namespace Nidium

#endif
