/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_threadlocalcontext_h__
#define binding_threadlocalcontext_h__

#include <unordered_map>
#include <jsapi.h>

#include "Core/Hash.h"

namespace Nidium {
namespace Binding {

struct nidiumProtoCacheElement
{
    JSClass *jsclass = nullptr;
    JS::Heap<JSObject *> m_Proto;
};

struct nidiumRootedThing
{
    nidiumRootedThing() {};

    enum class Type {
        kHeapObj,
        kHeapValue,
        kTenuredObj,
    } m_Type;

    union {
        JS::Heap<JSObject *> *heapobj = nullptr;
        JS::Heap<JS::Value> *heapvalue;

        JS::TenuredHeap<JSObject *> *tenuredobj;
    };

};


struct NidiumLocalContext {

    NidiumLocalContext(JSRuntime *rt, JSContext *cx) : rt(rt), cx(cx) {
        m_RootedObj = hashtbl_init(APE_HASH_INT);
    }

    bool isShuttingDown() const {
        return m_IsShuttingDown;
    }

    void shutdown() {
        m_IsShuttingDown = true;
        m_Protocache.clear();
    }

    void addProtoCache(JSClass *jsclass, JS::HandleObject proto)
    {
        nidiumProtoCacheElement &el = m_Protocache[jsclass];

        el.jsclass = jsclass;
        el.m_Proto = proto;
    }

    JSObject *getPrototypeFromJSClass(JSClass *jsclass) 
    {
        const nidiumProtoCacheElement &el = m_Protocache[jsclass];

        return el.jsclass == nullptr ? nullptr : el.m_Proto;
    }


    static NidiumLocalContext *Get();
    static void Init();

    static void InitJSThread(JSRuntime *rt, JSContext *cx);

    /*
        Automatically called when key is deleted by a thread
    */
    static void _destroy(void *data);

    static void _jstrace(JSTracer *trc, void *data);

    /*
        Rooting API
    */
    static void RootObjectUntilShutdown(JS::Heap<JSObject *> &obj);
    static void RootObjectUntilShutdown(JS::Heap<JS::Value> &obj);
    static void RootObjectUntilShutdown(JS::TenuredHeap<JSObject *> &obj);
    static void RootObjectUntilShutdown(JSObject *obj);

    template<typename T>
    static void UnrootObject(T &obj)
    {
        NidiumLocalContext *nlc = Get();

        uintptr_t addr = (uintptr_t)&obj;
        
        nlc->m_RootedThings.erase(addr);
        
    }

    /////

    JSRuntime *rt;
    JSContext *cx;
    struct _ape_htable *m_RootedObj;
    bool m_IsShuttingDown = false;
    Nidium::Core::Hash64<uintptr_t> m_JSUniqueInstance{64};

    std::unordered_map<JSClass *, nidiumProtoCacheElement>m_Protocache;
    std::unordered_map<uintptr_t, nidiumRootedThing>m_RootedThings;

};

}
}

#endif