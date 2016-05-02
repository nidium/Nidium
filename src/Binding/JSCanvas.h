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
}

using Nidium::Core::Messages;
using Nidium::Core::SharedMessages;
using Nidium::Graphics::CanvasHandler;

namespace Nidium {
namespace Binding {

class JSCanvas: public JSExposer<JSCanvas>, public Messages
{
public:
    virtual void onMessage(const SharedMessages::Message &msg);
    virtual void onMessageLost(const SharedMessages::Message &msg);
    static void RegisterObject(JSContext *cx);
    static JSObject *GenerateJSObject(JSContext *cx, int width, int height,
        CanvasHandler **out);

    JSCanvas(JS::HandleObject obj, JSContext *cx, CanvasHandler *handler);
    ~JSCanvas();

    CanvasHandler *getHandler() const {
        return m_CanvasHandler;
    }

    void setInherit(JS::HandleObject obj) {
        m_Inherit = obj;
    }

    JSObject *getInherit() const {
        return m_Inherit;
    }

private:
    CanvasHandler *m_CanvasHandler;
    JS::Heap<JSObject *> m_Inherit;
};

} // namespace Binding
} // namespace Nidium

#endif

