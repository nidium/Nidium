/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_classmapper_h__
#define binding_classmapper_h__

#include <jsapi.h>
#include "Binding/JSExposer.h"

namespace Nidium {
namespace Binding {

#define CLASSMAPPER_FN(cclass, name, argc) JS_FN(#name, (cclass::JSCall<&cclass::JS_##name, argc>), argc, NIDIUM_JS_FNPROPS)

template <typename T>
class ClassMapper
{
public:
    typedef bool (T::*JSCallback)(JSContext *, JS::CallArgs &);

    template <JSCallback U, int minarg>
    static bool JSCall(JSContext *cx, unsigned argc, JS::Value *vp)
    {
        NIDIUM_JS_PROLOGUE_CLASS(T, ClassMapper<T>::GetJSClass());

        /* TODO: Get the right method name */
        NIDIUM_JS_CHECK_ARGS("method", minarg);

        return (CppObj->*U)(cx, args);
    }

    static JSFunctionSpec *ListMethods()
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

        if ((obj = T::Constructor(cx, args)) == nullptr) {
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

    static JSObject *ExposeClass(JSContext *cx, const char *name)
    {

        JSClass *jsclass = ClassMapper<T>::GetJSClass();

        jsclass->name = name;
        jsclass->finalize = ClassMapper<T>::JSFinalizer;

        JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));

        return JS_InitClass(cx, global, JS::NullPtr(), jsclass,
                    ClassMapper<T>::JSConstructor, 0, NULL,
                    T::ListMethods(), NULL, NULL);
    }

    void root()
    {
        if (m_Rooted) {
            return;
        }

        NidiumJSObj(m_Cx)->rootObjectUntilShutdown(m_Instance);
        m_Rooted = true;
    }

    void unroot()
    {
        if (!m_Rooted) {
            return;
        }

        NidiumJSObj(m_Cx)->unrootObject(m_Instance);
        m_Rooted = false;
    }

    virtual ~ClassMapper()
    {
        JS_SetPrivate(m_Instance, nullptr);
        
        this->unroot();
    }

protected:

    JS::Heap<JSObject *> m_Instance;
    JS::Heap<JSObject *> m_Prototype;
    JSContext *m_Cx;
    bool m_Rooted;
};

template <typename T>
class ClassMapperWithEvents : public ClassMapper<T>
{
public:
    typedef bool (ClassMapperWithEvents<T>::*JSCallback)(JSContext *, JS::CallArgs &);

    template <JSCallback U, int minarg>
    static bool JSCallInternal(JSContext *cx, unsigned argc, JS::Value *vp)
    {
        NIDIUM_JS_PROLOGUE_CLASS(T, ClassMapper<T>::GetJSClass());

        /* TODO: Get the right method name */
        NIDIUM_JS_CHECK_ARGS("method", minarg);

        return (CppObj->*U)(cx, args);
    }


    bool JS_addEventListener(JSContext *cx, JS::CallArgs &args)
    {
        printf("Add Event listener\n");
        return true;
    }
    bool JS_removeEventListener(JSContext *cx, JS::CallArgs &args)
    {
        printf("Add Event listener\n");
        return true;
    }
    bool JS_fireEvent(JSContext *cx, JS::CallArgs &args)
    {
        printf("Add Event listener\n");
        return true;
    }


    static JSObject *ExposeClass(JSContext *cx, const char *name)
    {
        JS::RootedObject proto(cx, ClassMapper<T>::ExposeClass(cx, name));


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

};

} // namespace Binding
} // namespace Nidium

#endif