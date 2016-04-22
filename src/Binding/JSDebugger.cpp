/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <jswrapper.h>
#include <vm/Debugger.h>

#include "JSDebugger.h"
#include "JSConsole.h"

namespace Nidium {
namespace Binding {

// {{{ preamble

static JSClass DebuggerContext_class = {
    "DebuggerContext", JSCLASS_HAS_RESERVED_SLOTS(2),
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

// {{{ implementation

static bool nidium_debugger_run(JSContext* cx, unsigned argc, JS::Value* vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    NIDIUM_JS_CHECK_ARGS("run", 2);

    if (!args[0].isObject()) {
        JS_ReportError(cx, "Invalid DebuggerContext");
        return false;
    }

    JS::RootedObject debuggerContext(cx, js::CheckedUnwrap(args[0].toObjectOrNull()));

    if (!debuggerContext) {
        JS_ReportError(cx, "Invalid DebuggerContext");
        return false;
    }

    if (!JS_InstanceOf(cx, debuggerContext, &DebuggerContext_class, nullptr)) {
        return false;
    }

    if (!args[1].isObject() ||
            !JS_ObjectIsCallable(cx, args[1].toObjectOrNull())) {
        JS_ReportError(cx, "Invalid arguments. Second argument must be a function.");
        return false;
    }

    char *scoped = nullptr;
    {
        JS::RootedFunction fun(cx);
        JS::RootedString funStr(cx);

        if ((fun = JS_ValueToFunction(cx, args[1])) == nullptr ||
                (funStr = JS_DecompileFunction(cx, fun, 0)) == nullptr) {
            JS_ReportError(cx, "Invalid function");
            return false;
        }

        JSAutoByteString tmp(cx, funStr);
        scoped = new char[strlen(tmp.ptr()) + 128];

        sprintf(scoped, "return %c%s%s", '(', tmp.ptr(), ").apply(this, Array.prototype.slice.apply(arguments));");
    }

    JSCompartment *cpt = JS_EnterCompartment(cx, debuggerContext);

    JS::RootedValue gblVal(cx, JS_GetReservedSlot(debuggerContext, 0));
    JS::RootedObject gbl(cx, gblVal.toObjectOrNull());

    JS::RootedValue dbgVal(cx, JS_GetReservedSlot(debuggerContext, 1));
    JS::RootedObject dbg(cx, dbgVal.toObjectOrNull());

    JS::RootedValue ret(cx);
    JS::AutoSaveContextOptions asco(cx);
    JS::CompileOptions options(cx);
    JS::ContextOptionsRef(cx).setVarObjFix(true);

    options.setUTF8(true)
           .setFileAndLine("Debugger.run", 1)
           .setCompileAndGo(true);

    JS::AutoValueVector params(cx);
    params.resize(argc - 1);
    params[0].setObjectOrNull(dbg);
    for (int i = 2; i < argc; i++) {
        JS::RootedValue tmp(cx, args[i]);
        JS_WrapValue(cx, &tmp);
        params[i - 1] = tmp;
    }

    JS::RootedFunction fn(cx, JS::CompileFunction(cx, gbl, options, nullptr, 0, nullptr, scoped, strlen(scoped)));

    if (fn == NULL) {
        JS_LeaveCompartment(cx, cpt);
        JS_ReportError(cx, "Can't compile function");
        return false;
    }

    if (JS_CallFunction(cx, gbl, fn, params, &ret) == false) {
        JS_LeaveCompartment(cx, cpt);
        return false;
    }

    JS_LeaveCompartment(cx, cpt);

    JS_WrapValue(cx, &ret);
    args.rval().set(ret);

    return true;
}

static bool nidium_debugger_create(JSContext* cx, unsigned argc, JS::Value* vp)
{
    JS::RootedObject mainGbl(cx, JS::CurrentGlobalOrNull(cx));
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JS::RootedObject gbl(cx, NidiumJS::CreateJSGlobal(cx));
    if (!gbl) {
        fprintf(stderr, "Failed to create global object\n");
        return false;
    }

    JSCompartment *cpt = JS_EnterCompartment(cx, gbl);

    JSDebugger::registerObject(cx);
    /* Expose console object for easy "debugging" */
    JSConsole::registerObject(cx);

    JS::RootedScript script(cx);
    JS::AutoSaveContextOptions asco(cx);
    JS::CompileOptions options(cx);
    JS::RootedFunction cf(cx);
    JS::RootedValue ret(cx);

    JS::RootedObject debuggerContext(cx, JS_NewObject(cx, &DebuggerContext_class, JS::NullPtr(), JS::NullPtr()));

    JS::ContextOptionsRef(cx).setVarObjFix(true);
    options.setUTF8(true)
           .setFileAndLine("Debugger", 1)
           .setCompileAndGo(true);

    JS_WrapObject(cx, &mainGbl);

    const char *debuggerInit = "return (new Debugger(arguments[0]));";
    const char *fargs[1];
    fargs[0] = "global";

    cf = JS::CompileFunction(cx, gbl, options, "DebuggerCreate", 1, fargs, debuggerInit, strlen(debuggerInit));

    if (cf == NULL) {
        JS_LeaveCompartment(cx, cpt);
        JS_ReportError(cx, "Can't init Debugger");
        printf("failed to init\n");
        return false;
    }

    JS::AutoValueArray<1> params(cx);
    params[0].setObjectOrNull(mainGbl);

    if (JS_CallFunction(cx, gbl, cf, params, &ret) == false) {
        JS_LeaveCompartment(cx, cpt);
        JS_ReportPendingException(cx);
        JS_ReportError(cx, "Failed to init Debugger");
        return false;
    }

    JS::RootedValue gblVal(cx, OBJECT_TO_JSVAL(gbl));
    JS_SetReservedSlot(debuggerContext, 0, gblVal);
    JS_SetReservedSlot(debuggerContext, 1, ret);

    JS_LeaveCompartment(cx, cpt);

    JS_WrapObject(cx, &debuggerContext);
    args.rval().setObjectOrNull(debuggerContext);

    return true;
}

// {{{ registration

void JSDebugger::registerObject(JSContext *cx)
{
    JS::RootedObject gbl(cx, JS::CurrentGlobalOrNull(cx));

    JS_DefineDebuggerObject(cx, gbl);

    /* 
     * Debugger object must live in another compartment
     * so we extend the Debugger object with two static methods
     * - create : Initialize the Debugger in a new compartment
     * - run : Execute code inside the Debugger compartment
     */

    JS::RootedValue dbgVal(cx);
    JS_GetProperty(cx, gbl, "Debugger", &dbgVal);
    JS::RootedObject dbg(cx, dbgVal.toObjectOrNull());

    JS_DefineFunction(cx, dbg, "create", nidium_debugger_create, 0, 1);
    JS_DefineFunction(cx, dbg, "run", nidium_debugger_run, 0, 1);
}

} // namespace Binding
} // namespace Nidium

