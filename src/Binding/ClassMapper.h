/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_classmapper_h__
#define binding_classmapper_h__

#include <jsapi.h>
#include "Binding/JSExposer.h"
#include "Binding/JSEvents.h"

namespace Nidium {
namespace Binding {

#define CLASSMAPPER_FN(cclass, name, argc) \
    JS_FN(#name, (cclass::JSCall<&cclass::JS_##name, argc>), \
        argc, NIDIUM_JS_FNPROPS)

#define CLASSMAPPER_PROP_G(cclass, name) \
    {                                                                       \
        #name,                                                              \
        JSPROP_PERMANENT | /*JSPROP_READONLY |*/ JSPROP_ENUMERATE |         \
            JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,                        \
        {{JS_CAST_NATIVE_TO((cclass::JSGetter<&cclass::JSGetter_##name>),   \
            JSPropertyOp), nullptr}},                                       \
        JSOP_NULLWRAPPER                                                    \
    }

#define CLASSMAPPER_PROP_GS(cclass, name) \
    {                                                                       \
        #name,                                                              \
        JSPROP_PERMANENT | /*JSPROP_READONLY |*/ JSPROP_ENUMERATE |         \
            JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,                        \
        {{JS_CAST_NATIVE_TO((cclass::JSGetter<&cclass::JSGetter_##name>),   \
            JSPropertyOp), nullptr}},                                       \
        {{JS_CAST_NATIVE_TO((cclass::JSSetter<&cclass::JSSetter_##name>),   \
            JSStrictPropertyOp), nullptr}}                                  \
    }

#define NIDIUM_DECL_JSCALL(name) \
    bool JS_##name(JSContext *cx, JS::CallArgs &args)

#define NIDIUM_DECL_JSGETTER(name) \
    bool JSGetter_##name(JSContext *, JS::MutableHandleValue)

#define NIDIUM_DECL_JSSETTER(name) \
    bool JSSetter_##name(JSContext *, JS::MutableHandleValue)

#define NIDIUM_DECL_JSGETTERSETTER(name) \
    NIDIUM_DECL_JSGETTER(name); \
    NIDIUM_DECL_JSSETTER(name)

#define NIDIUM_DECL_JSTRACER() void JSTracer(class JSTracer *trc)

template <typename T>
class ClassMapper
{
public:

    enum ExposeFlags {
        kEmpty_ExposeFlag           = 0,
        kJSTracer_ExposeFlag        = 1 << 0
    };

    /**
     *  Expose an instantiable JS class |name| to the global namespace
     */
    static JSObject *ExposeClass(JSContext *cx, const char *name,
        int jsflags = 0, ExposeFlags flags = kEmpty_ExposeFlag,
        JS::HandleObject parent = JS::NullPtr())
    {

        JSClass *jsclass = ClassMapper<T>::GetJSClass();

        jsclass->name     = name;
        jsclass->finalize = ClassMapper<T>::JSFinalizer;
        jsclass->flags    = jsflags | JSCLASS_HAS_PRIVATE;

        if (flags & kJSTracer_ExposeFlag) {
            jsclass->trace = ClassMapper<T>::JSTrace;
        }

        JS::RootedObject sparent(cx);

        sparent = !parent.get() ? JS::CurrentGlobalOrNull(cx) : parent;

        return JS_InitClass(cx, sparent, JS::NullPtr(), jsclass,
                    ClassMapper<T>::JSConstructor, 0, NULL,
                    T::ListMethods(), T::ListProperties(), NULL);
    }

    /**
     *  Create an instance of an object (that is, not from the JS)
     */
    static JSObject *CreateObject(JSContext *cx, T *obj)
    {
        JS::RootedObject ret(
            cx, JS_NewObject(cx, ClassMapper<T>::GetJSClass(),
                JS::NullPtr(), JS::NullPtr()));

        obj->m_Instance = ret;
        obj->m_Cx = cx;
        obj->m_Rooted = false;

        JS_SetPrivate(ret, obj);

        return ret;
    }

    /**
     *  Get a ClassMapper<T> object given its JSObject.
     *  Return NULL if wrong source object
     *  This is the opposite of this->m_Instance
     */
    static T *GetInstance(JS::HandleObject obj)
    {
        if (JS_GetClass(obj) != ClassMapper<T>::GetJSClass()) {
            return nullptr;
        }

        return (T *)JS_GetPrivate(obj);
    }

    static T *GetInstanceUnsafe(JS::HandleObject obj)
    {
        return (T *)JS_GetPrivate(obj);
    }

    static bool IsOfClass(JS::HandleObject obj)
    {
        return (JS_GetClass(obj) == ClassMapper<T>::GetJSClass());
    }

    /**
     *  Get the underlying mapped JSObject
     */    
    JSObject *getJSObject()
    {
        return m_Instance;
    }

    /**
     *  Protect the object against the Garbage collector.
     *  By default, |this| is tied to its JSObject life, meaning it's delete'd
     *  when m_Instance becomes unreachable to the JS engine.
     *  
     *  When root()'d, it's up to the C++ code to delete the object or unroot()
     *  when needed.
     */    
    void root()
    {
        if (m_Rooted) {
            return;
        }

        NidiumJSObj(m_Cx)->rootObjectUntilShutdown(m_Instance);
        m_Rooted = true;
    }

    /**
     *  unroot a root()'d object.
     *  Give back control to the GC.
     */    
    void unroot()
    {
        if (!m_Rooted) {
            return;
        }

        NidiumJSObj(m_Cx)->unrootObject(m_Instance);
        m_Rooted = false;
    }

    /**
     *  It's automatically called by default by the JS engine during GC.
     *  If called manually, remaning reachable JS instance would trigger an
     *  Illegal instance upon method call.
     */      
    virtual ~ClassMapper()
    {
        JS_SetPrivate(m_Instance, nullptr);
        
        this->unroot();
    }

    virtual void JSTracer(class JSTracer *trc) {}

protected:
    typedef bool (T::*JSCallback)(JSContext *, JS::CallArgs &);
    typedef bool (T::*JSGetterCallback)(JSContext *, JS::MutableHandleValue);
    typedef bool (T::*JSSetterCallback)(JSContext *, JS::MutableHandleValue);

    template <JSCallback U, int minarg>
    static bool JSCall(JSContext *cx, unsigned argc, JS::Value *vp)
    {
        NIDIUM_JS_PROLOGUE_CLASS(T, ClassMapper<T>::GetJSClass());

        /* TODO: Get the right method name */
        NIDIUM_JS_CHECK_ARGS("method", minarg);

        return (CppObj->*U)(cx, args);
    }

    template <JSGetterCallback U>
    static bool JSGetter(JSContext *cx, unsigned argc, JS::Value *vp)
    {
        NIDIUM_JS_PROLOGUE_CLASS(T, ClassMapper<T>::GetJSClass());

        return (CppObj->*U)(cx, args.rval());
    }

    template <JSSetterCallback U>
    static bool JSSetter(JSContext *cx, unsigned argc, JS::Value *vp)
    {
        NIDIUM_JS_PROLOGUE_CLASS(T, ClassMapper<T>::GetJSClass());

        JS::RootedValue val(cx, args.get(0));

        bool ret = (CppObj->*U)(cx, &val);

        args.rval().set(val);

        return ret;
    }

    static void JSTrace(class JSTracer *trc, JSObject *obj)
    {
        T *CppObj = (T *)JS_GetPrivate(obj);

        if (CppObj) {
            CppObj->JSTracer(trc);
        }
    }

    static JSFunctionSpec *ListMethods()
    {
        return nullptr;
    }

    static JSPropertySpec *ListProperties()
    {
        return nullptr;
    }    

    static bool JSConstructor(JSContext *cx, unsigned argc, JS::Value *vp)
    {
        T *obj;
        JSClass *jsclass = ClassMapper<T>::GetJSClass();

        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

        if (!args.isConstructing()) {
            JS_ReportError(cx, "Bad constructor");
            return false;
        }

        JS::RootedObject ret(
            cx, JS_NewObjectForConstructor(cx, jsclass, args));

        if ((obj = T::Constructor(cx, args, ret)) == nullptr) {
            return false;
        }

        obj->m_Instance = ret;
        obj->m_Cx = cx;
        obj->m_Rooted = false;

        JS_SetPrivate(ret, obj);

        args.rval().setObjectOrNull(ret);
 
        return true;
    }

    static void JSFinalizer(JSFreeOp *fop, JSObject *obj)
    {
        T *cppobj = (T *)JS_GetPrivate(obj);

        if (cppobj) {
            delete cppobj;
        }
    }

    static JSClass *GetJSClass()
    {
        static JSClass jsclass = { NULL,
                                   JSCLASS_HAS_PRIVATE,
                                   JS_PropertyStub,
                                   JS_DeletePropertyStub,
                                   JS_PropertyStub,
                                   JS_StrictPropertyStub,
                                   JS_EnumerateStub,
                                   JS_ResolveStub,
                                   JS_ConvertStub,
                                   JSCLASS_NO_OPTIONAL_MEMBERS };

        return &jsclass;
    }

    JS::Heap<JSObject *> m_Instance;
    JSContext *m_Cx;
    bool m_Rooted;
};

template <typename T>
class ClassMapperWithEvents : public ClassMapper<T>
{
public:
    /*
        m_Cx and m_Instance are not visible at compilation time
        since we're extending a template.
    */
    using ClassMapper<T>::m_Cx;
    using ClassMapper<T>::m_Instance;

    typedef bool (ClassMapperWithEvents<T>::*JSCallback)(JSContext *, JS::CallArgs &);

    template <JSCallback U, int minarg>
    static bool JSCallInternal(JSContext *cx, unsigned argc, JS::Value *vp)
    {
        NIDIUM_JS_PROLOGUE_CLASS(T, ClassMapper<T>::GetJSClass());

        /* TODO: Get the right method name */
        NIDIUM_JS_CHECK_ARGS("method", minarg);

        return (CppObj->*U)(cx, args);
    }

    ClassMapperWithEvents() :
        m_Events(NULL)
    {

    }

    virtual ~ClassMapperWithEvents()
    {
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

    bool JS_addEventListener(JSContext *cx, JS::CallArgs &args)
    {
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

        this->addJSEvent(cname.ptr(), cb);

        return true;
    }
    bool JS_removeEventListener(JSContext *cx, JS::CallArgs &args)
    {
        JS::RootedString name(cx);
        JS::RootedValue cb(cx);

        if (!JS_ConvertArguments(cx, args, "S", name.address())) {
            return false;
        }

        JSAutoByteString cname(cx, name);

        if (args.length() == 1) {
            this->removeJSEvent(cname.ptr());
        } else {
            if (!JS_ConvertValue(cx, args[1], JSTYPE_FUNCTION, &cb)) {
                JS_ReportError(cx, "Bad callback given");
                return false;
            }

            this->removeJSEvent(cname.ptr(), cb);
        }

        return true;
    }

    bool JS_fireEvent(JSContext *cx, JS::CallArgs &args)
    {
        if (!m_Events) {
            return true;
        }

        JS::RootedString name(cx);
        JS::RootedObject evobj(cx);

        if (!JS_ConvertArguments(cx, args, "So", name.address(),
                                 evobj.address())) {
            return false;
        }

        if (!evobj) {
            JS_ReportError(cx, "Invalid event object");
            return false;
        }

        JSAutoByteString cname(cx, name);
        JS::RootedValue evjsobj(cx, JS::ObjectValue(*evobj));

        this->fireJSEvent(cname.ptr(), &evjsobj);

        return true;
    }


    static JSObject *ExposeClass(JSContext *cx, const char *name,
                int jsflags = 0,
                typename ClassMapper<T>::ExposeFlags flags =
                ClassMapper<T>::kEmpty_ExposeFlag)
    {
        JS::RootedObject proto(cx, ClassMapper<T>::ExposeClass(cx, name,
            jsflags, flags));


        static JSFunctionSpec funcs[] = {
            JS_FN("addEventListener",
                (T::template JSCallInternal<&T::JS_addEventListener, 2>),
                2, NIDIUM_JS_FNPROPS),
            JS_FN("removeEventListener",
                (T::template JSCallInternal<&T::JS_removeEventListener, 1>),
                1, NIDIUM_JS_FNPROPS),
            JS_FN("fireEvent",
                (T::template JSCallInternal<&T::JS_fireEvent, 2>),
                2, NIDIUM_JS_FNPROPS),
            JS_FN("on",
                (T::template JSCallInternal<&T::JS_addEventListener, 2>),
                2, NIDIUM_JS_FNPROPS),
            JS_FN("emit",
                (T::template JSCallInternal<&T::JS_fireEvent, 2>),
                2, NIDIUM_JS_FNPROPS),
            JS_FS_END
        };

        JS_DefineFunctions(cx, proto, funcs);

        return proto;
    }

    bool fireJSEvent(const char *name, JS::MutableHandleValue evobj)
    {
        JS::RootedObject thisobj(m_Cx, m_Instance);
        JS::AutoValueArray<1> params(m_Cx);
        JS::RootedValue callback(m_Cx);
        char onEv[128] = "on";

        params[0].set(evobj);

        strncat(onEv, name, 128 - 3);

        JS_GetProperty(m_Cx, thisobj, onEv, &callback);

        if (callback.isObject()
            && JS_ObjectIsCallable(m_Cx, callback.toObjectOrNull())) {
            JS::RootedValue rval(m_Cx);

            JS_CallFunctionValue(m_Cx, thisobj, callback, params, &rval);

            if (JS_IsExceptionPending(m_Cx)) {
                if (!JS_ReportPendingException(m_Cx)) {
                    JS_ClearPendingException(m_Cx);
                }
            }
        }

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

        events->fire(m_Cx, evobj, thisobj);

        return true;
    }

    void removeJSEvent(char *name)
    {
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

    void removeJSEvent(char *name, JS::HandleValue func)
    {
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

    void initEvents()
    {
        if (m_Events) {
            return;
        }

        m_Events = new Nidium::Core::Hash<JSEvents *>(32);
        /*
            Set Nidium::Core::Hash auto delete to false, since it's possible for
            an event to be deleted while it's fired. So we don't want to
            free the underlying object when removing it from the
           Nidium::Core::Hash
            (otherwise JSEvents::fire will attempt to use a freed object)
        */
        m_Events->setAutoDelete(false);
    }

    void addJSEvent(char *name, JS::HandleValue func)
    {
        initEvents();

        JSEvents *events = m_Events->get(name);
        if (!events) {
            events = new JSEvents(name);
            m_Events->set(name, events);
        }

        JSEvent *ev = new JSEvent(m_Cx, func);
        events->add(ev);
    }

    Nidium::Core::Hash<JSEvents *> *m_Events;
};

} // namespace Binding
} // namespace Nidium

#endif