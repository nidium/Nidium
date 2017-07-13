/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsutils_h__
#define binding_jsutils_h__

#include <jsapi.h>

#include "Binding/JSMacros.h"

bool JS_ConvertArguments(JSContext *cx, const JS::CallArgs &args, const char *format, ...);

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

    /*
        Convert a JS::Value to a double and return true if it's a string representing a percentage.
        e.g.

        val == "90%" returns true  and set out to 90.00
        val ==  90   returns false and set out to 90.00
    */
    static bool ValuePercent(JSContext *cx, JS::HandleValue val, double *out);

    static JSString *NewStringWithEncoding(JSContext *cx,
                                           const char *buf,
                                           size_t len,
                                           const char *encoding);

    static char16_t *
    Utf8ToUtf16(JSContext *cx, const char *str, size_t len, size_t *outputlen);
    static char *CurrentJSCaller(JSContext *cx = NULL);

    static JSFunction *ReportIfNotFunction(JSContext *cx, JS::HandleValue val);

    /*
        JSAPI only provides JS_NewArrayBufferWithContents() which takes ownership.
        We've some situation where we can't rely on zerocopy.
    */
    static JSObject *NewArrayBufferWithCopiedContents(JSContext *cx,
        size_t len, const void *data);

    static const char* GetFunctionNameBytes(JSContext* cx,
        JSFunction* fun, JSAutoByteString* bytes);

    static JSObject *NewObjectForConstructor(JSContext *cx,
        JSClass *jsclass, JS::CallArgs &args);
};

// {{{ JSTransferable
class JSTransferable
{
public:
    JSTransferable(JSContext *destCx, JSObject* destGlobal)
        : m_Val(destCx), m_DestGlobal(destGlobal), m_DestCx(destCx)
    {
        m_Val.get().setUndefined();
    }

    JSTransferable(JSContext *cx, JSContext *destCx,
                   JSObject *destGlobal, JS::HandleValue val)
        : JSTransferable(destCx, destGlobal)
    {
        this->set(cx, val);
    }

    bool isSet()
    {
        return m_Data != nullptr || !m_Val.get().isNullOrUndefined();
    }

    virtual bool set(JSContext *cx, JS::HandleValue val);

    JS::Value get();

    JSContext *getJSContext()
    {
        return m_DestCx;
    }

    void setPrivate(void *priv)
    {
        m_Private = priv;
    }
    template <typename T>
    T *getPrivate()
    {
        return reinterpret_cast<T *>(m_Private);
    }

    virtual ~JSTransferable();

protected:
    JS::PersistentRootedValue m_Val;
    JS::Heap<JSObject *> m_DestGlobal;
    JSContext *m_DestCx = nullptr;
    uint64_t *m_Data    = nullptr;
    size_t m_Bytes      = 0;

private:
    void *m_Private = nullptr;

    bool transfert();
};

class JSTransferableFunction : public JSTransferable
{
public:
    JSTransferableFunction(JSContext *destCx, JSObject *destGlobal)
        : JSTransferable(destCx, destGlobal) {}

    bool set(JSContext *cx, JS::HandleValue val) override;
    bool call(JS::HandleObject obj,
              JS::HandleValueArray params,
              JS::MutableHandleValue rval);

    virtual ~JSTransferableFunction(){}
};
// }}}

// {{{ JSObjectBuilder
class JSObjectBuilder
{
public:
    JSObjectBuilder(JSContext *cx, JSClass *clasp = NULL) : m_Obj(cx)
    {
        m_Cx  = cx;
        m_Obj = JS_NewObject(m_Cx, clasp);
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
        ndm_log(NDM_LOG_ERROR, "JSUtils", "JSObjectBuilder using a JSString is deprecated");
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
        return JS::ObjectValue(*m_Obj);
    }

    operator JSObject *()
    {
        return m_Obj;
    }

    operator JS::Value()
    {
        return JS::ObjectValue(*m_Obj);
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
