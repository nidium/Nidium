#ifndef binding_jsimage_h__
#define binding_jsimage_h__

#include <Core/Messages.h>
#include <IO/Stream.h>
#include <Binding/JSExposer.h>

namespace Nidium {
namespace Graphics {
    class Image;
}
namespace Binding {

class JSImage : public JSExposer<JSImage>, public Core::Messages
{
  public:

    JSImage(JS::HandleObject obj, JSContext *cx);
    virtual ~JSImage();

    Graphics::Image *m_Image;
    IO::Stream *m_Stream;

    static Graphics::Image *JSObjectToImage(JS::HandleObject obj);
    static void RegisterObject(JSContext *cx);
    static bool JSObjectIs(JSContext *cx, JS::HandleObject obj);
    static JSObject *BuildImageObject(JSContext *cx, Graphics::Image *image,
        const char name[] = NULL);

    void onMessage(const Core::SharedMessages::Message &msg);

private:
    bool setupWithBuffer(buffer *buf);
};

} // namespace Binding
} // namespace Nidium

#endif

