#ifndef nativejsimage_h__
#define nativejsimage_h__

#include <NativeJSExposer.h>
#include <NativeMessages.h>
#include <NativeStreamInterface.h>

enum {
    IMAGE_PROP_SRC
};

class NativeSkImage;

class NativeJSImage : public NativeJSExposer<NativeJSImage>,
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
    /*
    void onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType);
    void onProgress(size_t offset, size_t len,
        NativeHTTP::HTTPData *h, NativeHTTP::DataType) {};
    void onError(NativeHTTP::HTTPError err) {};*/
private:
    bool setupWithBuffer(buffer *buf);
};

#endif

