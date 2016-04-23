#ifndef binding_jsnidium_h__
#define binding_jsnidium_h__

#include <Binding/JSExposer.h>

namespace Nidium {
    namespace Graphics {
        class NativeSkia;
        class NativeCanvasHandler;
    }
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

