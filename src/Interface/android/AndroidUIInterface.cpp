/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "AndroidUIInterface.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <ape_netlib.h>

#include <SDL_config_android.h>
#include <SDL_syswm.h>
#include <SDL_events.h>

#include "System.h"
#include "Frontend/Context.h"
#include "Frontend/InputHandler.h"
#include "Binding/JSWindow.h"
#include "Graphics/CanvasHandler.h"

using Nidium::Binding::JSWindow;
using Nidium::Frontend::InputEvent;
using Nidium::Frontend::InputHandler;

namespace Nidium {
namespace Interface {

extern UIInterface *__NidiumUI;

AndroidUIInterface::AndroidUIInterface()
    : UIInterface(), m_Console(NULL)
{
}

void AndroidUIInterface::setGLContextAttribute()
{
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

}

int AndroidUIInterface::toLogicalSize(int size)
{
    System *sys = static_cast<System *>(SystemInterface::GetInstance());
    double pr = sys->backingStorePixelRatio();
    return ceil(size/pr);
}

bool AndroidUIInterface::createWindow(int width, int height)
{
    // Android has a fixed window size, so we ignore the width/height given
    // (that comes from the NML) and set the size to the size of the view
    System *sys = static_cast<System *>(SystemInterface::GetInstance());
    return UIInterface::createWindow(
        this->toLogicalSize(sys->getSurfaceWidth()),
        this->toLogicalSize(sys->getSurfaceHeight()));
}

void AndroidUIInterface::handleEvent(const SDL_Event *ev)
{
    if (ev->type == SDL_WINDOWEVENT && ev->window.event == SDL_WINDOWEVENT_RESIZED) {
        this->getNidiumContext()->setWindowSize(
            this->toLogicalSize(ev->window.data1),
            this->toLogicalSize(ev->window.data2));
    } else {
        UIInterface::handleEvent(ev);
    }
}

void AndroidUIInterface::quitApplication()
{
    exit(1);
}

void AndroidUIInterface::hitRefresh()
{
    this->restartApplication();
}

void AndroidUIInterface::onWindowCreated()
{
    m_Console = new DummyConsole(this);
}


void AndroidUIInterface::openFileDialog(const char *files[],
                                    void (*cb)(void *nof,
                                               const char *lst[],
                                               uint32_t len),
                                    void *arg,
                                    int flags)
{
    return;
}

void AndroidUIInterface::runLoop()
{
    APE_timer_create(m_Gnet, 1, UIInterface::HandleEvents,
                     static_cast<void *>(this));
    APE_loop_run(m_Gnet);
}

void AndroidUIInterface::onMessage(const Core::SharedMessages::Message &msg)
{
    InputHandler *inputHandler = m_NidiumCtx->getInputHandler();
    AndroidScrollMessage *info = static_cast<AndroidScrollMessage *>(msg.dataPtr());
    System *sys      = static_cast<System *>(SystemInterface::GetInstance());
    float pixelRatio = sys->backingStorePixelRatio();

    switch (msg.event()) {
        case kAndroidMessage_scroll: {
            InputEvent ev(InputEvent::kScroll_type,
                          info->x / pixelRatio,
                          info->y / pixelRatio);

            ev.m_data[0] = info->velocityY;
            ev.m_data[1] = info->velocityY;
            ev.m_data[2] = info->state;
            ev.m_data[3] = 0; // consumed

            inputHandler->pushEvent(ev);

        } break;
        case kAndroidMessage_fling: {
            Graphics::CanvasHandler *handler = inputHandler->getCurrentScrollHandler();

            if (!handler) return;

            int x = info->x / pixelRatio;
            int y = info->y / pixelRatio;

            handler->onScroll(x, y, 0, 0, info->state);
        } break;
    }

    delete info;
}

void AndroidUIInterface::onScroll(float x, float y,
                                  float velocityX, float velocityY,
                                  int state)
{
    AndroidScrollMessage *msg
        = new AndroidScrollMessage(x, y,
                                   velocityX, velocityY,
                                   static_cast<InputEvent::ScrollState>(state));

    this->postMessage(msg, kAndroidMessage_scroll);
}

void AndroidUIInterface::onFlingUpdate(int scrollX, int scrollY, bool finished)
{

    AndroidScrollMessage *msg
        = new AndroidScrollMessage(scrollX, scrollY,
                                   0, 0,
                                   finished
                                        ? InputEvent::kScrollState_end
                                        : InputEvent::kScrollState_move);

    this->postMessage(msg, kAndroidMessage_fling);
}

} // namespace Interface
} // namespace Nidium

#define NIDIUM_ANDROID_UI static_cast<Nidium::Interface::AndroidUIInterface *>(Nidium::Interface::__NidiumUI)
// Called before SDL_main, used to setup System class for Nidium
extern "C" void Java_com_nidium_android_Nidroid_nidiumInit(JNIEnv *env, jobject thiz, jobject nidroid)
{
    Nidium::Interface::SystemInterface::_interface = new Nidium::Interface::System(env, nidroid);
}

extern "C" void Java_com_nidium_android_Nidroid_onScroll(JNIEnv *env, jobject thiz,
                                                         float x, float y,
                                                         float velocityX, float velocityY,
                                                         int state)
{
    NIDIUM_ANDROID_UI->onScroll(x, y, velocityX, velocityY, state);
}

extern "C" void Java_com_nidium_android_Nidroid_onFlingUpdate(JNIEnv *env, jobject thiz,
                                                              int scrollX, int scrollY,
                                                              bool finished)
{
    NIDIUM_ANDROID_UI->onFlingUpdate(scrollX, scrollY, finished);
}
#undef NIDIUM_ANDROID_UI
