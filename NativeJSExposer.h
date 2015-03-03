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
    JS::CallArgs args = CallArgsFromVp(argc, vp); \
    JS::RootedObject thisobj(cx, JS_THIS_OBJECT(cx, vp)); \
    ofclass *CppObj = (ofclass *)JS_GetPrivate(thisobj);

#define JSNATIVE_PROLOGUE_CLASS(ofclass, fclass) \
    JS::CallArgs args = CallArgsFromVp(argc, vp); \
    (void)args;\
    JS::RootedObject thisobj(cx, JS_THIS_OBJECT(cx, vp)); \
    if (!thisobj) { \
        JS_ReportError(cx, "Illegal invocation"); \
        return false; \
    } \
    ofclass *CppObj = (ofclass *)JS_GetInstancePrivate(cx, thisobj, fclass, NULL); \
    if (!CppObj) { \
        JS_ReportError(cx, "Illegal invocation"); \
        return false; \
    }

#define NATIVE_CHECK_ARGS(fnname, minarg) \
    if (argc < minarg) { \
                         \
        char numBuf[12];  \
        snprintf(numBuf, sizeof numBuf, "%u", argc);  \
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSMSG_MORE_ARGS_NEEDED,  \
                             fnname, numBuf, (argc > 1 ? "s" : ""));  \
        return false;  \
    }


static JSClass NativeJSEvent_class = {
    "NativeJSEvent", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};


struct NativeJSEvent
{
    NativeJSEvent(JSContext *cx, jsval func) {
        once = false;
        next = prev = NULL;

        m_Cx = cx;
        m_Function = func;

        JS_AddValueRoot(m_Cx, &m_Function);
    }
    ~NativeJSEvent() {
        JS_RemoveValueRoot(m_Cx, &m_Function);
    }

    JSContext *m_Cx;
    jsval m_Function;

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

        JSObject *ret = JS_NewObject(cx, &NativeJSEvent_class, NULL, NULL);
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

    bool fire(jsval evobj, JSObject *thisobj) {
        NativeJSEvent *ev, *tmpEv;
        JS::Value rval;
        for (ev = m_Head; ev != NULL;) {
            // Use tmp in case the event was self deleted during trigger
            tmpEv = ev->next;
            JS_CallFunctionValue(ev->m_Cx, thisobj,
                ev->m_Function, 1, &evobj, &rval);

            ev = tmpEv;
        }
        return false;
    }

    NativeJSEvent *m_Head;
    NativeJSEvent *m_Queue;
    char *m_Name;
private:
    static JSBool native_jsevents_stopPropagation(JSContext *cx,
        unsigned argc, jsval *vp)
    {
        JS::CallArgs args = CallArgsFromVp(argc, vp);
        JS::RootedObject thisobj(cx, JS_THIS_OBJECT(cx, vp));
        if (!thisobj) {
            JS_ReportError(cx, "Illegal invocation");
            return false;
        }
        if (!JS_InstanceOf(cx, thisobj, &NativeJSEvent_class, NULL)) {
            JS_ReportError(cx, "Illegal invocation");
            return false;            
        }
        JS::Value cancelBubble = JS::BooleanValue(true);
        JS_SetProperty(cx, thisobj, "cancelBubble", &cancelBubble);

        return true;
    }
    static JSBool native_jsevents_stub(JSContext *cx,
        unsigned argc, jsval *vp)
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

    NativeJSExposer(JSObject *jsobj, JSContext *cx, bool impEvents = true) :
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
            return (T *)JS_GetInstancePrivate(cx, obj, T::jsclass, NULL);
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

