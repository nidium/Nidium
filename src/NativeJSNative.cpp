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

static JSBool native_showfps(JSContext *cx, unsigned argc, jsval *vp)
{
    JSBool show = JS_FALSE;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "b", &show)) {
        return JS_TRUE;
    }

    NativeJSNative::showFPS = (show == JS_TRUE) ? true : false;
    NativeJS *NJS = (NativeJS *)JS_GetRuntimePrivate(JS_GetRuntime(cx));

    if (show) {
        NJS->createDebugCanvas();
    }

    return JS_TRUE;
}

void NativeJSNative::registerObject(JSContext *cx, int width, int height)
{
    JSObject *NativeObj;
    JSObject *canvas;
    //JSObject *titleBar;

    NativeJS *NJS = (NativeJS *)JS_GetRuntimePrivate(JS_GetRuntime(cx));

    NativeObj = JS_DefineObject(cx, JS_GetGlobalObject(cx), "Native",
        &Native_class , NULL, 0);

    canvas = NativeJSCanvas::generateJSObject(cx, width, height);

    //titleBar = NativeJSCanvas::generateJSObject(cx, width, 35);
    //((NativeCanvasHandler *)JS_GetPrivate(canvas))->translate(0, 35);

    /* Set the newly generated CanvasHandler as first child of rootHandler */
    //NJS->rootHandler->addChild((NativeCanvasHandler *)JS_GetPrivate(titleBar));
    NJS->rootHandler->addChild((NativeCanvasHandler *)JS_GetPrivate(canvas));

    JS_DefineFunctions(cx, NativeObj, Native_funcs);
    JS_DefineProperty(cx, NativeObj, "canvas",
        OBJECT_TO_JSVAL(canvas), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
    /*JS_DefineProperty(cx, NativeObj, "titleBar",
        OBJECT_TO_JSVAL(titleBar), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);*/
}

