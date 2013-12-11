#ifndef nativeglcontext_h__
#define nativeglcontext_h__

#include <NativeUIInterface.h>
#include "NativeMacros.h"

typedef void *SDL_GLContext;

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

            SDL_GLContext oldctx = m_UI->getCurrentGLContext();

            /* The new context share with the "main" GL context */
            if (!m_UI->makeMainGLCurrent()) {
                NLOG("Cant make main current");
            }

            m_SDLGLCtx = m_UI->createSharedContext();
            if (m_SDLGLCtx == NULL) {
                NLOG("Cant create context");
            }
            
            /* Restore to the old GL Context */
            if (!m_UI->makeGLCurrent(oldctx)) {
                NLOG("Cant restore old ctx");
            }
            NLOG("New context created");
        }

        bool makeCurrent() {
            return m_UI->makeGLCurrent(m_SDLGLCtx);
        }

        ~NativeGLContext() {
            if (!wrapped) {
                m_UI->deleteGLContext(m_SDLGLCtx);
            }
        }
    private:
        SDL_GLContext m_SDLGLCtx;
        NativeUIInterface *m_UI;
        bool wrapped;
};
#endif