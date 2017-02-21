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

class JSVMCompartment;

class JSVM : public ClassMapper<JSVM>
{
public:
    JSVM();

    static void RegisterObject(JSContext *cx);
    static JSObject *RegisterModule(JSContext *cx);

    static JSFunctionSpec *ListStaticMethods();

protected:
    NIDIUM_DECL_JSCALL_STATIC(run);
    NIDIUM_DECL_JSCALL_STATIC(runInFunction);

private:
    static bool ParseCommonOptions(JSContext *cx,
                                   JS::HandleValue optionsValue,
                                   JSVMCompartment **compartment,
                                   JS::MutableHandleObject scope,
                                   JS::MutableHandleString filename,
                                   int32_t *lineOffset);

    static bool ParseRunData(JSContext *cx,
                             JS::HandleValue arg,
                             char16_t **data,
                             size_t *len);
};

class JSVMCompartment : public ClassMapper<JSVMCompartment>
{
public:
    static void RegisterObject(JSContext *cx, JS::HandleObject vm);

    static JSVMCompartment *
    Constructor(JSContext *cx, JS::CallArgs &args, JS::HandleObject obj);

    static JSClass *GetJSClass();

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
        return m_Global;
    }

    JSVMCompartment(JSContext *cx, JS::HandleObject obj, bool defineDebugger);
    virtual ~JSVMCompartment();

private:
    JS::Heap<JSObject *> m_Obj;
    JS::Heap<JSObject *> m_Global;

    bool get(JSContext *cx, JS::HandleId id, JS::MutableHandleValue vp);
    bool set(JSContext *cx,
             JS::HandleId id,
             JS::MutableHandleValue vp,
             JS::ObjectOpResult &result);
};

} // namespace Binding
} // namespace Nidium

#endif
