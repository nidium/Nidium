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
#include "Binding/ClassMapperWithEvents.h"


namespace Nidium {
namespace Binding {

class NidiumJS;

class JSThread : public ClassMapperWithEvents<JSThread>,
                 public Nidium::Core::Messages
{

public:
    JSThread();
    enum Thread
    {
        kThread_Message  = 0,
        kThread_Complete = 1
    };
    virtual ~JSThread();
    static void RegisterObject(JSContext *cx);
    void onComplete(JS::HandleValue vp);
    void onMessage(const Nidium::Core::SharedMessages::Message &msg);
    static JSThread *Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj);
    static JSFunctionSpec *ListMethods();

    JS::Heap<JSString *> m_JsFunction;
    JSRuntime *m_JsRuntime;
    JSRuntime *m_ParentRuntime;
    JSContext *m_JsCx;
    NidiumJS *m_Njs;
    struct
    {
        int argc;
        uint64_t **argv;
        size_t *nbytes;
    } m_Params;
    bool m_MarkedStop;

    pthread_t m_ThreadHandle;

    char *m_CallerFileName;
    uint32_t m_CallerLineNo;
protected:
    NIDIUM_DECL_JSCALL(start);
};
#endif

} // namespace Binding
} // namespace Nidium
