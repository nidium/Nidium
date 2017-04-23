/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSProcess.h"

#include <ape_netlib.h>

#include <pwd.h>
#include <grp.h>

#include "Core/Path.h"
#include "Binding/JSUtils.h"
#include "Binding/NidiumJS.h"

using Nidium::Core::Path;
using Nidium::Binding::JSUtils;

namespace Nidium {
namespace Binding {

// {{{ Preamble


static int ape_kill_handler(int code, ape_global *ape)
{
    NidiumJS *njs = NidiumJS::GetObject();
    JSContext *cx = njs->m_Cx;
    JS::RootedValue rval(cx);

    JSProcess *jProcess = JSProcess::GetInstanceSingleton();

    JS::RootedValue func(cx, JS_GetReservedSlot(jProcess->getJSObject(), 0));

    if (func.isObject() && JS::IsCallable(func.toObjectOrNull())) {
        JS_CallFunctionValue(cx, nullptr, func,
                             JS::HandleValueArray::empty(), &rval);

        return rval.isBoolean() ? !rval.toBoolean() : false;
    }

    return false;
}

// }}}

// {{{ Implementation
bool JSProcess::JS_getOwner(JSContext *cx, JS::CallArgs &args)
{
    int uid                 = getuid();
    int gid                 = getgid();
    struct passwd *userInfo = getpwuid(uid);
    struct group *groupInfo = getgrgid(gid);

    if (!userInfo || !groupInfo) {
        JS_ReportError(cx, "Failed to retrieve process owner");
        return false;
    }

    JS::RootedObject obj(cx, JS_NewPlainObject(cx));

    JS::RootedString userStr(cx, JS_NewStringCopyZ(cx, userInfo->pw_name));
    JS::RootedString groupStr(cx, JS_NewStringCopyZ(cx, groupInfo->gr_name));

    JS_DefineProperty(cx, obj, "uid", uid,
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
    JS_DefineProperty(cx, obj, "gid", gid,
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
    JS_DefineProperty(cx, obj, "user", userStr,
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
    JS_DefineProperty(cx, obj, "group", groupStr,
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);

    JS::RootedValue val(cx);
    val.setObjectOrNull(obj);

    args.rval().set(val);

    return true;
}

bool JSProcess::JS_setOwner(JSContext *cx, JS::CallArgs &args)
{
    struct passwd *userInfo = nullptr;
    struct group *groupInfo = nullptr;
    int argc = args.length();

    if (args[0].isNumber()) {
        int uid = args[0].toInt32();

        userInfo = getpwuid(uid);

        if (userInfo == nullptr) {
            if (errno == 0) {
                JS_ReportError(cx, "User ID \"%d\" not found", uid);
            } else {
                JS_ReportError(cx, "Error retrieving user ID \"%d\" error : %s",
                               uid, strerror(errno));
            }
            return false;
        }
    } else if (args[0].isString()) {
        JS::RootedString user(cx, args[0].toString());
        JSAutoByteString cuser;
        cuser.encodeUtf8(cx, user);

        userInfo = getpwnam(cuser.ptr());

        if (userInfo == nullptr) {
            if (errno == 0) {
                JS_ReportError(cx, "User name \"%s\" not found", cuser.ptr());
            } else {
                JS_ReportError(cx,
                               "Error retrieving user name \"%s\" error : %s",
                               cuser.ptr(), strerror(errno));
            }
            return false;
        }
    } else {
        JS_ReportError(cx,
                       "Invalid first argument (Number or String expected)");
        return false;
    }

    if (argc > 1 && args[1].isNumber()) {
        int gid = args[1].toInt32();

        groupInfo = getgrgid(gid);

        if (groupInfo == nullptr) {
            if (errno == 0) {
                JS_ReportError(cx, "Group ID \"%d\" not found", gid);
            } else {
                JS_ReportError(cx,
                               "Error retrieving group ID \"%d\" error : %s",
                               gid, strerror(errno));
            }
            return false;
        }
    } else if (argc > 1) {
        JS::RootedString group(cx, args[1].toString());
        JSAutoByteString cgroup;

        cgroup.encodeUtf8(cx, group);
        groupInfo = getgrnam(cgroup.ptr());

        if (groupInfo == nullptr) {
            if (errno == 0) {
                JS_ReportError(cx, "Group name \"%s\" not found", cgroup.ptr());
            } else {
                JS_ReportError(cx,
                               "Error retrieving group name \"%s\" error : %s",
                               cgroup.ptr(), strerror(errno));
            }
            return false;
        }
    }

    /*
        When dropping privileges from root, the setgroups call will
        remove any extraneous groups. If we don't call this, then
        even though our uid has dropped, we may still have groups
        that enable us to do super-user things. This will fail if we
        aren't root, so don't bother checking the return value, this
        is just done as an optimistic privilege dropping function.
    */
    setgroups(0, NULL);

    if (groupInfo != nullptr && setgid(groupInfo->gr_gid) != 0) {
        JS_ReportError(cx, "Failed to set group ID to \"%d\" error : %s",
                       groupInfo->gr_gid, strerror(errno));
        return false;
    }

    if (setuid(userInfo->pw_uid) != 0) {
        JS_ReportError(cx, "Failed to set user ID to \"%d\", error : %s",
                       userInfo->pw_uid, strerror(errno));
        return false;
    }

    if (groupInfo != nullptr) {
        if (initgroups(userInfo->pw_name, groupInfo->gr_gid) == 0) {
            JS_ReportError(cx,
                           "Failed to initialize supplementary group access "
                           "list. Error : %s",
                           strerror(errno));
            return false;
        }
    }

    return true;
}

bool JSProcess::JS_setSignalHandler(JSContext *cx, JS::CallArgs &args)
{

    if (!JSUtils::ReportIfNotFunction(cx, args[0])) {
        return false;
    }

    JS_SetReservedSlot(m_Instance, 0, args[0]);

    return true;
}

bool JSProcess::JS_exit(JSContext *cx, JS::CallArgs &args)
{
    int code = 0;

    if (args.length() > 0 && args[0].isInt32()) {
        code = args[0].toInt32();
    }

    exit(code);
    
    return true;
}

bool JSProcess::JS_shutdown(JSContext *cx, JS::CallArgs &args)
{
    APE_loop_stop();
    
    return true;
}

bool JSProcess::JS_cwd(JSContext *cx, JS::CallArgs &args)
{
    Path cur(Path::GetCwd());

    if (cur.dir() == NULL) {
        args.rval().setUndefined();
        return true;
    }

    JS::RootedString res(cx, JS_NewStringCopyZ(cx, cur.dir()));

    args.rval().setString(res);

    return true;
}

// }}}

// {{{ Registration

JSFunctionSpec *JSProcess::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSProcess, getOwner, 0),
        CLASSMAPPER_FN(JSProcess, setOwner, 1),
        CLASSMAPPER_FN(JSProcess, setSignalHandler, 1),
        CLASSMAPPER_FN(JSProcess, exit, 0),
        CLASSMAPPER_FN(JSProcess, shutdown, 0),
        CLASSMAPPER_FN(JSProcess, cwd, 0),
        JS_FS_END
    };

    return funcs;
}

void JSProcess::RegisterObject(JSContext *cx,
                               char **argv,
                               int argc,
                               int workerId)
{
    JSProcess::ExposeClass(cx, "NidiumProcess", JSCLASS_HAS_RESERVED_SLOTS(1));
    JS::RootedObject ProcessObj(cx,
        JSProcess::CreateUniqueInstance(cx, new JSProcess(), "process"));

    JS::RootedObject jsargv(cx, JS_NewArrayObject(cx, argc));

    for (int i = 0; i < argc; i++) {
        JS::RootedString jelem(cx, JS_NewStringCopyZ(cx, argv[i]));
        JS_SetElement(cx, jsargv, i, jelem);
    }

    JS::RootedValue jsargv_v(cx, JS::ObjectValue(*jsargv));
    JS_SetProperty(cx, ProcessObj, "argv", jsargv_v);

    JS::RootedValue workerid_v(cx, JS::Int32Value(workerId));
    JS_SetProperty(cx, ProcessObj, "workerId", workerid_v);

    NidiumJS::GetNet()->kill_handler = ape_kill_handler;
    //jProcess->m_SignalFunction.set(JS::NullHandleValue);
}

// }}}

} // namespace Binding
} // namespace Nidium
