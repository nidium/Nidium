#ifndef nativejsnative_h__
#define nativejsnative_h__

#include "NativeJSExposer.h"

class NativeSkia;

class NativeJSNative : public NativeJSExposer<NativeJSNative>
{
  public:
    static bool showFPS;
    static void registerObject(JSContext *cx, int width, int height);
};

#endif
