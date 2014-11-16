#ifndef nativeglcontext_h__
#define nativeglcontext_h__

#include <NativeUIInterface.h>
#include "NativeMacros.h"

#include <gl/GrGLInterface.h>

typedef void *SDL_GLContext;

class NativeGLContext 
{
    public:
        NativeGLContext(NativeUIInterface *ui,
            SDL_GLContext wrappedCtx = NULL, bool webgl = false) :
            m_UI(ui), m_Interface(NULL)
        {
            if (wrappedCtx) {
                m_SDLGLCtx = wrappedCtx;
                wrapped = true;

                this->createInterface();

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

            this->createInterface();
            
            /* Restore to the old GL Context */
            if (!m_UI->makeGLCurrent(oldctx)) {
                NLOG("Cant restore old ctx");
            }
        }

        inline SDL_GLContext getGLContext() const {
            return m_SDLGLCtx;
        }

        inline const GrGLInterface *iface() const {
            return m_Interface;
        }

        inline bool makeCurrent() {
            return m_UI->makeGLCurrent(m_SDLGLCtx);
        }

        ~NativeGLContext() {
            if (!wrapped) {
                m_UI->makeMainGLCurrent();
                /*
                    TODO: LEAK :
                    Whenever we try to delete a GL ctx, the screen become black.
                */
                //m_UI->deleteGLContext(m_SDLGLCtx);
            }

            if (m_Interface) {
                m_Interface->unref();
            }
        }

        NativeUIInterface *getUI() const {
            return m_UI;
        }

        static void GLCallback(const GrGLInterface *interface) {
            NativeGLContext *_this = (NativeGLContext *)interface->fCallbackData;
            _this->makeCurrent();
        }
    private:

        void createInterface() {
            this->makeCurrent();

            m_Interface = GrGLCreateNativeInterface();
            ((GrGLInterface *)m_Interface)->fCallback = NativeGLContext::GLCallback;
            ((GrGLInterface *)m_Interface)->fCallbackData = (uintptr_t)this;            
        }

        SDL_GLContext m_SDLGLCtx;
        NativeUIInterface *m_UI;
        bool wrapped;
        const GrGLInterface *m_Interface;
};
#endif