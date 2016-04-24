/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsexposer_h__
#define binding_jsexposer_h__

#include <jsapi.h>
#include <jsfriendapi.h>

#include "Core/TaskManager.h"

#include "NidiumJS.h"

// {{{ Macros
// {{{ JSClass macro's
#define NIDIUM_JS_PROLOGUE(ofclass) \
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp); \
    JS::RootedObject thisobj(cx, JS_THIS_OBJECT(cx, vp)); \
    ofclass *CppObj = (ofclass *)JS_GetPrivate(thisobj); \
    args.rval().setUndefined();

#define NIDIUM_JS_PROLOGUE_CLASS_NO_RET(ofclass, fclass) \
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp); \
    JS::RootedObject thisobj(cx, JS_THIS_OBJECT(cx, vp)); \
    if (!thisobj) { \
        JS_ReportError(cx, "Illegal invocation"); \
        return false; \
    } \
    ofclass *CppObj = (ofclass *)JS_GetInstancePrivate(cx, thisobj, fclass, NULL); \
    if (!CppObj) { \
        JS_ReportError(cx, "Illegal invocation"); \
        return false; \
    } \

#define NIDIUM_JS_PROLOGUE_CLASS(ofclass, fclass) \
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(ofclass, fclass) \
    args.rval().setUndefined();

#define NIDIUM_JS_CHECK_ARGS(fnname, minarg) \
    if (argc < minarg) { \
                         \
        char numBuf[12];  \
        snprintf(numBuf, sizeof numBuf, "%u", argc);  \
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSMSG_MORE_ARGS_NEEDED,  \
                             fnname, numBuf, (argc > 1 ? "s" : ""));  \
        return false;  \
    }
// }}}
// {{{ Object macro's
typedef bool (*register_module_t)(JSContext *cx, JS::HandleObject exports);

#define NidiumJSObj(cx) (Nidium::Binding::NidiumJS::GetObject(cx))

#define NIDIUM_JS_OBJECT_EXPOSE(name) \
    void  JS ## name::RegisterObject(JSContext *cx) \
    { \
        JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx)); \
        JS_InitClass(cx, global, JS::NullPtr(), &name ## _class, \
            nidium_ ## name ## _constructor, \
            0, NULL, NULL, NULL, NULL); \
    }

#define NIDIUM_JS_OBJECT_EXPOSE_NOT_INST(name) \
    void NidiumJS ## name::RegisterObject(JSContext *cx) \
    { \
        JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx)); \
        JS::RootedObject name ## Obj(cx, JS_DefineObject(cx, global, #name, \
            &name ## _class , NULL, 0)); \
        JS_DefineFunctions(cx, name ## Obj, name ## _funcs); \
        JS_DefineProperties(cx, name ## Obj, name ## _props); \
    }

#define NATIVE_OBJECT_EXPOSE(name) \
    void NativeJS ## name::RegisterObject(JSContext *cx) \
    { \
        JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx)); \
        JS_InitClass(cx, global, JS::NullPtr(), &name ## _class, \
            native_ ## name ## _constructor, \
            0, NULL, NULL, NULL, NULL); \
    }

#define NATIVE_OBJECT_EXPOSE_NOT_INST(name) \
    void NativeJS ## name::RegisterObject(JSContext *cx) \
    { \
        JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx)); \
        JS::RootedObject name ## Obj(cx, JS_DefineObject(cx, global, #name, \
            &name ## _class , NULL, 0)); \
        JS_DefineFunctions(cx, name ## Obj, name ## _funcs); \
        JS_DefineProperties(cx, name ## Obj, name ## _props); \
    }

#define NIDIUM_JS_REGISTER_MODULE(constructor) \
    extern "C" __attribute__((__visibility__("default"))) bool __NativeRegisterModule(JSContext *cx, JS::HandleObject exports) \
    { \
        return constructor(cx, exports); \
    }

#define NIDIUM_JSOBJ_SET_PROP_FLAGS(where, name, val, flags) JS_DefineProperty(m_Cx, where, \
    (const char *)name, val, flags)

