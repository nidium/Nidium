#ifndef nativejsimage_h__
#define nativejsimage_h__

#include "NativeJSExposer.h"

enum {
    IMAGE_PROP_SRC
};


class NativeJSImage : public NativeJSExposer
{
  public:
    static void registerObject(JSContext *cx);
    static bool JSObjectIs(JSContext *cx, JSObject *obj);
    NativeJSImage();
    ~NativeJSImage();
};

#endif