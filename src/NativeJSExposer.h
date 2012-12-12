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
            0, NULL, NULL, NULL, NULL); \
    }

#define NATIVE_OBJECT_EXPOSE_NOT_INST(name) \
    void NativeJS ## name::registerObject(JSContext *cx) \
    { \
        JSObject *name ## Obj; \
        name ## Obj = JS_DefineObject(cx, JS_GetGlobalObject(cx), #name, \
            &name ## _class , NULL, 0); \
        JS_DefineFunctions(cx, name ## Obj, name ## _funcs); \
    }

#endif