#define NIDIUM_JSOBJ_SET_PROP(where, name, val) NIDIUM_JSOBJ_SET_PROP_FLAGS(where, name, val, \
        JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

#define JSOBJ_CALLFUNCNAME(where, name, argv) \
    { \
        JS::RootedValue _oncallback(cx); \
        JS::RootedValue _rval(cx); \
        JS::RootedValue rval(cx); \
        if (JS_GetProperty(cx, where, name, &_oncallback) && \
            JS_TypeOfValue(cx, _oncallback) == JSTYPE_FUNCTION) { \
            JS_CallFunctionValue(cx, where, _oncallback, \
                argv, &_rval); \
        } \
    }
#define NIDIUM_JSOBJ_SET_PROP_CSTR(where, name, val) \
    { \
        JS::RootedString __n_rootedstring(m_Cx, JS_NewStringCopyZ(m_Cx, val)); \
        NIDIUM_JSOBJ_SET_PROP(where, name, __n_rootedstring); \
    }

#define NIDIUM_JSOBJ_SET_PROP_STR(where, name, val) NIDIUM_JSOBJ_SET_PROP(where, name, val)
#define NIDIUM_JSOBJ_SET_PROP_INT(where, name, val) NIDIUM_JSOBJ_SET_PROP(where, name, val)

#define NIDIUM_JS_INIT_OPT() JS::RootedValue __curopt(cx);

#define NIDIUM_JS_GET_OPT(obj, name) \
    if (obj && \
        JS_GetProperty(cx, obj, name, &__curopt) && \
        __curopt != JSVAL_VOID && \
        __curopt != JSVAL_NULL)
