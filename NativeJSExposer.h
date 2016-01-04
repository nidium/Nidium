/*
    NativeJS Core Library
    Copyright (C) 2013 Anthony Catel <paraboul@gmail.com>
    Copyright (C) 2013 Nicolas Trani <n.trani@weelya.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef nativejsexposer_h__
#define nativejsexposer_h__

#include <jsapi.h>
#include <jsfriendapi.h>

#include "NativeJS.h"
#include "NativeTaskManager.h"

#define JSNATIVE_PROLOGUE(ofclass) \
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp); \
    JS::RootedObject thisobj(cx, JS_THIS_OBJECT(cx, vp)); \
    ofclass *CppObj = (ofclass *)JS_GetPrivate(thisobj); \
    args.rval().setUndefined();

#define JSNATIVE_PROLOGUE_CLASS(ofclass, fclass) \
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
    args.rval().setUndefined();

#define NATIVE_CHECK_ARGS(fnname, minarg) \
    if (argc < minarg) { \
                         \
        char numBuf[12];  \
        snprintf(numBuf, sizeof numBuf, "%u", argc);  \
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSMSG_MORE_ARGS_NEEDED,  \
                             fnname, numBuf, (argc > 1 ? "s" : ""));  \
        return false;  \
    }

static const JSClass NativeJSEvent_class = {
    "NativeJSEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

struct NativeJSEvent
{
    NativeJSEvent(JSContext *cx, JS::Value func) : m_Function(cx) {
        once = false;
        next = prev = NULL;

        m_Cx = cx;
        m_Function = func;

    }
    ~NativeJSEvent() {

    }

    JSContext *m_Cx;
    JS::PersistentRootedValue m_Function;

    bool once;

    NativeJSEvent *next;
    NativeJSEvent *prev;
};

class NativeJSEvents
{
public:
    static JSObject *CreateEventObject(JSContext *cx) {
        static JSFunctionSpec NativeJSEvents_funcs[] = {
            JS_FN("stopPropagation",
                NativeJSEvents::native_jsevents_stopPropagation, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT /*| JSPROP_READONLY*/),
            JS_FN("preventDefault",
                NativeJSEvents::native_jsevents_stub, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT /*| JSPROP_READONLY*/),
            JS_FN("forcePropagation",
                NativeJSEvents::native_jsevents_stub, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT /*| JSPROP_READONLY*/),
            JS_FS_END
        };

        JS::RootedObject ret(cx, JS_NewObject(cx, &NativeJSEvent_class, JS::NullPtr(), JS::NullPtr()));
        JS_DefineFunctions(cx, ret, NativeJSEvents_funcs);
        return ret;
    }

    NativeJSEvents(char *name) :
        m_Head(NULL), m_Queue(NULL), m_Name(strdup(name)) {}
    ~NativeJSEvents() {
        NativeJSEvent *ev, *tmpEv;
        for (ev = m_Head; ev != NULL;) {
            tmpEv = ev->next;
            free(ev);

            ev = tmpEv;
        }
        free(m_Name);
    }

    void add(NativeJSEvent *ev) {
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

    bool fire(JS::Value evobj, JSObject *thisobj) {
        NativeJSEvent *ev, *tmpEv;
        for (ev = m_Head; ev != NULL;) {
            JS::AutoValueArray<1> params(ev->m_Cx);
            params[0].set(evobj);
            JS::RootedValue rval(ev->m_Cx);
            // Use tmp in case the event was self deleted during trigger
            tmpEv = ev->next;
            JS::RootedObject obj(ev->m_Cx, thisobj);
            JS::RootedValue fun(ev->m_Cx, ev->m_Function);
            JS_CallFunctionValue(ev->m_Cx, obj, fun, params, &rval);

            ev = tmpEv;
        }
        return false;
    }

    NativeJSEvent *m_Head;
    NativeJSEvent *m_Queue;
    char *m_Name;
private:
    static bool native_jsevents_stopPropagation(JSContext *cx,
        unsigned argc, JS::Value *vp)
    {
        JS::RootedObject thisobj(cx, JS_THIS_OBJECT(cx, vp));
        if (!thisobj) {
            JS_ReportError(cx, "Illegal invocation");
            return false;
        }
        if (!JS_InstanceOf(cx, thisobj, &NativeJSEvent_class, NULL)) {
            JS_ReportError(cx, "Illegal invocation");
            return false;
        }
        JS::RootedValue cancelBubble(cx, JS::BooleanValue(true));
        JS_SetProperty(cx, thisobj, "cancelBubble", cancelBubble);

        return true;
    }
    static bool native_jsevents_stub(JSContext *cx,
        unsigned argc, JS::Value *vp)
    {

        return true;
    }
};

