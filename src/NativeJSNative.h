#ifndef nativejsnative_h__
#define nativejsnative_h__

#include "NativeJSExposer.h"

class NativeSkia;

class NativeJSNative : public NativeJSExposer
{
  public:
  	static NativeSkia *context2D;
    static bool showFPS;
    static void registerObject(JSContext *cx);
};

#endif
