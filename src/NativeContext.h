#ifndef nativecontext_h__
#define nativecontext_h__

#include <stdint.h>
#include <stddef.h>
#include "NativeJS.h"
#include "NativeTypes.h"
#include "NativeGLResources.h"
#include <NativeMessages.h>
#include <NativeHash.h>

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

typedef struct _ape_global ape_global;

struct NativeJobQueue {
    void (*job)(void *arg);
    struct NativeJobQueue *next;
    void *arg;
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

    NativeGLState *getGLState() const {
        return m_GLState;
    }

    ShBuiltInResources *getShaderResources() {
        return &m_ShResources;
    }

    static NativeContext *getNativeClass(struct JSContext *cx) {
        return (NativeContext *)NativeJS::getNativeClass(cx)->getPrivate();
    }

    static NativeContext *getNativeClass(NativeJS *njs) {
        return (NativeContext *)njs->getPrivate();
    }

    static void glCallback(const GrGLInterface *interface);

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

    void execJobs();
    void execPendingCanvasChanges();

    static JSBool writeStructuredCloneOp(JSContext *cx, JSStructuredCloneWriter *w,
                                         JSObject *obj, void *closure);

    static JSObject *readStructuredCloneOp(JSContext *cx, JSStructuredCloneReader *r,
                                           uint32_t tag, uint32_t data, void *closure);
};

#endif
