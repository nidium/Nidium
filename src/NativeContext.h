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

    void callFrame();
    void createDebugCanvas();
    void postDraw();
    void frame();

    void setWindowSize(int width, int height);
    void sizeChanged(int w, int h);

    void setNML(NativeNML *nml) {
        this->m_NML = nml;
    }

    NativeNML *getNML() const {
        return this->m_NML;
    }

    private:
    NativeGLResources m_Resources;
    NativeJS *njs;
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

    struct {
        uint32_t passThroughProgram;
        uint32_t vao;
        uint32_t vbo[2];
        NativeVertices *vtx;
        struct {
            uint32_t u_projectionMatrix;
            uint32_t u_opacity;
            uint32_t u_resolution;
            uint32_t u_position;
            uint32_t u_padding;            
        } uniforms;
    } m_GL;

    void forceLinking();
    void loadNativeObjects(int width, int height);

    void initHandlers(int width, int height);

    /* Initialize default common GL objects (VBO, VAO, etc...) */
    bool initGLBase();
};

#endif