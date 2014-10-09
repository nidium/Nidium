#include "NativeJSConsole.h"
#include "NativeJS.h"
#include "NativeContext.h"
#include "NativeMacros.h"

#include <jsdbgapi.h>

static JSBool native_console_log(JSContext *cx, unsigned argc,
    jsval *vp);

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

    JS_FS_END
};



static JSBool native_console_log(JSContext *cx, unsigned argc,
    jsval *vp)
{
    jsval *argv;
    unsigned i;
    JSString *str;
    char *bytes;
    JSScript *parent;
    const char *filename_parent;
    unsigned lineno;

    JS_DescribeScriptedCaller(cx, &parent, &lineno);
    filename_parent = JS_GetScriptFilename(cx, parent);

    if (filename_parent == NULL) {
        filename_parent = "(null)";
    }

    const char *fname = strrchr(filename_parent, '/');
    if (fname != NULL) {
        filename_parent = &fname[1];
    }

    argv = JS_ARGV(cx, vp);

    for (i = 0; i < argc; i++) {
        str = JS_ValueToString(cx, argv[i]);
        if (!str)
            return false;
        bytes = JS_EncodeStringToUTF8(cx, str);
        if (!bytes)
            return false;
        if (i) {
            printf(" ");
        } else {
            printf("[%s:%d] ", filename_parent, lineno);
        }
        printf("%s", bytes);

        JS_free(cx, bytes);
    }
    printf("\n");

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return true;
}

void NativeJSconsole::registerObject(JSContext *cx)
{
    JSObject *consoleObj;
    consoleObj = JS_DefineObject(cx, JS_GetGlobalObject(cx), "console",
        &console_class , NULL, 0);
    JS_DefineFunctions(cx, consoleObj, console_funcs);
}

