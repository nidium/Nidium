/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsthread_h__
#define binding_jsthread_h__

#include <pthread.h>

#include "Core/SharedMessages.h"
#include "Core/Messages.h"
#include "JSExposer.h"


namespace Nidium {
namespace Binding {

class NidiumJS;

class JSThread : public JSExposer<JSThread>, public Nidium::Core::Messages
{
  public:
    JSThread(JS::HandleObject obj, JSContext *cx);
    ~JSThread();
    static void RegisterObject(JSContext *cx);
    void onComplete(JS::HandleValue vp);
    void onMessage(const Nidium::Core::SharedMessages::Message &msg);

    JS::Heap<JSString *> jsFunction;

    JSRuntime *jsRuntime;
    JSContext *jsCx;
    JSObject *jsObject;
    NidiumJS *njs;
    struct {
        int argc;
        uint64_t **argv;
        size_t *nbytes;
    } params;
    bool markedStop;

    pthread_t threadHandle;

    char *m_CallerFileName;
    uint32_t m_CallerLineNo;
};

enum {
    NIDIUM_THREAD_MESSAGE = 0,
    NIDIUM_THREAD_COMPLETE = 1
};

#endif

} // namespace Binding
} // namespace Nidium

