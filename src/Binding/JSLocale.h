#ifndef binding_jsnidium_h__
#define binding_jsnidium_h__

#include <Binding/JSExposer.h>

namespace Nidium {
    namespace Graphics {
        class SkiaContext;
        class CanvasHandler;
    }
namespace Binding {

class JSLocale : public JSExposer<JSLocale>
{
  public:
    JSLocale(JS::HandleObject obj, JSContext *cx) :
        JSExposer<JSLocale>(obj, cx)
    {

    }
    ~JSLocale() {};

    static void RegisterObject(JSContext *cx);

    static const char *GetJSObjectName() {
        return "native";
    }

    static JSClass *jsclass;
};

} // namespace Binding
} // namespace Nidium

#endif

