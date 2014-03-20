#include "NativeUIInterface.h"
#include <NativeContext.h>
#include <NativeTaskManager.h>
#include <NativePath.h>
#include <unistd.h>

#include <NativeHTTPStream.h>
#include <NativeFileStream.h>
#include <NativePrivateStream.h>

#include <SDL.h>
#define GL_GLEXT_PROTOTYPES
#if __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

NativeUIInterface::NativeUIInterface() :
    m_isOffscreen(false), m_FBO(0), m_FrameBuffer(NULL)
{
    NativePath::registerScheme(SCHEME_DEFINE("file://",    NativeFileStream,    false), true); // default
    NativePath::registerScheme(SCHEME_DEFINE("private://", NativePrivateStream, false));
    NativePath::registerScheme(SCHEME_DEFINE("http://",    NativeHTTPStream,    true));

    NativeTaskManager::createManager();
}

bool NativeUIInterface::makeMainGLCurrent()
{
    return (SDL_GL_MakeCurrent(this->win, m_mainGLCtx) == 0);
}

SDL_GLContext NativeUIInterface::getCurrentGLContext()
{
    return SDL_GL_GetCurrentContext();
}

bool NativeUIInterface::makeGLCurrent(SDL_GLContext ctx)
{
    return (SDL_GL_MakeCurrent(this->win, ctx) == 0);
}

SDL_GLContext NativeUIInterface::createSharedContext()
{
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0 );
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32 );
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    return SDL_GL_CreateContext(this->win);    
}

void NativeUIInterface::deleteGLContext(SDL_GLContext ctx)
{
    SDL_GL_DeleteContext(ctx);
}

void NativeUIInterface::refresh()
{
    int oswap = SDL_GL_GetSwapInterval();
    SDL_GL_SetSwapInterval(0);

    if (this->NativeCtx) {
        this->makeMainGLCurrent();
        this->NativeCtx->frame();
    }

    SDL_GL_SwapWindow(this->win);

    SDL_GL_SetSwapInterval(oswap);
}

void NativeUIInterface::centerWindow()
{
    SDL_SetWindowPosition(this->win, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED);
}

void NativeUIInterface::getScreenSize(int *width, int *height)
{
    SDL_Rect bounds;
    int displayIndex = SDL_GetWindowDisplayIndex(this->win);

    SDL_GetDisplayBounds(displayIndex, &bounds);

    if (width) *width = bounds.w;
    if (height) *height = bounds.h;
}

void NativeUIInterface::setWindowPosition(int x, int y)
{
    SDL_SetWindowPosition(this->win,
        (x == NATIVE_WINDOWPOS_UNDEFINED_MASK) ? SDL_WINDOWPOS_UNDEFINED_MASK : x,
        (y == NATIVE_WINDOWPOS_UNDEFINED_MASK) ? SDL_WINDOWPOS_UNDEFINED_MASK : y);
}

void NativeUIInterface::getWindowPosition(int *x, int *y)
{
    SDL_GetWindowPosition(this->win, x, y);
}

void NativeUIInterface::setWindowSize(int w, int h)
{
    this->width = w;
    this->height = h;

    SDL_SetWindowSize(this->win, w, h);    
}

void NativeUIInterface::setWindowFrame(int x, int y, int w, int h)
{
    if (x == NATIVE_WINDOWPOS_CENTER_MASK) x = SDL_WINDOWPOS_CENTERED;
    if (y == NATIVE_WINDOWPOS_CENTER_MASK) y = SDL_WINDOWPOS_CENTERED;

    this->setWindowSize(w, h);
    this->setWindowPosition(x, y);
}

int NativeUIInterface::useOffScreenRendering(bool val)
{
    if (!val && m_isOffscreen) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        m_isOffscreen = false;
        m_FBO = 0;
        free(m_FrameBuffer);
        m_FrameBuffer = NULL;
        // todo : delete fbo & renderbuffer
        return 0;
    }

    if (val && !m_isOffscreen) {
        GLuint fbo, render_buf;
        glGenFramebuffers(1, &fbo);
        glGenRenderbuffers(1, &render_buf);

        glBindRenderbuffer(GL_RENDERBUFFER, render_buf);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, width, height);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, render_buf);

        m_FBO = fbo;

        m_FrameBuffer = (uint8_t *)malloc(width*height*4);

        SDL_HideWindow(win);

        return fbo;
    }

    return 0;
}
