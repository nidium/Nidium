/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "ThreadLocalContext.h"
#include <pthread.h>

namespace Nidium {
namespace Binding {

static pthread_key_t g_NidiumThreadContextKey;

NidiumLocalContext *NidiumLocalContext::Get()
{
    return static_cast<NidiumLocalContext *>(pthread_getspecific(g_NidiumThreadContextKey));
}

void NidiumLocalContext::Init()
{
    pthread_key_create(&g_NidiumThreadContextKey, NidiumLocalContext::_destroy);
}

void NidiumLocalContext::InitJSThread(JSContext *cx)
{
    NidiumLocalContext *nlc = new NidiumLocalContext(cx);
    pthread_setspecific(g_NidiumThreadContextKey, nlc);

    JS_AddExtraGCRootsTracer(cx, NidiumLocalContext::_jstrace, nlc);
}

void NidiumLocalContext::_destroy(void *data)
{
    NidiumLocalContext *nlc = static_cast<NidiumLocalContext *>(data);

    delete nlc;
}

void NidiumLocalContext::_jstrace(JSTracer *trc, void *data)
{
    NidiumLocalContext *nlc = static_cast<NidiumLocalContext *>(data);

    if (nlc->isShuttingDown()) {
        return;
    }

    _jstraceMember(trc, nlc->m_RootedThings);
    _jstraceMember(trc, nlc->m_RootedThingsRef);
}

template <typename T>
void NidiumLocalContext::_jstraceMember(JSTracer *trc, T map)
{

    for (auto &it : map) {
        nidiumRootedThing &thing = it.second;

        switch(thing.m_Type) {
            case nidiumRootedThing::Type::kHeapObj:
                JS_CallObjectTracer(trc, thing.heapobj, "nidiumrootHeapObj");
                break;
            case nidiumRootedThing::Type::kHeapValue:
                JS_CallValueTracer(trc, thing.heapvalue, "nidiumrootHeapValue");
                break;
            case nidiumRootedThing::Type::kHeapStr:
                JS_CallStringTracer(trc, thing.heapstr, "nidiumrootHeapString");
                break;
            case nidiumRootedThing::Type::kTenuredObj:
                JS_CallTenuredObjectTracer(trc, thing.tenuredobj, "nidiumrootTeanuredHeapObj");
                break;
            default:
            break;
        }
    }
}

nidiumRootedThingRef *NidiumLocalContext::RootNonHeapObjectUntilShutdown(JSObject *obj)
{
    NidiumLocalContext *nlc = Get();
    uint64_t uid = nlc->getUniqueId();

    nidiumRootedThingRef &thing = nlc->m_RootedThingsRef[uid];
    thing.m_Ref = obj;
    thing.m_Id  = uid;
    thing.m_Type = nidiumRootedThing::Type::kHeapObj;
    thing.heapobj = &thing.m_Ref;

    return &thing;
}

void NidiumLocalContext::RootObjectUntilShutdown(JS::Heap<JSObject *> &obj)
{
    uintptr_t addr = (uintptr_t)&obj;
    NidiumLocalContext *nlc = Get();

    nidiumRootedThing &thing = nlc->m_RootedThings[addr];

    thing.m_Type = nidiumRootedThing::Type::kHeapObj;
    thing.heapobj = &obj;
}

void NidiumLocalContext::RootObjectUntilShutdown(JS::Heap<JSString *> &obj)
{
    uintptr_t addr = (uintptr_t)&obj;
    NidiumLocalContext *nlc = Get();

    nidiumRootedThing &thing = nlc->m_RootedThings[addr];

    thing.m_Type = nidiumRootedThing::Type::kHeapStr;
    thing.heapstr = &obj;
}

void NidiumLocalContext::RootObjectUntilShutdown(JS::Heap<JS::Value> &obj)
{
    uintptr_t addr = (uintptr_t)&obj;
    NidiumLocalContext *nlc = Get();

    nidiumRootedThing &thing = nlc->m_RootedThings[addr];

    thing.m_Type = nidiumRootedThing::Type::kHeapValue;
    thing.heapvalue = &obj;
}

void NidiumLocalContext::RootObjectUntilShutdown(JS::TenuredHeap<JSObject *> &obj)
{
    uintptr_t addr = (uintptr_t)&obj;
    NidiumLocalContext *nlc = Get();

    nidiumRootedThing &thing = nlc->m_RootedThings[addr];

    thing.m_Type = nidiumRootedThing::Type::kTenuredObj;
    thing.tenuredobj = &obj;
}

}
}
