#ifndef nml_context_h__
#define nml_context_h__

#include <stdint.h>
#include <stddef.h>
#include <vector>

#include <ape_pool.h>
#include <ape_netlib.h>

#include <GLSLANG/ShaderLang.h>

#include <Binding/NidiumJS.h>

#include "NML/Types.h"
#include "Graphics/GLResources.h"

namespace Nidium {
    namespace Interface {
        class NativeUIInterface;
    }
    namespace Net {
        class WebSocketServer;
        class WebSocketClientConnection;
    }
    namespace Graphics {
        class NativeSkia;
        class NativeCanvasHandler;
        class NativeCanvasContext;
        class NativeGLState;
    }
    namespace Binding {
        class NidiumJS;

        class NativeJSwindow;
    }
namespace NML {

class NativeNML;

struct NativeJobQueue {
    void (*job)(void *arg);
    struct NativeJobQueue *next;
    void *arg;
};

// {{ NativeInputEvent
static const char * NativeInputEvent_Names[] = {
    "mousemove",
    "mousedown",
    "mouseup",
    "dblclick",
    "dragstart",
    "dragend",
    "dragover",
    "drop",
    "drag"
};

class NativeInputEvent
{
public:
    enum Type {
        kMouseMove_Type,
        kMouseClick_Type,
        kMouseClickRelease_Type,
        kMouseDoubleClick_Type,
        kMouseDragStart_Type,
        kMouseDragEnd_Type,
        kMouseDragOver_Type,
        kMouseDrop_Type,
        kMouseDrag_Type
    };

    NativeInputEvent(Type type, int ix, int iy,
        uint32_t *idata = NULL, uint8_t idata_len = 0) :
        m_x(ix), m_y(iy), m_Next(NULL), m_PassThroughEvent(NULL), m_Handler(NULL),
        m_Origin(NULL), m_depthAffectedCanvas(0), m_Type(type) {

        if (idata && idata_len <= 8) {
            memcpy(m_data, idata, sizeof(uint32_t) * idata_len);
        }
    }

    NativeInputEvent *dupWithHandler(Nidium::Graphics::NativeCanvasHandler *handler) {
        NativeInputEvent *dup = new NativeInputEvent(*this);
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
        return NativeInputEvent_Names[type];
    }

    NativeInputEvent *getEventForNextCanvas() const {
        return m_PassThroughEvent;
    }

    int m_x, m_y;
    uint32_t m_data[8];
    NativeInputEvent *m_Next;
    NativeInputEvent *m_PassThroughEvent;
    Nidium::Graphics::NativeCanvasHandler *m_Handler;
    NativeInputEvent *m_Origin;
    unsigned m_depthAffectedCanvas;
private:
    Type m_Type;
};
// }}}

struct GrGLInterface;

// {{{ NativeContext
class NativeContext : public Nidium::Core::Messages
{
    public:

    friend class Nidium::Graphics::NativeCanvasHandler;

    NativeContext(Nidium::Interface::NativeUIInterface *nui, NativeNML *nml,
        int width, int height, ape_global *net);
    ~NativeContext();

    Nidium::Interface::NativeUIInterface *getUI() const {
        return m_UI;
    }
    Nidium::Graphics::NativeCanvasHandler *getRootHandler() const {
        return m_RootHandler;
    }

    Nidium::Binding::NidiumJS *getNJS() const {
        return m_JS;
    }

    NativeNML *getNML() const {
        return m_NML;
    }

    Nidium::Binding::NativeJSwindow *getJSWindow() const {
        return m_JSWindow;
    }

    void setJSWindow(Nidium::Binding::NativeJSwindow *obj) {
        m_JSWindow = obj;
    }

    inline Nidium::Graphics::NativeGLState *getGLState() const {
        return m_GLState;
    }

    void setGLState(Nidium::Graphics::NativeGLState *state) {
        m_GLState = state;
    }

    ShBuiltInResources *getShaderResources() {
        return &m_ShResources;
    }

    static NativeContext *GetObject() {
        return static_cast<NativeContext *>(Nidium::Binding::NidiumJS::GetObject(NULL)->getPrivate());
    }

