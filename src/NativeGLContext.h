#ifndef nativeglcontext_h__
#define nativeglcontext_h__

#include <NativeUIInterface.h>
#include "NativeMacros.h"

typedef void *SDL_GLContext;

class NativeGLContext 
{
    public:
        NativeGLContext(NativeUIInterface *ui,
            SDL_GLContext wrappedCtx = NULL, bool webgl = false) :
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

            m_SDLGLCtx = m_UI->createSharedContext(webgl);
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

        NativeUIInterface *getUI() const {
            return m_UI;
        }
    private:
        SDL_GLContext m_SDLGLCtx;
        NativeUIInterface *m_UI;
        bool wrapped;
};
#endif