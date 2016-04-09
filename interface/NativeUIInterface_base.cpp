/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <SDL.h>

#include <NativeHTTPStream.h>
#include <NativePrivateStream.h>

#include "NativeSystemStream.h"
#include "NativeContext.h"
#include "NativeOpenGLHeader.h"

NativeUIInterface::NativeUIInterface() :
    m_CurrentCursor(NativeUIInterface::ARROW), m_NativeCtx(NULL), m_Nml(NULL),
    m_Win(NULL), m_Gnet(NULL), m_Width(0), m_Height(0), m_FilePath(NULL),
    m_Initialized(false), m_IsOffscreen(false), m_ReadPixelInBuffer(false),
    m_Hidden(false), m_FBO(0), m_FrameBuffer(NULL), m_Console(NULL),
    m_MainGLCtx(NULL), m_SystemMenu(this)
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
    if (!m_MainGLCtx) return false;
    return (SDL_GL_MakeCurrent(this->m_Win, m_MainGLCtx) == 0);
}

SDL_GLContext NativeUIInterface::getCurrentGLContext()
{
    return SDL_GL_GetCurrentContext();
}

bool NativeUIInterface::makeGLCurrent(SDL_GLContext ctx)
{
    return (SDL_GL_MakeCurrent(this->m_Win, ctx) == 0);
}

SDL_GLContext NativeUIInterface::createSharedContext(bool webgl)
{
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

    SDL_GLContext created = SDL_GL_CreateContext(this->m_Win);

    return created;
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

    if (this->m_NativeCtx) {
        this->makeMainGLCurrent();
        this->m_NativeCtx->frame();
    }

    SDL_GL_SwapWindow(this->m_Win);

    SDL_GL_SetSwapInterval(oswap);
}

void NativeUIInterface::centerWindow()
{
    SDL_SetWindowPosition(this->m_Win, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED);
}

void NativeUIInterface::getScreenSize(int *width, int *height)
{
    SDL_Rect bounds;
    int displayIndex = SDL_GetWindowDisplayIndex(this->m_Win);

    SDL_GetDisplayBounds(displayIndex, &bounds);

    if (width) *width = bounds.w;
    if (height) *height = bounds.h;
}

void NativeUIInterface::setWindowPosition(int x, int y)
{
    SDL_SetWindowPosition(this->m_Win,
        (x == NATIVE_WINDOWPOS_UNDEFINED_MASK) ? SDL_WINDOWPOS_UNDEFINED_MASK : x,
        (y == NATIVE_WINDOWPOS_UNDEFINED_MASK) ? SDL_WINDOWPOS_UNDEFINED_MASK : y);
}

void NativeUIInterface::getWindowPosition(int *x, int *y)
{
    SDL_GetWindowPosition(this->m_Win, x, y);
}

void NativeUIInterface::setWindowSize(int w, int h)
{
    this->m_Width = w;
    this->m_Height = h;

    SDL_SetWindowSize(this->m_Win, w, h);
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
    if (val && !m_ReadPixelInBuffer) {
        this->initPBOs();
    } else if (!val && m_ReadPixelInBuffer) {

        glDeleteBuffers(NUM_PBOS, m_PBOs.pbo);
        free(m_FrameBuffer);
    }
    m_ReadPixelInBuffer = val;
}

void NativeUIInterface::initPBOs()
{
    if (m_ReadPixelInBuffer) {
        return;
    }

    uint32_t screenPixelSize = m_Width * 2 * m_Height * 2 * 4;

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
    if (!m_ReadPixelInBuffer) {
        this->toggleOfflineBuffer(true);
    }

    //uint32_t screenPixelSize = m_Width * 2 * m_Height * 2 * 4;

    glReadBuffer(GL_COLOR_ATTACHMENT0);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBOs.pbo[m_PBOs.gpu2vram]);

    glReadPixels(0, 0, m_Width*2, m_Height*2, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBOs.pbo[m_PBOs.vram2sys]);
    uint8_t *ret = (uint8_t *)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    if (!ret) {
        uint32_t err = glGetError();
        fprintf(stderr, "Failed to map buffer: Error %d\n", err);
        return NULL;
    }

    /* Flip Y pixels (row by row) */
    for (uint32_t i = 0; i < m_Height * 2; i++) {
        memcpy(m_FrameBuffer + i * m_Width * 2 * 4,
            &ret[(m_Height*2 - i - 1) * m_Width * 2 * 4], m_Width*2*4);
    }

    //memcpy(m_FrameBuffer, ret, screenPixelSize);

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
    if (!val && m_IsOffscreen) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        m_IsOffscreen = false;
        m_FBO = 0;
        free(m_FrameBuffer);
        m_FrameBuffer = NULL;
        // TODO : delete fbo & renderbuffer
        return 0;
    }

    if (val && !m_IsOffscreen) {
        GLuint fbo, render_buf;
        glGenFramebuffers(1, &fbo);
        glGenRenderbuffers(1, &render_buf);

        glBindRenderbuffer(GL_RENDERBUFFER, render_buf);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, m_Width, m_Height);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, render_buf);

        m_FBO = fbo;

        m_FrameBuffer = (uint8_t *)malloc(m_Width*m_Height*4);

        SDL_HideWindow(m_Win);

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
    if (m_NativeCtx && m_NativeCtx->getNJS()) {
        m_NativeCtx->getNJS()->gc();
    }

    this->restartApplication();
}

void NativeUIInterface::hideWindow()
{
    if (!m_Hidden) {
        m_Hidden = true;
        SDL_HideWindow(m_Win);

        set_timer_to_low_resolution(&this->m_Gnet->timersng, 1);
    }
}

void NativeUIInterface::showWindow()
{
    if (m_Hidden) {
        m_Hidden = false;
        SDL_ShowWindow(m_Win);

        set_timer_to_low_resolution(&this->m_Gnet->timersng, 0);
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

