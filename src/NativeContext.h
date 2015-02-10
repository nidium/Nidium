#ifndef nativecontext_h__
#define nativecontext_h__

#include <stdint.h>
#include <stddef.h>
#include "NativeJS.h"
#include "NativeTypes.h"
#include "NativeGLResources.h"
#include <NativeMessages.h>
#include <NativeHash.h>
#include <vector>
#include <ape_pool.h>

#include "GLSLANG/ShaderLang.h"

class NativeSkia;
class NativeCanvasHandler;
class NativeUIInterface;
class NativeJS;
class NativeNML;
class NativeCanvasContext;
class NativeGLState;
class NativeWebSocketListener;
class NativeWebSocketClientConnection;
class NativeJSwindow;

typedef struct _ape_global ape_global;

struct NativeJobQueue {
    void (*job)(void *arg);
    struct NativeJobQueue *next;
    void *arg;
};

class NativeInputEvent
{
public:
    enum Type {
        kMouseMove_Type = 1,
        kMouseClick_Type,
        kMouseClickRelease_Type,
        kMouseDoubleClick_Type
    };

    NativeInputEvent(Type type, int ix, int iy,
        uint32_t *idata = NULL, uint8_t idata_len = 0) :
        x(ix), y(iy), m_Next(NULL), m_Handler(NULL), m_Type(type)  {

        if (idata && idata_len <= 8) {
            memcpy(data, idata, sizeof(uint32_t) * idata_len);
        }
    }

    NativeInputEvent *dupWithHandler(NativeCanvasHandler *handler) {
        NativeInputEvent *dup = new NativeInputEvent(*this);
        dup->m_Handler = handler;

        return dup;
    }

    int x, y;
    uint32_t data[8];
    NativeInputEvent *m_Next;
    NativeCanvasHandler *m_Handler;
private:
    Type m_Type;
};


class GrGLInterface;

class NativeContext : public NativeMessages
{
    public:

    friend class NativeCanvasHandler;
    
    NativeContext(NativeUIInterface *nui, NativeNML *nml,
        int width, int height, ape_global *net);
    ~NativeContext();

    NativeUIInterface *getUI() const {
        return m_UI;
    }
    NativeCanvasHandler *getRootHandler() const {
        return m_RootHandler;
    }

    NativeJS *getNJS() const {
        return m_JS;
    }

    NativeNML *getNML() const {
        return m_NML;
    }

    NativeJSwindow *getJSWindow() const {
        return m_JSWindow;
    }

    void setJSWindow(NativeJSwindow *obj) {
        m_JSWindow = obj;
    }

    inline NativeGLState *getGLState() const {
        return m_GLState;
    }

    void setGLState(NativeGLState *state) {
        m_GLState = state;
    }

    ShBuiltInResources *getShaderResources() {
        return &m_ShResources;
    }

    static NativeContext *getNativeClass() {
        return (NativeContext *)NativeJS::getNativeClass(NULL)->getPrivate();
    }

    static NativeContext *getNativeClass(struct JSContext *cx) {
        return (NativeContext *)NativeJS::getNativeClass(cx)->getPrivate();
    }

    static NativeContext *getNativeClass(NativeJS *njs) {
        return (NativeContext *)njs->getPrivate();
    }

    void callFrame();
    void createDebugCanvas();
    void postDraw();
    void frame(bool draw = true);

    // called during offline rendering
    void rendered(uint8_t *pdata, int width, int height);

    void setWindowSize(int width, int height);
    void setWindowFrame(int x, int y, int width, int height);
    void sizeChanged(int w, int h);

    void setNML(NativeNML *nml) {
        this->m_NML = nml;
    }

    void sizeNeedUpdate() {
        m_SizeDirty = true;
    }

    bool isSizeDirty() const {
        return m_SizeDirty;
    }

    NativeHash<NativeBytecodeScript *> preload;

    void onMessage(const NativeSharedMessages::Message &msg);
    void addJob(void (*job)(void *arg), void *arg);

    NativeCanvasHandler *getCanvasById(const char *str) {
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

    private:
    NativeGLResources         m_Resources;
    NativeJS *                m_JS;
    NativeCanvasHandler *     m_RootHandler;
    NativeCanvasHandler *     m_DebugHandler;
    NativeUIInterface *       m_UI;
    NativeNML *               m_NML;
    NativeGLState *           m_GLState;
    NativeWebSocketListener * m_WS;
    NativeWebSocketClientConnection *m_WSClient;
    ShBuiltInResources        m_ShResources;
    NativeJSwindow *          m_JSWindow;
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

    bool initShaderLang();
    void initHandlers(int width, int height);
    struct {
        struct NativeJobQueue *head;
        struct NativeJobQueue *queue;
    } m_Jobs;

    /* Hash of all canvases (key: identifier string) */
    NativeHash<NativeCanvasHandler *> m_CanvasList;
    /* Hash of all canvases with pending jobs (key: addr) */
    NativeHash64<NativeCanvasHandler *> m_CanvasPendingJobs;
    std::vector<NativeCanvasHandler *> m_CanvasOrderedEvents;

    ape_pool_list_t m_CanvasEventsCanvas;

    void execJobs();
    void execPendingCanvasChanges();
    void triggerEvents();

    static JSBool writeStructuredCloneOp(JSContext *cx, JSStructuredCloneWriter *w,
                                         JSObject *obj, void *closure);

    static JSObject *readStructuredCloneOp(JSContext *cx, JSStructuredCloneReader *r,
                                           uint32_t tag, uint32_t data, void *closure);
};

#endif
