#ifndef nativejs_h__
#define nativejs_h__

#include <stdint.h>
#include <stddef.h>
#include "NativeTypes.h"
#include <jspubtd.h>


enum {
    NATIVE_KEY_SHIFT = 1 << 0,
    NATIVE_KEY_ALT = 1 << 1,
    NATIVE_KEY_CTRL = 1 << 2
};

struct native_thread_msg
{
    uint64_t *data;
    size_t nbytes;
    class JSObject *callee;
};

class NativeSharedMessages;
class NativeSkia;
class NativeCanvasHandler;
class NativeUIInterface;
class NativeJSModules;
struct _ape_htable;

typedef struct _ape_global ape_global;

class NativeJS
{
    private:   
        void LoadGlobalObjects(NativeSkia *, int width, int height);
        NativeJSModules *modules;
        JSObject *jsobjdocument;
    public:
        JSContext *cx;
        NativeSharedMessages *messages;
        NativeSkia *surface;
        NativeCanvasHandler *rootHandler;
        NativeCanvasHandler *debugHandler;
        NativeUIInterface *UI;
        bool shutdown;
        struct _ape_htable *rootedObj;
        struct _ape_global *net;

        NativeJS(int width, int height, NativeUIInterface *inUI, ape_global *net);
        ~NativeJS();
        
        int LoadApplication(const char *path);
        void Loaded();
        static void CopyProperties(JSContext *cx, JSObject *source, JSObject *into);
        static int LoadScriptReturn(JSContext *cx,
            const char *filename, JS::Value *ret);
        int LoadScriptContent(const char *data, size_t len,
            const char *filename);
        int LoadScript(const char *filename);
        int LoadScript(const char *filename, JSObject *gbl);
        void callFrame();

        void createDebugCanvas();
        void rootObjectUntilShutdown(JSObject *obj);
        void unrootObject(JSObject *obj);
        void postDraw();
        void windowFocus();
        void windowBlur();
        void mouseWheel(int xrel, int yrel, int x, int y);
        void mouseMove(int x, int y, int xrel, int yrel);
        void mouseClick(int x, int y, int state, int button);
        void assetReady(const NMLTag &tag);
        void textInput(const char *data);
        void keyupdown(int keycode, int mod, int state, int repeat);
        void gc();
        void bindNetObject(ape_global *net);
        void forceLinking();
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

};

#endif
