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
        && !__curopt.isNullOrUndefined())

#define NIDIUM_JS_GET_OPT_TYPE(obj, name, type)             \
    if (obj && JS_GetProperty(cx, obj, name, &__curopt)     \
        && !__curopt.isNullOrUndefined() \
        && __curopt.is##type())

#define JSOBJ_CALLFUNCNAME(where, name, argv)                           \
    {                                                                   \
        JS::RootedValue _oncallback(cx);                                \
        JS::RootedValue _rval(cx);                                      \
        if (JS_GetProperty(cx, where, name, &_oncallback)               \
            && JS_TypeOfValue(cx, _oncallback) == JSTYPE_FUNCTION) {    \
            JS_CallFunctionValue(cx, where, _oncallback, argv, &_rval); \
        }                                                               \
    }

#define JSOBJ_CALLFUNCNAME_RVAL(where, name, argv, rval)                \
    {                                                                   \
        JS::RootedValue _oncallback(cx);                                \
        JS::RootedValue rval(cx);                                       \
        if (JS_GetProperty(cx, where, name, &_oncallback)               \
            && JS_TypeOfValue(cx, _oncallback) == JSTYPE_FUNCTION) {    \
            JS_CallFunctionValue(cx, where, _oncallback, argv, &rval);  \
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

#define NIDIUM_JSOBJ_SET_PROP_FLOAT(where, name, val) \
    NIDIUM_JSOBJ_SET_PROP(where, name, (double)val)

#define NIDIUM_JSOBJ_SET_PROP_DOUBLE(where, name, val) \
    NIDIUM_JSOBJ_SET_PROP(where, name, val)

#endif

