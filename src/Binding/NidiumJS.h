/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_nidiumjs_h__
#define binding_nidiumjs_h__

#include <stdint.h>
#include <stddef.h>


#include <jspubtd.h>
#include <jsapi.h>
#include <js/StructuredClone.h>
#include <js/Conversions.h>

#include "Core/Hash.h"
#include "Core/Messages.h"
#include "Core/SharedMessages.h"


struct _ape_htable;

namespace Nidium {
namespace Core {
class Context;
}
namespace Binding {

class JSModules;
template<typename T>class ClassMapper;

#define NIDIUM_JS_FNPROPS JSPROP_ENUMERATE | JSPROP_PERMANENT

struct nidium_thread_msg
{
    uint64_t *data;
    size_t nbytes;
    class JSObject *callee;
};

typedef struct _ape_global ape_global;
typedef void (*nidium_thread_message_t)(
    JSContext *cx, Nidium::Core::SharedMessages::Message *msg);

typedef struct _NidiumBytecodeScript
{
    const char *name;
    int size;
    const unsigned char *data;
} NidiumBytecodeScript;


struct NidiumLocalContext {

    NidiumLocalContext(JSRuntime *rt, JSContext *cx) : rt(rt), cx(cx) {
        m_RootedObj = hashtbl_init(APE_HASH_INT);
    }

    bool isShuttingDown() const {
        return m_IsShuttingDown;
    }

    JSRuntime *rt;
    JSContext *cx;
    struct _ape_htable *m_RootedObj;
    bool m_IsShuttingDown = false;
    Nidium::Core::Hash64<uintptr_t> m_JSUniqueInstance{64};

};

class NidiumJSDelegate;

class NidiumJS
{
public:
    explicit NidiumJS(ape_global *net, Core::Context *context);
    ~NidiumJS();

    enum Sctag
    {
        kSctag_Function = JS_SCTAG_USER_MIN + 1,
        kSctag_Hidden,
        kSctag_Max
    };

    JSContext *m_Cx;
    Nidium::Core::SharedMessages *m_Messages;

    Nidium::Core::Hash<JSObject *> m_JsObjects;

    struct _ape_htable *m_RootedObj;
    struct _ape_global *m_Net;

    nidium_thread_message_t *m_RegisteredMessages;
    int m_RegisteredMessagesIdx;
    int m_RegisteredMessagesSize;

    static NidiumJS *GetObject(JSContext *cx = NULL);
    static ape_global *GetNet();
    static void InitNet(ape_global *net);

    static void Init();

    Core::Context *getContext()
    {
        return m_Context;
    }

    JSContext *getJSContext() const
    {
        return this->m_Cx;
    }

    bool isShuttingDown() const
    {
        return m_Shutdown;
    }

    void setStrictMode(bool val)
    {
        m_JSStrictMode = val;
    }

    void loadGlobalObjects();

    static void CopyProperties(JSContext *cx,
                               JS::HandleObject source,
                               JS::MutableHandleObject into);
    static int LoadScriptReturn(JSContext *cx,
                                const char *data,
                                size_t len,
                                const char *filename,
                                JS::MutableHandleValue ret);
    static int LoadScriptReturn(JSContext *cx,
                                const char *filename,
                                JS::MutableHandleValue ret);
    int LoadScriptContent(const char *data, size_t len, const char *filename);

    char *LoadScriptContentAndGetResult(const char *data,
                                        size_t len,
                                        const char *filename);
    int LoadScript(const char *filename);
    int LoadBytecode(NidiumBytecodeScript *script);
    int LoadBytecode(void *data, int size, const char *filename);

    void rootObjectUntilShutdown(JSObject *obj);
    void unrootObject(JSObject *obj);
    void gc();
    void bindNetObject(ape_global *net);

    int registerMessage(nidium_thread_message_t cbk);
    void registerMessage(nidium_thread_message_t cbk, int id);
    void postMessage(void *dataPtr, int ev);

    static JSStructuredCloneCallbacks *m_JsScc;
    static JSObject *readStructuredCloneOp(JSContext *cx,
                                           JSStructuredCloneReader *r,
                                           uint32_t tag,
                                           uint32_t data,
                                           void *closure);

    static bool writeStructuredCloneOp(JSContext *cx,
                                       JSStructuredCloneWriter *w,
                                       JS::HandleObject obj,
                                       void *closure);

    void logf(const char *format, ...);
    void log(const char *format);
    void logclear();

    void setStructuredCloneAddition(WriteStructuredCloneOp write,
                                    ReadStructuredCloneOp read)
    {
        m_StructuredCloneAddition.write = write;
        m_StructuredCloneAddition.read  = read;
    }

    ReadStructuredCloneOp getReadStructuredCloneAddition() const
    {
        return m_StructuredCloneAddition.read;
    }
    WriteStructuredCloneOp getWriteStructuredCloneAddition() const
    {
        return m_StructuredCloneAddition.write;
    }

    static void UnrootObject(JSObject *obj);
    static void RootObjectUntilShutdown(JSObject *obj);
    static JSObject *CreateJSGlobal(JSContext *cx, NidiumJS *njs = nullptr);
    static void SetJSRuntimeOptions(JSRuntime *rt);
    static void InitThreadContext(JSRuntime *rt, JSContext *cx);
    static void DestroyThreadContext(void *data);
    static NidiumLocalContext *GetLocalContext();

private:
    JSModules *m_Modules;
    bool m_Shutdown;
    JSCompartment *m_Compartment;
    bool m_JSStrictMode;
    Core::Context *m_Context;

    struct
    {
        WriteStructuredCloneOp write;
        ReadStructuredCloneOp read;
    } m_StructuredCloneAddition;
};

} // namespace Binding
} // namespace Nidium

#endif
