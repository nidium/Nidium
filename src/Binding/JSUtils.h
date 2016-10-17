/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsutils_h__
#define binding_jsutils_h__

#include <jspubtd.h>
#include <jsapi.h>

#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

class JSUtils
{
public:
    /*
        output a jsval corresponding to the given data.
        Encoding accepts : utf8, NULL, anything else.
        In case encoding is NULL, the jsval is an arraybuffer.
    */
    static bool StrToJsval(JSContext *cx,
                           const char *buf,
                           size_t len,
                           JS::MutableHandleValue jval,
                           const char *encoding);

    static JSString *NewStringWithEncoding(JSContext *cx,
                                           const char *buf,
                                           size_t len,
                                           const char *encoding);

    static char16_t *
    Utf8ToUtf16(JSContext *cx, const char *str, size_t len, size_t *outputlen);
    static char *CurrentJSCaller(JSContext *cx = NULL);
};

// {{{ JSObjectBuilder
class JSObjectBuilder
{
public:
    JSObjectBuilder(JSContext *cx, JSClass *clasp = NULL) : m_Obj(cx)
    {
        m_Cx  = cx;
        m_Obj = JS_NewObject(m_Cx, clasp, JS::NullPtr(), JS::NullPtr());
    };

    JSObjectBuilder(JSContext *cx, JS::HandleObject wrapped) : m_Obj(cx)
    {
        m_Obj = wrapped;
        m_Cx  = cx;
    };

    void set(const char *name, JS::HandleValue jval)
    {
        JS::RootedObject obj(m_Cx, m_Obj);
        NIDIUM_JSOBJ_SET_PROP(obj, name, jval);
    }

    void set(const char *name, JS::HandleObject jobj)
    {
        JS::RootedObject obj(m_Cx, m_Obj);
        JS::RootedValue jval(m_Cx);
        jval.setObjectOrNull(jobj);
        NIDIUM_JSOBJ_SET_PROP(obj, name, jval);
    }

    void set(const char *name, JS::HandleString value)
    {
        JS::RootedObject obj(m_Cx, m_Obj);
        NIDIUM_JSOBJ_SET_PROP_STR(obj, name, value);
    }

    void set(const char *name, JSString *str)
    {
        printf("JSObjectBuilder using a JSString is deprecated\n");
        exit(1);
    }

    void set(const char *name, const char *value)
    {
        JS::RootedObject obj(m_Cx, m_Obj);
        NIDIUM_JSOBJ_SET_PROP_CSTR(obj, name, value);
    }

    void set(const char *name, uint32_t value)
    {
        JS::RootedObject obj(m_Cx, m_Obj);
        NIDIUM_JSOBJ_SET_PROP_INT(obj, name, value);
    }

    void set(const char *name, int32_t value)
    {
        JS::RootedObject obj(m_Cx, m_Obj);
        NIDIUM_JSOBJ_SET_PROP_INT(obj, name, value);
    }

    void set(const char *name, double value)
    {
        JS::RootedObject obj(m_Cx, m_Obj);
        NIDIUM_JSOBJ_SET_PROP(obj, name, value);
    }

    void set(const char *name, bool value)
    {
        JS::RootedObject obj(m_Cx, m_Obj);
        NIDIUM_JSOBJ_SET_PROP(obj, name, value);
    }

    JSObject *obj() const
    {
        return m_Obj;
    }

    JS::Value jsval() const
    {
        return OBJECT_TO_JSVAL(m_Obj);
    }

    operator JSObject *()
    {
        return m_Obj;
    }

    operator JS::Value()
    {
        return OBJECT_TO_JSVAL(m_Obj);
    }

    ~JSObjectBuilder(){};

private:
    JS::PersistentRootedObject m_Obj;
    JSContext *m_Cx;
};
// }}}

} // namespace Binding
} // namespace Nidium

#endif
