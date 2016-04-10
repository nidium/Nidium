#ifndef nativejssystem_h__
#define nativejssystem_h__

#include <JS/NativeJSExposer.h>

class NativeJSSystem : public NativeJSExposer<NativeJSSystem>
{
  public:
    static void registerObject(JSContext *cx);
};

#endif

