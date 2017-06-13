/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_classmapperwithevents_h__
#define binding_classmapperwithevents_h__

#include <jsapi.h>

#include "Binding/ClassMapper.h"
#include "Binding/JSEvents.h"

namespace Nidium {
namespace Binding {

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

        this->addJSEventListener(cname.ptr(), args[1]);

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
    
    void addJSEventListener(char *name, JS::HandleValue func)
    {
        initEvents();

        JSEvents *events = m_Events->get(name);
        if (!events) {
            events = new JSEvents(name);
            m_Events->set(name, events);
        }

        JSEventListener *ev = new JSEventListener(m_Cx, func);
        events->add(ev);
    }

    Nidium::Core::Hash<JSEvents *> *m_Events;
};


}}

#endif