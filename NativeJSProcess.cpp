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

#include <pwd.h>
#include <grp.h>

#include <native_netlib.h>

static void Process_Finalize(JSFreeOp *fop, JSObject *obj);

static bool native_process_setuser(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_setSignalHandler(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_process_exit(JSContext *cx, unsigned argc, JS::Value *vp);

extern int setgroups(size_t __n, __const gid_t *__groups);
extern int initgroups(const char * user, gid_t group);

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
    JS_FN("setUser", native_process_setuser, 1, NATIVE_JS_FNPROPS),
    JS_FN("setSignalHandler", native_setSignalHandler, 1, NATIVE_JS_FNPROPS),
    JS_FN("exit", native_process_exit, 1, NATIVE_JS_FNPROPS),
    JS_FS_END
};

static bool native_process_setuser(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString user(cx);
    JS::RootedString group(cx);
    JSAutoByteString cuser;
    JSAutoByteString cgroup;
    struct passwd *pwd;
    struct group *grp;
    int groupOk = 2;
    int userOk = 2;

    JSNATIVE_PROLOGUE_CLASS(NativeJSProcess, &Process_class);

    if (!JS_ConvertArguments(cx, args, "S/S", user.address(), group.address())) {
        return false;
    }
    if (user.get()) {
        cuser.encodeUtf8(cx, user);
        pwd = getpwnam(cuser.ptr());
        if (pwd->pw_uid == 0) {
            return false;
        }
        userOk = (setuid(pwd->pw_uid) != -1);
    }
    if (group.get()) {
        cgroup.encodeUtf8(cx, group);
        grp = getgrnam(cgroup.ptr());
        if (grp->gr_gid == 0) {
            return false;
        }
        //@TODO: check capabities
        groupOk = (setgid(grp->gr_gid) != -1);
    }
    setgroups(0, NULL);
    if (cuser.ptr() && cgroup.ptr()) {
        initgroups(cuser.ptr(), grp->gr_gid);
    }
    if (getuid() == 0 || getgid() == 0) {
        fprintf(stderr, "Running as root!");
    }
    args.rval().setBoolean(!(userOk == 0 || groupOk == 0));

    return true;
}

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
    ape_global *ape = (ape_global *)JS_GetContextPrivate(cx);
    ape->is_running = 0;

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

