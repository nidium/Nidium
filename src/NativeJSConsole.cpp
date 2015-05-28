#include "NativeJSConsole.h"
#include "NativeJS.h"
#include "NativeContext.h"
#include "NativeMacros.h"

#include <NativeJSProfiler.h>
#include <jsdbgapi.h>

#include "NativeServer.h"

static bool native_console_log(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_console_profile_start(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_console_profile_end(JSContext *cx, unsigned argc,
    JS::Value *vp);

static JSClass console_class = {
    "Console", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSFunctionSpec console_funcs[] = {
    JS_FN("log", native_console_log, 0, 0),
    JS_FN("info", native_console_log, 0, 0),
    JS_FN("error", native_console_log, 0, 0),
    JS_FN("warn", native_console_log, 0, 0),
    JS_FN("profile", native_console_profile_start, 0, 0),
    JS_FN("profileEnd", native_console_profile_end, 0, 0),
    JS_FS_END
};

static bool native_console_profile_start(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NativeProfiler *tracer = NativeProfiler::getInstance(cx);
    tracer->start(NULL);

    return true;
}
static bool native_console_profile_end(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    NativeProfiler *tracer = NativeProfiler::getInstance(cx);
    tracer->stop();
    JS::RootedValue val(cx, OBJECT_TO_JSVAL(tracer->getJSObject()));

    args.rval().set(val);

    return true;
}

static bool native_console_log(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    unsigned i;
    char *bytes;
    JS::RootedScript parent(cx);
    const char *filename_parent;
    unsigned lineno;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS_DescribeScriptedCaller(cx, &parent.get(), &lineno);
    filename_parent = JS_GetScriptFilename(cx, parent);

    if (filename_parent == NULL) {
        filename_parent = "(null)";
    }

    const char *fname = strrchr(filename_parent, '/');
    if (fname != NULL) {
        filename_parent = &fname[1];
    }

    NativeContext *nctx = NativeContext::getNativeClass(cx);

    for (i = 0; i < args.length(); i++) {
        JS::RootedString str(cx, args[i].toString());
        if (!str)
            return false;
        bytes = JS_EncodeStringToUTF8(cx, str);
        if (!bytes)
            return false;
        if (i) {
            printf(" ");
        } else {
            printf("(worker %d) [%s:%d] ", nctx->getWorker()->getIdentifier(), filename_parent, lineno);
        }
        printf("%s", bytes);

        JS_free(cx, bytes);
    }
    printf("\n");

    args.rval().setUndefined();

    return true;
}

void NativeJSconsole::registerObject(JSContext *cx)
{
    JS::RootedObject consoleObj(cx, JS_DefineObject(cx, JS_GetGlobalObject(cx),
        "console", &console_class , NULL, 0));
    JS_DefineFunctions(cx, consoleObj, console_funcs);
}

