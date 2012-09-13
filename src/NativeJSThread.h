#ifndef nativejsthread_h__
#define nativejsthread_h__

#include "NativeJSExposer.h"
#include <pthread.h>

class NativeJS;


class NativeJSThread : public NativeJSExposer
{
  public:
    NativeJSThread();
    ~NativeJSThread();
    static void registerObject(JSContext *cx);

    JSString *jsFunction;
    JSRuntime *jsRuntime;
    JSContext *jsCx;
    JSObject *jsObject;
    NativeJS *njs;

    pthread_t threadHandle;
};

#endif