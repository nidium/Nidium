#include <GL/gl.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_video.h>
#include <SDL_syswm.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

#include "NativeX11UIInterface.h"
#include "NativeSystem.h"

NativeSystemInterface *NativeSystemInterface::_interface = new NativeSystem();

int ape_running = 1;
int _nativebuild = 1002;
unsigned long _ape_seed;

int main(int argc, char **argv)
{
    NativeX11UIInterface UI;
    _ape_seed = time(NULL) ^ (getpid() << 16);

    if (!UI.runApplication("native.npa")) {
        return 0;
    }
    
    UI.runLoop();

    return 0;
}

