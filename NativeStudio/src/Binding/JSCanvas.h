#ifndef binding_jscanvas_h__
#define binding_jscanvas_h__

#include <Binding/JSExposer.h>
#include <Core/Messages.h>

namespace Nidium {
namespace Interface {
    class UIInterface;
}
namespace Graphics {
    class CanvasHandler;
}
namespace Binding {

class JSCanvas: public JSExposer<JSCanvas>, public Core::Messages
{
public:
    virtual void onMessage(const Core::SharedMessages::Message &msg);
    virtual void onMessageLost(const Core::SharedMessages::Message &msg);
    static void RegisterObject(JSContext *cx);
    static JSObject *GenerateJSObject(JSContext *cx, int width, int height,
        Graphics::CanvasHandler **out);

    JSCanvas(JS::HandleObject obj, JSContext *cx, Graphics::CanvasHandler *handler);
    ~JSCanvas();

    Graphics::CanvasHandler *getHandler() const {
        return m_CanvasHandler;
    }

    void setInherit(JS::HandleObject obj) {
        m_Inherit = obj;
    }

    JSObject *getInherit() const {
        return m_Inherit;
    }

private:
    Graphics::CanvasHandler *m_CanvasHandler;
    JS::Heap<JSObject *> m_Inherit;
};

} // namespace Binding
} // namespace Nidium

#endif

