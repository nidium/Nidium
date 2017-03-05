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

#include "Core/TaskManager.h"
#include "Net/HTTPStream.h"
#include "SystemInterface.h"
#include "Frontend/Context.h"
#include "Frontend/InputHandler.h"
#include "Graphics/GLHeader.h"
#include "Binding/JSWindow.h"
#include "SDL_keycode_translate.h"
#include "Macros.h"

#define NIDIUM_TITLEBAR_HEIGHT 0
#define NIDIUM_VSYNC 1

uint32_t ttfps = 0;

using Nidium::Core::Path;
using Nidium::Core::TaskManager;
using Nidium::Net::HTTPStream;
using Nidium::Binding::JSWindow;
using Nidium::Frontend::Context;
using Nidium::Frontend::NML;
using Nidium::Frontend::InputEvent;

namespace Nidium {
namespace Interface {

// {{{ UIInterface
UIInterface::UIInterface()
    : m_CurrentCursor(UIInterface::ARROW), m_NidiumCtx(NULL), m_Nml(NULL),
      m_Win(NULL), m_Gnet(APE_init()), m_Width(0), m_Height(0),
      m_FilePath(NULL), m_Initialized(false), m_IsOffscreen(false),
      m_ReadPixelInBuffer(false), m_Hidden(false), m_FBO(0),
      m_FrameBuffer(NULL), m_Console(NULL), m_MainGLCtx(NULL),
      m_SystemMenu(this)
{
}

void UIInterface::setGLContextAttribute()
{
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); 
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
}

bool UIInterface::createWindow(int width, int height)
{
    if (!m_Initialized) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == -1) {
            NUI_LOG("Can't init SDL:  %s\n", SDL_GetError());
            return false;
        }

        this->setGLContextAttribute();

        m_Win = SDL_CreateWindow(
            "nidium", 100, 100, width, height,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL /* | SDL_WINDOW_FULLSCREEN*/);

        if (m_Win == NULL) {
            NUI_LOG("Cant create window (SDL)\n");
            return false;
        }

        this->setupWindow();

        m_Width  = width;
        m_Height = height;

        if ((m_MainGLCtx = SDL_GL_CreateContext(m_Win)) == NULL) {
            NUI_LOG("Failed to create OpenGL context : %s", SDL_GetError());
            return false;
        }

        this->initControls();
        SDL_StartTextInput();

        /*
            Enable vertical sync
        */
        if (SDL_GL_SetSwapInterval(NIDIUM_VSYNC) == -1) {
            fprintf(stderr, "Cant vsync\n");
        }

        // glViewport(0, 0, width*2, height*2);
        NUI_LOG("[DEBUG] OpenGL %s", glGetString(GL_VERSION));

        this->onWindowCreated();

        this->m_Initialized = true;
    }

    this->setWindowFrame(NIDIUM_WINDOWPOS_UNDEFINED_MASK,
                         NIDIUM_WINDOWPOS_UNDEFINED_MASK, width, height);

    /*
        This will create root canvas, initial size and so on
    */
    m_NidiumCtx->setUIObject(this);

    return true;
}

