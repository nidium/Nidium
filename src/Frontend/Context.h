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

#include <ape_pool.h>
#include <ape_netlib.h>

#include <GLSLANG/ShaderLang.h>

#include <Binding/Nidiumcore.h>

#include "Graphics/GLResources.h"

namespace Nidium {
    namespace Interface {
        class UIInterface;
    }
    namespace Net {
        class WebSocketServer;
        class WebSocketClientConnection;
    }
    namespace Graphics {
        class SkiaContext;
        class CanvasHandler;
        class CanvasContext;
        class GLState;
    }
    namespace Binding {
        class Nidiumcore;
        class JSWindow;
    }
}

using Nidium::Core::Hash;
using Nidium::Core::Messages;
using Nidium::Core::SharedMessages;
using Nidium::Interface::UIInterface;
using Nidium::Net::WebSocketServer;
using Nidium::Net::WebSocketClientConnection;
using Nidium::Binding::JSWindow;
using Nidium::Binding::Nidiumcore;
using Nidium::Binding::NidiumBytecodeScript;
using Nidium::Graphics::CanvasHandler;
using Nidium::Graphics::GLState;
using Nidium::Graphics::GLResources;

namespace Nidium {
namespace Frontend {

class NML;

struct JobQueue {
    void (*job)(void *arg);
    struct JobQueue *next;
    void *arg;
};

// {{ InputEvent
static const char * InputEvent_Names[] = {
    "mousemove",
    "mousedown",
    "mouseup",
    "dblclick",
    "dragstart",
    "dragend",
    "dragover",
    "drop",
    "drag",
    "mousewheel"
};

class InputEvent
{
public:
    enum Type {
        kMouseMove_Type = 0,
        kMouseClick_Type,
        kMouseClickRelease_Type,
        kMouseDoubleClick_Type,
        kMouseDragStart_Type,
        kMouseDragEnd_Type,
        kMouseDragOver_Type,
        kMouseDrop_Type,
        kMouseDrag_Type,
        kMouseWheel_Type
    };

    InputEvent(Type type, int ix, int iy,
        uint32_t *idata = NULL, uint8_t idata_len = 0) :
        m_x(ix), m_y(iy), m_Next(NULL), m_PassThroughEvent(NULL), m_Handler(NULL),
        m_Origin(NULL), m_depthAffectedCanvas(0), m_Type(type) {

        if (idata && idata_len <= 8) {
            memcpy(m_data, idata, sizeof(uint32_t) * idata_len);
        }
    }

    InputEvent *dupWithHandler(CanvasHandler *handler) {
        InputEvent *dup = new InputEvent(*this);
        dup->m_Handler = handler;
        dup->m_Origin = this;

        m_PassThroughEvent = dup;

        return dup;
    }

    Type getType() const {
        return m_Type;
    }

    void inc() {
        m_depthAffectedCanvas++;
    }

    unsigned getDepth() const {
        return m_depthAffectedCanvas;
    }

    static const char *GetName(int type) {
        return InputEvent_Names[type];
    }

    InputEvent *getEventForNextCanvas() const {
        return m_PassThroughEvent;
    }

    void setData(int index, uint32_t data) {
        m_data[index] = data;
    }

    int m_x, m_y;
    uint32_t m_data[8];
    InputEvent *m_Next;
    InputEvent *m_PassThroughEvent;
    CanvasHandler *m_Handler;
    InputEvent *m_Origin;
    unsigned m_depthAffectedCanvas;
private:
    Type m_Type;
};
// }}}

struct GrGLInterface;

// {{{ Context
class Context : public Messages
{
    public:

    friend class Nidium::Graphics::CanvasHandler;

    Context(UIInterface *nui, NML *nml,
        int width, int height, ape_global *net);
    ~Context();

    UIInterface *getUI() const {
        return m_UI;
    }
    CanvasHandler *getRootHandler() const {
        return m_RootHandler;
    }

    Nidiumcore *getNJS() const {
        return m_JS;
    }

    NML *getNML() const {
        return m_NML;
    }

