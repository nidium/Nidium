#ifndef nativejs_h__
#define nativejs_h__

#include <stdint.h>
#include <stddef.h>

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
struct _ape_htable;

typedef struct _ape_global ape_global;

class NativeJS
{
    private:   
        void LoadGlobalObjects(NativeSkia *, int width, int height);

    public:
        struct JSContext *cx;
        NativeSharedMessages *messages;
        NativeSkia *surface;
        NativeCanvasHandler *rootHandler;
        NativeUIInterface *UI;
        bool shutdown;
        struct _ape_htable *rootedObj;

        NativeJS(int width, int height);
        ~NativeJS();
        
        int LoadScriptContent(const char *data);
        int LoadScript(const char *filename);
        void callFrame();
        void rootObjectUntilShutdown(JSObject *obj);
        void unrootObject(JSObject *obj);
        void postDraw();
        void bufferSound(int16_t *data, int len);
        void mouseWheel(int xrel, int yrel, int x, int y);
        void mouseMove(int x, int y, int xrel, int yrel);
        void mouseClick(int x, int y, int state, int button);
        void textInput(const char *data);
        void keyupdown(int keycode, int mod, int state, int repeat);
        void gc();
        void bindNetObject(ape_global *net);
        void forceLinking();
        uint32_t currentFPS;

};

#endif
