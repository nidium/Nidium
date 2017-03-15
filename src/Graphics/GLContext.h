/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef graphics_glcontext_h__
#define graphics_glcontext_h__

#include <gl/GrGLInterface.h>
#include <gl/GrGLDefines.h>

#include "Interface/UIInterface.h"

#include "Frontend/Context.h"

typedef void *SDL_GLContext;

namespace Nidium {
namespace Interface {
extern UIInterface *__NidiumUI;
}
namespace Graphics {

// {{{ Macro's
/*
    Make the context pointed by IFACE current and make a GL call
    e.g. NIDIUM_GL_CALL(this->context, Clear(0, 0, 0, 0));
*/

#define NIDIUM_GL_MAIN_IFACE                           \
    (Nidium::Interface::__NidiumUI->getNidiumContext() \
         ->getGLState()                                \
         ->getNidiumGLContext())

#ifndef NIDIUM_ENABLE_GL_ERROR

#define NIDIUM_GL_CALL(IFACE, X)                   \
    do {                                           \
        GLContext::GLCallback(IFACE->m_Interface); \
        (IFACE)->m_Interface->fFunctions.f##X;     \
    } while (false)

#define NIDIUM_GL_CALL_RET(IFACE, X, RET)              \
    do {                                               \
        GLContext::GLCallback(IFACE->m_Interface);     \
        (RET) = (IFACE)->m_Interface->fFunctions.f##X; \
    } while (false)

#else
#define NIDIUM_GL_CALL(IFACE, X)                                         \
    do {                                                                 \
        uint32_t __err;                                                  \
        GLContext::GLCallback(IFACE->m_Interface);                       \
        (IFACE)->m_Interface->fFunctions.f##X;                           \
        if ((__err = (IFACE)->m_Interface->fFunctions.fGetError())       \
            != GR_GL_NO_ERROR) {                                         \
            ndm_logf(NDM_LOG_ERROR, "GLError",                           \
                     "gl%s() returned %d", #X, __err);                   \
        }                                                                \
    } while (false)

#define NIDIUM_GL_CALL_RET(IFACE, X, RET)                                \
    do {                                                                 \
        uint32_t __err;                                                  \
        GLContext::GLCallback(IFACE->m_Interface);                       \
        (RET) = (IFACE)->m_Interface->fFunctions.f##X;                   \
        if ((__err = (IFACE)->m_Interface->fFunctions.fGetError())       \
            != GR_GL_NO_ERROR) {                                         \
            ndm_logf(NDM_LOG_ERROR, "GLError",                           \
                     "gl%s() returned %d", #X, __err);                   \
        }                                                                \
    } while (false)
#endif

#define NIDIUM_GL_CALL_MAIN(X) NIDIUM_GL_CALL((NIDIUM_GL_MAIN_IFACE), X)

#define NIDIUM_GL_CALL_RET_MAIN(X, RET) \
    NIDIUM_GL_CALL_RET(NIDIUM_GL_MAIN_IFACE, X, RET)
// }}}

// {{{ GLContext
class GLContext
{
public:
    GLContext(Interface::UIInterface *ui,
              SDL_GLContext wrappedCtx = NULL,
              bool webgl = false)
        : m_Interface(NULL), m_UI(ui)
    {
        if (wrappedCtx) {
            m_SDLGLCtx = wrappedCtx;
            m_Wrapped  = true;

            this->createInterface();

            return;
        }

        m_Wrapped = false;

        SDL_GLContext oldctx = m_UI->getCurrentGLContext();

        /* The new context share with the "main" GL context */
        if (!m_UI->makeMainGLCurrent()) {
            ndm_log(NDM_LOG_ERROR, "GLContext", "Can't make main current");
        }

        m_SDLGLCtx = m_UI->createSharedContext(webgl);
        if (m_SDLGLCtx == NULL) {
            ndm_log(NDM_LOG_ERROR, "GLContext", "Can't create context");
        }

        this->createInterface();

        /* Restore to the old GL Context */
        if (!m_UI->makeGLCurrent(oldctx)) {
            ndm_log(NDM_LOG_ERROR, "GLContext", "Can't restore old ctx");
        }
    }

    inline SDL_GLContext getGLContext() const
    {
        return m_SDLGLCtx;
    }

    inline const GrGLInterface *iface() const
    {
        return m_Interface;
    }

    inline bool makeCurrent()
    {
        return m_UI->makeGLCurrent(m_SDLGLCtx);
    }

    ~GLContext()
    {
        if (!m_Wrapped) {
            m_UI->makeMainGLCurrent();
            /*
                TODO: LEAK :
                Whenever we try to delete a GL ctx, the screen become black.
            */
            // m_UI->deleteGLContext(m_SDLGLCtx);
        }

        if (m_Interface) {
            m_Interface->unref();
        }
    }

    Interface::UIInterface *getUI() const
    {
        return m_UI;
    }

    inline static void GLCallback(const GrGLInterface *interface)
    {
        //@TODO static_cast
        GLContext *_this = (GLContext *)interface->fCallbackData;
        _this->makeCurrent();
    }

    const GrGLInterface *m_Interface;

private:
    void createInterface()
    {
        this->makeCurrent();

        m_Interface = GrGLCreateNativeInterface();

        if (!m_Interface) {
            ndm_log(NDM_LOG_ERROR, "GLContext",
                "Fatal OpenGL Error: Failed to create GrGL "
                "Interface...exiting");
            exit(1);
        }
        // TODO: new style cast
        ((GrGLInterface *)m_Interface)->fCallback = GLContext::GLCallback;
        ((GrGLInterface *)m_Interface)->fCallbackData = (uintptr_t)(this);
    }

    SDL_GLContext m_SDLGLCtx;
    Interface::UIInterface *m_UI;
    bool m_Wrapped;
};
// }}}

} // namespace Graphics
} // namespace Nidium

#endif