    JSWindow *getJSWindow() const {
        return m_JSWindow;
    }

    void setJSWindow(JSWindow *obj) {
        m_JSWindow = obj;
    }

    inline GLState *getGLState() const {
        return m_GLState;
    }

    void setGLState(GLState *state) {
        m_GLState = state;
    }

    ShBuiltInResources *getShaderResources() {
        return &m_ShResources;
    }

    static Context *GetObject() {
        return static_cast<Context *>(Nidiumcore::GetObject(NULL)->getPrivate());
    }

    static Context *GetObject(struct JSContext *cx) {
        return static_cast<Context *>(Nidiumcore::GetObject(cx)->getPrivate());
    }

    static Context *GetObject(Nidiumcore *njs) {
        return static_cast<Context *>(njs->getPrivate());
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

    void setNML(NML *nml) {
        m_NML = nml;
    }

    void sizeNeedUpdate() {
        m_SizeDirty = true;
    }

    bool isSizeDirty() const {
        return m_SizeDirty;
    }

    Hash<NidiumBytecodeScript *> m_Preload;

    void onMessage(const SharedMessages::Message &msg);
    void addJob(void (*job)(void *arg), void *arg);

    CanvasHandler *getCanvasById(const char *str) {
        return m_CanvasList.get(str);
    }

    void addInputEvent(InputEvent *ev);
    void resetInputEvents() {
        m_InputEvents.head = NULL;
        m_InputEvents.queue = NULL;
    }

    void clearInputEvents() {
        InputEvent *tmp;
        for (InputEvent *ev = m_InputEvents.head; ev != NULL; ev = tmp) {
            tmp = ev->m_Next;

            delete(ev);
        }
        m_InputEvents.head = NULL;
        m_InputEvents.queue = NULL;
    }

    InputEvent *getInputEvents() const {
        return m_InputEvents.head;
    }

    void setCurrentClickedHandler(CanvasHandler *handler) {
        m_CurrentClickedHandler = handler;
    }

    CanvasHandler *getCurrentClickedHandler() const {
        return m_CurrentClickedHandler;
    }

    static void CreateAndAssemble(UIInterface *ui, ape_global *gnet);

    private:
    GLResources                m_Resources;
    Nidiumcore *               m_JS;
    CanvasHandler *            m_RootHandler;
    CanvasHandler *            m_DebugHandler;
#ifdef DEBUG
    CanvasHandler *            m_Debug2Handler;
#endif
    UIInterface *              m_UI;
    NML *                      m_NML;
    GLState *                  m_GLState;
    WebSocketServer *          m_WS;
    WebSocketClientConnection *m_WSClient;
    ShBuiltInResources         m_ShResources;
    JSWindow *                 m_JSWindow;
    bool                       m_SizeDirty;

    struct {
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
    } m_Stats;

    struct {
        InputEvent *head;
        InputEvent *queue;
    } m_InputEvents;

    void forceLinking();
    void loadNativeObjects(int width, int height);

    void initStats();
    bool initShaderLang();
    void initHandlers(int width, int height);
    struct {
        struct JobQueue *head;
        struct JobQueue *queue;
    } m_Jobs;

    /* Hash of all canvases (key: identifier string) */
    Hash<CanvasHandler *> m_CanvasList;
    /* Hash of all canvases with pending jobs (key: addr) */
    Hash64<CanvasHandler *> m_CanvasPendingJobs;
    std::vector<CanvasHandler *> m_CanvasOrderedEvents;

    ape_pool_list_t m_CanvasEventsCanvas;

    CanvasHandler *m_CurrentClickedHandler;

    void execJobs();
    void execPendingCanvasChanges();
    void triggerEvents();

    static bool WriteStructuredCloneOp(JSContext *cx, JSStructuredCloneWriter *w,
                                         JS::HandleObject obj, void *closure);

    static JSObject *ReadStructuredCloneOp(JSContext *cx, JSStructuredCloneReader *r,
                                           uint32_t tag, uint32_t data, void *closure);
};
// }}}

} // namespace Frontend
} // namespace Nidium

#endif