template <typename T>
class NativeJSExposer
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

    NativeJSExposer(JS::HandleObject jsobj, JSContext *cx, bool impEvents = true) :
        m_JSObject(jsobj), m_Cx(cx), m_Events(NULL)
    {
        static JSFunctionSpec NativeJSEvent_funcs[] = {
            JS_FN("addEventListener",
                NativeJSExposer<T>::native_jsevent_addEventListener, 2, JSPROP_ENUMERATE | JSPROP_PERMANENT /*| JSPROP_READONLY*/),
            JS_FN("fireEvent",
                NativeJSExposer<T>::native_jsevent_fireEvent, 2, JSPROP_ENUMERATE | JSPROP_PERMANENT /*| JSPROP_READONLY*/),
            JS_FS_END
        };

        if (impEvents) {
            JS_DefineFunctions(cx, jsobj, NativeJSEvent_funcs);
        }
    }

    virtual ~NativeJSExposer() {
        if (m_Events) {
            delete m_Events;
        }
    }

    static const char *getJSObjectName() { return NULL; }

    static JSObject *getJSGlobalObject(NativeJS *njs) {
        JSObject *jobj;
        const char *name = T::getJSObjectName();

        if ((jobj = njs->jsobjects.get(name)) == NULL) {
            return NULL;
        }

        return jobj;
    }

    static JSObject *getJSGlobalObject(JSContext *cx) {
        return T::getJSGlobalObject(NativeJS::getNativeClass(cx));
    }

    static T* getNativeClass(JSObject *obj, JSContext *cx = NULL)
    {
        if (cx != NULL) {
            if (JS_GetClass(obj) == T::jsclass) {
                return (T *)JS_GetPrivate(obj);
            }
            return NULL;
        }
        return (T *)JS_GetPrivate(obj);
    }

    static T* getNativeClass(NativeJS *njs) {
        JSObject *obj = T::getJSGlobalObject(njs);
        if (obj == NULL) {
            return NULL;
        }
        return (T *)JS_GetPrivate(obj);
    }

    static T* getNativeClass(JSContext *cx) {
        return T::getNativeClass(NativeJS::getNativeClass(cx));
    }

    bool fireJSEvent(const char *name, JS::MutableHandleValue evobj) {
        if (!m_Events) {
            return false;
        }
        /*
        if (0 && !JS_InstanceOf(m_Cx, evobj.toObjectOrNull(),
            &NativeJSEvent_class, NULL)) {
            evobj.setUndefined();
        }*/
        NativeJSEvents *events = m_Events->get(name);
        if (!events) {
            return false;
        }

        events->fire(evobj, m_JSObject);

        return true;
    }

  protected:
    void initEvents() {
        if (m_Events) {
            return;
        }

        m_Events = new NativeHash<NativeJSEvents *>(32);
        m_Events->setAutoDelete(true);
    }

    void addJSEvent(char *name, JS::Value func) {
        initEvents();

        NativeJSEvents *events = m_Events->get(name);
        if (!events) {
            events = new NativeJSEvents(name);
            m_Events->set(name, events);
        }

        NativeJSEvent *ev = new NativeJSEvent(m_Cx, func);
        events->add(ev);
    }

    JS::Heap<JSObject *>m_JSObject;

    JSContext *m_Cx;
    NativeHash<NativeJSEvents *> *m_Events;

    static JSClass *jsclass;
