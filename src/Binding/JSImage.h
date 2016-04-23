#ifndef nidium_jsimage_h__
#define nidium_jsimage_h__

#include <Core/Messages.h>
#include <IO/Stream.h>
#include <Binding/JSExposer.h>

class NativeSkImage;

namespace Nidium {
namespace Binding {


class NativeJSImage : public JSExposer<NativeJSImage>,
                      public Core::Messages
{
  public:

    NativeJSImage(JS::HandleObject obj, JSContext *cx);
    virtual ~NativeJSImage();

    NativeSkImage *m_Image;
    IO::Stream *m_Stream;

    static NativeSkImage *JSObjectToNativeSkImage(JS::HandleObject obj);
    static void registerObject(JSContext *cx);
    static bool JSObjectIs(JSContext *cx, JS::HandleObject obj);
    static JSObject *buildImageObject(JSContext *cx, NativeSkImage *image,
        const char name[] = NULL);

    void onMessage(const Core::SharedMessages::Message &msg);

private:
    bool setupWithBuffer(buffer *buf);
};

} // namespace Nidium
} // namespace Binding

#endif

