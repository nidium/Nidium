/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "JSProcess.h"

#include <native_netlib.h>

namespace Nidium {
namespace Binding {

static void Process_Finalize(JSFreeOp *fop, JSObject *obj);
static bool nidium_setSignalHandler(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_process_exit(JSContext *cx, unsigned argc, JS::Value *vp);

static JSClass Process_class = {
    "NativeProcess", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Process_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

JSClass *JSProcess::jsclass = &Process_class;

template<>
JSClass *JSExposer<JSProcess>::jsclass = &Process_class;

static JSFunctionSpec Process_funcs[] = {
    JS_FN("setSignalHandler", nidium_setSignalHandler, 1, NATIVE_JS_FNPROPS),
    JS_FN("exit", nidium_process_exit, 1, NATIVE_JS_FNPROPS),
    JS_FS_END
};

static bool nidium_setSignalHandler(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSProcess, &Process_class);
    NIDIUM_JS_CHECK_ARGS("setSignalHandler", 1);

    JS::RootedValue func(cx);

    if (!JS_ConvertValue(cx, args[0], JSTYPE_FUNCTION, &func)) {
        JS_ReportWarning(cx, "setSignalHandler: bad callback");
        return true;
    }

    CppObj->m_SignalFunction = func;

    return true;
}

static bool nidium_process_exit(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSProcess, &Process_class);

    int code = 0;

    if (argc > 0 && args[0].isInt32()) {
        code = args[0].toInt32();
    }

    exit(code);

    return true;
}

static void Process_Finalize(JSFreeOp *fop, JSObject *obj)
{
    JSProcess *jProcess = JSProcess::getNativeClass(obj);

    if (jProcess != NULL) {
        delete jProcess;
    }
}

static int ape_kill_handler(int code, ape_global *ape)
{
    NativeJS *njs = NativeJS::getNativeClass();
    JSContext *cx = njs->cx;
    JS::RootedValue     rval(cx);

    JSProcess *jProcess = JSProcess::getNativeClass(njs);

    JS::RootedValue func(cx, jProcess->m_SignalFunction);

    if (func.isObject() && JS_ObjectIsCallable(cx, func.toObjectOrNull())) {
        JS_CallFunctionValue(cx, JS::NullPtr(), func, JS::HandleValueArray::empty(), &rval);

        return rval.isBoolean() ? !rval.toBoolean() : false;
    }

    return false;
}

void JSProcess::registerObject(JSContext *cx, char **argv, int argc, int workerId)
{
    NativeJS *njs = NativeJS::getNativeClass(cx);
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedObject ProcessObj(cx, JS_DefineObject(cx, global, JSProcess::getJSObjectName(),
        &Process_class , NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY));

    JSProcess *jProcess = new JSProcess(ProcessObj, cx);

    JS_SetPrivate(ProcessObj, jProcess);

    njs->jsobjects.set(JSProcess::getJSObjectName(), ProcessObj);

    JS_DefineFunctions(cx, ProcessObj, Process_funcs);

    JS::RootedObject jsargv(cx, JS_NewArrayObject(cx, argc));

    for (int i = 0; i < argc; i++) {
        JS::RootedString jelem(cx, JS_NewStringCopyZ(cx, argv[i]));
        JS_SetElement(cx, jsargv, i, jelem);
    }

    JS::RootedValue jsargv_v(cx, OBJECT_TO_JSVAL(jsargv));
    JS_SetProperty(cx, ProcessObj, "argv", jsargv_v);

    JS::RootedValue workerid_v(cx, JS::Int32Value(workerId));
    JS_SetProperty(cx, ProcessObj, "workerId", workerid_v);

    NativeJS::getNet()->kill_handler = ape_kill_handler;
    jProcess->m_SignalFunction.set(JS::NullHandleValue);

}

} // namespace Binding
} // namespace Nidium

