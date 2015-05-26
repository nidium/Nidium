#ifndef nativeglcontext_h__
#define nativeglcontext_h__

#include <NativeUIInterface.h>
#include "NativeMacros.h"

#include <gl/GrGLInterface.h>
#include <gl/GrGLDefines.h>

#include <NativeContext.h>

typedef void *SDL_GLContext;


/*
    Make the context pointed by IFACE current and make a GL call
    e.g. NATIVE_GL_CALL(this->context, Clear(0, 0, 0, 0));
*/

#define NATIVE_GL_MAIN_IFACE (__NativeUI->getNativeContext()->getGLState()->getNativeGLContext())

#ifndef NATIVE_ENABLE_GL_ERROR

    #define NATIVE_GL_CALL(IFACE, X)                         \
        do {                                                 \
            NativeGLContext::GLCallback(IFACE->m_Interface);  \
            (IFACE)->m_Interface->fFunctions.f##X;            \
        } while (false)

    #define NATIVE_GL_CALL_RET(IFACE, X, RET)   \
        do {                                    \
            NativeGLContext::GLCallback(IFACE->m_Interface);  \
            (RET) =  (IFACE)->m_Interface->fFunctions.f##X;   \
        } while (false)

#else
    #define NATIVE_GL_CALL(IFACE, X)                         \
        do {                                                 \
            uint32_t __err;                                  \
            NativeGLContext::GLCallback(IFACE->m_Interface); \
            (IFACE)->m_Interface->fFunctions.f##X;           \
            if ((__err = (IFACE)->m_Interface->fFunctions.fGetError()) != GR_GL_NO_ERROR) { \
                NLOG("[Nidium GL Error : gl%s() returned %d", #X, __err);    \
            } \
        } while (false)

    #define NATIVE_GL_CALL_RET(IFACE, X, RET)   \
        do {                                    \
            uint32_t __err; \
            NativeGLContext::GLCallback(IFACE->m_Interface);  \
            (RET) =  (IFACE)->m_Interface->fFunctions.f##X;   \
            if ((__err = (IFACE)->m_Interface->fFunctions.fGetError()) != GR_GL_NO_ERROR) { \
                NLOG("[Nidium GL Error : gl%s() returned %d", #X, __err);    \
            } \
        } while (false)
#endif

#define NATIVE_GL_CALL_MAIN(X) NATIVE_GL_CALL((NATIVE_GL_MAIN_IFACE), X)

#define NATIVE_GL_CALL_RET_MAIN(X, RET) NATIVE_GL_CALL_RET(NATIVE_GL_MAIN_IFACE, X, RET)


class NativeGLContext
{
    public:
        NativeGLContext(NativeUIInterface *ui,
            SDL_GLContext wrappedCtx = NULL, bool webgl = false) :
            m_Interface(NULL), m_UI(ui)
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

        inline static void GLCallback(const GrGLInterface *interface) {
            NativeGLContext *_this = (NativeGLContext *)interface->fCallbackData;
            _this->makeCurrent();
        }

        const GrGLInterface *m_Interface;
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
};
#endif
