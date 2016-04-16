#ifndef nativejsimage_h__
#define nativejsimage_h__

#include <Core/Messages.h>
#include <IO/Stream.h>
#include <Binding/JSExposer.h>

class NativeSkImage;

class NativeJSImage : public Nidium::Binding::JSExposer<NativeJSImage>,
                      public Nidium::Core::Messages
{
  public:

    NativeJSImage(JS::HandleObject obj, JSContext *cx);
    virtual ~NativeJSImage();

    NativeSkImage *m_Image;
    Nidium::IO::Stream *m_Stream;

    static NativeSkImage *JSObjectToNativeSkImage(JS::HandleObject obj);
    static void registerObject(JSContext *cx);
    static bool JSObjectIs(JSContext *cx, JS::HandleObject obj);
    static JSObject *buildImageObject(JSContext *cx, NativeSkImage *image,
        const char name[] = NULL);

    void onMessage(const Nidium::Core::SharedMessages::Message &msg);

private:
    bool setupWithBuffer(buffer *buf);
};

#endif

