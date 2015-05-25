#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <NativeJSExposer.h>

static JSBool hello(JSContext *cx, unsigned argc, jsval *vp)
{
    printf("Hello from C\n");
    return true;
}

static JSFunctionSpec TestModule_funcs[] = {
    JS_FN("hello", hello, 0, 0),
    JS_FS_END
};

static JSPropertySpec TestModule_props[] = {
    {"bar", 0, 
        JSPROP_ENUMERATE, 
        JSOP_NULLWRAPPER, 
        JSOP_NULLWRAPPER},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

bool registerCallback(JSContext *cx, JSObject *exports) {
    JS_DefineFunctions(cx, exports, TestModule_funcs);
    JS_DefineProperties(cx, exports, TestModule_props);

    JS::Value bar;
    bar.setInt32(42);

    JS_SetProperty(cx, exports, "bar", &bar);

    printf("regsiter callback called\n");

    return true;
}
 
NATIVE_REGISTER_MODULE(registerCallback)
