/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsvm_h__
#define binding_jsvm_h__

#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

class JSVMSandbox;

class JSVM : public ClassMapper<JSVM>
{
public:
    JSVM();

    static void RegisterObject(JSContext *cx);
    static JSObject *RegisterModule(JSContext *cx);

    static JSFunctionSpec *ListStaticMethods();

protected:
    NIDIUM_DECL_JSCALL_STATIC(run);

private:
    static bool ParseOptions(JSContext *cx,
                             JS::HandleValue optionsValue,
                             JSVMSandbox **compartment,
                             JS::MutableHandleObject scope,
                             JS::MutableHandleString filename,
                             int32_t *lineOffset,
                             bool *debugger);

    static bool GetRunData(JSContext *cx,
                           JS::HandleValue arg,
                           char16_t **data,
                           size_t *len);
};

class JSVMSandbox : public ClassMapper<JSVMSandbox>
{
public:
    friend JSVM;

    enum
    {
        kSandbox_HasDebugger = 0x01,
        kSandbox_HasStdClass = 0x02
    } Flags;

    static JSClass *GetJSClass();
    static JSObject *CreateObject(JSContext *cx, JSVMSandbox *instance);

    static bool Getter(JSContext *cx,
                       JS::HandleObject obj,
                       JS::HandleId id,
                       JS::MutableHandleValue vp);
    static bool Setter(JSContext *cx,
                       JS::HandleObject obj,
                       JS::HandleId id,
                       JS::MutableHandleValue vp,
                       JS::ObjectOpResult &result);

    JSObject *getGlobal()
    {
        return m_SandboxGlobal;
    }

    JSObject *getUserObject()
    {
        return m_Obj;
    }

    bool hasDebugger()
    {
        return m_FLags & kSandbox_HasDebugger;
    }

    bool hasStdClass()
    {
        return m_FLags & kSandbox_HasStdClass;
    }

    JSVMSandbox(JSContext *cx, JS::HandleObject obj, int flags);
    virtual ~JSVMSandbox();

private:
    JS::Heap<JSObject *> m_Obj;
    JS::Heap<JSObject *> m_SandboxGlobal;
    JS::Heap<JSObject *> m_MainGlobal;
    int m_FLags;

    bool copy(JSContext *cx, JS::HandleObject from, JS::HandleObject to);
    bool get(JSContext *cx, JS::HandleId id, JS::MutableHandleValue vp);
    bool set(JSContext *cx,
             JS::HandleId id,
             JS::MutableHandleValue vp,
             JS::ObjectOpResult &result);
};

} // namespace Binding
} // namespace Nidium

#endif
