/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_classmapper_h__
#define binding_classmapper_h__

#include <jsapi.h>
#include <assert.h>
#include <string>

#include "Binding/JSMacros.h"
#include "Binding/JSEvents.h"

#include "Binding/JSUtils.h"
#include "Binding/ThreadLocalContext.h"

namespace Nidium {
namespace Binding {


#define CLASSMAPPER_FN(cclass, name, argc) \
    JS_FN(#name, (cclass::JSCall<&cclass::JS_##name, argc>), \
        argc, NIDIUM_JS_FNPROPS)

#define CLASSMAPPER_FN_STATIC(cclass, name, argc) \
    JS_FN(#name, (cclass::JSCallStatic<&cclass::JSStatic_##name, argc>), \
        argc, NIDIUM_JS_FNPROPS)

#define CLASSMAPPER_FN_ALIAS(cclass, name, argc, alias) \
    JS_FN(#name, (cclass::JSCall<&cclass::JS_##alias, argc>), \
        argc, NIDIUM_JS_FNPROPS)


#define CLASSMAPPER_PROP_G_ALIAS(cclass, name, alias) \
    {                                                                       \
        #name,                                                              \
        JSPROP_PERMANENT | /*JSPROP_READONLY |*/ JSPROP_ENUMERATE |         \
            JSPROP_SHARED,                                                  \
        {{  cclass::JSGetter<&cclass::JSGetter_##alias>, nullptr}},         \
        {{ nullptr, nullptr }}                                              \
    }

#define CLASSMAPPER_PROP_GS_ALIAS(cclass, name, alias) \
    {                                                                       \
        #name,                                                              \
        JSPROP_PERMANENT | /*JSPROP_READONLY |*/ JSPROP_ENUMERATE |         \
            JSPROP_SHARED,                        \
        {{cclass::JSGetter<&cclass::JSGetter_##alias>, nullptr}},           \
        {{cclass::JSSetter<&cclass::JSSetter_##alias>, nullptr}}           \
    }

#define CLASSMAPPER_PROP_GS(cclass, name) \
    CLASSMAPPER_PROP_GS_ALIAS(cclass, name, name)
#define CLASSMAPPER_PROP_G(cclass, name) \
    CLASSMAPPER_PROP_G_ALIAS(cclass, name, name)

#define NIDIUM_DECL_JSCALL(name) \
    bool JS_##name(JSContext *cx, JS::CallArgs &args)

#define NIDIUM_DECL_JSCALL_STATIC(name) \
    static bool JSStatic_##name(JSContext *cx, JS::CallArgs &args)

#define NIDIUM_DECL_JSGETTER(name) \
    bool JSGetter_##name(JSContext *cx, JS::MutableHandleValue vp)

#define NIDIUM_DECL_JSSETTER(name) \
    bool JSSetter_##name(JSContext *cx, JS::MutableHandleValue vp)

#define NIDIUM_DECL_JSGETTERSETTER(name) \
    NIDIUM_DECL_JSGETTER(name); \
    NIDIUM_DECL_JSSETTER(name)

#define NIDIUM_DECL_JSTRACER() \
    inline void jsTrace(class JSTracer *trc) override

#define CLASSMAPPER_PROLOGUE_NO_RET()                          \
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);          \
    JS::RootedObject thisobj(cx);                              \
    if (T::GetInstance == ClassMapper<T>::GetInstance) {       \
        if (!args.thisv().isObject()) {                        \
            JS_ReportError(cx, "Illegal invocation");          \
            return false;                                      \
        }                                                      \
        thisobj = &args.thisv().toObject();                    \
    }

#define CLASSMAPPER_PROLOGUE_CLASS_NO_RET(ofclass) \
    CLASSMAPPER_PROLOGUE_NO_RET()                          \
    ofclass *CppObj = T::GetInstance(thisobj, cx);         \
    if (!CppObj) {                                         \
        JS_ReportError(cx, "Illegal invocation");          \
        return false;                                      \
    }

#define CLASSMAPPER_PROLOGUE_CLASS(ofclass)    \
    CLASSMAPPER_PROLOGUE_CLASS_NO_RET(ofclass) \
    args.rval().setUndefined();

#define NIDIUM_JS_REGISTER_MODULE(constructor)                      \
    extern "C" __attribute__((__visibility__("default"))) bool      \
    __NidiumRegisterModule(JSContext *cx, JS::HandleObject exports) \
    {                                                               \
        return constructor(cx, exports);                            \
    }

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
    template<int ctor_minarg = 0>
    static JSObject *ExposeClass(JSContext *cx, const char *name,
        int jsflags = 0, ExposeFlags flags = kEmpty_ExposeFlag,
        JS::HandleObject parent = nullptr)
    {
        JSClass *jsclass = T::GetJSClass();

#ifdef DEBUG
        if (jsclass != ClassMapper<T>::GetJSClass()) {
            ndm_logf(NDM_LOG_DEBUG, "Classmapper", "JSClass is overriden for %s", name);
        }
        assert(jsclass->name == NULL || strcmp(jsclass->name, name) == 0);
#endif

        jsclass->name     = name;
        jsclass->finalize = ClassMapper<T>::JSFinalizer;
        jsclass->flags   |= jsflags | JSCLASS_HAS_PRIVATE;

        if (flags & kJSTracer_ExposeFlag) {
            jsclass->trace = ClassMapper<T>::JSTrace;
        }

        JS::RootedObject sparent(cx);

        sparent = !parent.get() ? JS::CurrentGlobalOrNull(cx) : parent;

        /*
            TODO: Should we root the proto?
        */
        JS::RootedObject proto(cx, JS_InitClass(cx, sparent, nullptr, jsclass,
                    ClassMapper<T>::JSConstructor<ctor_minarg>,
                    ctor_minarg, T::ListProperties(),
                    T::ListMethods(), NULL,
                    T::ListStaticMethods()));

        NidiumLocalContext *nlc = NidiumLocalContext::Get();

        nlc->addProtoCache(jsclass, proto);

        return proto;
    }

    /*
        Create a simple object (without any instance).
        Only static methods are implemented
    */
    static JSObject *ExposeObject(JSContext *cx, const char *name,
        JS::HandleObject parent = nullptr)
    {
        JSClass *jsclass = T::GetJSClass();
        jsclass->name    = name;

        JS::RootedObject ret(cx, JS_NewObject(cx, jsclass));

        if (T::ListStaticMethods()) {
            JS_DefineFunctions(cx, ret, T::ListStaticMethods());
        }

        if (parent) {
            JS::RootedValue rval(cx, JS::ObjectValue(*ret));
            JS_SetProperty(cx, parent, name, rval);
        }

        return ret;
    }

    static void AssociateObject(JSContext *cx, T *obj, JS::HandleObject jsobj,
        bool implement = false)
    {
        obj->m_Instance = jsobj;
        obj->m_Cx = cx;
        obj->m_Rooted = false;

        if (JS_GetPrivate(jsobj) == NULL) {
            JS_SetPrivate(jsobj, obj);
        }

        if (implement) {
            JS_DefineProperties(cx, jsobj, T::ListProperties());
            JS_DefineFunctions(cx, jsobj, T::ListMethods());
        }
    }

    /**
     *  Create an instance of an object (that is, not from the JS)
     */
    static inline JSObject *CreateObject(JSContext *cx, T *obj)
    {
#ifdef DEBUG
        JSClass *jsclass = T::GetJSClass();
        assert(jsclass->name != NULL);
#endif
        NidiumLocalContext *nlc = NidiumLocalContext::Get();

        JS::RootedObject proto(cx,
            nlc->getPrototypeFromJSClass(T::GetJSClass()));

        JS::RootedObject ret(cx,
                JS_NewObjectWithGivenProto(cx,
                T::GetJSClass(), proto));

        ClassMapper<T>::AssociateObject(cx, obj, ret);

        return ret;
    }

    void setUniqueInstance()
    {
        /* Always root singleton since they might be replaced
           by the user on the global namespace
        */
        this->root();

        NidiumLocalContext *nlc = NidiumLocalContext::Get();

        nlc->m_JSUniqueInstance.set((uintptr_t)T::GetJSClass(),
            (uintptr_t)this);
    }

    /**
     *  Create a singleton and expose the instance to the global object
     */
    static JSObject *CreateUniqueInstance(JSContext *cx, T *obj,
        const char *name = nullptr)
    {
        JS::RootedObject ret(cx, CreateObject(cx, obj));

#ifdef DEBUG
        JSClass *jsclass = T::GetJSClass();
        assert(jsclass->name != NULL);
        /* CX doesn't match local thread CX */
        assert(NidiumLocalContext::Get()->cx == cx);
#endif

        obj->setUniqueInstance();

        JS::RootedValue val(cx);
        val.setObject(*ret);

        if (name == nullptr) {
            name = GetClassName();
        }

        JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));

        JS_SetProperty(cx, global, name, val);

        return ret;
    }

    static inline const char *GetClassName()
    {
        JSClass *ret = T::GetJSClass();

        assert(ret->name != NULL);

        return ret->name;
    }

    /**
     *  Get a ClassMapper<T> object given its JSObject.
     *  Return NULL if wrong source object
     *  This is the opposite of this->m_Instance
     */
    static inline T *GetInstance(JSObject *obj,
        JSContext *cx = nullptr)
    {
        if (obj == nullptr || JS_GetClass(obj) != T::GetJSClass()) {
            return nullptr;
        }

        return (T *)JS_GetPrivate(obj);
    }

    static inline T *GetInstanceUnsafe(JSObject *obj,
        JSContext *cx = nullptr)
    {
        if (obj == nullptr) {
            return nullptr;
        }

        return (T *)JS_GetPrivate(obj);
    }

    /**
     *  Get a singleton ClassMapper<T> object.
     *  It's used for object created with CreateUniqueInstance()
     */
    static inline T *GetInstanceSingleton()
    {
        NidiumLocalContext *nlc = NidiumLocalContext::Get();

        return reinterpret_cast<T *>(nlc->m_JSUniqueInstance.get(
            (uintptr_t)T::GetJSClass()));
    }

    static inline bool InstanceOf(JSObject *obj)
    {
        if (obj == nullptr) {
            return false;
        }

        return (JS_GetClass(obj) == T::GetJSClass());
    }

    static inline bool InstanceOf(JS::Value val)
    {
        if (val.isNullOrUndefined() || !val.isObject()) {
            return false;
        }

        return (JS_GetClass(&val.toObject()) == T::GetJSClass());
    }

    /**
     *  Get the underlying mapped JSObject
     */
    JSObject inline *getJSObject() const
    {
        return m_Instance;
    }

    JSContext inline *getJSContext() const
    {
        return m_Cx;
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
#ifdef DEBUG
        /*
            Assert if root() was called in ::Constructor() or
            before CreateObject().
            (That is, before an AssociateObject internal call)
        */
        assert(m_Instance != nullptr);
#endif
        if (m_Rooted) {
            return;
        }

        NidiumLocalContext::RootObjectUntilShutdown(m_Instance);
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

        NidiumLocalContext::UnrootObject(m_Instance);
        m_Rooted = false;
    }

    /**
     *  It's automatically called by default by the JS engine during GC.
     *  If called manually, remaning reachable JS instance would trigger an
     *  Illegal instance upon method call.
     */
    virtual ~ClassMapper()
    {
        if (!m_Instance) {
            return;
        }
        JS_SetPrivate(m_Instance, nullptr);

        this->unroot();
    }

    virtual inline void jsTrace(class JSTracer *trc) {}

protected:
    typedef bool (T::*JSCallback)(JSContext *, JS::CallArgs &);
    typedef bool (*JSCallbackStatic)(JSContext *, JS::CallArgs &);
    typedef bool (T::*JSGetterCallback)(JSContext *, JS::MutableHandleValue);
    typedef bool (T::*JSSetterCallback)(JSContext *, JS::MutableHandleValue);

    template <JSCallback U, int minarg>
    static inline bool JSCall(JSContext *cx, unsigned argc, JS::Value *vp)
    {
        CLASSMAPPER_PROLOGUE_CLASS(T);

        /* TODO: Get the right method name */
        if (!args.requireAtLeast(cx, "method", minarg)) {
            return false;
        }

        return (CppObj->*U)(cx, args);
    }

    template <JSCallbackStatic U, int minarg>
    static inline bool JSCallStatic(JSContext *cx, unsigned argc, JS::Value *vp)
    {
        CLASSMAPPER_PROLOGUE_NO_RET()

        if (!args.requireAtLeast(cx, "method", minarg)) {
            return false;
        }

        args.rval().setUndefined();

        return (*U)(cx, args);
    }

    template <JSGetterCallback U>
    static inline bool JSGetter(JSContext *cx, unsigned argc, JS::Value *vp)
    {
        CLASSMAPPER_PROLOGUE_CLASS(T);

        return (CppObj->*U)(cx, args.rval());
    }

    template <JSSetterCallback U>
    static inline bool JSSetter(JSContext *cx, unsigned argc, JS::Value *vp)
    {
        CLASSMAPPER_PROLOGUE_CLASS(T);

        JS::RootedValue val(cx, args.get(0));

        bool ret = (CppObj->*U)(cx, &val);

        args.rval().set(val);

        return ret;
    }

    static inline void JSTrace(class JSTracer *trc, JSObject *obj)
    {
        T *CppObj = (T *)JS_GetPrivate(obj);

        if (CppObj) {
            CppObj->jsTrace(trc);
        }
    }

    static JSFunctionSpec *ListMethods()
    {
        return nullptr;
    }

    static JSFunctionSpec *ListStaticMethods()
    {
        return nullptr;
    }

    static JSPropertySpec *ListProperties()
    {
        return nullptr;
    }

    static inline T *Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj)
    {
        JS_ReportError(cx, "Illegal constructor");

        return nullptr;
    }

    template<int ctor_minarg = 0>
    static  bool JSConstructor(JSContext *cx, unsigned argc, JS::Value *vp)
    {
        T *obj;
        JSClass *jsclass = T::GetJSClass();

        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

        if (!args.isConstructing()) {
            JS_ReportError(cx, "Bad constructor");
            return false;
        }

        if (!args.requireAtLeast(cx, "constructor", ctor_minarg)) {
            return false;
        }

        JS::RootedObject ret(
            cx, JSUtils::NewObjectForConstructor(cx, jsclass, args));

        if ((obj = T::Constructor(cx, args, ret)) == nullptr) {
            return false;
        }

        ClassMapper<T>::AssociateObject(cx, obj, ret);

        args.rval().setObjectOrNull(ret);

        return true;
    }

    static inline void JSFinalizer(JSFreeOp *fop, JSObject *obj)
    {
        T *cppobj = (T *)JS_GetPrivate(obj);

        if (cppobj) {
            delete cppobj;
        }
    }

    static inline JSClass *GetJSClass()
    {
        static JSClass jsclass = { NULL,
                                   JSCLASS_HAS_PRIVATE};

        return &jsclass;
    }

    JS::Heap<JSObject *> m_Instance;
    JSContext *m_Cx = nullptr;
    bool m_Rooted = false;
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
    static inline bool JSCallInternal(JSContext *cx, unsigned argc, JS::Value *vp)
    {
        CLASSMAPPER_PROLOGUE_CLASS(T);

        /* TODO: Get the right method name */
        if (!args.requireAtLeast(cx, "method", minarg)) {
            return false;
        }

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

        if (!JS_ConvertArguments(cx, args, "S", name.address())) {
            return false;
        }

        if (!JSUtils::ReportIfNotFunction(cx, args[1])) {
            return false;
        }

        JSAutoByteString cname(cx, name);

        this->addJSEvent(cname.ptr(), args[1]);

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
            if (!JSUtils::ReportIfNotFunction(cx, args[1])) {
                return false;
            }

            this->removeJSEvent(cname.ptr(), args[1]);
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

    template <int ctor_minarg = 0>
    static JSObject *ExposeClass(JSContext *cx, const char *name,
                int jsflags = 0,
                typename ClassMapper<T>::ExposeFlags flags =
                ClassMapper<T>::kEmpty_ExposeFlag)
    {
        JS::RootedObject proto(cx,
            ClassMapper<T>::template ExposeClass<ctor_minarg>(cx, name,
            jsflags, flags));


        JS_DefineFunctions(cx, proto, GetJSEventsFunctions());

        return proto;
    }

    static void AssociateObject(JSContext *cx, T *obj, JS::HandleObject jsobj,
        bool implement = false)
    {
        ClassMapper<T>::AssociateObject(cx, obj, jsobj, implement);

        JS_DefineFunctions(cx, jsobj, GetJSEventsFunctions());
    }

    bool fireJSEvent(const char *name, JS::MutableHandleValue ev)
    {
        JS::RootedObject thisobj(m_Cx, m_Instance);
        JS::RootedObject evObj(m_Cx, ev.toObjectOrNull());
        JS::AutoValueArray<1> params(m_Cx);
        JS::RootedValue callback(m_Cx);
        JS::RootedValue type(m_Cx);
        std::string onEv = "on" + std::string(name);

        params[0].set(ev);

        JS_GetProperty(m_Cx, evObj, "type", &type);
        if (type.isUndefined()) {
            JS::RootedValue typeStr(m_Cx);
            if (JSUtils::StrToJsval(m_Cx,  name, strlen(name), &typeStr, "utf8")) {
                JS_SetProperty(m_Cx, evObj, "type", typeStr);
            }
        }

        JS_GetProperty(m_Cx, thisobj, onEv.c_str(), &callback);

        if (callback.isObject()
            && JS::IsCallable(callback.toObjectOrNull())) {
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
        if (0 && !JS_InstanceOf(m_Cx, ev.toObjectOrNull(),
            &JSEvent_class, NULL)) {
            ev.setUndefined();
        }*/

        JSEvents *events = m_Events->get(name);
        if (!events) {
            return false;
        }

        events->fire(m_Cx, ev, thisobj);

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

    static JSFunctionSpec *GetJSEventsFunctions()
    {
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

        return funcs;
    }

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
