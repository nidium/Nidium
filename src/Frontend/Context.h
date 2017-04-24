/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef frontend_context_h__
#define frontend_context_h__

#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <unordered_map>

#include <ape_pool.h>
#include <ape_netlib.h>

#include <GLSLANG/ShaderLang.h>

#include "Binding/NidiumJS.h"
#include "Binding/ThreadLocalContext.h"

#include "Graphics/GLResources.h"

#include "Core/Context.h"
#include "Frontend/InputHandler.h"

#include "Graphics/SurfaceCache.h"

#include <Yoga.h>

class GrContext;

namespace Nidium {
namespace Interface {
class UIInterface;
}

namespace Graphics {
class SkiaContext;
class CanvasHandler;
class CanvasContext;
class GLState;
}
namespace Binding {
class NidiumJS;
class JSWindow;
}
namespace Frontend {

class NML;

class LocalContext {
public:

    GrContext *getGrContext() {
        return m_GrContext;
    }

    void setGrContext(GrContext *grcontext) {
        m_GrContext = grcontext;
    }

    static LocalContext *Get() {
        Binding::NidiumLocalContext *nlc = Binding::NidiumLocalContext::Get();

        if (!nlc) {
            return nullptr;
        }

        return (LocalContext *)nlc->ptr;
    }
private:
    GrContext *m_GrContext = nullptr;
};

struct GrGLInterface;

// {{{ Context
class Context : public Core::Context
{
public:
    friend class Nidium::Graphics::CanvasHandler;

    Context(ape_global *net);
    virtual ~Context();

    Interface::UIInterface *getUI() const
    {
        return m_UI;
    }
    Graphics::CanvasHandler *getRootHandler() const
    {
        return m_RootHandler;
    }

    NML *getNML() const
    {
        return m_NML;
    }

    Binding::JSWindow *getJSWindow() const
    {
        return m_JSWindow;
    }

    void setJSWindow(Binding::JSWindow *obj)
    {
        m_JSWindow = obj;
    }

    inline Graphics::GLState *getGLState() const
    {
        return m_GLState;
    }

    void setGLState(Graphics::GLState *state)
    {
        m_GLState = state;
    }

    ShBuiltInResources *getShaderResources()
    {
        return &m_ShResources;
    }

    ShShaderOutput getShaderOutputVersion()
    {
        return m_ShShaderOutput;
    }


    void callFrame();
    void createDebugCanvas();
#if DEBUG
    void createDebug2Canvas();
#endif
    void postDraw();
    void frame(bool draw = true);

    // called during offline rendering
    void rendered(uint8_t *pdata, int width, int height);

    void setWindowSize(int width, int height);
    void setWindowFrame(int x, int y, int width, int height);
    void sizeChanged(int w, int h);

    void setUIObject(Interface::UIInterface *ui);
    void setNML(NML *nml)
    {
        m_NML = nml;
    }

    void sizeNeedUpdate()
    {
        m_SizeDirty = true;
    }

    bool isSizeDirty() const
    {
        return m_SizeDirty;
    }

    Core::Hash<Binding::NidiumBytecodeScript *> m_Preload;

    Graphics::CanvasHandler *getCanvasById(const char *str)
    {
        return m_CanvasList.get(str);
    }

    Graphics::CanvasHandler *getCanvasByIdx(uint64_t idx)
    {
        if (m_CanvasListIdx.count(idx)) {
            return m_CanvasListIdx.at(idx);
        }

        return nullptr;
    }

    InputHandler *getInputHandler()
    {
        return &m_InputHandler;
    }

    void log(const char *str) override;
    void logClear() override;
    void logShow() override;
    void logHide() override;

    YGConfigRef m_YogaConfig;
    uint64_t m_CanvasCreatedIdx = 8;
    Graphics::SurfaceCache m_ContextCache;
private:
    Graphics::GLResources m_Resources;
    Graphics::CanvasHandler *m_RootHandler;
    Graphics::CanvasHandler *m_DebugHandler;
#ifdef DEBUG
    Graphics::CanvasHandler *m_Debug2Handler;
#endif
    Interface::UIInterface *m_UI;
    NML *m_NML;
    InputHandler m_InputHandler;

    Graphics::GLState *m_GLState;
    ShBuiltInResources m_ShResources;
    ShShaderOutput m_ShShaderOutput;
    Binding::JSWindow *m_JSWindow;
    bool m_SizeDirty;

    struct
    {
        uint64_t nframe;
        uint64_t starttime;
        uint64_t lastmeasuredtime;
        uint64_t lastdifftime;
        uint32_t cumulframe;
        float cumultimems;
        float samples[60];
        float fps;
        float minfps;
        float sampleminfps;

        int repaint = 0;
        int resize = 0;
        int composed = 0;
    } m_Stats;

    void statsIncRepaint() {
        m_Stats.repaint++;
    }

    void statsIncResize() {
        m_Stats.resize++;
    }

    void forceLinking();
    void loadNativeObjects(int width, int height);

    void initStats();
    bool initShaderLang();
    void initHandlers(int width, int height);


    /* Hash of all canvases (key: numeric identifier) */
    std::unordered_map<uint64_t, Graphics::CanvasHandler *> m_CanvasListIdx;
    /* Hash of all canvases (key: string identifier) */
    Core::Hash<Graphics::CanvasHandler *> m_CanvasList;

    std::vector<Graphics::CanvasHandler *> m_CanvasOrderedEvents;

    ape_pool_list_t m_CanvasEventsCanvas;

    Graphics::CanvasHandler *m_CurrentClickedHandler;

    void triggerEvents();

    static bool WriteStructuredCloneOp(JSContext *cx,
                                       JSStructuredCloneWriter *w,
                                       JS::HandleObject obj,
                                       void *closure);

    static JSObject *ReadStructuredCloneOp(JSContext *cx,
                                           JSStructuredCloneReader *r,
                                           uint32_t tag,
                                           uint32_t data,
                                           void *closure);
};
// }}}

} // namespace Frontend
} // namespace Nidium

#endif