#define NIDIUM_JS_GET_OPT_TYPE(obj, name, type) \
    if (obj && \
        JS_GetProperty(cx, obj, name, &__curopt) && \
        __curopt != JSVAL_VOID && \
        __curopt != JSVAL_NULL && \
        __curopt.is ## type())
// }}}
// {{{ Getter / Setter macro's
/*
    Tinyid were removed in SM31.
    This template act as a workaround (create a unique getter/setter and keep a unique identifier)
*/
#define NIDIUM_JS_SETTER(tinyid, setter) \
    {{JS_CAST_NATIVE_TO((JSPropertyAccessors::Setter<tinyid, setter>), JSStrictPropertyOp), nullptr}}
#define NIDIUM_JS_SETTER_WRS(tinyid, setter) \
    {{JS_CAST_NATIVE_TO((JSPropertyAccessors::SetterWithReservedSlot<tinyid, setter>), JSStrictPropertyOp), nullptr}}
#define NIDIUM_JS_GETTER(tinyid, getter) \
    {{JS_CAST_NATIVE_TO((JSPropertyAccessors::Getter<tinyid, getter>), JSPropertyOp), nullptr}}
#define NIDIUM_JS_STUBGETTER(tinyid) \
    {{JS_CAST_NATIVE_TO((JSPropertyAccessors::NullGetter<tinyid>), JSPropertyOp), nullptr}}

/* Getter only */
#define NIDIUM_JS_PSG(name, tinyid, getter_func) \
    {name, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS, \
        NIDIUM_JS_GETTER(tinyid, getter_func), \
        JSOP_NULLWRAPPER}

/* Setter only */
#define NIDIUM_JS_PSS(name, tinyid, setter_func) \
    {name, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS, \
        NIDIUM_JS_STUBGETTER(tinyid), \
        NIDIUM_JS_SETTER_WRS(tinyid, setter_func)}

/* Both */
#define NIDIUM_JS_PSGS(name, tinyid, getter_func, setter_func) \
    {name, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS, \
        NIDIUM_JS_GETTER(tinyid, getter_func), \
        NIDIUM_JS_SETTER(tinyid, setter_func)}
// }}}
// }}}

namespace Nidium {
namespace Binding {

// {{{ JSEvent
static const JSClass JSEvent_class = {
    "NidiumEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

struct JSEvent
{
    JSEvent(JSContext *cx, JS::HandleValue func) : m_Function(func) {
        once = false;
        next = prev = NULL;

        m_Cx = cx;

        NidiumJS::GetObject(m_Cx)->rootObjectUntilShutdown(func.toObjectOrNull());
    }

    ~JSEvent() {
        NidiumJS::GetObject(m_Cx)->unrootObject(m_Function.toObjectOrNull());
    }

    JSContext *m_Cx;
    JS::Heap<JS::Value> m_Function;

    bool once;

    JSEvent *next;
    JSEvent *prev;
};
// }}}

// {{{ JSEvents
class JSEvents
{
public:
    static JSObject *CreateEventObject(JSContext *cx) {
        static JSFunctionSpec JSEvents_funcs[] = {
            JS_FN("stopPropagation",
                JSEvents::Nidium_jsevents_stopPropagation, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT /*| JSPROP_READONLY*/),
            JS_FN("preventDefault",
                JSEvents::Nidium_jsevents_stub, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT /*| JSPROP_READONLY*/),
            JS_FN("forcePropagation",
                JSEvents::Nidium_jsevents_stub, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT /*| JSPROP_READONLY*/),
            JS_FS_END
        };

        JS::RootedObject ret(cx, JS_NewObject(cx, &JSEvent_class, JS::NullPtr(), JS::NullPtr()));
        JS_DefineFunctions(cx, ret, JSEvents_funcs);
        return ret;
    }

    JSEvents(char *name) :
        m_Head(NULL), m_Queue(NULL), m_Name(strdup(name)),
        m_TmpEv(NULL), m_IsFiring(false), m_DeleteAfterFire(false) {}

    ~JSEvents() {
        JSEvent *ev, *tmpEv;
        for (ev = m_Head; ev != NULL;) {
            tmpEv = ev->next;
            delete ev;

            ev = tmpEv;
        }
        free(m_Name);
    }

    void add(JSEvent *ev) {
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

    bool fire(JS::HandleValue evobj, JSObject *thisobj) {
        JSEvent *ev;
        JSContext *cx;
        m_IsFiring = true;

        for (ev = m_Head; ev != NULL;) {
            JS::AutoValueArray<1> params(ev->m_Cx);
            JS::RootedValue rval(ev->m_Cx);
            JS::RootedObject obj(ev->m_Cx, thisobj);
            JS::RootedValue fun(ev->m_Cx, ev->m_Function);

            params[0].set(evobj);

            /*
                Keep a reference to the next event in case the event is self
                deleted during the trigger. In case the next event(s) are also
                deleted, m_TmpEv will be updated by JSEvents::remove()
                to the next valid event.
            */
            m_TmpEv = ev->next;
            cx = ev->m_Cx;

            JS_CallFunctionValue(ev->m_Cx, obj, fun, params, &rval);

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
        m_TmpEv = nullptr;

        return false;
    }

    void remove() {
        if (m_IsFiring) {
            m_DeleteAfterFire = true;
        } else {
            delete this;
        }
    }

    void remove(JS::HandleValue func) {
        JSEvent *ev;
        for (ev = m_Head; ev != nullptr;) {
            if (ev->m_Function == func) {
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

    JSEvent *m_Head;
    JSEvent *m_Queue;
    char *m_Name;

private:
    JSEvent *m_TmpEv;
    bool m_IsFiring;
    bool m_DeleteAfterFire;

    static bool Nidium_jsevents_stopPropagation(JSContext *cx,
        unsigned argc, JS::Value *vp)
    {
        JS::RootedObject thisobj(cx, JS_THIS_OBJECT(cx, vp));
        if (!thisobj) {
            JS_ReportError(cx, "Illegal invocation");
            return false;
        }
        if (!JS_InstanceOf(cx, thisobj, &JSEvent_class, NULL)) {
            JS_ReportError(cx, "Illegal invocation");
            return false;
        }
        JS::RootedValue cancelBubble(cx, JS::BooleanValue(true));
        JS_SetProperty(cx, thisobj, "cancelBubble", cancelBubble);

        return true;
    }
    static bool Nidium_jsevents_stub(JSContext *cx,
        unsigned argc, JS::Value *vp)
    {

        return true;
    }
};
// }}}

// {{{ JSExposer
template <typename T>
class JSExposer
{
  public:
    JSObject *getJSObject() const {
        return m_JSObject;
    }

    JSObject **getJSObjectAddr()  {
        return &m_JSObject;
    }

    JSContext *getJSContext() const {
        return m_Cx;
    }

    void setJSObject(JSObject *obj) {
        m_JSObject = obj;
    }

    void setJSContext(JSContext *cx) {
        m_Cx = cx;
    }

    JSExposer(JS::HandleObject jsobj, JSContext *cx, bool impEvents = true) :
        m_JSObject(jsobj), m_Cx(cx), m_Events(NULL)
    {
        static JSFunctionSpec JSEvent_funcs[] = {
            JS_FN("addEventListener",
                JSExposer<T>::Nidium_jsevent_addEventListener, 2, JSPROP_ENUMERATE | JSPROP_PERMANENT /*| JSPROP_READONLY*/),
            JS_FN("removeEventListener",
                JSExposer<T>::Nidium_jsevent_removeEventListener, 1, JSPROP_ENUMERATE | JSPROP_PERMANENT),
            JS_FN("fireEvent",
                JSExposer<T>::Nidium_jsevent_fireEvent, 2, JSPROP_ENUMERATE | JSPROP_PERMANENT /*| JSPROP_READONLY*/),
            JS_FS_END
        };

        if (impEvents) {
            JS_DefineFunctions(cx, jsobj, JSEvent_funcs);
        }
    }

    virtual ~JSExposer() {
        if (m_Events) {
            /*
                It's safe to enable again the auto delete feature from
                Nidium::Core::Hash since we are sure at this point that no event
                is currently fired.
            */
            m_Events->setAutoDelete(true);
            delete m_Events;
        }
    }

    static const char *GetJSObjectName() { return NULL; }

    static JSObject *GetJSGlobalObject(NidiumJS *njs) {
        JSObject *jobj;
        const char *name = T::GetJSObjectName();

        if ((jobj = njs->jsobjects.get(name)) == NULL) {
            return NULL;
        }

        return jobj;
    }

    static JSObject *GetJSGlobalObject(JSContext *cx) {
        return T::GetJSGlobalObject(NidiumJS::GetObject(cx));
    }

    static T* GetObject(JSObject *obj, JSContext *cx = NULL)
    {
        if (cx != NULL) {
            if (JS_GetClass(obj) == T::jsclass) {
                return (T *)JS_GetPrivate(obj);
            }
            return NULL;
        }
        return (T *)JS_GetPrivate(obj);
    }

    static T* GetObject(NidiumJS *njs) {
        JSObject *obj = T::GetJSGlobalObject(njs);
        if (obj == NULL) {
            return NULL;
        }
        return (T *)JS_GetPrivate(obj);
    }

    static T* GetObject(JSContext *cx) {
        return T::GetObject(NidiumJS::GetObject(cx));
    }

    bool fireJSEvent(const char *name, JS::MutableHandleValue evobj) {
        if (!m_Events) {
            return false;
        }
        /*
        if (0 && !JS_InstanceOf(m_Cx, evobj.toObjectOrNull(),
            &JSEvent_class, NULL)) {
            evobj.setUndefined();
        }*/
        JSEvents *events = m_Events->get(name);
        if (!events) {
            return false;
        }

        events->fire(evobj, m_JSObject);

        return true;
    }

    void removeJSEvent(char *name) {
        if (!m_Events) {
            return;
        }

        JSEvents *events = m_Events->get(name);
        if (!events) {
            return;
        }

        m_Events->erase(name);
        events->remove();
    }

    void removeJSEvent(char *name, JS::HandleValue func) {
        if (!m_Events) {
            return;
        }

        JSEvents *events = m_Events->get(name);
        if (!events) {
            return;
        }

        events->remove(func);
    }

  protected:
    void initEvents() {
        if (m_Events) {
            return;
        }

        m_Events = new Nidium::Core::Hash<JSEvents *>(32);
        /*
            Set Nidium::Core::Hash auto delete to false, since it's possible for
            an event to be deleted while it's fired. So we don't want to
            free the underlying object when removing it from the Nidium::Core::Hash
            (otherwise JSEvents::fire will attempt to use a freed object)
        */
        m_Events->setAutoDelete(false);
    }

    void addJSEvent(char *name, JS::HandleValue func) {
        initEvents();

        JSEvents *events = m_Events->get(name);
        if (!events) {
            events = new JSEvents(name);
            m_Events->set(name, events);
        }

        JSEvent *ev = new JSEvent(m_Cx, func);
        events->add(ev);
    }

    JS::Heap<JSObject *>m_JSObject;

    JSContext *m_Cx;
    Nidium::Core::Hash<JSEvents *> *m_Events;

    static JSClass *jsclass;
private:
    static bool Nidium_jsevent_fireEvent(JSContext *cx,
        unsigned argc, JS::Value *vp)
    {
        NIDIUM_JS_PROLOGUE_CLASS(JSExposer<T>, JSExposer<T>::jsclass);

        NIDIUM_JS_CHECK_ARGS("fireEvent", 2);

        if (!CppObj->m_Events) {
            return true;
        }

        JS::RootedString name(cx);
        JS::RootedObject evobj(cx);

        if (!JS_ConvertArguments(cx, args, "So", name.address(), evobj.address())) {
            return false;
        }

        if (!evobj) {
            JS_ReportError(cx, "Invalid event object");
            return false;
        }

        JSAutoByteString cname(cx, name);
        JS::RootedValue evjsobj(cx, JS::ObjectValue(*evobj));

        CppObj->fireJSEvent(cname.ptr(), &evjsobj);

        return true;
    }
    static bool Nidium_jsevent_addEventListener(JSContext *cx,
        unsigned argc, JS::Value *vp)
    {
        NIDIUM_JS_PROLOGUE_CLASS(JSExposer<T>, JSExposer<T>::jsclass);

        NIDIUM_JS_CHECK_ARGS("addEventListener", 2);

        JS::RootedString name(cx);
        JS::RootedValue cb(cx);

        if (!JS_ConvertArguments(cx, args, "S", name.address())) {
            return false;
        }

        if (!JS_ConvertValue(cx, args[1], JSTYPE_FUNCTION, &cb)) {
            JS_ReportError(cx, "Bad callback given");
            return false;
        }

        JSAutoByteString cname(cx, name);

        CppObj->addJSEvent(cname.ptr(), cb);

        return true;
    }

    static bool Nidium_jsevent_removeEventListener(JSContext *cx,
        unsigned argc, JS::Value *vp)
    {
        NIDIUM_JS_PROLOGUE_CLASS(JSExposer<T>, JSExposer<T>::jsclass);

        NIDIUM_JS_CHECK_ARGS("removeEventListener", 1);

        JS::RootedString name(cx);
        JS::RootedValue cb(cx);

        if (!JS_ConvertArguments(cx, args, "S", name.address())) {
            return false;
        }

        JSAutoByteString cname(cx, name);

        if (argc == 1) {
            CppObj->removeJSEvent(cname.ptr());
        } else {
            if (!JS_ConvertValue(cx, args[1], JSTYPE_FUNCTION, &cb)) {
                JS_ReportError(cx, "Bad callback given");
                return false;
            }

            CppObj->removeJSEvent(cname.ptr(), cb);
        }

        return true;
    }
};
// }}}

// {{{ JSAsyncHandler
#define NIDIUM_ASYNC_MAXCALLBACK 4
class JSAsyncHandler : public Nidium::Core::Managed
{
public:
    JSAsyncHandler(JSContext *ctx) :
        m_Ctx(ctx) {
        memset(m_CallBack, 0, sizeof(m_CallBack));
    }

    virtual ~JSAsyncHandler() {
        if (m_Ctx == NULL) {
            return;
        }

        for (int i = 0; i < NIDIUM_ASYNC_MAXCALLBACK; i++) {
            if (m_CallBack[i] != NULL) {
                NidiumJS::GetObject(m_Ctx)->unrootObject(m_CallBack[i]);
            }
        }
    }

    void setCallback(int idx, JSObject *callback)
    {
        if (idx >= NIDIUM_ASYNC_MAXCALLBACK || m_Ctx == NULL) {
            return;
        }

        if (m_CallBack[idx] != NULL) {
            NidiumJS::GetObject(m_Ctx)->unrootObject(m_CallBack[idx]);
        }

        if (callback) {
            NidiumJS::GetObject(m_Ctx)->rootObjectUntilShutdown(callback);
        }
        m_CallBack[idx] = callback;
    }

    JSObject *getCallback(int idx) const {
        if (idx >= NIDIUM_ASYNC_MAXCALLBACK || m_Ctx == NULL) {
            return NULL;
        }

        return m_CallBack[idx];
    }

    JSContext *getJSContext() const {
        return m_Ctx;
    }

    virtual void onMessage(const Nidium::Core::SharedMessages::Message &msg)=0;
private:
    JSContext *m_Ctx;
    JSObject *m_CallBack[NIDIUM_ASYNC_MAXCALLBACK];
};
// }}}

/*  TODO: add a way to define whether object life define JSObject life
    (addroot/unroot or if jsobject life define obj life (finalizer))
*/

// {{{ JSObjectMapper
template <typename T>
class JSObjectMapper
{
public:
    JSObjectMapper(JSContext *cx, const char *name) :
        m_JSObj(cx), m_JSCx(cx)
    {
        static JSClass jsclass = {
            NULL, JSCLASS_HAS_PRIVATE,
            JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
            JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JSCLASS_NO_OPTIONAL_MEMBERS
        };

        if (jsclass.name == NULL) {
            jsclass.name = name;
        }

        m_JSClass = &jsclass;

        m_JSObj = JS_NewObject(m_JSCx, m_JSClass, JS::NullPtr(), JS::NullPtr());
        JS_SetPrivate(m_JSObj, static_cast<T *>(this));
    }
    virtual ~JSObjectMapper()
    {
        JS_SetPrivate(m_JSObj, NULL);
    }

    JSObject *getJSObject() const {
        return m_JSObj;
    }
    /*
        TODO : need to check against instance
    */
    static T *GetObject(JSObject *jsobj) {
        return (T *)JS_GetPrivate(jsobj);
    }
protected:
    JSClass *m_JSClass;

    JS::PersistentRootedObject m_JSObj;
    JSContext *m_JSCx;
};
// }}}

// {{{ JSObjectBuilder
class JSObjectBuilder
{
public:
    JSObjectBuilder(JSContext *cx, JSClass *clasp = NULL) : m_Obj(cx) {
        m_Cx = cx;
        m_Obj = JS_NewObject(m_Cx, clasp, JS::NullPtr(), JS::NullPtr());
    };

    JSObjectBuilder(JSContext *cx, JS::HandleObject wrapped) : m_Obj(cx) {
        m_Obj = wrapped;
        m_Cx = cx;
    };

    void set (const char *name, JS::HandleValue jval) {
        JS::RootedObject obj(m_Cx, m_Obj);
        NIDIUM_JSOBJ_SET_PROP(obj, name, jval);
    }

    void set(const char *name, JS::HandleString value) {
        JS::RootedObject obj(m_Cx, m_Obj);
        NIDIUM_JSOBJ_SET_PROP_STR(obj, name, value);
    }

    void set(const char *name, JSString *str) {
        printf("JSObjectBuilder using a JSString is deprecated\n");
        exit(1);
    }

    void set(const char *name, const char *value) {
        JS::RootedObject obj(m_Cx, m_Obj);
        NIDIUM_JSOBJ_SET_PROP_CSTR(obj, name, value);
    }

    void set(const char *name, uint32_t value) {
        JS::RootedObject obj(m_Cx, m_Obj);
        NIDIUM_JSOBJ_SET_PROP_INT(obj, name, value);
    }

    void set(const char *name, int32_t value) {
        JS::RootedObject obj(m_Cx, m_Obj);
        NIDIUM_JSOBJ_SET_PROP_INT(obj, name, value);
    }

    void set(const char *name, double value) {
        JS::RootedObject obj(m_Cx, m_Obj);
        NIDIUM_JSOBJ_SET_PROP(obj, name, value);
    }

    void set(const char *name, bool value) {
        JS::RootedObject obj(m_Cx, m_Obj);
        NIDIUM_JSOBJ_SET_PROP(obj, name, value);
    }

    JSObject *obj() const {
        return m_Obj;
    }

    JS::Value jsval() const {
        return OBJECT_TO_JSVAL(m_Obj);
    }

    operator JSObject*() {
        return m_Obj;
    }

    operator JS::Value() {
        return OBJECT_TO_JSVAL(m_Obj);
    }

    ~JSObjectBuilder() {};

private:
    JS::PersistentRootedObject m_Obj;
    JSContext *m_Cx;
};
// }}}

// {{{ JSPropertyAccessors
struct JSPropertyAccessors
{
    typedef bool
    (* JSGetterOp)(JSContext *cx, JS::HandleObject obj, uint8_t id,
                           bool strict, JS::MutableHandleValue vp);

    typedef bool
    (* JSSetterOp)(JSContext *cx, JS::HandleObject obj, uint8_t id,
                           JS::MutableHandleValue vp);

    template <uint8_t TINYID, JSGetterOp FN>
    static bool Setter(JSContext *cx, unsigned argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        JS::RootedObject obj(cx, JS_THIS_OBJECT(cx, vp));

        if (!obj) return false;
        JS::RootedValue val(cx, args.get(0));
        bool ret = FN(cx, obj, TINYID, true, &val);

        args.rval().set(val);

        return ret;
    }

    template <uint8_t TINYID, JSGetterOp FN>
    static bool SetterWithReservedSlot(JSContext *cx, unsigned argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        JS::RootedObject obj(cx, JS_THIS_OBJECT(cx, vp));

        if (!obj) return false;
        JS::RootedValue val(cx, args.get(0));
        bool ret = FN(cx, obj, TINYID, true, &val);
#if 0
        args.rval().set(val);
#endif
        /* We need this to be sure that the value set is properly rooted */
        JS_SetReservedSlot(obj, TINYID, val);

        return ret;
    }

    template <uint8_t TINYID, JSSetterOp FN>
    static bool Getter(JSContext *cx, unsigned argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        JS::RootedObject obj(cx, JS_THIS_OBJECT(cx, vp));

        if (!obj) return false;

        return FN(cx, obj, TINYID, args.rval());
    }

    template <uint8_t TINYID>
    static bool NullGetter(JSContext *cx, unsigned argc, JS::Value *vp) {

        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        JS::RootedObject obj(cx, JS_THIS_OBJECT(cx, vp));

        if (!obj) return false;

        args.rval().set(JS_GetReservedSlot(obj, TINYID));

        return true;
    }

};
// }}}

} // namespace Binding
} // namespace Nidium

#endif

