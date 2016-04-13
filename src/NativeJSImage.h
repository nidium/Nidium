#ifndef nativejsimage_h__
#define nativejsimage_h__

#include <Core/NativeMessages.h>
#include <IO/NativeStreamInterface.h>
#include <Binding/JSExposer.h>

class NativeSkImage;

class NativeJSImage : public Nidium::Binding::JSExposer<NativeJSImage>,
                      public NativeMessages
{
  public:

    NativeJSImage(JS::HandleObject obj, JSContext *cx);
    virtual ~NativeJSImage();

    NativeSkImage *m_Image;
    NativeBaseStream *m_Stream;

    static NativeSkImage *JSObjectToNativeSkImage(JS::HandleObject obj);
    static void registerObject(JSContext *cx);
    static bool JSObjectIs(JSContext *cx, JS::HandleObject obj);
    static JSObject *buildImageObject(JSContext *cx, NativeSkImage *image,
        const char name[] = NULL);

    void onMessage(const NativeSharedMessages::Message &msg);

private:
    bool setupWithBuffer(buffer *buf);
};

#endif

