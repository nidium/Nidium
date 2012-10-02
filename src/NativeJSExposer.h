#ifndef nativejsexposer_h__
#define nativejsexposer_h__

#ifdef __linux__
  #define UINT32_MAX 4294967295u
#endif

#include <jsapi.h>
#include <jsfriendapi.h>

class NativeJSExposer
{
  public:
    JSContext *cx;
};

#define NATIVE_OBJECT_EXPOSE(name) \
	void NativeJS ## name::registerObject(JSContext *cx) \
	{ \
	    JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, &name ## _class, \
	        native_ ## name ## _constructor, \
	        1, NULL, NULL, NULL, NULL); \
	}

#endif