/*
    NativeJS Core Library
    Copyright (C) 2014 Anthony Catel <paraboul@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "NativeJSProcess.h"

#include <native_netlib.h>

static void Process_Finalize(JSFreeOp *fop, JSObject *obj);
static bool native_setSignalHandler(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_process_exit(JSContext *cx, unsigned argc, JS::Value *vp);

static JSClass Process_class = {
    "NativeProcess", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Process_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

JSClass *NativeJSProcess::jsclass = &Process_class;

template<>
JSClass *NativeJSExposer<NativeJSProcess>::jsclass = &Process_class;

static JSFunctionSpec Process_funcs[] = {
    JS_FN("setSignalHandler", native_setSignalHandler, 1, NATIVE_JS_FNPROPS),
    JS_FN("exit", native_process_exit, 1, NATIVE_JS_FNPROPS),
    JS_FS_END
};

static bool native_setSignalHandler(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeJSProcess, &Process_class);
    NATIVE_CHECK_ARGS("setSignalHandler", 1);

    JS::RootedValue func(cx);

    if (!JS_ConvertValue(cx, args[0], JSTYPE_FUNCTION, &func)) {
        JS_ReportWarning(cx, "setSignalHandler: bad callback");
        return true;
    }

    CppObj->m_SignalFunction = func;

    return true;
}

static bool native_process_exit(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeJSProcess, &Process_class);

    int code = 0;

    if (argc > 0 && args[0].isInt32()) {
        code = args[0].toInt32();
    }

    exit(code);

    return true;
}

static void Process_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSProcess *jProcess = NativeJSProcess::getNativeClass(obj);

    if (jProcess != NULL) {
        delete jProcess;
    }
}

static int ape_kill_handler(int code, ape_global *ape)
{
    NativeJS *njs = NativeJS::getNativeClass();
    JSContext *cx = njs->cx;
    JS::RootedValue     rval(cx);

    NativeJSProcess *jProcess = NativeJSProcess::getNativeClass(njs);

    JS::RootedValue func(cx, jProcess->m_SignalFunction);

    if (func.isObject() && JS_ObjectIsCallable(cx, func.toObjectOrNull())) {
        JS_CallFunctionValue(cx, JS::NullPtr(), func, JS::HandleValueArray::empty(), &rval);

        return rval.isBoolean() ? !rval.toBoolean() : false;
    }

    return false;
}

void NativeJSProcess::registerObject(JSContext *cx, char **argv, int argc, int workerId)
{
    NativeJS *njs = NativeJS::getNativeClass(cx);
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedObject ProcessObj(cx, JS_DefineObject(cx, global, NativeJSProcess::getJSObjectName(),
        &Process_class , NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY));

    NativeJSProcess *jProcess = new NativeJSProcess(ProcessObj, cx);

    JS_SetPrivate(ProcessObj, jProcess);

    njs->jsobjects.set(NativeJSProcess::getJSObjectName(), ProcessObj);

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

