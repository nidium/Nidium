#ifndef nativejsnative_h__
#define nativejsnative_h__

#include <NativeJSExposer.h>

class NativeSkia;
class NativeCanvasHandler;

class NativeJSNative : public NativeJSExposer<NativeJSNative>
{
  public:
    NativeJSNative(JS::HandleObject obj, JSContext *cx) :
        NativeJSExposer<NativeJSNative>(obj, cx)
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

