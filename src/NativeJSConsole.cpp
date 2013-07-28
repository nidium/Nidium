#include "NativeJSConsole.h"
#include "NativeJS.h"
#include "NativeUIInterface.h"


static JSBool native_console_log(JSContext *cx, unsigned argc,
    jsval *vp);

static JSBool native_console_hide(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_console_show(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_console_clear(JSContext *cx, unsigned argc,
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
    JS_FN("hide", native_console_hide, 0, 0),
    JS_FN("show", native_console_show, 0, 0),
    JS_FN("clear", native_console_clear, 0, 0),
    JS_FS_END
};

static JSBool native_console_hide(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NativeJS::getNativeClass(cx)->UI->getConsole()->hide();

    return JS_TRUE;
}

static JSBool native_console_show(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NativeJS::getNativeClass(cx)->UI->getConsole()->show();

    return JS_TRUE;
}

static JSBool native_console_clear(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NativeJS::getNativeClass(cx)->UI->getConsole()->clear();

    return JS_TRUE;
}

static JSBool native_console_log(JSContext *cx, unsigned argc,
    jsval *vp)
{
    jsval *argv;
    unsigned i;
    JSString *str;
    char *bytes;
    NativeJS *NJS = NativeJS::getNativeClass(cx);

    argv = JS_ARGV(cx, vp);
    for (i = 0; i < argc; i++) {
        str = JS_ValueToString(cx, argv[i]);
        if (!str)
            return false;
        bytes = JS_EncodeStringToUTF8(cx, str);
        if (!bytes)
            return false;
        if (i) {
           NJS->UI->getConsole()->log(" "); 
        }
        NJS->UI->getConsole()->log(bytes);

        JS_free(cx, bytes);
    }
    NJS->UI->getConsole()->log("\n"); 

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

