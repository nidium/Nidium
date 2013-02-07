#include "NativeJSWindow.h"
#include "NativeJS.h"
#include "NativeUIInterface.h"

static JSBool native_window_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp);

enum {
    WINDOW_PROP_TITLE
};

static JSPropertySpec window_props[] = {
    {"title", WINDOW_PROP_TITLE, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_NULLWRAPPER,
        JSOP_WRAPPER(native_window_prop_set)},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSBool native_window_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
    switch(JSID_TO_INT(id)) {
        case WINDOW_PROP_TITLE:
        {
            if (!JSVAL_IS_STRING(vp)) {
                return JS_TRUE;
            }
            JSAutoByteString title(cx, JSVAL_TO_STRING(vp));      
            NativeJSObj(cx)->UI->setWindowTitle(title.ptr());
            break;      
        }

        default:
            break;
    }
    return JS_TRUE;
}


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

