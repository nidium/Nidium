#ifndef binding_jsimage_h__
#define binding_jsimage_h__

#include <Core/Messages.h>
#include <IO/Stream.h>
#include <Binding/JSExposer.h>

namespace Nidium {
    namespace Graphics {
        class Image;
    }
}

using Nidium::Core::Messages;
using Nidium::Core::SharedMessages;
using Nidium::IO::Stream;
using Nidium::Graphics::Image;

namespace Nidium {
namespace Binding {

class JSImage : public JSExposer<JSImage>, public Messages
{
  public:

    JSImage(JS::HandleObject obj, JSContext *cx);
    virtual ~JSImage();

    Image *m_Image;
    Stream *m_Stream;

    static Image *JSObjectToImage(JS::HandleObject obj);
    static void RegisterObject(JSContext *cx);
    static bool JSObjectIs(JSContext *cx, JS::HandleObject obj);
    static JSObject *BuildImageObject(JSContext *cx, Image *image,
        const char name[] = NULL);

    void onMessage(const SharedMessages::Message &msg);

private:
    bool setupWithBuffer(buffer *buf);
};

} // namespace Binding
} // namespace Nidium

#endif

