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
    void onComplete(jsval *vp);

    JSString *jsFunction;
    JSRuntime *jsRuntime;
    JSContext *jsCx;
    JSObject *jsObject;
    NativeJS *njs;
    struct {
        int argc;
        uint64_t **argv;
        size_t *nbytes;
    } params;
    bool markedStop;

    pthread_t threadHandle;
};

enum {
    NATIVE_THREAD_MESSAGE,
    NATIVE_THREAD_COMPLETE
};

#endif
