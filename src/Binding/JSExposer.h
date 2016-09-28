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

#include "Binding/NidiumJS.h"
#include "Binding/JSMacros.h"

// {{{ Macros
// {{{ JSClass macro's
#define NIDIUM_JS_PROLOGUE()    \
    NIDIUM_JS_PROLOGUE_NO_RET() \
    args.rval().setUndefined();

#define NIDIUM_JS_CONSTRUCTOR_PROLOGUE()                      \
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);         \
    if (!args.isConstructing()) {                             \
        JS_ReportError(cx, "Illegal constructor invocation"); \
        return false;                                         \
    }

#define NIDIUM_JS_PROLOGUE_CLASS_NO_RET_FOR_OBJ(ofclass, fclass, obj) \
    ofclass *CppObj = static_cast<ofclass *>(                         \
        JS_GetInstancePrivate(cx, obj, fclass, NULL));                \
    if (!CppObj) {                                                    \
        JS_ReportError(cx, "Illegal invocation");                     \
        return false;                                                 \
    }

// }}}
// {{{ Object macro's

#define NidiumJSObj(cx) (Nidium::Binding::NidiumJS::GetObject(cx))

#define NIDIUM_JS_OBJECT_EXPOSE(name)                                         \
    void JS##name::RegisterObject(JSContext *cx)                              \
    {                                                                         \
        JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));             \
        JS_InitClass(cx, global, JS::NullPtr(), &name##_class,                \
                     nidium_##name##_constructor, 0, NULL, NULL, NULL, NULL); \
    }

#define NIDIUM_JS_OBJECT_EXPOSE_NOT_INST(name)                               \
    void JS##name::RegisterObject(JSContext *cx)                             \
    {                                                                        \
        JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));            \
        JS::RootedObject name##Obj(                                          \
            cx, JS_DefineObject(cx, global, #name, &name##_class, NULL, 0)); \
        JS_DefineFunctions(cx, name##Obj, name##_funcs);                     \
        JS_DefineProperties(cx, name##Obj, name##_props);                    \
    }

#define NIDIUM_JS_REGISTER_MODULE(constructor)                      \
    extern "C" __attribute__((__visibility__("default"))) bool      \
    __NidiumRegisterModule(JSContext *cx, JS::HandleObject exports) \
    {                                                               \
        return constructor(cx, exports);                            \
    }

// }}}

// }}}

namespace Nidium {
namespace Binding {


// {{{ JSExposer
template <typename T>
class JSExposer
{
public:
    JSObject *getJSObject() const
    {
        return m_JSObject;
    }

    JSObject **getJSObjectAddr()
    {
        return &m_JSObject;
    }

    JSContext *getJSContext() const
    {
        return m_Cx;
    }

    void setJSObject(JSObject *obj)
    {
        m_JSObject = obj;
    }

    void setJSContext(JSContext *cx)
    {
        m_Cx = cx;
    }

    JSExposer(JS::HandleObject jsobj, JSContext *cx, bool impEvents = true)
        : m_JSObject(jsobj), m_Cx(cx)
    {
        if (impEvents) {
            printf("Warning events has moved\n");
        }
    }

    static void InstallEventsOnPrototype(JSContext *cx, JS::HandleObject proto)
    {
        printf("Warning events has moved\n");
    }

    virtual ~JSExposer()
    {

    }

    static const char *GetJSObjectName()
    {
        return NULL;
    }

    static JSObject *GetJSGlobalObject(NidiumJS *njs)
    {
        JSObject *jobj;
        const char *name = T::GetJSObjectName();

        if ((jobj = njs->m_JsObjects.get(name)) == NULL) {
            return NULL;
        }

        return jobj;
    }

    static JSObject *GetJSGlobalObject(JSContext *cx)
    {
        return T::GetJSGlobalObject(NidiumJS::GetObject(cx));
    }

    static T *GetObject(JSObject *obj, JSContext *cx = NULL)
    {
        if (cx != NULL) {
            if (JS_GetClass(obj) == T::jsclass) {
                return static_cast<T *>(JS_GetPrivate(obj));
            }
            return NULL;
        }
        return static_cast<T *>(JS_GetPrivate(obj));
    }

    static T *GetObject(NidiumJS *njs)
    {
        JSObject *obj = T::GetJSGlobalObject(njs);
        if (obj == NULL) {
            return NULL;
        }
        return static_cast<T *>(JS_GetPrivate(obj));
    }

    static T *GetObject(JSContext *cx)
    {
        return T::GetObject(NidiumJS::GetObject(cx));
    }


protected:

    JS::Heap<JSObject *> m_JSObject;

    JSContext *m_Cx;

    static JSClass *jsclass;

private:
};
// }}}

} // namespace Binding
} // namespace Nidium

#endif
