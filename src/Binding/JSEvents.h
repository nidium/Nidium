/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsevents_h__
#define binding_jsevents_h__

#include <jsapi.h>

#include "Binding/NidiumJS.h"
#include "Binding/ThreadLocalContext.h"
#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

struct JSEventListener
{
    JSEventListener(JSContext *cx, JS::HandleValue func) : m_Function(func)
    {
        m_Once = false;
        next = prev = NULL;

        m_Cx = cx;

        NidiumLocalContext::RootObjectUntilShutdown(m_Function);
    }

    ~JSEventListener()
    {
        NidiumLocalContext::UnrootObject(m_Function);
    }

    JSContext *m_Cx;
    JS::Heap<JS::Value> m_Function;

    bool m_Once;

    JSEventListener *next;
    JSEventListener *prev;
};

class JSEvent : public ClassMapper<JSEvent>
{
public:
    static void RegisterObject(JSContext *cx);
    static JSFunctionSpec *ListMethods();
protected:

    NIDIUM_DECL_JSCALL(stopPropagation);
    NIDIUM_DECL_JSCALL(forcePropagation);
    NIDIUM_DECL_JSCALL(preventDefault);
};

class JSEvents
{
public:
    static JSObject *CreateEventObject(JSContext *cx);
    static JSObject *
    CreateErrorEventObject(JSContext *cx, int code, const char *error);

    JSEvents(char *name)
        : m_Head(NULL), m_Queue(NULL), m_Name(strdup(name)), m_TmpEv(NULL),
          m_IsFiring(false), m_DeleteAfterFire(false)
    {
    }

    ~JSEvents()
    {
        JSEventListener *ev, *tmpEv;
        for (ev = m_Head; ev != NULL;) {
            tmpEv = ev->next;
            delete ev;

            ev = tmpEv;
        }
        free(m_Name);
    }

    void add(JSEventListener *ev)
    {
        ev->prev = m_Queue;
        ev->next = NULL;

        if (m_Head == NULL) {
            m_Head = ev;
        }

        if (m_Queue) {
            m_Queue->next = ev;
        }

        m_Queue = ev;
    }

    bool fire(JSContext *cx, JS::HandleValue evobj, JS::HandleObject obj)
    {
        JSEventListener *ev;
        m_IsFiring = true;

        JS::RootedObject thisobj(cx, obj);
        JS::AutoValueArray<1> params(cx);
        params[0].set(evobj);

        for (ev = m_Head; ev != NULL;) {
            JS::RootedValue rval(cx);
            JS::RootedValue fun(cx, ev->m_Function);

            /*
                Keep a reference to the next event in case the event is self
                deleted during the trigger. In case the next event(s) are also
                deleted, m_TmpEv will be updated by JSEvents::remove()
                to the next valid event.
            */
            m_TmpEv = ev->next;

            JS_CallFunctionValue(cx, thisobj, fun, params, &rval);

            if (JS_IsExceptionPending(cx)) {
                if (!JS_ReportPendingException(cx)) {
                    JS_ClearPendingException(cx);
                }
            }

            if (m_DeleteAfterFire) {
                delete this;
                return false;
            }

            ev = m_TmpEv;
        }

        m_IsFiring = false;
        m_TmpEv    = nullptr;

        return false;
    }

    void remove()
    {
        if (m_IsFiring) {
            m_DeleteAfterFire = true;
        } else {
            delete this;
        }
    }

    void remove(JS::HandleValue func)
    {
        JSEventListener *ev;
        for (ev = m_Head; ev != nullptr;) {
            if (ev->m_Function.address() == func.address()) {
                if (ev->prev) {
                    ev->prev->next = ev->next;
                } else {
                    m_Head = nullptr;
                }

                if (ev->next) {
                    ev->next->prev = ev->prev;
                } else {
                    m_Queue = ev->prev;
                }

                /*
                    The event currently deleted, is the next to be
                    fired, update the pointer to the next event.
                */
                if (ev == m_TmpEv) {
                    m_TmpEv = ev->next;
                }

                delete ev;

                return;
            }
        }
    }

    JSEventListener *m_Head;
    JSEventListener *m_Queue;
    char *m_Name;

private:
    JSEventListener *m_TmpEv;
    bool m_IsFiring;
    bool m_DeleteAfterFire;
};

} // namespace Binding
} // namespace Nidium

#endif
