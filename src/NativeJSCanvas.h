#ifndef nativejscanvas_h__
#define nativejscanvas_h__

#include "NativeJSExposer.h"

class NativeJSCanvas: public NativeJSExposer
{
  public:
    static void registerObject(JSContext *cx);
};

#endif

