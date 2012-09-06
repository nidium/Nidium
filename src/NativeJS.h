#ifndef nativejs_h__
#define nativejs_h__

#include <jsapi.h>

#include "NativeSkia.h"

/* JSAPI binding (singleton) */
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
        void gc();
        uint32_t currentFPS;
};

#endif