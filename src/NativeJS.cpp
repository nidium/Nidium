/**
 **   Copyright (c) 2012 All Right Reserved, Troll Face Studio
 **
 **   Authors :
 **       * Anthony Catel <a.catel@trollfacestudio.com>
 **/

#include "NativeJS.h"
#include "NativeSkia.h"
#include <stdio.h>
#include <jsapi.h>

enum {
    CANVAS_PROP_FILLSTYLE = 1,
    CANVAS_PROP_STROKESTYLE,
    CANVAS_PROP_LINEWIDTH
};

static JSClass global_class = {
    "_GLOBAL", JSCLASS_GLOBAL_FLAGS | JSCLASS_IS_GLOBAL,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass canvas_class = {
    "canvas", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};


/******** Natives ********/
static JSBool Print(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_fillRect(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_strokeRect(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_clearRect(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_fillText(JSContext *cx, unsigned argc, jsval *vp);
/*************************/

/******** Setters ********/

static JSBool native_canvas_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, jsval *vp);


static JSPropertySpec canvas_props[] = {
    {"fillStyle", CANVAS_PROP_FILLSTYLE, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"strokeStyle", CANVAS_PROP_STROKESTYLE, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"lineWidth", CANVAS_PROP_LINEWIDTH, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {NULL}
};
/*************************/



static JSFunctionSpec glob_funcs[] = {
    
    JS_FN("echo", Print, 0, 0),

    JS_FS_END
};

static JSFunctionSpec canvas_funcs[] = {
    
    JS_FN("fillRect", native_canvas_fillRect, 4, 0),
    JS_FN("fillText", native_canvas_fillText, 3, 0),
    JS_FN("strokeRect", native_canvas_strokeRect, 4, 0),
    JS_FN("clearRect", native_canvas_clearRect, 4, 0),
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

static JSBool native_canvas_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, jsval *vp)
{

    switch(JSID_TO_INT(id)) {
        case CANVAS_PROP_FILLSTYLE:
        {
            if (!JSVAL_IS_STRING(*vp)) {
                *vp = JSVAL_NULL;

                return JS_TRUE;
            }
            JSAutoByteString colorName(cx, JSVAL_TO_STRING(*vp));
            NativeSkia::getInstance().setFillColor(colorName.ptr());          
        }
            
        break;
        case CANVAS_PROP_STROKESTYLE:
        {
            if (!JSVAL_IS_STRING(*vp)) {
                *vp = JSVAL_NULL;

                return JS_TRUE;
            }
            JSAutoByteString colorName(cx, JSVAL_TO_STRING(*vp));
            NativeSkia::getInstance().setStrokeColor(colorName.ptr());          
        }
            
        break;
        case CANVAS_PROP_LINEWIDTH:
        {
            if (!JSVAL_IS_NUMBER(*vp)) {
                *vp = JSVAL_NULL;
                return JS_TRUE;
            }

            NativeSkia::getInstance().setLineWidth(JSVAL_TO_INT(*vp));
        }
        break;
        default:
            break;
    }


    return JS_TRUE;
}

static JSBool native_canvas_fillRect(JSContext *cx, unsigned argc, jsval *vp)
{
    int x, y, width, height;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "iiii", &x, &y, &width, &height)) {
        return JS_TRUE;
    }

    NativeSkia::getInstance().drawRect(x, y, width+x, height+y, 0);

    return JS_TRUE;
}

static JSBool native_canvas_strokeRect(JSContext *cx, unsigned argc, jsval *vp)
{
    int x, y, width, height;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "iiii", &x, &y, &width, &height)) {
        return JS_TRUE;
    }

    NativeSkia::getInstance().drawRect(x, y, width+x, height+y, 1);

    return JS_TRUE;
}

static JSBool native_canvas_clearRect(JSContext *cx, unsigned argc, jsval *vp)
{
    int x, y, width, height;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "iiii", &x, &y, &width, &height)) {
        return JS_TRUE;
    }

    NativeSkia::getInstance().clearRect(x, y, width+x, height+y);

    return JS_TRUE;
}

static JSBool native_canvas_fillText(JSContext *cx, unsigned argc, jsval *vp)
{
    int x, y, maxwidth;
    JSString *str;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "Sii/i",
            &str, &x, &y, &maxwidth)) {
        return JS_TRUE;
    }

    JSAutoByteString text(cx, str);

    NativeSkia::getInstance().drawText(text.ptr(), x, y);

    return JS_TRUE;
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

    LoadCanvasObject();
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
    JSObject *gbl = JS_GetGlobalObject(cx);
    JSObject *canvasObj;

    canvasObj = JS_DefineObject(cx, gbl, "canvas", &canvas_class, NULL, 0);
    JS_DefineFunctions(cx, canvasObj, canvas_funcs);

    JS_DefineProperties(cx, canvasObj, canvas_props);
}

