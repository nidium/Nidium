#ifndef nativecontext_h__
#define nativecontext_h__

#include <stdint.h>
#include <stddef.h>
#include "NativeJS.h"

class NativeSkia;
class NativeCanvasHandler;
class NativeUIInterface;
class NativeJS;
class NativeNML;

typedef struct _ape_global ape_global;

class NativeContext
{
    public:
    NativeContext(NativeUIInterface *nui, NativeNML *nml,
        int width, int height, ape_global *net);
    ~NativeContext();

    NativeUIInterface *getUI() const {
        return this->UI;
    }
    NativeCanvasHandler *getRootHandler() const {
        return this->rootHandler;
    }

    NativeJS *getNJS() const {
        return this->njs;
    }

    static NativeContext *getNativeClass(struct JSContext *cx) {
        return (NativeContext *)NativeJS::getNativeClass(cx)->getPrivate();
    }

    static NativeContext *getNativeClass(NativeJS *njs) {
        return (NativeContext *)njs->getPrivate();
    }

    void initHandlers(int width, int height);
    void callFrame();
    void createDebugCanvas();
    void postDraw();

    void setWindowSize(int width, int height);
    void sizeChanged(int w, int h);

    void setNML(NativeNML *nml) {
        this->m_NML = nml;
    }

    NativeNML *getNML() const {
        return this->m_NML;
    }

    private:
    NativeJS *njs;
    NativeSkia *surface;
    NativeCanvasHandler *rootHandler;
    NativeCanvasHandler *debugHandler;
    NativeUIInterface *UI;
    NativeNML *m_NML;

    uint32_t currentFPS;

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
    } stats;

    void forceLinking();
    void loadNativeObjects(int width, int height);
};

#endif