void UIInterface::handleEvent(const SDL_Event *event)
{
    JSWindow *window = NULL;
    if (this->isContextReady()) {
        this->makeMainGLCurrent();
        window = JSWindow::GetObject(this->m_NidiumCtx->getNJS());
    }

    switch (event->type) {
        case SDL_WINDOWEVENT:
            if (window) {
                switch (event->window.event) {
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        window->windowFocus();
                        break;
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        window->windowBlur();
                        break;
                    default:
                        break;
                }
            }
            break;
        case SDL_FINGERMOTION:
        case SDL_FINGERDOWN:
        case SDL_FINGERUP: {
            int width, height;
            this->getScreenSize(&width, &height);

            int x = event->tfinger.x * width;
            int y = event->tfinger.y * height;

            InputEvent::Type eventType = InputEvent::kTouchMove_Type;
            if (event->type != SDL_FINGERMOTION) {
                eventType = event->type == SDL_FINGERUP
                                ? InputEvent::kTouchEnd_Type
                                : InputEvent::kTouchStart_Type;
            }

            InputEvent *ev = new InputEvent(eventType, x, y);
            m_NidiumCtx->getInputHandler()->pushEvent(ev);
        } break;
        case SDL_TEXTINPUT:
            if (window && &event->text.text[0]
                && strlen(event->text.text) > 0) {
                window->textInput(event->text.text);
            }
            break;
        case SDL_USEREVENT:
            break;
        case SDL_QUIT:
            if (window && !window->onClose()) {
                break;
            }
            this->stopApplication();
            SDL_Quit();
            this->quitApplication();

            break;
        case SDL_MOUSEMOTION:
            if (window) {
                window->mouseMove(event->motion.x,
                                  event->motion.y - NIDIUM_TITLEBAR_HEIGHT,
                                  event->motion.xrel, event->motion.yrel);
            }
            break;
        case SDL_MOUSEWHEEL: {
            int cx, cy;
            SDL_GetMouseState(&cx, &cy);
            if (window) {
                window->mouseWheel(event->wheel.x, event->wheel.y, cx,
                                   cy - NIDIUM_TITLEBAR_HEIGHT);
            }
            break;
        }
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            if (window) {
                window->mouseClick(event->button.x,
                                   event->button.y - NIDIUM_TITLEBAR_HEIGHT,
                                   event->button.state, event->button.button,
                                   event->button.clicks);
            }
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            int keyCode = 0;
            int mod     = 0;

            if ((&event->key)->keysym.sym == SDLK_r
                && (event->key.keysym.mod & KMOD_CTRL)
                && event->type == SDL_KEYDOWN) {

                if (m_PendingRefresh) {
                    break;
                }

                m_PendingRefresh = true;

                this->hitRefresh();

                break;
            }
            if (event->key.keysym.sym >= 97 && event->key.keysym.sym <= 122) {
                keyCode = event->key.keysym.sym - 32;
            } else {
                keyCode = SDL_KEYCODE_TO_DOMCODE(event->key.keysym.sym);
            }

            if (event->key.keysym.mod & KMOD_SHIFT
                || SDL_KEYCODE_GET_CODE(keyCode) == 16) {
                mod |= kKeyModifier_Shift;
            }
            if (event->key.keysym.mod & KMOD_ALT
                || SDL_KEYCODE_GET_CODE(keyCode) == 18) {
                mod |= kKeyModifier_Alt;
            }
            if (event->key.keysym.mod & KMOD_CTRL
                || SDL_KEYCODE_GET_CODE(keyCode) == 17) {
                mod |= kKeyModifier_Control;
            }
            if (event->key.keysym.mod & KMOD_GUI
                || SDL_KEYCODE_GET_CODE(keyCode) == 91) {
                mod |= kKeyModifier_Meta;
            }
            if (window) {
                window->keyupdown(SDL_KEYCODE_GET_CODE(keyCode), mod,
                                  event->key.state, event->key.repeat,
                                  SDL_KEYCODE_GET_LOCATION(keyCode));
            }

            break;
        }
    }
}

int UIInterface::HandleEvents(void *arg)
{
    UIInterface *uii = static_cast<UIInterface *>(arg);

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
		uii->handleEvent(&event);
    }

    if (ttfps % 300 == 0 && uii->isContextReady()) {
        uii->m_NidiumCtx->getNJS()->gc();
    }

    if (uii->m_CursorNeedsUpdate) {
        uii->setSystemCursor(uii->m_CurrentCursor);
        uii->m_CursorNeedsUpdate = false;
    }

    if (uii->isContextReady()) {
        uii->makeMainGLCurrent();
        uii->m_NidiumCtx->frame(true);
    }

    if (uii->getConsole()) {
        uii->getConsole()->flush();
    }

    if (uii->getFBO() != 0 && uii->m_NidiumCtx) {
#ifndef NIDIUM_OPENGLES2
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        glReadPixels(0, 0, uii->getWidth(), uii->getHeight(), GL_RGBA,
                     GL_UNSIGNED_BYTE, uii->getFrameBufferData());
        uint8_t *pdata = uii->getFrameBufferData();

        uii->m_NidiumCtx->rendered(pdata, uii->getWidth(), uii->getHeight());
#endif
    } else {
        uii->makeMainGLCurrent();
        SDL_GL_SwapWindow(uii->m_Win);
    }

    ttfps++;
    return 16;
}

bool UIInterface::isContextReady()
{
    return (this->m_NidiumCtx && m_NidiumCtx->getUI());
}

void UIInterface::OnNMLLoaded(void *arg)
{
    UIInterface *UI = static_cast<UIInterface *>(arg);
    UI->onNMLLoaded();
}

void UIInterface::onNMLLoaded()
{
    if (!this->createWindow(this->m_Nml->getMetaWidth(),
                            this->m_Nml->getMetaHeight()
                                + NIDIUM_TITLEBAR_HEIGHT)) {
        exit(2);
    }

    this->setWindowTitle(this->m_Nml->getMetaTitle());
}

bool UIInterface::makeMainGLCurrent()
{
    if (!m_MainGLCtx) return false;
    return (SDL_GL_MakeCurrent(this->m_Win, m_MainGLCtx) == 0);
}

SDL_GLContext UIInterface::getCurrentGLContext()
{
    return SDL_GL_GetCurrentContext();
}

