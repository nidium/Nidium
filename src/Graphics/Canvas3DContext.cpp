#include "Graphics/Canvas3DContext.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SystemInterface.h>

#include "Graphics/OpenGLHeader.h"

namespace Nidium {
    namespace Binding {
         extern JSClass WebGLRenderingContext_class;
         extern JSConstDoubleSpec WebGLRenderingContext_const;
    }
}

namespace Nidium {
namespace Graphics {

#define GL_CALL(X) NATIVE_GL_CALL(m_GLState->getNativeGLContext(), X)
#define GL_CALL_RET(X, RET) NATIVE_GL_CALL_RET(m_GLState->getNativeGLContext(), X, RET)

NativeCanvas3DContext::~NativeCanvas3DContext()
{
    this->cleanUp();
}

NativeCanvas3DContext::NativeCanvas3DContext(NativeCanvasHandler *handler,
    JSContext *cx, int width, int height, Nidium::Interface::NativeUIInterface *ui) :
    NativeCanvasContext(handler), m_Flags(0)
{
    m_Mode = CONTEXT_WEBGL;

    memset(&m_CachedPixels, 0, sizeof(m_CachedPixels));
    memset(&m_GLObjects, 0, sizeof(m_GLObjects));

    m_JsObj = JS_NewObject(cx, &Nidium::Binding::WebGLRenderingContext_class, JS::NullPtr(), JS::NullPtr());
    JS::RootedObject obj(cx, m_JsObj);
    JS_DefineConstDoubles(cx, obj, &Nidium::Binding::WebGLRenderingContext_const);

    m_JsCx  = cx;

    JS_SetPrivate(m_JsObj, this);

    float ratio = Nidium::Interface::NativeSystemInterface::getInstance()->backingStorePixelRatio();

    m_Device.width = width * ratio;
    m_Device.height = height * ratio;

    this->createFBO(m_Device.width, m_Device.height);
}

#if 0
static bool native_Canvas3DContext_constructor(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS_ReportError(cx, "Illegal constructor");
    return false;
}
#endif

void NativeCanvas3DContext::translate(double x, double y)
{
    if (m_CachedPixels.pixels) {
        free(m_CachedPixels.pixels);
        m_CachedPixels.pixels = NULL;
    }

    m_CachedPixels.width = 0;
    m_CachedPixels.height = 0;
}

void NativeCanvas3DContext::setSize(int width, int height, bool redraw)
{
    if (width == m_Device.width && height == m_Device.height) {
        return;
    }

    this->cleanUp();

    float ratio = Nidium::Interface::NativeSystemInterface::getInstance()->backingStorePixelRatio();

    m_Device.width = width * ratio;
    m_Device.height = height * ratio;

    this->createFBO(m_Device.width, m_Device.height);
}

void NativeCanvas3DContext::setScale(double x, double y, double px, double py)
{

}

void NativeCanvas3DContext::clear(uint32_t color)
{
    GL_CALL(ClearColor(0., 0., 0., 0.));
    GL_CALL(Clear(GL_COLOR_BUFFER_BIT));
}

void NativeCanvas3DContext::flush()
{
    GL_CALL(Flush());

    GL_CALL(BindFramebuffer(GR_GL_READ_FRAMEBUFFER, m_GLObjects.fbo_sampled));
    GL_CALL(BindFramebuffer(GR_GL_DRAW_FRAMEBUFFER, m_GLObjects.fbo));

    GL_CALL(BlitFramebuffer(0, 0, m_Device.width, m_Device.height,
        0, 0, m_Device.width, m_Device.height,
        GR_GL_COLOR_BUFFER_BIT, GR_GL_NEAREST));

    GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, m_GLObjects.fbo_sampled));
}

uint32_t NativeCanvas3DContext::getTextureID() const
{
    return m_GLObjects.texture;
}

/* Returns the size in device pixel */
void NativeCanvas3DContext::getSize(int *width, int *height) const
{
    if (width) *width = m_Device.width;
    if (height) *height = m_Device.height;
}

void NativeCanvas3DContext::cleanUp()
{

    if (m_GLObjects.texture) {
        GL_CALL(DeleteTextures(1, &m_GLObjects.texture));
        m_GLObjects.texture = 0;
    }

    if (m_GLObjects.fbo) {
        GL_CALL(DeleteFramebuffers(1, &m_GLObjects.fbo));
        m_GLObjects.fbo = 0;
    }

    if (m_GLObjects.vao) {
        GL_CALL(DeleteVertexArrays(1, &m_GLObjects.vao));
        m_GLObjects.vao = 0;
    }

    if (m_GLObjects.renderbuffer) {
        GL_CALL(DeleteRenderbuffers(1, &m_GLObjects.renderbuffer));
        m_GLObjects.renderbuffer = 0;
    }
    if (m_GLObjects.colorbuffer) {
        GL_CALL(DeleteRenderbuffers(1, &m_GLObjects.colorbuffer));
        m_GLObjects.colorbuffer = 0;
    }

    if (m_GLObjects.fbo_sampled) {
        GL_CALL(DeleteFramebuffers(1, &m_GLObjects.fbo_sampled));
        m_GLObjects.fbo_sampled = 0;
    }

    GL_CALL(BindTexture(GL_TEXTURE_2D, 0));
    GL_CALL(BindFramebuffer(GL_FRAMEBUFFER, 0));
}

