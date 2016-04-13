#ifndef nativejscanvas_h__
#define nativejscanvas_h__

#include <Binding/JSExposer.h>
#include <Core/NativeMessages.h>

class NativeCanvasHandler;

class NativeJSCanvas: public Nidium::Binding::JSExposer<NativeJSCanvas>, public NativeMessages
{
public:
    virtual void onMessage(const NativeSharedMessages::Message &msg);
    virtual void onMessageLost(const NativeSharedMessages::Message &msg);
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

#endif

