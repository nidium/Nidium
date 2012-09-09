#ifndef nativejs_h__
#define nativejs_h__

#include <jsapi.h>

#include "NativeSkia.h"

/* JSAPI binding (singleton) */

enum {
    NATIVE_KEY_SHIFT = 1 << 0,
    NATIVE_KEY_ALT = 1 << 1,
    NATIVE_KEY_CTRL = 1 << 2
};

typedef struct _ape_global ape_global;

class NativeJS
{
    private:   
        void LoadCanvasObject(NativeSkia *);

        //void operator=(NativeJS const&);

        JSContext *cx;
        jsval func;
    public:
        NativeSkia *nskia;
        NativeJS();
        ~NativeJS();
        int LoadScript(const char *filename);
        void callFrame();
        void bufferSound(int16_t *data, int len);
        void mouseWheel(int xrel, int yrel, int x, int y);
        void mouseMove(int x, int y, int xrel, int yrel);
        void mouseClick(int x, int y, int state, int button);
        void textInput(const char *data);
        void keyupdown(int keycode, int mod, int state, int repeat);
        void gc();
        void bindNetObject(ape_global *net);
        uint32_t currentFPS;
};

#endif