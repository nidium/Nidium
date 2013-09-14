#include "NativeJSNative.h"
#include "NativeJSCanvas.h"
#include "NativeJS.h"
#include "NativeCanvasHandler.h"
#include "NativeContext.h"
#include "NativeUIInterface.h"
#include "NativeSystemInterface.h"
#include <jsstr.h>

bool NativeJSNative::showFPS = false;

static void Native_Finalize(JSFreeOp *fop, JSObject *obj);

static JSClass Native_class = {
    "Native", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Native_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

JSClass *NativeJSNative::jsclass = &Native_class;

static JSBool native_showfps(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_setPasteBuffer(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_getPasteBuffer(JSContext *cx, unsigned argc, jsval *vp);

static JSFunctionSpec Native_funcs[] = {
    JS_FN("showFPS", native_showfps, 1, 0),
    JS_FN("setPasteBuffer", native_setPasteBuffer, 1, 0),
    JS_FN("getPasteBuffer", native_getPasteBuffer, 0, 0),
    JS_FS_END
};

void Native_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSNative *jnative = NativeJSNative::getNativeClass(obj);

    if (jnative != NULL) {
        delete jnative;
    }    
}

static JSBool native_setPasteBuffer(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *str;
    JS::CallArgs args = CallArgsFromVp(argc, vp);

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "S",
        &str)) {
        return JS_TRUE;
    }

    char *text = JS_EncodeStringToUTF8(cx, str);

    NativeContext::getNativeClass(cx)->getUI()->setClipboardText(text);

    js_free(text);

    return JS_TRUE;
}

static JSBool native_getPasteBuffer(JSContext *cx, unsigned argc, jsval *vp)
{
    using namespace js;
    JSString *str;
    JS::CallArgs args = CallArgsFromVp(argc, vp);

    char *text = NativeContext::getNativeClass(cx)->getUI()->getClipboardText();

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

    if (show) {
        NativeContext::getNativeClass(cx)->createDebugCanvas();
    }

    return JS_TRUE;
}

void NativeJSNative::registerObject(JSContext *cx, int width, int height)
{
    JSObject *NativeObj;
    JSObject *canvas;

    NativeJSNative *jnative = new NativeJSNative();

    //JSObject *titleBar;

    NativeObj = JS_DefineObject(cx, JS_GetGlobalObject(cx),
        NativeJSNative::getJSObjectName(),
        &Native_class , NULL, 0);

    jnative->jsobj = NativeObj;
    jnative->cx = cx;

    JS_SetPrivate(NativeObj, jnative);

    canvas = NativeJSCanvas::generateJSObject(cx, width, height);

    jnative->handler = (NativeCanvasHandler *)JS_GetPrivate(canvas);

    NativeJS::getNativeClass(cx)->jsobjects.set(
        NativeJSNative::getJSObjectName(), NativeObj);

    //titleBar = NativeJSCanvas::generateJSObject(cx, width, 35);
    //((NativeCanvasHandler *)JS_GetPrivate(canvas))->translate(0, 35);

    /* Set the newly generated CanvasHandler as first child of rootHandler */
    //NJS->rootHandler->addChild((NativeCanvasHandler *)JS_GetPrivate(titleBar));
    NativeContext::getNativeClass(cx)->getRootHandler(
        )->addChild(jnative->handler);

    JS_DefineFunctions(cx, NativeObj, Native_funcs);
    JS_DefineProperty(cx, NativeObj, "canvas",
        OBJECT_TO_JSVAL(canvas), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
    /*JS_DefineProperty(cx, NativeObj, "titleBar",
        OBJECT_TO_JSVAL(titleBar), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);*/
}

