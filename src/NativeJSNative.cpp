#include "NativeJSNative.h"
#include "NativeSkia.h"

bool NativeJSNative::showFPS = false;
NativeSkia *NativeJSNative::context2D = NULL;

static JSClass Native_class = {
    "Native", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSBool native_showfps(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_getContext(JSContext *cx, unsigned argc, jsval *vp);

static JSFunctionSpec Native_funcs[] = {
    JS_FN("showFPS", native_showfps, 1, 0),
    JS_FN("getContext", native_getContext, 1, 0),
    JS_FS_END
};

static JSBool native_getContext(JSContext *cx, unsigned argc, jsval *vp)
{
    if (NativeJSNative::context2D == NULL) {
        NativeJSNative::context2D = new NativeSkia();
        
    }

    return JS_TRUE;
}

static JSBool native_showfps(JSContext *cx, unsigned argc, jsval *vp)
{
    JSBool show = JS_FALSE;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "b", &show)) {
        return JS_TRUE;
    }

    NativeJSNative::showFPS = (show == JS_TRUE) ? true : false;

    return JS_TRUE;
}

NATIVE_OBJECT_EXPOSE_NOT_INST(Native)

