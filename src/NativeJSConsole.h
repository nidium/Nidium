#ifndef nativejsconsole_h__
#define nativejsconsole_h__

#include <Binding/JSExposer.h>

class NativeJSconsole : public Nidium::Binding::JSExposer<NativeJSconsole>
{
  public:
    static void registerObject(JSContext *cx);
};

#endif

