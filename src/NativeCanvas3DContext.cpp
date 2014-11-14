#include "NativeCanvas3DContext.h"
#include "NativeJSCanvas.h"

#include "NativeMacros.h"

#include "NativeGLState.h"

#include <NativeSystemInterface.h>

#define GL_GLEXT_PROTOTYPES
#if __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#define CANVASCTX_GETTER(obj) ((class NativeCanvas3DContext *)JS_GetPrivate(obj))

void Canvas3DContext_finalize(JSFreeOp *fop, JSObject *obj);

extern JSClass WebGLRenderingContext_class;
extern JSConstDoubleSpec WebGLRenderingContext_const;


void Canvas3DContext_finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeCanvas3DContext *canvasctx = CANVASCTX_GETTER(obj);
    if (canvasctx != NULL) {
        delete canvasctx;
    }
}

NativeCanvas3DContext::~NativeCanvas3DContext()
{
    this->cleanUp();
}

NativeCanvas3DContext::NativeCanvas3DContext(NativeCanvasHandler *handler,
    JSContext *cx, int width, int height, NativeUIInterface *ui) :
    NativeCanvasContext(handler)
{
    m_Mode = CONTEXT_WEBGL;

    memset(&m_GLObjects, 0, sizeof(m_GLObjects));

    jsobj = JS_NewObject(cx, &WebGLRenderingContext_class, NULL, NULL);

    JS_DefineConstDoubles(cx, jsobj, &WebGLRenderingContext_const);

    jscx  = cx;

    JS_SetPrivate(jsobj, this);

    float ratio = NativeSystemInterface::getInstance()->backingStorePixelRatio();

    m_Device.width = width * ratio;
    m_Device.height = height * ratio;

    this->createFBO(m_Device.width, m_Device.height);
}

static JSBool native_Canvas3DContext_constructor(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JS_ReportError(cx, "Illegal constructor");
    return JS_FALSE;
}


void NativeCanvas3DContext::translate(double x, double y)
{

}

void NativeCanvas3DContext::setSize(int width, int height, bool redraw)
{
    if (width == m_Device.width && height == m_Device.height) {
        return;
    }

    this->cleanUp();

    float ratio = NativeSystemInterface::getInstance()->backingStorePixelRatio();

    m_Device.width = width * ratio;
    m_Device.height = height * ratio;

    this->createFBO(m_Device.width, m_Device.height);
}

void NativeCanvas3DContext::setScale(double x, double y, double px, double py)
{

}

void NativeCanvas3DContext::clear(uint32_t color)
{
    m_GLState->makeGLCurrent();

    glClearColor(0., 0., 0., 0.);
    glClear(GL_COLOR_BUFFER_BIT);
}

void NativeCanvas3DContext::flush()
{
    m_GLState->makeGLCurrent();
    glFlush();
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
    printf("Clearup on %p\n", this);
    m_GLState->makeGLCurrent();

    if (m_GLObjects.texture) {
        glDeleteTextures(1, &m_GLObjects.texture);
        m_GLObjects.texture = 0;
    }

    if (m_GLObjects.fbo) {
        glDeleteFramebuffers(1, &m_GLObjects.fbo);
        m_GLObjects.fbo = 0;
    }

    if (m_GLObjects.vao) {
        glDeleteVertexArraysAPPLE(1, &m_GLObjects.vao);
        m_GLObjects.vao = 0;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool NativeCanvas3DContext::createFBO(int width, int height)
{
    /*
        Create a WebGL context with passthrough program
    */

    if (!m_GLState) {
        m_GLState = new NativeGLState(__NativeUI, true, true);
        m_GLState->setShared(false);
    }

    /*
        Following call are made on the newly created OpenGL Context
    */

    m_GLState->makeGLCurrent();

    glGenTextures(1, &m_GLObjects.texture);
    glBindTexture(GL_TEXTURE_2D, m_GLObjects.texture);

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
    glGenFramebuffers(1, &m_GLObjects.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_GLObjects.fbo);

    /* Set the FBO backing store using the new texture */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, m_GLObjects.texture, 0);

    GLenum status;
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    switch(status) {
        case GL_FRAMEBUFFER_COMPLETE:
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            printf("fbo unsupported\n");
            return false;
        default:
            printf("fbo fatal error\n");
            return false;
    }

    glClearColor(0., 0., 0., 0.);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, width, height);

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#ifdef GL_POINT_SPRITE
    glEnable(GL_POINT_SPRITE);
#endif

    /* Vertex Array Buffer are required in GL3.0+ */
    uint32_t vao;
    glGenVertexArraysAPPLE(1, &m_GLObjects.vao);
    glBindVertexArrayAPPLE(m_GLObjects.vao);

    /*
        Cull face is enabled by default on WebGL
    */
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    /*
        We keep the newly created framebuffer bound.
    */

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_MULTISAMPLE);

    //glShadeModel(GL_SMOOTH);
    //glDepthFunc(GL_LEQUAL);
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    //GL_CALL(CppObj, Enable(GL_FRAGMENT_PRECISION_HIGH));

    return true;
}