bool UIInterface::makeGLCurrent(SDL_GLContext ctx)
{
    return (SDL_GL_MakeCurrent(this->m_Win, ctx) == 0);
}

SDL_GLContext UIInterface::createSharedContext(bool webgl)
{
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

    SDL_GLContext created = SDL_GL_CreateContext(this->m_Win);

    return created;
}

void UIInterface::setWindowTitle(const char *name)
{
    SDL_SetWindowTitle(m_Win,
                       (name == NULL || *name == '\0' ? "nidium" : name));
}

const char *UIInterface::getWindowTitle() const
{
    return SDL_GetWindowTitle(m_Win);
}

void UIInterface::setClipboardText(const char *text)
{
    SDL_SetClipboardText(text);
}

char *UIInterface::getClipboardText()
{
    return SDL_GetClipboardText();
}


void UIInterface::setCursor(CURSOR_TYPE type)
{
    if (m_CurrentCursor != type) {
        m_CursorNeedsUpdate = true;
        m_CurrentCursor = type;
        printf("set new cursor %d\n", type);
    }
}


void UIInterface::deleteGLContext(SDL_GLContext ctx)
{
    SDL_GL_DeleteContext(ctx);
}

void UIInterface::quit()
{
    this->stopApplication();
    SDL_Quit();
}

void UIInterface::refresh()
{
    int oswap = SDL_GL_GetSwapInterval();
    SDL_GL_SetSwapInterval(0);

    if (this->m_NidiumCtx) {
        this->makeMainGLCurrent();
        this->m_NidiumCtx->frame();
    }

    SDL_GL_SwapWindow(this->m_Win);

    SDL_GL_SetSwapInterval(oswap);
}

void UIInterface::centerWindow()
{
    SDL_SetWindowPosition(this->m_Win, SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED);
}

void UIInterface::getScreenSize(int *width, int *height)
{
    SDL_Rect bounds;
    int displayIndex = SDL_GetWindowDisplayIndex(this->m_Win);

    SDL_GetDisplayBounds(displayIndex, &bounds);

    if (width) *width = bounds.w;
    if (height) *height = bounds.h;
}

void UIInterface::setWindowPosition(int x, int y)
{
    SDL_SetWindowPosition(this->m_Win, (x == NIDIUM_WINDOWPOS_UNDEFINED_MASK)
                                           ? SDL_WINDOWPOS_UNDEFINED_MASK
                                           : x,
                          (y == NIDIUM_WINDOWPOS_UNDEFINED_MASK)
                              ? SDL_WINDOWPOS_UNDEFINED_MASK
                              : y);
}

void UIInterface::getWindowPosition(int *x, int *y)
{
    SDL_GetWindowPosition(this->m_Win, x, y);
}

void UIInterface::setWindowSize(int w, int h)
{
    this->m_Width  = w;
    this->m_Height = h;

    SDL_SetWindowSize(this->m_Win, w, h);
}

void UIInterface::setWindowFrame(int x, int y, int w, int h)
{
    if (x == NIDIUM_WINDOWPOS_CENTER_MASK) x = SDL_WINDOWPOS_CENTERED;
    if (y == NIDIUM_WINDOWPOS_CENTER_MASK) y = SDL_WINDOWPOS_CENTERED;

    this->setWindowSize(w, h);
    this->setWindowPosition(x, y);
}

void UIInterface::toggleOfflineBuffer(bool val)
{
#ifndef NIDIUM_OPENGLES
    if (val && !m_ReadPixelInBuffer) {
        this->initPBOs();
    } else if (!val && m_ReadPixelInBuffer) {

        glDeleteBuffers(NUM_PBOS, m_PBOs.pbo);
        free(m_FrameBuffer);
    }
    m_ReadPixelInBuffer = val;
#endif
}

void UIInterface::initPBOs()
{
#ifndef NIDIUM_OPENGLES2
    if (m_ReadPixelInBuffer) {
        return;
    }

    float pixelRatio = SystemInterface::GetInstance()->backingStorePixelRatio();
    uint32_t screenPixelSize = m_Width * pixelRatio * m_Height * pixelRatio * 4;

    glGenBuffers(NUM_PBOS, m_PBOs.pbo);
    for (int i = 0; i < NUM_PBOS; i++) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBOs.pbo[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, screenPixelSize, 0, GL_DYNAMIC_READ);
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    m_PBOs.vram2sys = 0;
    m_PBOs.gpu2vram = NUM_PBOS - 1;

    m_FrameBuffer = static_cast<uint8_t *>(malloc(screenPixelSize));
#endif
}

