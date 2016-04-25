#ifndef binding_jsnidium_h__
#define binding_jsnidium_h__

#include <Binding/JSExposer.h>

namespace Nidium {
    namespace Graphics {
        class NativeSkia;
        class CanvasHandler;
    }
namespace Binding {

class JSNidium : public JSExposer<JSNidium>
{
  public:
    JSNidium(JS::HandleObject obj, JSContext *cx) :
        JSExposer<JSNidium>(obj, cx)
    {

    }
    ~JSNidium() {};

    static void RegisterObject(JSContext *cx);

    static const char *GetJSObjectName() {
        return "native";
    }

    static JSClass *jsclass;
};

} // namespace Nidium
} // namespace Binding

#endif

