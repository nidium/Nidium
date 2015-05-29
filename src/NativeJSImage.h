#ifndef nativejsimage_h__
#define nativejsimage_h__

#include "NativeJSExposer.h"
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

    NativeJSImage(JSObject *obj, JSContext *cx);
    virtual ~NativeJSImage();

    NativeSkImage *img;
    NativeBaseStream *m_Stream;

    static NativeSkImage *JSObjectToNativeSkImage(JSObject *obj);
    static void registerObject(JSContext *cx);
    static bool JSObjectIs(JSContext *cx, JSObject *obj);
    static JSObject *buildImageObject(JSContext *cx, NativeSkImage *image,
        const char name[] = NULL);
    static JSObject *classe;

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

