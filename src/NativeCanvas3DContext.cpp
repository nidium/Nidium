#include "NativeCanvas3DContext.h"
#include "NativeJSCanvas.h"

#include "NativeMacros.h"

#include "NativeGLState.h"

#include <NativeSystemInterface.h>

#define GL_GLEXT_PROTOTYPES
#if __APPLE__
#include <OpenGL/gl3.h>
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
}

NativeCanvas3DContext::NativeCanvas3DContext(NativeCanvasHandler *handler,
    JSContext *cx, int width, int height, NativeUIInterface *ui) :
    NativeCanvasContext(handler)
{
    m_Mode = CONTEXT_WEBGL;
    m_GLObjects.fbo = 0;
    m_GLObjects.texture = 0;

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

bool NativeCanvas3DContext::createFBO(int width, int height)
{
    /*
        Create a WebGL context with passthrough program
    */
    m_GLState = new NativeGLState(__NativeUI, true, true);
    m_GLState->setShared(false);

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
            printf("fbo created\n");
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            printf("fbo unsupported\n");
            return false;
        default:
            printf("fbo fatal error\n");
            return false;
    }

    printf("New FBO on %p at %d\n", this, m_GLObjects.fbo);
    this->makeGLCurrent();

    // ??? keep framebuffer bound?
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}