#ifndef nativejsimage_h__
#define nativejsimage_h__

#include "NativeJSExposer.h"
#include "NativeHTTP.h"

enum {
    IMAGE_PROP_SRC
};

class NativeSkImage;

class NativeJSImage : public NativeJSExposer, public NativeHTTPDelegate
{
  public:

    NativeJSImage();
    ~NativeJSImage();

    NativeSkImage *img;
    JSObject *jsobj;

    static NativeSkImage *JSObjectToNativeSkImage(JSObject *obj);
    static void registerObject(JSContext *cx);
    static bool JSObjectIs(JSContext *cx, JSObject *obj);
    static JSObject *buildImageObject(JSContext *cx, NativeSkImage *image,
    	const char name[] = NULL);
    static JSObject *classe;

    void onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType);
    void onProgress(size_t offset, size_t len,
        NativeHTTP::HTTPData *h, NativeHTTP::DataType) {}
};

#endif
