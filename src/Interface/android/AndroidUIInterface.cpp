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

#include "System.h"

namespace Nidium {
namespace Interface {
// {{{ AndroidUIInterface
AndroidUIInterface::AndroidUIInterface()
    : UIInterface(), m_Console(NULL)
{
}

void AndroidUIInterface::setGLContextAttribute()
{
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5 );
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5 );
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5 );
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0 );
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32 );
	//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

}

bool AndroidUIInterface::createWindow(int width, int height)
{
    // Android has a fixed window size, so we ignore the width/height given
    // (that comes from the NML) and set the size to the size of the view
    System *sys = static_cast<System *>(SystemInterface::GetInstance());
    double pr = sys->backingStorePixelRatio();
    return UIInterface::createWindow((sys->getSurfaceWidth()/pr) + 1, (sys->getSurfaceHeight()/pr) + 1);
}

void AndroidUIInterface::quitApplication()
{
    //this->processGtkPendingEvents();

    exit(1);
}

void AndroidUIInterface::hitRefresh()
{
    //this->processGtkPendingEvents();

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
    //APE_timer_create(m_Gnet, 1, ProcessSystemLoop, static_cast<void *>(this));
    APE_loop_run(m_Gnet);
}
// }}}
} // namespace Interface
} // namespace Nidium