private:
    static bool native_jsevent_fireEvent(JSContext *cx,
        unsigned argc, JS::Value *vp)
    {
        JSNATIVE_PROLOGUE_CLASS(NativeJSExposer<T>, NativeJSExposer<T>::jsclass);

        NATIVE_CHECK_ARGS("fireEvent", 2);

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
    static bool native_jsevent_addEventListener(JSContext *cx,
        unsigned argc, JS::Value *vp)
    {
        JSNATIVE_PROLOGUE_CLASS(NativeJSExposer<T>, NativeJSExposer<T>::jsclass);

        NATIVE_CHECK_ARGS("addEventListener", 2);

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
};

#define NATIVE_ASYNC_MAXCALLBACK 4
class NativeJSAsyncHandler : public NativeManaged
{
public:
    NativeJSAsyncHandler(JSContext *ctx) :
        m_Ctx(ctx) {
        memset(m_CallBack, 0, sizeof(m_CallBack));
    }

    virtual ~NativeJSAsyncHandler() {
        if (m_Ctx == NULL) {
            return;
        }

        for (int i = 0; i < NATIVE_ASYNC_MAXCALLBACK; i++) {
            if (m_CallBack[i] != NULL) {
                NativeJS::getNativeClass(m_Ctx)->unrootObject(m_CallBack[i]);
            }
        }
    }

    void setCallback(int idx, JSObject *callback)
    {
        if (idx >= NATIVE_ASYNC_MAXCALLBACK || m_Ctx == NULL) {
            return;
        }

        if (m_CallBack[idx] != NULL) {
            NativeJS::getNativeClass(m_Ctx)->unrootObject(m_CallBack[idx]);
        }

        if (callback) {
            NativeJS::getNativeClass(m_Ctx)->rootObjectUntilShutdown(callback);
        }
        m_CallBack[idx] = callback;
    }

    JSObject *getCallback(int idx) const {
        if (idx >= NATIVE_ASYNC_MAXCALLBACK || m_Ctx == NULL) {
            return NULL;
        }

        return m_CallBack[idx];
    }

    JSContext *getJSContext() const {
        return m_Ctx;
    }

    virtual void onMessage(const NativeSharedMessages::Message &msg)=0;
private:
    JSContext *m_Ctx;
    JSObject *m_CallBack[NATIVE_ASYNC_MAXCALLBACK];
};

/*  TODO: add a way to define whether object life define JSObject life
    (addroot/unroot or if jsobject life define obj life (finalizer))
*/

template <typename T>
class NativeJSObjectMapper
{
public:
    NativeJSObjectMapper(JSContext *cx, const char *name) :
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
    virtual ~NativeJSObjectMapper()
    {
        JS_SetPrivate(m_JSObj, NULL);
    }

    JSObject *getJSObject() const {
        return m_JSObj;
    }
    /*
        TODO : need to check against instance
    */
    static T *getObject(JSObject *jsobj) {
        return (T *)JS_GetPrivate(jsobj);
    }
protected:
    JSClass *m_JSClass;

    JS::PersistentRootedObject m_JSObj;
    JSContext *m_JSCx;
};

typedef bool (*register_module_t)(JSContext *cx, JS::HandleObject exports);

#define NativeJSObj(cx) (NativeJS::getNativeClass(cx))

#define NATIVE_OBJECT_EXPOSE(name) \
    void NativeJS ## name::registerObject(JSContext *cx) \
    { \
        JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx)); \
        JS_InitClass(cx, global, JS::NullPtr(), &name ## _class, \
            native_ ## name ## _constructor, \
            0, NULL, NULL, NULL, NULL); \
    }

#define NATIVE_OBJECT_EXPOSE_NOT_INST(name) \
    void NativeJS ## name::registerObject(JSContext *cx) \
    { \
        JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx)); \
        JS::RootedObject name ## Obj(cx, JS_DefineObject(cx, global, #name, \
            &name ## _class , NULL, 0)); \
        JS_DefineFunctions(cx, name ## Obj, name ## _funcs); \
        JS_DefineProperties(cx, name ## Obj, name ## _props); \
    }

#define NATIVE_REGISTER_MODULE(constructor) \
    extern "C" __attribute__((__visibility__("default"))) bool __NativeRegisterModule(JSContext *cx, JS::HandleObject exports) \
    { \
        return constructor(cx, exports); \
    }

#define JSOBJ_SET_PROP_FLAGS(where, name, val, flags) JS_DefineProperty(m_Cx, where, \
    (const char *)name, val, flags)

#define JSOBJ_SET_PROP(where, name, val) JSOBJ_SET_PROP_FLAGS(where, name, val, \
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
#define JSOBJ_SET_PROP_CSTR(where, name, val) \
    { \
        JS::RootedString __n_rootedstring(m_Cx, JS_NewStringCopyZ(m_Cx, val)); \
        JSOBJ_SET_PROP(where, name, __n_rootedstring); \
    }

#define JSOBJ_SET_PROP_STR(where, name, val) JSOBJ_SET_PROP(where, name, val)
#define JSOBJ_SET_PROP_INT(where, name, val) JSOBJ_SET_PROP(where, name, val)

#define JS_INITOPT() JS::RootedValue __curopt(cx);

