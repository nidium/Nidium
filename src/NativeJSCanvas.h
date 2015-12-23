#ifndef nativejscanvas_h__
#define nativejscanvas_h__

#include <NativeJSExposer.h>
#include <NativeMessages.h>

class NativeCanvasHandler;

class NativeJSCanvas: public NativeJSExposer<NativeJSCanvas>, public NativeMessages
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
    JSObject *m_Inherit;
};

#endif

