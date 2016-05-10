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
#include "Binding/JSExposer.h"


namespace Nidium {
namespace Binding {

class Nidiumcore;

class JSThread : public JSExposer<JSThread>, public Nidium::Core::Messages
{

  public:
    JSThread(JS::HandleObject obj, JSContext *cx);
     enum {
        kThread_Message = 0,
        kThread_Complete = 1
    };
   ~JSThread();
    static void RegisterObject(JSContext *cx);
    void onComplete(JS::HandleValue vp);
    void onMessage(const Nidium::Core::SharedMessages::Message &msg);

    JS::Heap<JSString *> m_JsFunction;

    JSRuntime *m_JsRuntime;
    JSContext *m_JsCx;
    JSObject *m_JsObject;
    Nidiumcore *m_Njs;
    struct {
        int argc;
        uint64_t **argv;
        size_t *nbytes;
    } m_Params;
    bool m_MarkedStop;

    pthread_t m_ThreadHandle;

    char *m_CallerFileName;
    uint32_t m_CallerLineNo;
};
#endif

} // namespace Binding
} // namespace Nidium

