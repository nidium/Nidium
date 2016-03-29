#include "NativeJSConsole.h"

#include <string.h>
#include <math.h>

static bool native_console_log(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_console_write(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_console_hide(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_console_show(JSContext *cx, unsigned argc,
    JS::Value *vp);
static bool native_console_clear(JSContext *cx, unsigned argc,
    JS::Value *vp);

static JSClass console_class = {
    "Console", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSFunctionSpec console_funcs[] = {
    JS_FN("log", native_console_log, 0, NATIVE_JS_FNPROPS),
    JS_FN("write", native_console_write, 0, NATIVE_JS_FNPROPS),
    JS_FN("info", native_console_log, 0, NATIVE_JS_FNPROPS),
    JS_FN("error", native_console_log, 0, NATIVE_JS_FNPROPS),
    JS_FN("warn", native_console_log, 0, NATIVE_JS_FNPROPS),
    JS_FN("hide", native_console_hide, 0, NATIVE_JS_FNPROPS),
    JS_FN("show", native_console_show, 0, NATIVE_JS_FNPROPS),
    JS_FN("clear", native_console_clear, 0, NATIVE_JS_FNPROPS),
    JS_FS_END
};

static bool native_console_hide(JSContext *cx, unsigned argc,
    JS::Value *vp)
{

#if 0
    if (NativeContext::getNativeClass(cx)->getUI()->getConsole()) {
        NativeContext::getNativeClass(cx)->getUI()->getConsole()->hide();
    }
#endif
    return true;
}

static bool native_console_show(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
#if 0
    NativeContext::getNativeClass(cx)->getUI()->getConsole(true)->show();
#endif
    return true;
}

static bool native_console_clear(JSContext *cx, unsigned argc,
    JS::Value *vp)
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

static bool native_console_log(JSContext *cx, unsigned argc,
    JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    unsigned i;
    char *bytes;
    const char *filename_parent;
    unsigned lineno;

    JS::AutoFilename filename;
    JS::DescribeScriptedCaller(cx, &filename, &lineno);

    NativeJS *js = NativeJS::getNativeClass(cx);
    filename_parent = filename.get();
    if (filename_parent == NULL) {
        filename_parent = "(null)";
    }

    const char *fname = strrchr(filename_parent, '/');
    if (fname != NULL) {
        filename_parent = &fname[1];
    }

    for (i = 0; i < args.length(); i++) {

        JS::RootedString str(cx, JS::ToString(cx, args[i]));
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
    JS::RootedObject consoleObj(cx, JS_DefineObject(cx,
        JS::CurrentGlobalOrNull(cx), "console",
        &console_class, NULL, 0));

    JS_DefineFunctions(cx, consoleObj, console_funcs);
}

