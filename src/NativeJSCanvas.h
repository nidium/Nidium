#ifndef nativejscanvas_h__
#define nativejscanvas_h__

#include "NativeJSExposer.h"

class NativeCanvasHandler;

class NativeJSCanvas: public NativeJSExposer<NativeJSCanvas>
{
  public:
    static void registerObject(JSContext *cx);
    static JSObject *generateJSObject(JSContext *cx, int width, int height,
        NativeCanvasHandler **out);
};

#endif
