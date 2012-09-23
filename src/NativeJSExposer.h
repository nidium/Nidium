#ifndef nativejsexposer_h__
#define nativejsexposer_h__

#define UINT32_MAX 4294967295u
#include <jsapi.h>
#include <jsfriendapi.h>

class NativeJSExposer
{
  public:
    JSContext *cx;
};

#endif