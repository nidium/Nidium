/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "IOSUIInterface.h"
#include "System.h"
#include "Frontend/Context.h"

#include <SDL_config.h>
#include <ape_netlib.h>
#include <SDL.h>
#include <SDL_syswm.h>

#include "Graphics/GLHeader.h"
#include "Binding/JSWindow.h"

using Nidium::Binding::JSWindow;

namespace Nidium {
namespace Interface {

extern UIInterface *__NidiumUI;

IOSUIInterface::IOSUIInterface()
    : UIInterface(), m_Console(NULL)
{
}

void IOSUIInterface::setGLContextAttribute()
{
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Finally, if your application completely redraws the screen each frame,
    // you may find significant performance improvement by setting the attribute SDL_GL_RETAINED_BACKING to 0.
    SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    fprintf(stderr, "iOS set gl context\n");
}

int IOSUIInterface::toLogicalSize(int size)
{
    System *sys = static_cast<System *>(SystemInterface::GetInstance());
    double pr = sys->backingStorePixelRatio();
    return ceil(size/pr);
}

bool IOSUIInterface::createWindow(int width, int height)
{
    // iOS/tvOS has a fixed window size, so we ignore the width/height given
    // (that comes from the NML) and set the size to the size of the view
    // FIXME : Find out how to get real values
    System *sys = static_cast<System *>(SystemInterface::GetInstance());
    return UIInterface::createWindow(1920, 1080);
}

void IOSUIInterface::handleEvent(const SDL_Event *ev)
{
    switch (ev->type) {
        // When the device is rotated, notify nidium of the screen resize
        case SDL_WINDOWEVENT:
            if (ev->window.event == SDL_WINDOWEVENT_RESIZED) {
                this->getNidiumContext()->setWindowSize(
                        this->toLogicalSize(ev->window.data1),
                        this->toLogicalSize(ev->window.data2));
            }
            break;
#ifdef NDM_TARGET_TVOS
        // Forward apple TV remote controller click (SDL_SCANCODE_SELECT) as mouse click
        case SDL_KEYUP:
        case SDL_KEYDOWN: {
            JSWindow *window = NULL;
            if (!this->isContextReady()
                    || !(window = JSWindow::GetObject(m_NidiumCtx->getNJS()))) {
                UIInterface::handleEvent(ev);
            }

            switch(ev->key.keysym.scancode) {
                case SDL_SCANCODE_SELECT:
                    window->mouseClick(ev->motion.x, ev->motion.y,
                            ev->type == SDL_KEYDOWN ? true : false, 1 /* left click */,
                            ev->button.clicks);
                    return;
            }
            break;
        }
        // SDL fakes apple TV remote controller touch event as
        // mouse button down/up. Ignore them.
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            return;
#endif
    }

    UIInterface::handleEvent(ev);
}

void IOSUIInterface::quitApplication()
{
    exit(1);
}

void IOSUIInterface::hitRefresh()
{
    this->restartApplication();
}

void IOSUIInterface::bindFramebuffer()
{
    UIInterface::bindFramebuffer();
    glBindRenderbuffer(GL_RENDERBUFFER, m_FBO);
}

void IOSUIInterface::onWindowCreated()
{
    /*
        iOS/tvos doesnt use window-system framebuffer.
        SDL already generate a fbo and renderbuffer for us (which is not 0)
    */

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);

    SDL_GetWindowWMInfo(m_Win, &info);

    m_FBO = info.info.uikit.framebuffer;
    m_Console = new DummyConsole(this);
}

void IOSUIInterface::openFileDialog(const char *files[],
                                    void (*cb)(void *nof,
                                    const char *lst[],
                                    uint32_t len),
                                    void *arg,
                                    int flags)
{
    return;
}

void IOSUIInterface::runLoop()
{
    APE_timer_create(m_Gnet, 1, UIInterface::HandleEvents,
                     static_cast<void *>(this));
    APE_loop_run(m_Gnet);
}
} // namespace Interface
} // namespace Nidium
