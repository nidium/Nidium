#ifndef nativejsimage_h__
#define nativejsimage_h__

#include "NativeJSExposer.h"
#include "NativeStream.h"

enum {
    IMAGE_PROP_SRC
};

class NativeSkImage;

class NativeJSImage : public NativeJSExposer, public NativeStreamDelegate
{
  public:

    NativeJSImage();
    virtual ~NativeJSImage();

    NativeSkImage *img;
    JSObject *jsobj;
    NativeStream *stream;

    static NativeSkImage *JSObjectToNativeSkImage(JSObject *obj);
    static void registerObject(JSContext *cx);
    static bool JSObjectIs(JSContext *cx, JSObject *obj);
    static JSObject *buildImageObject(JSContext *cx, NativeSkImage *image,
    	const char name[] = NULL);
    static JSObject *classe;

    void onGetContent(const char *data, size_t len);
    /*
    void onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType);
    void onProgress(size_t offset, size_t len,
        NativeHTTP::HTTPData *h, NativeHTTP::DataType) {};
    void onError(NativeHTTP::HTTPError err) {};*/
};

#endif
