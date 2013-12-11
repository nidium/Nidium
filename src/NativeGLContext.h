#ifndef nativeglcontext_h__
#define nativeglcontext_h__

#include <NativeUIInterface.h>
#include "NativeMacros.h"
#include <SDL_Video.h>

/*
    Make the context pointed by IFANCE current and make a GL call
    e.g. NATIVE_GL_CALL(this->context, Clear(0, 0, 0, 0));
*/
#define NATIVE_GL_CALL(IFACE, X)                                \
    do {                                                        \
        (IFACE)->makeCurrent();                                 \
        gl##X;                                                  \
    } while (false)

class NativeGLContext 
{
    public:
        NativeGLContext(NativeUIInterface *ui,
            SDL_GLContext wrappedCtx = NULL) :
            m_UI(ui)
        {
            if (wrappedCtx) {
                m_SDLGLCtx = wrappedCtx;
                wrapped = true;
                return;
            }
            wrapped = false;

            SDL_GLContext oldctx = SDL_GL_GetCurrentContext();

            /* The new context share with the "main" GL context */
            if (!m_UI->makeMainGLCurrent()) {
                NLOG("Cant make main current");
            }
            SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5 );
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5 );
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5 );
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0 );
            SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32 );
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
            m_SDLGLCtx = SDL_GL_CreateContext(m_UI->win);
            if (m_SDLGLCtx == NULL) {
                NLOG("Cant create context");
            }
            
            /* Restore to the old GL Context */
            if (SDL_GL_MakeCurrent(m_UI->win, oldctx) != 0) {
                NLOG("Cant restore old ctx");
            }
            NLOG("New context created");
        }

        bool makeCurrent() {
            return (SDL_GL_MakeCurrent(m_UI->win, m_SDLGLCtx) == 0);
        }

        ~NativeGLContext() {
            if (!wrapped) {
                SDL_GL_DeleteContext(m_SDLGLCtx);
            }
        }
    private:
        SDL_GLContext m_SDLGLCtx;
        NativeUIInterface *m_UI;
        bool wrapped;
};
#endif