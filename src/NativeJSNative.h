#ifndef nativejsnative_h__
#define nativejsnative_h__

#include "NativeJSExposer.h"


class NativeJSNative : public NativeJSExposer
{
  public:
    static bool showFPS;
    static void registerObject(JSContext *cx);
};

#endif