#include <GL/gl.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_video.h>
#include <SDL_syswm.h>
#include <pthread.h>

#include "NativeX11UIInterface.h"

int ape_running = 1;

int main(int argc, char **argv)
{
    NativeX11UIInterface UI;
    if (!UI.runApplication("native.npa")) {
        return 0;
    }
    
    UI.runLoop();

    return 0;
}

