#ifndef nativejsimage_h__
#define nativejsimage_h__

#include "NativeJSExposer.h"

enum {
    IMAGE_PROP_SRC
};

class NativeSkImage;

class NativeJSImage : public NativeJSExposer
{
  public:
    static void registerObject(JSContext *cx);
    static bool JSObjectIs(JSContext *cx, JSObject *obj);
    static JSObject *buildImageObject(JSContext *cx, NativeSkImage *image,
    	const char name[] = NULL);
    static JSObject *classe;
    NativeJSImage();
    ~NativeJSImage();
};

#endif