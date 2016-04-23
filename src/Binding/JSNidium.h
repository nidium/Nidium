#ifndef nidium_jsnidium_h__
#define nidium_jsnidium_h__

#include <Binding/JSExposer.h>

class NativeSkia;
class NativeCanvasHandler;

namespace Nidium {
namespace Binding {

class NativeJSNative : public JSExposer<NativeJSNative>
{
  public:
    NativeJSNative(JS::HandleObject obj, JSContext *cx) :
        JSExposer<NativeJSNative>(obj, cx)
    {

    }
    ~NativeJSNative() {};

    static void registerObject(JSContext *cx);

    static const char *getJSObjectName() {
        return "native";
    }

    static JSClass *jsclass;
};

} // namespace Nidium
} // namespace Binding

#endif