uint8_t *UIInterface::readScreenPixel()
{
#ifndef NIDIUM_OPENGLES2
    if (!m_ReadPixelInBuffer) {
        this->toggleOfflineBuffer(true);
    }

    float pixelRatio = SystemInterface::GetInstance()->backingStorePixelRatio();
    int width = m_Width * pixelRatio;
    int height = m_Height * pixelRatio;

    glReadBuffer(GL_COLOR_ATTACHMENT0);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBOs.pbo[m_PBOs.gpu2vram]);

    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_PBOs.pbo[m_PBOs.vram2sys]);
    uint8_t *ret = (uint8_t *)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    if (!ret) {
        uint32_t err = glGetError();
        fprintf(stderr, "Failed to map buffer: Error %d\n", err);
        return NULL;
    }

    /* Flip Y pixels (row by row) */
    for (uint32_t i = 0; i < height; i++) {
        memcpy(m_FrameBuffer + i * width * 4,
                &ret[(height - i - 1) * width * 4], 
                width * 4);
    }

    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    int temp = m_PBOs.pbo[0];
    for (int i = 1; i < NUM_PBOS; i++)
        m_PBOs.pbo[i - 1]    = m_PBOs.pbo[i];
    m_PBOs.pbo[NUM_PBOS - 1] = temp;

    return m_FrameBuffer;
#else
    return nullptr;
#endif
}

int UIInterface::useOffScreenRendering(bool val)
{
#ifndef NIDIUM_OPENGLES2
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
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, render_buf);

        m_FBO = fbo;

        m_FrameBuffer = (uint8_t *)malloc(m_Width * m_Height * 4);

        SDL_HideWindow(m_Win);

        return fbo;
    }

    return 0;
#else
    return 1;
#endif
}

void UIInterface::refreshApplication(bool clearConsole)
{

    if (clearConsole) {
        UIConsole *console = this->getConsole();
        if (console && !console->hidden()) {
            console->clear();
        }
    }

    /* Trigger GC before refreshing */
    if (m_NidiumCtx && m_NidiumCtx->getNJS()) {
        m_NidiumCtx->getNJS()->gc();
    }

    this->restartApplication();
}

bool UIInterface::runApplication(const char *path)
{
    if (path != this->m_FilePath) {
        if (this->m_FilePath) {
            free(this->m_FilePath);
        }
        this->m_FilePath = strdup(path);
    }
    if (path == NULL || strlen(path) < 5) {
        return false;
    }
    //    FILE *main = fopen("index.nml", "r");
    //    const char *ext = &path[strlen(path)-4];

    m_NidiumCtx = new Context(this->m_Gnet);
    m_Nml = new NML(this->m_Gnet);
    m_Nml->setNJS(m_NidiumCtx->getNJS());

    m_Nml->loadFile(path, UIInterface::OnNMLLoaded, this);

    return true;
}

void UIInterface::stopApplication()
{
    if (this->m_Nml) {
        delete this->m_Nml;
        this->m_Nml = NULL;
    }
    if (this->m_NidiumCtx) {
        delete this->m_NidiumCtx;
        this->m_NidiumCtx = NULL;
    }

    m_PendingRefresh = false;

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    /* Also clear the front buffer */
    SDL_GL_SwapWindow(this->m_Win);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void UIInterface::restartApplication(const char *path)
{
    this->stopApplication();
    this->runApplication(path == NULL ? this->m_FilePath : path);
}

void UIInterface::hideWindow()
{
    if (!m_Hidden) {
        m_Hidden = true;
        SDL_HideWindow(m_Win);

        APE_timer_setlowresolution(this->m_Gnet, 1);
    }
}

void UIInterface::showWindow()
{
    if (m_Hidden) {
        m_Hidden = false;
        SDL_ShowWindow(m_Win);

        APE_timer_setlowresolution(this->m_Gnet, 0);
    }
}

void UIInterface::hideCursor(bool state)
{
    SDL_ShowCursor(!state);
}

// }}}

// {{{ SystemMenu
void SystemMenu::addItem(SystemMenuItem *item)
{
    item->m_Next = m_Items;
    m_Items      = item;
}

void SystemMenu::deleteItems()
{
    SystemMenuItem *tmp = NULL, *cur = m_Items;
    while (cur != NULL) {
        tmp = cur->m_Next;
        delete cur;
        cur = tmp;
    }

    m_Items = NULL;
}

void SystemMenu::setIcon(const uint8_t *data, size_t width, size_t height)
{
    m_Icon.data   = data;
    m_Icon.len    = width * height * 4;
    m_Icon.width  = width;
    m_Icon.height = height;
}

SystemMenu::SystemMenu(UIInterface *ui) : m_UI(ui)
{
    m_Items     = NULL;
    m_Icon.data = NULL;
    m_Icon.len  = 0;
}

SystemMenu::~SystemMenu()
{
    this->deleteItems();
}
// }}}

} // namespace Interface
} // namespace Nidium