uint8_t *NativeCanvas3DContext::getPixels()
{
    this->flush();

    if (m_CachedPixels.pixels &&
        (m_CachedPixels.width != m_Device.width ||
        m_CachedPixels.height != m_Device.height)) {

        free(m_CachedPixels.pixels);
        m_CachedPixels.pixels = NULL;
    }

    if (!m_CachedPixels.pixels) {
        m_CachedPixels.width = m_Device.width;
        m_CachedPixels.height = m_Device.height;

        m_CachedPixels.pixels = (uint8_t *)malloc(m_CachedPixels.width *
            m_CachedPixels.height * 4);
    }

    GL_CALL(ReadPixels(0, 0, m_Device.width, m_Device.height,
        GL_RGBA, GL_UNSIGNED_BYTE, m_CachedPixels.pixels));

    return m_CachedPixels.pixels;
}

bool NativeCanvas3DContext::createFBO(int width, int height)
{
    /*
        Create a WebGL context with passthrough program
    */
    if (!m_GLState) {
        m_GLState = new NativeGLState(Nidium::Interface::__NativeUI, true, true);
        m_GLState->setShared(false);
    }

    GL_CALL(Enable(GR_GL_MULTISAMPLE));

    /*
        Following call are made on the newly created OpenGL Context
    */
    GL_CALL(GenTextures(1, &m_GLObjects.texture));
    GL_CALL(BindTexture(GR_GL_TEXTURE_2D, m_GLObjects.texture));

    GL_CALL(TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_MIN_FILTER, GR_GL_LINEAR));
    GL_CALL(TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_MAG_FILTER, GR_GL_LINEAR));

    /* Allocate memory for the new texture */
    GL_CALL(TexImage2D(
            GR_GL_TEXTURE_2D,
            0,
            GR_GL_RGBA,
            width, height,
            0,
            GR_GL_RGBA,
            GR_GL_UNSIGNED_BYTE,
            NULL));

    GL_CALL(BindTexture(GR_GL_TEXTURE_2D, 0));

    /* Generate the FBO */
    GL_CALL(GenFramebuffers(1, &m_GLObjects.fbo));
    GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, m_GLObjects.fbo));

    /* Set the FBO backing store using the new texture */
    GL_CALL(FramebufferTexture2D(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0,
        GR_GL_TEXTURE_2D, m_GLObjects.texture, 0));

    if (!validateCurrentFBO()) {
        printf("Failed on FBO step 1\n");
        exit(1);
        return false;
    }

    GL_CALL(GenFramebuffers(1, &m_GLObjects.fbo_sampled));
    GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, m_GLObjects.fbo_sampled));

    /* Generate color render buffer */
    GL_CALL(GenRenderbuffers(1, &m_GLObjects.colorbuffer));
    GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, m_GLObjects.colorbuffer));
    GL_CALL(RenderbufferStorageMultisample(GR_GL_RENDERBUFFER, 4, GR_GL_RGBA8, width, height));
    GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0, GR_GL_RENDERBUFFER, m_GLObjects.colorbuffer));

    /* Generate depth render buffer */
    GL_CALL(GenRenderbuffers(1, &m_GLObjects.renderbuffer));
    GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, m_GLObjects.renderbuffer));
    GL_CALL(RenderbufferStorageMultisample(GR_GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height));
    GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_DEPTH_ATTACHMENT, GR_GL_RENDERBUFFER, m_GLObjects.renderbuffer));
    GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_STENCIL_ATTACHMENT, GR_GL_RENDERBUFFER, m_GLObjects.renderbuffer));

    if (!validateCurrentFBO()) {
        printf("Failde on FBO step 2\n");
        exit(1);
        return false;
    }

    GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, 0));
    GL_CALL(ClearColor(0., 0., 0., 0.));

    GL_CALL(Viewport(0, 0, width, height));

    GL_CALL(Enable(GR_GL_VERTEX_PROGRAM_POINT_SIZE));

    /* Vertex Array Buffer are required in GL3.0+ */
    GL_CALL(GenVertexArrays(1, &m_GLObjects.vao));
    GL_CALL(BindVertexArray(m_GLObjects.vao));

    /*
        Cull face is enabled by default on WebGL
    */
    GL_CALL(FrontFace(GR_GL_CCW));
    GL_CALL(Enable(GR_GL_CULL_FACE));
    GL_CALL(Enable(GR_GL_TEXTURE_2D));

#if 0
    GL_CALL(ShadeModel(GL_SMOOTH));
    GL_CALL(DepthFunc(GL_LEQUAL));
    GL_CALL(Hint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST));
    GL_CALL(Hint(GL_POLYGON_SMOOTH_HINT, GL_NICEST));
    GL_CALL(CppObj, Enable(GL_FRAGMENT_PRECISION_HIGH));
    GL_CALL(DepthMask(GL_TRUE));
    GL_CALL(DepthFunc(GL_LEQUAL));
    GL_CALL(DepthRange(0.f, 1.f));
#endif

    GL_CALL(Enable(GR_GL_DEPTH_TEST));
    GL_CALL(ClearDepth(1.0));

    GL_CALL(Clear(GR_GL_COLOR_BUFFER_BIT | GR_GL_DEPTH_BUFFER_BIT));

    return true;
}

} // namespace Graphics
} // namespace Nidium

