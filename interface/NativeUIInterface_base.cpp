#include "NativeUIInterface.h"
#include <NativeContext.h>
#include <NativeTaskManager.h>
#include <NativePath.h>
#include <unistd.h>

#include <NativeHTTPStream.h>
#include <NativeFileStream.h>
#include <NativePrivateStream.h>
#include <NativeNFSStream.h>
#include <NativeSystemStream.h>

#include <SDL.h>
#define GL_GLEXT_PROTOTYPES
#if __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

NativeUIInterface::NativeUIInterface() :
    m_isOffscreen(false), m_FBO(0), m_FrameBuffer(NULL),
    m_readPixelInBuffer(false), m_Hidden(false), m_SystemMenu(this)
{
    NativePath::registerScheme(SCHEME_DEFINE("file://",    NativeFileStream,    false), true); // default
    NativePath::registerScheme(SCHEME_DEFINE("private://", NativePrivateStream, false));
#if 1
    NativePath::registerScheme(SCHEME_DEFINE("system://",  NativeSystemStream,  false));
    NativePath::registerScheme(SCHEME_DEFINE("user://",    NativeUserStream,    false));
#endif
    NativePath::registerScheme(SCHEME_DEFINE("http://",    NativeHTTPStream,    true));
    NativePath::registerScheme(SCHEME_DEFINE("https://",   NativeHTTPStream,    true));
    NativePath::registerScheme(SCHEME_DEFINE("nvfs://",    NativeNFSStream,     false));

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

SDL_GLContext NativeUIInterface::createSharedContext(bool webgl)
{
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

    if (webgl) {
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8 );
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8 );
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8 );
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0 );
        SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32 );
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);        
    } else {
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5 );
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5 );
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5 );
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0 );
        SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32 );
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    }
    return SDL_GL_CreateContext(this->win);    
}

void NativeUIInterface::deleteGLContext(SDL_GLContext ctx)
{
    SDL_GL_DeleteContext(ctx);
}

void NativeUIInterface::quit()
{
    this->stopApplication();
    SDL_Quit();
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

void NativeUIInterface::toggleOfflineBuffer(bool val)
{
    if (val && !m_readPixelInBuffer) {
        this->initPBOs();
    } else if (!val && m_readPixelInBuffer) {

        glDeleteBuffers(NUM_PBOS, m_PBOs.pbo);
        free(m_FrameBuffer);
    }
    m_readPixelInBuffer = val;
}

void NativeUIInterface::initPBOs()
{
    if (m_readPixelInBuffer) {
        return;
    }

    uint32_t screenPixelSize = width * 2 * height * 2 * 4;

    glGenBuffers(NUM_PBOS, m_PBOs.pbo);
    for (int i = 0; i < NUM_PBOS; i++) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBOs.pbo[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, screenPixelSize, 0, GL_DYNAMIC_READ);        
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    m_PBOs.vram2sys = 0;
    m_PBOs.gpu2vram = NUM_PBOS-1;

    m_FrameBuffer = (uint8_t *)malloc(screenPixelSize);
}

uint8_t *NativeUIInterface::readScreenPixel()
{
    if (!m_readPixelInBuffer) {
        this->toggleOfflineBuffer(true);
    }

    uint32_t screenPixelSize = width * 2 * height * 2 * 4;

    glReadBuffer(GL_COLOR_ATTACHMENT0);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBOs.pbo[m_PBOs.gpu2vram]);
    glReadPixels(0, 0, width*2, height*2, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBOs.pbo[m_PBOs.vram2sys]);
    uint8_t *ret = (uint8_t *)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    if (!ret) {
        uint32_t err = glGetError();
        printf("Failed to map buffer\n");
        return NULL;
    }

    memcpy(m_FrameBuffer, ret, screenPixelSize);

    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    int temp = m_PBOs.pbo[0];
    for (int i=1; i<NUM_PBOS; i++)
        m_PBOs.pbo[i-1] = m_PBOs.pbo[i];
    m_PBOs.pbo[NUM_PBOS - 1] = temp;

    return m_FrameBuffer;
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

void NativeUIInterface::refreshApplication(bool clearConsole)
{

    if (clearConsole) {
        NativeUIConsole *console = this->getConsole();
        if (console && !console->hidden()) {
            console->clear();
        }
    }

    /* Trigger GC before refreshing */
    if (NativeCtx && NativeCtx->getNJS()) {
        NativeCtx->getNJS()->gc();
    }

    this->restartApplication();
}

void NativeUIInterface::hideWindow()
{
    if (!m_Hidden) {
        m_Hidden = true;
        SDL_HideWindow(win);

        set_timer_to_low_resolution(&this->gnet->timersng, 1);
    }
}

void NativeUIInterface::showWindow()
{
    if (m_Hidden) {
        m_Hidden = false;
        SDL_ShowWindow(win);

        set_timer_to_low_resolution(&this->gnet->timersng, 0);
    }
}

void NativeSystemMenu::addItem(NativeSystemMenuItem *item)
{
    item->m_Next = m_Items;
    m_Items = item;
}

void NativeSystemMenu::deleteItems()
{
    NativeSystemMenuItem *tmp = NULL, *cur = m_Items;
    while (cur != NULL) {
        tmp = cur->m_Next;
        delete cur;
        cur = tmp;
    }

    m_Items = NULL;
}

void NativeSystemMenu::setIcon(const uint8_t *data, size_t width, size_t height)
{
    m_Icon.data = data;
    m_Icon.len = width * height * 4;
    m_Icon.width = width;
    m_Icon.height = height;
}

NativeSystemMenu::NativeSystemMenu(NativeUIInterface *ui) : m_UI(ui)
{
    m_Items = NULL;
    m_Icon.data = NULL;
    m_Icon.len = 0;
}

NativeSystemMenu::~NativeSystemMenu()
{
    this->deleteItems();
}
