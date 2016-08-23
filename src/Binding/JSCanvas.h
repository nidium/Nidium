#ifndef binding_jscanvas_h__
#define binding_jscanvas_h__

#include "Binding/JSExposer.h"
#include "Binding/ClassMapper.h"
#include "Core/Messages.h"

namespace Nidium {
namespace Interface {
class UIInterface;
}
namespace Graphics {
class CanvasHandler;
}
namespace Binding {

class JSCanvas : public ClassMapperWithEvents<JSCanvas>, public Core::Messages
{
public:
    virtual void onMessage(const Core::SharedMessages::Message &msg);
    virtual void onMessageLost(const Core::SharedMessages::Message &msg);
    static void RegisterObject(JSContext *cx);
    static JSObject *GenerateJSObject(JSContext *cx,
                                      int width,
                                      int height,
                                      Graphics::CanvasHandler **out);

    JSCanvas(Graphics::CanvasHandler *handler);
    ~JSCanvas();

    static JSCanvas *Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj);
    static JSFunctionSpec *ListMethods();

    Graphics::CanvasHandler *getHandler() const
    {
        return m_CanvasHandler;
    }

protected:

    void JSTracer(class JSTracer *trc);

    bool JS_getContext(JSContext *cx, JS::CallArgs &args);
    bool JS_setContext(JSContext *cx, JS::CallArgs &args);
    bool JS_addSubCanvas(JSContext *cx, JS::CallArgs &args);
    bool JS_insertBefore(JSContext *cx, JS::CallArgs &args);
    bool JS_insertAfter(JSContext *cx, JS::CallArgs &args);
    bool JS_removeFromParent(JSContext *cx, JS::CallArgs &args);
    bool JS_bringToFront(JSContext *cx, JS::CallArgs &args);
    bool JS_sendToBack(JSContext *cx, JS::CallArgs &args);
    bool JS_getParent(JSContext *cx, JS::CallArgs &args);
    bool JS_getFirstChild(JSContext *cx, JS::CallArgs &args);
    bool JS_getLastChild(JSContext *cx, JS::CallArgs &args);
    bool JS_getNextSibling(JSContext *cx, JS::CallArgs &args);
    bool JS_getPrevSibling(JSContext *cx, JS::CallArgs &args);
    bool JS_getChildren(JSContext *cx, JS::CallArgs &args);
    bool JS_getVisibleRect(JSContext *cx, JS::CallArgs &args);
    bool JS_setCoordinates(JSContext *cx, JS::CallArgs &args);
    bool JS_translate(JSContext *cx, JS::CallArgs &args);
    bool JS_show(JSContext *cx, JS::CallArgs &args);
    bool JS_hide(JSContext *cx, JS::CallArgs &args);
    bool JS_setSize(JSContext *cx, JS::CallArgs &args);
    bool JS_clear(JSContext *cx, JS::CallArgs &args);
    bool JS_setZoom(JSContext *cx, JS::CallArgs &args);
    bool JS_setScale(JSContext *cx, JS::CallArgs &args);
private:
    Graphics::CanvasHandler *m_CanvasHandler;
};

} // namespace Binding
} // namespace Nidium

#endif
