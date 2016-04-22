#ifndef nidium_jsnidium_h__
#define nidium_jsnidium_h__

#include <Binding/JSExposer.h>

class NativeSkia;
class NativeCanvasHandler;

class NativeJSNative : public Nidium::Binding::JSExposer<NativeJSNative>
{
  public:
    NativeJSNative(JS::HandleObject obj, JSContext *cx) :
        Nidium::Binding::JSExposer<NativeJSNative>(obj, cx)
    {

    }
    ~NativeJSNative() {};

    static void registerObject(JSContext *cx);

    static const char *getJSObjectName() {
        return "native";
    }

    static JSClass *jsclass;
};

#endif

