#include "NativeJSNative.h"
#include "NativeSkia.h"
#include "NativeJSCanvas.h"

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

