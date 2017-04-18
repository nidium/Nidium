/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsfs_h__
#define binding_jsfs_h__

#include "Core/Messages.h"
#include "Core/TaskManager.h"
#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

// {{{ JSAsyncHandler
#define NIDIUM_ASYNC_MAXCALLBACK 4
class JSAsyncHandler : public Nidium::Core::Managed
{
public:
    JSAsyncHandler(JSContext *ctx) : m_Ctx(ctx)
    {
        memset(m_CallBack, 0, sizeof(m_CallBack));
    }

    virtual ~JSAsyncHandler()
    {
        if (m_Ctx == NULL) {
            return;
        }

        for (int i = 0; i < NIDIUM_ASYNC_MAXCALLBACK; i++) {
            if (m_CallBack[i] != NULL) {
                NidiumLocalContext::UnrootObject(m_CallBack[i]);
            }
        }
    }

    void setCallback(int idx, JSObject *callback)
    {
        if (idx >= NIDIUM_ASYNC_MAXCALLBACK || m_Ctx == NULL) {
            return;
        }

        if (m_CallBack[idx] != NULL) {
            NidiumLocalContext::UnrootObject(m_CallBack[idx]);
        }

        if (callback) {
           //NidiumLocalContext::RootObjectUntilShutdown(callback);
        }
        m_CallBack[idx] = callback;
    }

    JSObject *getCallback(int idx) const
    {
        if (idx >= NIDIUM_ASYNC_MAXCALLBACK || m_Ctx == NULL) {
            return NULL;
        }

        return m_CallBack[idx];
    }

    JSContext *getJSContext() const
    {
        return m_Ctx;
    }

    virtual void onMessage(const Nidium::Core::SharedMessages::Message &msg)
        = 0;

private:
    JSContext *m_Ctx;
    JSObject *m_CallBack[NIDIUM_ASYNC_MAXCALLBACK];
};
// }}}

class JSFS : public ClassMapper<JSFS>, public Nidium::Core::Managed
{
public:
    static void RegisterObject(JSContext *cx);
    static JSFunctionSpec *ListMethods();
protected:
    NIDIUM_DECL_JSCALL(readDir);
    NIDIUM_DECL_JSCALL(isDir);
    NIDIUM_DECL_JSCALL(isFile);
};

} // namespace Binding
} // namespace Nidium

#endif
