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

class nidiumRootedThing
{
public:
    enum class Type {
        kHeapObj,
        kHeapValue,
        kHeapStr,
        kTenuredObj,
    } m_Type;

    union {
        JS::Heap<JSObject *> *heapobj = nullptr;
        JS::Heap<JS::Value> *heapvalue;
        JS::Heap<JSString *> *heapstr;
        JS::TenuredHeap<JSObject *> *tenuredobj;
    };
};

class nidiumRootedThingRef : public nidiumRootedThing
{
public:
    JSObject *get() {
        return m_Ref;
    }

    JS::Heap<JSObject *> m_Ref;
    uint64_t m_Id = 0;
};

struct NidiumLocalContext {

    NidiumLocalContext(JSRuntime *rt, JSContext *cx) : rt(rt),
                    cx(cx), ptr(nullptr) {
        
    }

    bool isShuttingDown() const {
        return m_IsShuttingDown;
    }

    void shutdown() {
        m_IsShuttingDown = true;
        m_Protocache.clear();
        m_RootedThings.clear();
        m_RootedThingsRef.clear();
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

        if (el.jsclass == nullptr) return nullptr;
        else return el.m_Proto;
    }

    uint64_t getUniqueId()
    {
        return ++m_UniqueId;
    }

    static NidiumLocalContext *Get();
    static void Init();

    static void InitJSThread(JSRuntime *rt, JSContext *cx);

    /*
        Automatically called when key is deleted by a thread
    */
    static void _destroy(void *data);

    static void _jstrace(JSTracer *trc, void *data);

    template<typename T>
    static void _jstraceMember(JSTracer *trc, T data);

    /*
        Rooting API
    */
    static void RootObjectUntilShutdown(JS::Heap<JSString *> &obj);
    static void RootObjectUntilShutdown(JS::Heap<JSObject *> &obj);
    static void RootObjectUntilShutdown(JS::Heap<JS::Value> &obj);
    static void RootObjectUntilShutdown(JS::TenuredHeap<JSObject *> &obj);
    static nidiumRootedThingRef *RootNonHeapObjectUntilShutdown(JSObject *obj);


    static void UnrootObject(nidiumRootedThingRef *ref)
    {
        if (!ref || ref->m_Id == 0) {
            return;
        }

        NidiumLocalContext *nlc = Get();

        nlc->m_RootedThingsRef.erase(ref->m_Id);
    }

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

    void *ptr;

    bool m_IsShuttingDown = false;
    Nidium::Core::Hash64<uintptr_t> m_JSUniqueInstance{64};

    std::unordered_map<JSClass *, nidiumProtoCacheElement>m_Protocache;
    std::unordered_map<uintptr_t, nidiumRootedThing>m_RootedThings;
    std::unordered_map<uint64_t, nidiumRootedThingRef>m_RootedThingsRef;

    uint64_t m_UniqueId = 0;
};

}
}

#endif

