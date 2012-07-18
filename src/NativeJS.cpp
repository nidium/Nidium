/**
 **   Copyright (c) 2012 All Right Reserved, Troll Face Studio
 **
 **   Authors :
 **       * Anthony Catel <a.catel@trollfacestudio.com>
 **/

#include "NativeJS.h"
#include <stdio.h>
#include <jsapi.h>



static JSClass global_class = {
    "_GLOBAL", JSCLASS_GLOBAL_FLAGS | JSCLASS_IS_GLOBAL,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL
};
static JSBool
Print(JSContext *cx, unsigned argc, jsval *vp);

static JSFunctionSpec glob_funcs[] = {
    
    JS_FN("echo", Print, 0, 0),

    JS_FS_END
};

static void reportError(JSContext *cx, const char *message, JSErrorReport *report)
{
    fprintf(stdout, "%s:%u:%s\n",
            report->filename ? report->filename : "<no filename>",
            (unsigned int) report->lineno,
            message);
}

static JSBool
PrintInternal(JSContext *cx, unsigned argc, jsval *vp, FILE *file)
{
    jsval *argv;
    unsigned i;
    JSString *str;
    char *bytes;

    argv = JS_ARGV(cx, vp);
    for (i = 0; i < argc; i++) {
        str = JS_ValueToString(cx, argv[i]);
        if (!str)
            return false;
        bytes = JS_EncodeString(cx, str);
        if (!bytes)
            return false;
        fprintf(file, "%s%s", i ? " " : "", bytes);
        JS_free(cx, bytes);
    }

    fputc('\n', file);
    fflush(file);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return true;
}

static JSBool
Print(JSContext *cx, unsigned argc, jsval *vp)
{
    return PrintInternal(cx, argc, vp, stdout);
}

NativeJS::NativeJS()
{
    JSRuntime *rt;
    JSObject *gbl;

    JS_SetCStringsAreUTF8();

    if ((rt = JS_NewRuntime(1024L * 1024L * 128L)) == NULL) {
        printf("Failed to init JS runtime\n");
        return;
    }

    if ((cx = JS_NewContext(rt, 8192)) == NULL) {
        printf("Failed to init JS context\n");
        return;     
    }

    JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE);
    JS_SetVersion(cx, JSVERSION_LATEST);

    JS_SetErrorReporter(cx, reportError);

    gbl = JS_NewGlobalObject(cx, &global_class, NULL);


    if (!JS_InitStandardClasses(cx, gbl))
        return;

    JS_SetGlobalObject(cx, gbl);
    JS_DefineFunctions(cx, gbl, glob_funcs);

}

int NativeJS::LoadScript(const char *filename)
{

    JSObject *gbl = JS_GetGlobalObject(cx);

    JSScript *script = JS_CompileUTF8File(cx, gbl, filename);

    if (script == NULL) {
        return 0;
    }

    JS_ExecuteScript(cx, gbl, script, NULL);

    return 1;
}

void NativeJS::LoadCanvasObject()
{
    
}

