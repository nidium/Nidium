/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <jswrapper.h>

#include "Binding/JSDebugger.h"
#include "Binding/JSConsole.h"


namespace Nidium {
namespace Binding {


JSDebuggerCompartment *JSDebuggerCompartment::Constructor(JSContext *cx,
                                                          JS::CallArgs &args,
                                                          JS::HandleObject obj)
{
    JSDebuggerCompartment *debuggerCpt = new JSDebuggerCompartment(cx);

    if (JS_IsExceptionPending(cx)) {
        delete debuggerCpt;
        return nullptr;
    }

    return debuggerCpt;
}

JSDebuggerCompartment::~JSDebuggerCompartment()
{
    NidiumLocalContext::UnrootObject(m_Debugger);
}

JSDebuggerCompartment::JSDebuggerCompartment(JSContext *cx)
{
    JS::RootedObject mainGbl(cx, JS::CurrentGlobalOrNull(cx));
    if (!mainGbl) {
        JS_ReportErrorUTF8(cx, "Cannot get current global");
        return;
    }

    // New global object & compartment for the Debugger
    JS::RootedObject gbl(cx, NidiumJS::CreateJSGlobal(cx));
    if (!gbl) {
        JS_ReportErrorUTF8(cx, "Cannot create global for Debugger compartment");
        return;
    }

    m_Global      = gbl;
    m_Compartment = JS_EnterCompartment(cx, gbl);

    // Expose the Debugger object & console inside the compartment
    JS_DefineDebuggerObject(cx, gbl);
    JSConsole::RegisterObject(cx);

    // The global object is going to be shared with the Debugger compartment
    // we need to wrap it.
    JS_WrapObject(cx, &mainGbl);

    JS_LeaveCompartment(cx, m_Compartment);

    JS::RootedValue rval(cx);
    JS::AutoValueArray<1> params(cx);
    params[0].setObjectOrNull(mainGbl);

    this->run(cx, "return (new Debugger(arguments[0]));", params, &rval);

    if (!rval.isObject()) {
        JS_ReportErrorUTF8(
            cx,
            "Failed to initialize the Debugger : Debugger is not an object");
        return;
    }

    m_Debugger = rval.toObjectOrNull();

    NidiumLocalContext::RootObjectUntilShutdown(m_Debugger);
}

bool JSDebuggerCompartment::JS_run(JSContext *cx, JS::CallArgs &args)
{
    unsigned int argc = args.length();

    if (!args[0].isObject()
        || !JS::IsCallable(args[0].toObjectOrNull())) {
        JS_ReportErrorUTF8(cx, "First argument must be a function.");
        return false;
    }

    char *scoped = nullptr;
    {
        JS::RootedFunction fun(cx);
        JS::RootedString funStr(cx);

        if ((fun = JS_ValueToFunction(cx, args[0])) == nullptr
            || (funStr = JS_DecompileFunction(cx, fun, 0)) == nullptr) {
            JS_ReportErrorUTF8(cx, "Invalid function");
            return false;
        }

        JSAutoByteString tmp(cx, funStr);
        scoped = new char[strlen(tmp.ptr()) + 128];

        sprintf(scoped, "return %c%s%s", '(', tmp.ptr(),
                ").apply(this, Array.prototype.slice.apply(arguments));");
    }

    JS::RootedValue rval(cx);

    JS_EnterCompartment(cx, m_Global);

    JS::AutoValueVector params(cx);
    params.resize(argc);

    params[0].setObjectOrNull(m_Debugger);

    for (int i = 1; i < argc; i++) {
        JS::RootedValue tmp(cx, args[i]);
        JS_WrapValue(cx, &tmp);
        params[i].set(tmp);
    }

    JS_LeaveCompartment(cx, m_Compartment);

    bool ok = this->run(cx, scoped, params, &rval);

    args.rval().set(rval);

    return ok;
}

bool JSDebuggerCompartment::run(JSContext *cx,
                                const char *funStr,
                                const JS::HandleValueArray &args,
                                JS::MutableHandleValue rval)
{
    JS_EnterCompartment(cx, m_Global);

    // Automatically wrap rval for ease of use
    JS_WrapValue(cx, rval);

    JS::RootedObject gbl(cx, m_Global);
    JS::AutoSaveContextOptions asco(cx);
    JS::CompileOptions options(cx);

    options.setUTF8(true)
        .setFileAndLine("Debugger.run", 1);

    JS::AutoObjectVector scopeChain(cx);

    JS::RootedFunction fn(cx);

    if (!JS::CompileFunction(cx, scopeChain, options, nullptr, 0,
                        nullptr, funStr, strlen(funStr), &fn)) {

        JS_LeaveCompartment(cx, m_Compartment);
        JS_ReportErrorUTF8(cx, "Can't compile function");
        return false;
    }


    JS::AutoValueArray<1> foo(cx);

    bool ok = JS_CallFunction(cx, gbl, fn, args, rval);

    JS_LeaveCompartment(cx, m_Compartment);

    return ok;
}

JSFunctionSpec *JSDebuggerCompartment::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSDebuggerCompartment, run, 1),
        JS_FS_END 
    };

    return funcs;
}


} // namespace Binding
} // namespace Nidium
