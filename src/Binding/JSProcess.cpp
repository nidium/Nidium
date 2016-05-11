/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSProcess.h"

#include <ape_netlib.h>

#include "Core/Path.h"
#include "Binding/JSUtils.h"

using Nidium::Core::Path;
using Nidium::Binding::JSUtils;

namespace Nidium {
namespace Binding {

// {{{ Preamble

static void Process_Finalize(JSFreeOp *fop, JSObject *obj);
static bool nidium_process_setSignalHandler(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_process_exit(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_process_cwd(JSContext *cx, unsigned argc, JS::Value *vp);

static JSClass Process_class = {
    "NidiumProcess", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Process_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

JSClass *JSProcess::jsclass = &Process_class;

template<>
JSClass *JSExposer<JSProcess>::jsclass = &Process_class;

static JSFunctionSpec Process_funcs[] = {
    JS_FN("setSignalHandler", nidium_process_setSignalHandler, 1, NIDIUM_JS_FNPROPS),
    JS_FN("exit", nidium_process_exit, 1, NIDIUM_JS_FNPROPS),
    JS_FN("cwd", nidium_process_cwd, 0, NIDIUM_JS_FNPROPS),
    JS_FS_END
};

static int ape_kill_handler(int code, ape_global *ape)
{
    NidiumJS *njs = NidiumJS::GetObject();
    JSContext *cx = njs->m_Cx;
    JS::RootedValue     rval(cx);

    JSProcess *jProcess = JSProcess::GetObject(njs);

    JS::RootedValue func(cx, jProcess->m_SignalFunction);

    if (func.isObject() && JS_ObjectIsCallable(cx, func.toObjectOrNull())) {
        JS_CallFunctionValue(cx, JS::NullPtr(), func, JS::HandleValueArray::empty(), &rval);

        return rval.isBoolean() ? !rval.toBoolean() : false;
    }

    return false;
}

// }}}

// {{{ Implementation

static bool nidium_process_setSignalHandler(JSContext *cx, unsigned argc, JS::Value *vp)
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

    quick_exit(code);

    return true;
}

static bool nidium_process_cwd(JSContext *cx, unsigned argc, JS::Value *vp)
{
    Path cur(JSUtils::CurrentJSCaller(cx), false, true);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (cur.dir() == NULL) {
        args.rval().setUndefined();
        return true;
    }

    JS::RootedString res(cx, JS_NewStringCopyZ(cx, cur.dir()));

    args.rval().setString(res);

    return true;
}


static void Process_Finalize(JSFreeOp *fop, JSObject *obj)
{
    JSProcess *jProcess = JSProcess::GetObject(obj);

    if (jProcess != NULL) {
        delete jProcess;
    }
}

// }}}

// {{{ Registration

void JSProcess::RegisterObject(JSContext *cx, char **argv, int argc, int workerId)
{
    NidiumJS *njs = NidiumJS::GetObject(cx);
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedObject ProcessObj(cx, JS_DefineObject(cx, global, JSProcess::GetJSObjectName(),
        &Process_class , NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY));

    JSProcess *jProcess = new JSProcess(ProcessObj, cx);

    JS_SetPrivate(ProcessObj, jProcess);

    njs->m_JsObjects.set(JSProcess::GetJSObjectName(), ProcessObj);

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

    NidiumJS::GetNet()->kill_handler = ape_kill_handler;
    jProcess->m_SignalFunction.set(JS::NullHandleValue);

}

// }}}

} // namespace Binding
} // namespace Nidium

