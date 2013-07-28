#include "NativeJSNative.h"
#include "NativeSkia.h"
#include "NativeJSCanvas.h"
#include "NativeJS.h"
#include "NativeUIInterface.h"
#include "NativeSystemInterface.h"
#include <jsstr.h>

bool NativeJSNative::showFPS = false;

static JSClass Native_class = {
    "Native", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSBool native_showfps(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_setPasteBuffer(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_getPasteBuffer(JSContext *cx, unsigned argc, jsval *vp);

static JSFunctionSpec Native_funcs[] = {
    JS_FN("showFPS", native_showfps, 1, 0),
    JS_FN("setPasteBuffer", native_setPasteBuffer, 1, 0),
    JS_FN("getPasteBuffer", native_getPasteBuffer, 0, 0),
    JS_FS_END
};

static JSBool native_setPasteBuffer(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *str;
    JS::CallArgs args = CallArgsFromVp(argc, vp);

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "S",
        &str)) {
        return JS_TRUE;
    }

    char *text = JS_EncodeStringToUTF8(cx, str);

    NativeJS *NJS = NativeJS::getNativeClass(cx);

    NJS->UI->setClipboardText(text);

    js_free(text);

    return JS_TRUE;
}

static JSBool native_getPasteBuffer(JSContext *cx, unsigned argc, jsval *vp)
{
    using namespace js;
    JSString *str;
    JS::CallArgs args = CallArgsFromVp(argc, vp);

    NativeJS *NJS = NativeJS::getNativeClass(cx);

    char *text = NJS->UI->getClipboardText();

    if (text == NULL) {
        args.rval().setNull();
        return true;
    }

    size_t len = strlen(text)*2;
    jschar *jsc = new jschar[len];
    js:InflateUTF8StringToBufferReplaceInvalid(cx, text, strlen(text), jsc, &len);

    JSString *jret = JS_NewUCStringCopyN(cx, jsc, len);

    args.rval().set(STRING_TO_JSVAL(jret));

    free(text);
    delete[] jsc;

    return true;
}

static JSBool native_showfps(JSContext *cx, unsigned argc, jsval *vp)
{
    JSBool show = JS_FALSE;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "b", &show)) {
        return JS_TRUE;
    }

    NativeJSNative::showFPS = (show == JS_TRUE) ? true : false;
    NativeJS *NJS = NativeJS::getNativeClass(cx);

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

    NativeJS *NJS = NativeJS::getNativeClass(cx);

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

