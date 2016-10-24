/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsmacros_h__
#define binding_jsmacros_h__

#include <jsfriendapi.h>

#define NidiumJSObj(cx) (Nidium::Binding::NidiumJS::GetObject(cx))

#define NIDIUM_JS_INIT_OPT() JS::RootedValue __curopt(cx);

#define NIDIUM_JS_GET_OPT(obj, name)                    \
    if (obj && JS_GetProperty(cx, obj, name, &__curopt) \
        && __curopt != JSVAL_VOID && __curopt != JSVAL_NULL)

#define NIDIUM_JS_GET_OPT_TYPE(obj, name, type)             \
    if (obj && JS_GetProperty(cx, obj, name, &__curopt)     \
        && __curopt != JSVAL_VOID && __curopt != JSVAL_NULL \
        && __curopt.is##type())

#define NIDIUM_JS_CHECK_ARGS(fnname, minarg)                         \
    if (argc < minarg) {                                             \
                                                                     \
        char numBuf[12];                                             \
        snprintf(numBuf, sizeof numBuf, "%u", argc);                 \
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL,           \
                             JSMSG_MORE_ARGS_NEEDED, fnname, numBuf, \
                             (argc > 1 ? "s" : ""));                 \
        return false;                                                \
    }

#define JSOBJ_CALLFUNCNAME(where, name, argv)                           \
    {                                                                   \
        JS::RootedValue _oncallback(cx);                                \
        JS::RootedValue _rval(cx);                                      \
        JS::RootedValue rval(cx);                                       \
        if (JS_GetProperty(cx, where, name, &_oncallback)               \
            && JS_TypeOfValue(cx, _oncallback) == JSTYPE_FUNCTION) {    \
            JS_CallFunctionValue(cx, where, _oncallback, argv, &_rval); \
        }                                                               \
    }

#define NIDIUM_JSOBJ_SET_PROP_FLAGS(where, name, val, flags)                  \
    JS_DefineProperty(m_Cx, where, reinterpret_cast<const char *>(name), val, \
                      flags)

#define NIDIUM_JSOBJ_SET_PROP(where, name, val)                         \
    NIDIUM_JSOBJ_SET_PROP_FLAGS(where, name, val, JSPROP_PERMANENT      \
                                                      | JSPROP_READONLY \
                                                      | JSPROP_ENUMERATE)

#define NIDIUM_JSOBJ_SET_PROP_CSTR(where, name, val)                           \
    {                                                                          \
        JS::RootedString __n_rootedstring(m_Cx, JS_NewStringCopyZ(m_Cx, val)); \
        NIDIUM_JSOBJ_SET_PROP(where, name, __n_rootedstring);                  \
    }

#define NIDIUM_JSOBJ_SET_PROP_STR(where, name, val) \
    NIDIUM_JSOBJ_SET_PROP(where, name, val)

#define NIDIUM_JSOBJ_SET_PROP_INT(where, name, val) \
    NIDIUM_JSOBJ_SET_PROP(where, name, val)

/* Getter only */
// XXX : JSPROP_READONLY make an assertion in jsobj.cpp when calling
// Object.getOwnPropertyDescriptor

#define NIDIUM_JS_PSG(name, tinyid, getter_func)                        \
    {                                                                   \
        name, JSPROP_PERMANENT | /*JSPROP_READONLY |*/ JSPROP_ENUMERATE \
                  | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS,            \
            NIDIUM_JS_GETTER(tinyid, getter_func), JSOP_NULLWRAPPER     \
    }

/* Setter only */
#define NIDIUM_JS_PSS(name, tinyid, setter_func)                  \
    {                                                             \
        name, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED \
                  | JSPROP_NATIVE_ACCESSORS,                      \
            NIDIUM_JS_STUBGETTER(tinyid),                         \
            NIDIUM_JS_SETTER_WRS(tinyid, setter_func)             \
    }

/* Both */
#define NIDIUM_JS_PSGS(name, tinyid, getter_func, setter_func)    \
    {                                                             \
        name, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED \
                  | JSPROP_NATIVE_ACCESSORS,                      \
            NIDIUM_JS_GETTER(tinyid, getter_func),                \
            NIDIUM_JS_SETTER(tinyid, setter_func)                 \
    }

// {{{ Getter / Setter macro's
/*
    Tinyid were removed in SM31.
    This template act as a workaround (create a unique getter/setter and keep a
   unique identifier)
*/
#define NIDIUM_JS_SETTER(tinyid, setter)                                                                                   \
    {                                                                                                                      \
        {                                                                                                                  \
            JS_CAST_NATIVE_TO((Nidium::Binding::JSPropertyAccessors::Setter<tinyid, setter>), JSStrictPropertyOp), nullptr \
        }                                                                                                                  \
    }
#define NIDIUM_JS_SETTER_WRS(tinyid, setter)                                                                                               \
    {                                                                                                                                      \
        {                                                                                                                                  \
            JS_CAST_NATIVE_TO((Nidium::Binding::JSPropertyAccessors::SetterWithReservedSlot<tinyid, setter>), JSStrictPropertyOp), nullptr \
        }                                                                                                                                  \
    }
#define NIDIUM_JS_GETTER(tinyid, getter)                                                                             \
    {                                                                                                                \
        {                                                                                                            \
            JS_CAST_NATIVE_TO((Nidium::Binding::JSPropertyAccessors::Getter<tinyid, getter>), JSPropertyOp), nullptr \
        }                                                                                                            \
    }
#define NIDIUM_JS_STUBGETTER(tinyid)                                                                             \
    {                                                                                                            \
        {                                                                                                        \
            JS_CAST_NATIVE_TO((Nidium::Binding::JSPropertyAccessors::NullGetter<tinyid>), JSPropertyOp), nullptr \
        }                                                                                                        \
    }
// }}}

namespace Nidium {
namespace Binding {

// {{{ JSPropertyAccessors
struct JSPropertyAccessors
{
    typedef bool (*JSGetterOp)(JSContext *cx,
                               JS::HandleObject obj,
                               uint8_t id,
                               bool strict,
                               JS::MutableHandleValue vp);

    typedef bool (*JSSetterOp)(JSContext *cx,
                               JS::HandleObject obj,
                               uint8_t id,
                               JS::MutableHandleValue vp);

    template <uint8_t TINYID, JSGetterOp FN>
    static bool Setter(JSContext *cx, unsigned argc, JS::Value *vp)
    {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        JS::RootedObject obj(cx, JS_THIS_OBJECT(cx, vp));

        if (!obj) return false;
        JS::RootedValue val(cx, args.get(0));
        bool ret = FN(cx, obj, TINYID, true, &val);

        args.rval().set(val);

        return ret;
    }

    template <uint8_t TINYID, JSGetterOp FN>
    static bool
    SetterWithReservedSlot(JSContext *cx, unsigned argc, JS::Value *vp)
    {
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
    static bool Getter(JSContext *cx, unsigned argc, JS::Value *vp)
    {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        JS::RootedObject obj(cx, JS_THIS_OBJECT(cx, vp));

        if (!obj) return false;

        return FN(cx, obj, TINYID, args.rval());
    }

    template <uint8_t TINYID>
    static bool NullGetter(JSContext *cx, unsigned argc, JS::Value *vp)
    {

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

