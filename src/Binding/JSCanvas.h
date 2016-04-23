#ifndef binding_jscanvas_h__
#define binding_jscanvas_h__

#include <Binding/JSExposer.h>
#include <Core/Messages.h>


class NativeCanvasHandler;

namespace Nidium {
namespace Binding {

class NativeJSCanvas: public JSExposer<NativeJSCanvas>, public Core::Messages
{
public:
    virtual void onMessage(const Core::SharedMessages::Message &msg);
    virtual void onMessageLost(const Core::SharedMessages::Message &msg);
    static void registerObject(JSContext *cx);
    static JSObject *generateJSObject(JSContext *cx, int width, int height,
        NativeCanvasHandler **out);

    NativeJSCanvas(JS::HandleObject obj, JSContext *cx, NativeCanvasHandler *handler);
    ~NativeJSCanvas();

    NativeCanvasHandler *getHandler() const {
        return m_CanvasHandler;
    }

    void setInherit(JS::HandleObject obj) {
        m_Inherit = obj;
    }

    JSObject *getInherit() const {
        return m_Inherit;
    }

private:
    NativeCanvasHandler *m_CanvasHandler;
    JS::Heap<JSObject *> m_Inherit;
};

} // namespace Nidium
} // namespace Binding

#endif

