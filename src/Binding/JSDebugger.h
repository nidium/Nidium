/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsdebugger_h__
#define binding_jsdebugger_h__

#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

class JSDebuggerCompartment : public ClassMapper<JSDebuggerCompartment>
{
public:
    static void RegisterObject(JSContext *cx)
    {
        JSDebuggerCompartment::ExposeClass<0>(cx, "DebuggerCompartment");
    }

    static JSDebuggerCompartment *
    Constructor(JSContext *cx, JS::CallArgs &args, JS::HandleObject obj);

    static JSFunctionSpec *ListMethods();

    JSDebuggerCompartment(JSContext *cx);
    virtual ~JSDebuggerCompartment();

protected:
    NIDIUM_DECL_JSCALL(run);

private:
    JS::Heap<JSObject *> m_Debugger;
    JS::Heap<JSObject *> m_Global;
    JSCompartment *m_Compartment;

    bool run(JSContext *cx,
             const char *funStr,
             const JS::HandleValueArray &args,
             JS::MutableHandleValue rval);
};

} // namespace Binding
} // namespace Nidium

#endif
