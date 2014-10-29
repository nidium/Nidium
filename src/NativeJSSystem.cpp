#include "NativeJSSystem.h"
#include <NativeJS.h>

#include "NativeContext.h"
#include "NativeMacros.h"

#ifdef __linux__
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include <errno.h>
#include <sys/stat.h>

static JSBool native_system_getOpenFileStats(JSContext *cx, unsigned argc,
    jsval *vp);


static JSClass system_class = {
    "System", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSFunctionSpec system_funcs[] = {
    JS_FN("getOpenFileStats", native_system_getOpenFileStats, 0, 0),

    JS_FS_END
};


static JSBool native_system_getOpenFileStats(JSContext *cx, unsigned argc,
    jsval *vp)
{
    struct rlimit rl;
    struct stat   stats;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
        JS_ReportWarning(cx, "Couldnt get limits values");
        args.rval().setNull();
        return true;
    }

    JSObject *ret = JS_NewObject(cx, NULL, NULL, NULL);

    JSOBJ_SET_PROP_INT(ret, "cur", rl.rlim_cur);
    JSOBJ_SET_PROP_INT(ret, "max", rl.rlim_max);

    int fdcounter = 0, sockcounter = 0, othercount = 0;

    for (int i = 0; i <= rl.rlim_cur; i++ ) {
        if (fstat(i, &stats) == 0) {
            fdcounter++;
            if (S_ISSOCK(stats.st_mode)) {
                sockcounter++;
            } else {
                othercount++;
            }
        } else if (errno != EBADF) {
            fdcounter++;
        }

    }

    JSOBJ_SET_PROP_INT(ret, "open", fdcounter);
    JSOBJ_SET_PROP_INT(ret, "sockets", sockcounter);
    JSOBJ_SET_PROP_INT(ret, "files", othercount);

    args.rval().setObjectOrNull(ret);

    return true;
}

void NativeJSSystem::registerObject(JSContext *cx)
{
    JSObject *systemObj;
    systemObj = JS_DefineObject(cx, JS_GetGlobalObject(cx), "System",
        &system_class , NULL, 0);
    JS_DefineFunctions(cx, systemObj, system_funcs);
}

