#include "NativeJSNative.h"
#include "NativeSkia.h"
#include "NativeJSCanvas.h"
#include "NativeJS.h"

bool NativeJSNative::showFPS = false;

static JSClass Native_class = {
    "Native", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSBool native_showfps(JSContext *cx, unsigned argc, jsval *vp);

static JSFunctionSpec Native_funcs[] = {
    JS_FN("showFPS", native_showfps, 1, 0),
    JS_FS_END
};

#if 0
/* Lazy resolve the "main" canvas surface */
static JSBool native_getContext(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *obj;

    if (NativeSkia::glsurface == NULL) {
        printf("No gl surface\n");
        return JS_TRUE;
    }

    if (NativeJSNative::context2D == NULL) {
        NativeJSNative::context2D = new NativeSkia();

        if (!NativeJSNative::context2D->bindOnScreen(1024, 768)) {
            JS_SET_RVAL(cx, vp, JSVAL_NULL);
            printf("failed to get 2DContext\n");
            return JS_TRUE;
        }

        NativeSkia::glsurface->addSubCanvas(NativeJSNative::context2D);

        obj = NativeJSCanvas::generateJSObject(cx,
                            NativeJSNative::context2D);
    } else {
        obj = NativeJSNative::context2D->obj;
    }

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));

    return JS_TRUE;
}
#endif

static JSBool native_showfps(JSContext *cx, unsigned argc, jsval *vp)
{
    JSBool show = JS_FALSE;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "b", &show)) {
        return JS_TRUE;
    }

    NativeJSNative::showFPS = (show == JS_TRUE) ? true : false;

    return JS_TRUE;
}

void NativeJSNative::registerObject(JSContext *cx)
{
    JSObject *NativeObj;
    JSObject *canvas;
    NativeJS *NJS = (NativeJS *)JS_GetRuntimePrivate(JS_GetRuntime(cx));

    NativeObj = JS_DefineObject(cx, JS_GetGlobalObject(cx), "Native",
        &Native_class , NULL, 0);

    canvas = NativeJSCanvas::generateJSObject(cx, 1024, 768);

    /* Set the newly generated CanvasHandler as first child of rootHandler */
    NJS->rootHandler->addChild((NativeCanvasHandler *)JS_GetPrivate(canvas));

    JS_DefineFunctions(cx, NativeObj, Native_funcs);
    JS_DefineProperty(cx, NativeObj, "canvas",
        OBJECT_TO_JSVAL(canvas), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
}

