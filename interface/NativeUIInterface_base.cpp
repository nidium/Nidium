#include "NativeUIInterface.h"
#include <SDL.h>

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