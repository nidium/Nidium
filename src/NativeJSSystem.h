#ifndef nativejssystem_h__
#define nativejssystem_h__

#include <Binding/JSExposer.h>

class NativeJSSystem : public Nidium::Binding::JSExposer<NativeJSSystem>
{
  public:
    static void registerObject(JSContext *cx);
};

#endif