    bool fireJSEvent(const char *name, jsval evobj) {
        if (!m_Events) {
            return false;
        }
        if (0 && !JS_InstanceOf(m_Cx, evobj.toObjectOrNull(),
            &NativeJSEvent_class, NULL)) {
            evobj.setUndefined();
        }
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

    void addJSEvent(char *name, jsval func) {
        initEvents();

        NativeJSEvents *events = m_Events->get(name);
        if (!events) {
            events = new NativeJSEvents(name);
            m_Events->set(name, events);
        }

        NativeJSEvent *ev = new NativeJSEvent(m_Cx, func);
        events->add(ev);
    }


    JSObject *m_JSObject;
    JSContext *m_Cx;
    NativeHash<NativeJSEvents *> *m_Events;

    static JSClass *jsclass;
private:
    static JSBool native_jsevent_fireEvent(JSContext *cx,
        unsigned argc, jsval *vp)
    {
        JSNATIVE_PROLOGUE_CLASS(NativeJSExposer<T>, NativeJSExposer<T>::jsclass);

        NATIVE_CHECK_ARGS("fireEvent", 2);

        JSString *name;
        JSObject *evobj;

        if (!JS_ConvertArguments(cx, 2, args.array(), "So", &name, &evobj)) {
            return false;
        }

        JSAutoByteString cname(cx, name);

        CppObj->fireJSEvent(cname.ptr(), OBJECT_TO_JSVAL(evobj));

        return true;
    }
    static JSBool native_jsevent_addEventListener(JSContext *cx,
        unsigned argc, jsval *vp)
    {
        JSNATIVE_PROLOGUE_CLASS(NativeJSExposer<T>, NativeJSExposer<T>::jsclass);

        NATIVE_CHECK_ARGS("addEventListener", 2);

        JSString *name;
        JS::Value cb;

        if (!JS_ConvertArguments(cx, 1, args.array(), "S", &name)) {
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
        m_JSCx(cx)
    {
        static JSClass jsclass = {
            NULL, JSCLASS_HAS_PRIVATE,
            JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
            JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub
        };

        if (jsclass.name == NULL) {
            jsclass.name = name;
        }

        m_JSClass = &jsclass;

        m_JSObj = JS_NewObject(m_JSCx, m_JSClass, NULL, NULL);
        JS_SetPrivate(m_JSObj, static_cast<T *>(this));
        JS_AddObjectRoot(m_JSCx, &m_JSObj);
    }
    virtual ~NativeJSObjectMapper()
    {
        JS_SetPrivate(m_JSObj, NULL);
        JS_RemoveObjectRoot(m_JSCx, &m_JSObj);
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
    JSObject *m_JSObj;
    JSContext *m_JSCx;
};


typedef bool (*register_module_t)(JSContext *cx, JSObject *exports);

#define NativeJSObj(cx) (NativeJS::getNativeClass(cx))

#define NATIVE_OBJECT_EXPOSE(name) \
    void NativeJS ## name::registerObject(JSContext *cx) \
    { \
        JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, &name ## _class, \
            native_ ## name ## _constructor, \
            0, NULL, NULL, NULL, NULL); \
    }

#define NATIVE_OBJECT_EXPOSE_NOT_INST(name) \
    void NativeJS ## name::registerObject(JSContext *cx) \
    { \
        JSObject *name ## Obj; \
        name ## Obj = JS_DefineObject(cx, JS_GetGlobalObject(cx), #name, \
            &name ## _class , NULL, 0); \
        JS_DefineFunctions(cx, name ## Obj, name ## _funcs); \
        JS_DefineProperties(cx, name ## Obj, name ## _props); \
    }

#define NATIVE_REGISTER_MODULE(constructor) \
    extern "C" __attribute__((__visibility__("default"))) bool __NativeRegisterModule(JSContext *cx, JSObject *exports) \
    { \
        return constructor(cx, exports); \
    }


#define JSOBJ_SET_PROP_FLAGS(where, name, val, flags) JS_DefineProperty(m_Cx, where, \
    (const char *)name, val, NULL, NULL, flags)

#define JSOBJ_SET_PROP(where, name, val) JSOBJ_SET_PROP_FLAGS(where, name, val, \
        JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

#define JSOBJ_CALLFUNCNAME(where, name, argc, argv) \
    { \
        JS::Value oncallback, rval; \
        if (JS_GetProperty(cx, where, name, &oncallback) && \
            JS_TypeOfValue(cx, oncallback) == JSTYPE_FUNCTION) { \
            JS_CallFunctionValue(cx, where, oncallback, \
                argc, argv, &rval); \
        } \
    }
#define JSOBJ_SET_PROP_CSTR(where, name, val) JSOBJ_SET_PROP(where, name, STRING_TO_JSVAL(JS_NewStringCopyZ(m_Cx, val)))
#define JSOBJ_SET_PROP_STR(where, name, val) JSOBJ_SET_PROP(where, name, STRING_TO_JSVAL(val))
#define JSOBJ_SET_PROP_INT(where, name, val) JSOBJ_SET_PROP(where, name, INT_TO_JSVAL(val))



#define JS_INITOPT() JS::Value __curopt;

#define JSGET_OPT(obj, name) if (obj && JS_GetProperty(cx, obj, name, &__curopt) && __curopt != JSVAL_VOID && __curopt != JSVAL_NULL)
#define JSGET_OPT_TYPE(obj, name, type) if (obj && JS_GetProperty(cx, obj, name, &__curopt) && __curopt != JSVAL_VOID && __curopt != JSVAL_NULL && __curopt.is ## type())


class NativeJSObjectBuilder
{
public:
    NativeJSObjectBuilder(JSContext *cx, JSClass *clasp = NULL) {
        m_Cx = cx;
        m_Obj = JS_NewObject(m_Cx, clasp, NULL, NULL);
    };

    NativeJSObjectBuilder(JSContext *cx, JSObject *wrapped) {
        m_Obj = wrapped;
        m_Cx = cx;
    };

    void set (const char *name, JS::Value jval) {
        JSOBJ_SET_PROP(m_Obj, name, jval);
    }

    void set(const char *name, JSString *value) {
        JSOBJ_SET_PROP_STR(m_Obj, name, value);
    }

    void set(const char *name, const char *value) {
        JSOBJ_SET_PROP_CSTR(m_Obj, name, value);
    }

    void set(const char *name, uint32_t value) {
        JSOBJ_SET_PROP_INT(m_Obj, name, value);
    }

    void set(const char *name, int32_t value) {
        JSOBJ_SET_PROP_INT(m_Obj, name, value);
    }

    void set(const char *name, double value) {
        JSOBJ_SET_PROP(m_Obj, name, JS_NumberValue(value));
    }

    void set(const char *name, bool value) {
        JSOBJ_SET_PROP(m_Obj, name, BOOLEAN_TO_JSVAL(value));
    }

    JSObject *obj() const {
        return m_Obj;
    }

    JS::Value jsval() const {
        return OBJECT_TO_JSVAL(m_Obj);
    }

    ~NativeJSObjectBuilder(){};

private:
    JSObject *m_Obj;
    JSContext *m_Cx;
};


#endif
