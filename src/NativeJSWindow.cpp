#include "NativeJSWindow.h"


static JSClass window_class = {
    "Window", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSFunctionSpec window_funcs[] = {
    
    JS_FS_END
};


NATIVE_OBJECT_EXPOSE_NOT_INST(window)

