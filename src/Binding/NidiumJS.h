/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_nidiumjs_h__
#define binding_nidiumjs_h__

#include <stdint.h>
#include <stddef.h>
#include <unordered_map>

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

typedef struct _NidiumBytecodeScript
{
    const char *name;
    int size;
    const unsigned char *data;
} NidiumBytecodeScript;


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

    Nidium::Core::Hash<JSObject *> m_JsObjects;

    struct _ape_htable *m_RootedObj;
    struct _ape_global *m_Net;

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
    int LoadScriptContent(const char *data, size_t len, const char *filename,
        JS::HandleObject scope = nullptr);

    char *LoadScriptContentAndGetResult(const char *data,
                                        size_t len,
                                        const char *filename);
    int LoadScript(const char *filename);
    int LoadBytecode(NidiumBytecodeScript *script);
    int LoadBytecode(void *data, int size, const char *filename);

    void gc();
    void bindNetObject(ape_global *net);

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

    static JSObject *CreateJSGlobal(JSContext *cx, NidiumJS *njs = nullptr);
    static void SetJSRuntimeOptions(JSRuntime *rt, bool strictmode = false);

private:
    JSModules *m_Modules;
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