    static NativeContext *GetObject(struct JSContext *cx) {
        return static_cast<NativeContext *>(Nidium::Binding::NidiumJS::GetObject(cx)->getPrivate());
    }

    static NativeContext *GetObject(Nidium::Binding::NidiumJS *njs) {
        return static_cast<NativeContext *>(njs->getPrivate());
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

    void setNML(NativeNML *nml) {
        m_NML = nml;
    }

    void sizeNeedUpdate() {
        m_SizeDirty = true;
    }

    bool isSizeDirty() const {
        return m_SizeDirty;
    }

    Nidium::Core::Hash<Nidium::Binding::NidiumBytecodeScript *> preload;

    void onMessage(const Nidium::Core::SharedMessages::Message &msg);
    void addJob(void (*job)(void *arg), void *arg);

    Nidium::Graphics::NativeCanvasHandler *getCanvasById(const char *str) {
        return m_CanvasList.get(str);
    }

    void addInputEvent(NativeInputEvent *ev);
    void resetInputEvents() {
        m_InputEvents.head = NULL;
        m_InputEvents.queue = NULL;
    }

    void clearInputEvents() {
        NativeInputEvent *tmp;
        for (NativeInputEvent *ev = m_InputEvents.head; ev != NULL; ev = tmp) {
            tmp = ev->m_Next;

            delete(ev);
        }
        m_InputEvents.head = NULL;
        m_InputEvents.queue = NULL;
    }

    NativeInputEvent *getInputEvents() const {
        return m_InputEvents.head;
    }

    void setCurrentClickedHandler(Nidium::Graphics::NativeCanvasHandler *handler) {
        m_CurrentClickedHandler = handler;
    }

    Nidium::Graphics::NativeCanvasHandler *getCurrentClickedHandler() const {
        return m_CurrentClickedHandler;
    }

    static void CreateAndAssemble(Nidium::Interface::NativeUIInterface *ui, ape_global *gnet);

    private:
    Nidium::Graphics::NativeGLResources         m_Resources;
    Nidium::Binding::NidiumJS *m_JS;
    Nidium::Graphics::NativeCanvasHandler *     m_RootHandler;
    Nidium::Graphics::NativeCanvasHandler *     m_DebugHandler;
#ifdef DEBUG
    Nidium::Graphics::NativeCanvasHandler *     m_Debug2Handler;
#endif
    Nidium::Interface::NativeUIInterface *       m_UI;
    NativeNML *               m_NML;
    Nidium::Graphics::NativeGLState *           m_GLState;
    Nidium::Net::WebSocketServer * m_WS;
    Nidium::Net::WebSocketClientConnection *m_WSClient;
    ShBuiltInResources        m_ShResources;
    Nidium::Binding::NativeJSwindow *          m_JSWindow;
    bool                      m_SizeDirty;

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
        NativeInputEvent *head;
        NativeInputEvent *queue;
    } m_InputEvents;

    void forceLinking();
    void loadNativeObjects(int width, int height);

    void initStats();
    bool initShaderLang();
    void initHandlers(int width, int height);
    struct {
        struct NativeJobQueue *head;
        struct NativeJobQueue *queue;
    } m_Jobs;

    /* Hash of all canvases (key: identifier string) */
    Nidium::Core::Hash<Nidium::Graphics::NativeCanvasHandler *> m_CanvasList;
    /* Hash of all canvases with pending jobs (key: addr) */
    Nidium::Core::Hash64<Nidium::Graphics::NativeCanvasHandler *> m_CanvasPendingJobs;
    std::vector<Nidium::Graphics::NativeCanvasHandler *> m_CanvasOrderedEvents;

    ape_pool_list_t m_CanvasEventsCanvas;

    Nidium::Graphics::NativeCanvasHandler *m_CurrentClickedHandler;

    void execJobs();
    void execPendingCanvasChanges();
    void triggerEvents();

    static bool WriteStructuredCloneOp(JSContext *cx, JSStructuredCloneWriter *w,
                                         JS::HandleObject obj, void *closure);

    static JSObject *ReadStructuredCloneOp(JSContext *cx, JSStructuredCloneReader *r,
                                           uint32_t tag, uint32_t data, void *closure);
};
// }}}

} // namespace NML
} // namespace Nidium

#endif