#define JSGET_OPT(obj, name) \
    if (obj && \
        JS_GetProperty(cx, obj, name, &__curopt) && \
        __curopt != JSVAL_VOID && \
        __curopt != JSVAL_NULL)
#define JSGET_OPT_TYPE(obj, name, type) \
    if (obj && \
        JS_GetProperty(cx, obj, name, &__curopt) && \
        __curopt != JSVAL_VOID && \
        __curopt != JSVAL_NULL && \
        __curopt.is ## type())

class NativeJSObjectBuilder
{
public:
    NativeJSObjectBuilder(JSContext *cx, JSClass *clasp = NULL) : m_Obj(cx) {
        m_Cx = cx;
        m_Obj = JS_NewObject(m_Cx, clasp, JS::NullPtr(), JS::NullPtr());
    };

    NativeJSObjectBuilder(JSContext *cx, JSObject *wrapped) : m_Obj(cx) {
        m_Obj = wrapped;
        m_Cx = cx;
    };

    void set (const char *name, JS::HandleValue jval) {
        JS::RootedObject obj(m_Cx, m_Obj);
        JSOBJ_SET_PROP(obj, name, jval);
    }

    void set(const char *name, JS::HandleString value) {
        JS::RootedObject obj(m_Cx, m_Obj);
        JSOBJ_SET_PROP_STR(obj, name, value);
    }

    void set(const char *name, JSString *str) {
        printf("NativeJSObjectBuilder using a JSString is deprecated\n");
        exit(1);
    }

    void set(const char *name, const char *value) {
        JS::RootedObject obj(m_Cx, m_Obj);
        JSOBJ_SET_PROP_CSTR(obj, name, value);
    }

    void set(const char *name, uint32_t value) {
        JS::RootedObject obj(m_Cx, m_Obj);
        JSOBJ_SET_PROP_INT(obj, name, value);
    }

    void set(const char *name, int32_t value) {
        JS::RootedObject obj(m_Cx, m_Obj);
        JSOBJ_SET_PROP_INT(obj, name, value);
    }

    void set(const char *name, double value) {
        JS::RootedObject obj(m_Cx, m_Obj);
        JSOBJ_SET_PROP(obj, name, value);
    }

    void set(const char *name, bool value) {
        JS::RootedObject obj(m_Cx, m_Obj);
        JSOBJ_SET_PROP(obj, name, value);
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

    ~NativeJSObjectBuilder() {};

private:
    JS::PersistentRootedObject m_Obj;
    JSContext *m_Cx;
};

/*
    Tinyid were removed in SM31.
    This template act as a workaround (create a unique getter/setter and keep a unique identifier)
*/
#define NATIVE_JS_SETTER(tinyid, setter) \
    {JS_CAST_NATIVE_TO((NativeJSPropertyAccessors::Setter<tinyid, setter>), JSStrictPropertyOp), nullptr}
#define NATIVE_JS_GETTER(tinyid, getter) \
    {JS_CAST_NATIVE_TO((NativeJSPropertyAccessors::Getter<tinyid, getter>), JSPropertyOp), nullptr}
#define NATIVE_JS_STUBGETTER() \
    {JS_CAST_NATIVE_TO((NativeJSPropertyAccessors::NullGetter), JSPropertyOp), nullptr}

struct NativeJSPropertyAccessors
{
    typedef bool
    (* NativeJSGetterOp)(JSContext *cx, JS::HandleObject obj, uint8_t id,
                           bool strict, JS::MutableHandleValue vp);

    typedef bool
    (* NativeJSSetterOp)(JSContext *cx, JS::HandleObject obj, uint8_t id,
                           JS::MutableHandleValue vp);

    template <uint8_t TINYID, NativeJSGetterOp FN>
    static bool Setter(JSContext *cx, unsigned argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        JS::RootedObject obj(cx, JS_THIS_OBJECT(cx, vp));

        if (!obj) return false;
        JS::RootedValue val(cx, args.get(0));
        bool ret = FN(cx, obj, TINYID, true, &val);

        args.rval().set(val);

        return ret;
    }

    template <uint8_t TINYID, NativeJSSetterOp FN>
    static bool Getter(JSContext *cx, unsigned argc, JS::Value *vp) {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        JS::RootedObject obj(cx, JS_THIS_OBJECT(cx, vp));

        if (!obj) return false;

        return FN(cx, obj, TINYID, args.rval());
    }

    static bool NullGetter(JSContext *cx, unsigned argc, JS::Value *vp) {
        return true;
    }

};

#endif

