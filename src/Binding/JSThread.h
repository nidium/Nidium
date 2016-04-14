/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsthread_h__
#define binding_jsthread_h__

#include <pthread.h>

#include "Core/NativeSharedMessages.h"
#include "Core/NativeMessages.h"
#include "JSExposer.h"

class NativeJS;

namespace Nidium {
namespace Binding {

class JSThread : public Nidium::Binding::JSExposer<JSThread>, public NativeMessages
{
  public:
    JSThread(JS::HandleObject obj, JSContext *cx);
    ~JSThread();
    static void registerObject(JSContext *cx);
    void onComplete(JS::HandleValue vp);
    void onMessage(const NativeSharedMessages::Message &msg);

    JS::PersistentRootedString jsFunction;

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

    char *m_CallerFileName;
    uint32_t m_CallerLineno;
};

enum {
    NIDIUM_THREAD_MESSAGE = 0,
    NIDIUM_THREAD_COMPLETE = 1
};

#endif

} // namespace Binding
} // namespace Nidium
