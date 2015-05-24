#include "NativeJSConsole.h"
#include "NativeJS.h"
//#include "NativeContext.h"

#include "NativeJSProfiler.h"

#include <jsdbgapi.h>

static JSBool native_console_log(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_console_write(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_console_hide(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_console_show(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_console_clear(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_console_profile_start(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_console_profile_end(JSContext *cx, unsigned argc,
    jsval *vp);

static JSClass console_class = {
    "Console", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSFunctionSpec console_funcs[] = {
    JS_FN("log", native_console_log, 0, 0),
    JS_FN("write", native_console_write, 0, 0),
    JS_FN("info", native_console_log, 0, 0),
    JS_FN("error", native_console_log, 0, 0),
    JS_FN("warn", native_console_log, 0, 0),
    JS_FN("hide", native_console_hide, 0, 0),
    JS_FN("show", native_console_show, 0, 0),
    JS_FN("clear", native_console_clear, 0, 0),
    JS_FN("profile", native_console_profile_start, 0, 0),
    JS_FN("profileEnd", native_console_profile_end, 0, 0),
    JS_FS_END
};

static JSBool native_console_hide(JSContext *cx, unsigned argc,
    jsval *vp)
{

#if 0
    if (NativeContext::getNativeClass(cx)->getUI()->getConsole()) {
        NativeContext::getNativeClass(cx)->getUI()->getConsole()->hide();
    }
#endif
    return true;
}

static JSBool native_console_show(JSContext *cx, unsigned argc,
    jsval *vp)
{
#if 0
    NativeContext::getNativeClass(cx)->getUI()->getConsole(true)->show();
#endif
    return true;
}

static JSBool native_console_clear(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NativeJS *js = NativeJS::getNativeClass(cx);

    js->logclear();
#if 0
    if (NativeContext::getNativeClass(cx)->getUI()->getConsole()) {
        NativeContext::getNativeClass(cx)->getUI()->getConsole()->clear();
    }
#endif
    return true;
}

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
    NativeJS *js = NativeJS::getNativeClass(cx);

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
            js->log(" ");
        } else {
            js->logf("[%s:%d] ", filename_parent, lineno);
        }
        js->log(bytes);

        JS_free(cx, bytes);
    }
    js->log("\n");

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return true;
}

static JSBool native_console_write(JSContext *cx, unsigned argc,
    jsval *vp)
{
    JSString *str;
    NativeJS *js = NativeJS::getNativeClass(cx);

    NATIVE_CHECK_ARGS("write", 1);

    str = JS_ValueToString(cx, JS_ARGV(cx, vp)[0]);
    if (!str) {
        JS_ReportError(cx, "Bad argument");
        return false;
    }

    JSAutoByteString cstr;

    cstr.encodeUtf8(cx, str);

    js->log(cstr.ptr());
    js->log("\n");

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return true;
}

static JSBool native_console_profile_start(JSContext *cx, unsigned argc,
    jsval *vp)
{
    /*
    JSString *tmp;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &tmp)) {
        return false;
    }

    JSAutoByteString name(cx, tmp);
    */

    NativeProfiler *tracer = NativeProfiler::getInstance(cx);
    tracer->start(NULL);

    return true;
}
static JSBool native_console_profile_end(JSContext *cx, unsigned argc,
    jsval *vp)
{
    NativeProfiler *tracer = NativeProfiler::getInstance(cx);
    tracer->stop();
    JS::Value val(OBJECT_TO_JSVAL(tracer->getJSObject()));
    JS_SET_RVAL(cx, vp, val);
    return true;
}


void NativeJSconsole::registerObject(JSContext *cx)
{
    JSObject *consoleObj;
    consoleObj = JS_DefineObject(cx, JS_GetGlobalObject(cx), "console",
        &console_class , NULL, 0);
    JS_DefineFunctions(cx, consoleObj, console_funcs);
}

