#include "NativeJSConsole.h"
#include "NativeJS.h"
#include "NativeContext.h"
#include "NativeMacros.h"

#include <js/OldDebugAPI.h>

#include "NativeServer.h"

static bool native_console_log(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_console_write(JSContext *cx, unsigned argc,
    JS::Value *vp);

static JSClass console_class = {
    "Console", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSFunctionSpec console_funcs[] = {
    JS_FN("log", native_console_log, 0, 0),
    JS_FN("write", native_console_write, 0, 0),
    JS_FN("info", native_console_log, 0, 0),
    JS_FN("error", native_console_log, 0, 0),
    JS_FN("warn", native_console_log, 0, 0),
    JS_FS_END
};

static bool native_console_log(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    unsigned i;
    char *bytes;
    JS::RootedScript parent(cx);
    const char *filename_parent;
    unsigned lineno;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JS::AutoFilename af;
    JS::DescribeScriptedCaller(cx, &af, &lineno);

    filename_parent = af.get();

    if (filename_parent == NULL) {
        filename_parent = "(null)";
    }

    const char *fname = strrchr(filename_parent, '/');
    if (fname != NULL) {
        filename_parent = &fname[1];
    }

    NativeContext *nctx = NativeContext::getNativeClass(cx);

    for (i = 0; i < args.length(); i++) {
        JS::RootedString str(cx, JS::ToString(cx, args[i]));
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

static bool native_console_write(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    NativeJS *js = NativeJS::getNativeClass(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    NATIVE_CHECK_ARGS("write", 1);

    JS::RootedString str(cx, args[0].toString());
    if (!str) {
        JS_ReportError(cx, "Bad argument");
        return false;
    }

    JSAutoByteString cstr;

    cstr.encodeUtf8(cx, str);

    js->log(cstr.ptr());

    args.rval().setUndefined();
    return true;
}

void NativeJSconsole::registerObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedObject consoleObj(cx, JS_DefineObject(cx, global,
        "console", &console_class , nullptr, 0));
    JS_DefineFunctions(cx, consoleObj, console_funcs);
}

