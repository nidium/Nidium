#ifndef nativecontext_h__
#define nativecontext_h__

#include <stdint.h>
#include <stddef.h>
#include "NativeJS.h"
#include "NativeTypes.h"
#include "NativeGLResources.h"

class NativeSkia;
class NativeCanvasHandler;
class NativeUIInterface;
class NativeJS;
class NativeNML;
class NativeCanvasContext;
class NativeGLState;

typedef struct _ape_global ape_global;

class NativeContext : public NativeJSDelegate
{
    public:
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

    static NativeContext *getNativeClass(struct JSContext *cx) {
        return (NativeContext *)NativeJS::getNativeClass(cx)->getPrivate();
    }

    static NativeContext *getNativeClass(NativeJS *njs) {
        return (NativeContext *)njs->getPrivate();
    }

    void callFrame();
    void createDebugCanvas();
    void postDraw();
    void frame();

    void setWindowSize(int width, int height);
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

    // NativeJS delegate
    bool onLoad(NativeJS *njs, char *filename, int argc, jsval *vp);

    private:
    NativeGLResources m_Resources;
    NativeJS *m_JS;
    NativeCanvasHandler *m_RootHandler;
    NativeCanvasHandler *m_DebugHandler;
    NativeUIInterface *m_UI;
    NativeNML *m_NML;
    NativeGLState *m_GLState;

    bool m_SizeDirty;

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

    void initHandlers(int width, int height);
};

#endif
