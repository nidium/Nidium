#include "NativeJSWindow.h"
#include "NativeJS.h"
#include "NativeUIInterface.h"

#define CONST_STR_LEN(x) x, x ? sizeof(x) - 1 : 0

static JSBool native_window_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp);

enum {
    WINDOW_PROP_TITLE,
    WINDOW_PROP_CURSOR
};

static struct native_cursors {
    const char *str;
    NativeUIInterface::CURSOR_TYPE type;
} native_cursors_list[] = {
    {"arrow",               NativeUIInterface::ARROW},
    {"beam",                NativeUIInterface::BEAM},
    {"pointer",             NativeUIInterface::POINTING},
    {"drag",                NativeUIInterface::CLOSEDHAND},
    {NULL,                  NativeUIInterface::NOCHANGE}
};

static JSPropertySpec window_props[] = {
    {"title", WINDOW_PROP_TITLE, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_NULLWRAPPER,
        JSOP_WRAPPER(native_window_prop_set)},
    {"cursor", WINDOW_PROP_CURSOR, JSPROP_PERMANENT | JSPROP_ENUMERATE,
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
        case WINDOW_PROP_CURSOR:
        {
            if (!JSVAL_IS_STRING(vp)) {
                return JS_TRUE;
            }
            JSAutoByteString type(cx, JSVAL_TO_STRING(vp));

            for (int i = 0; native_cursors_list[i].str != NULL; i++) {
                if (strncasecmp(native_cursors_list[i].str, type.ptr(),
                    strlen(native_cursors_list[i].str)) == 0) {
                    NativeJSObj(cx)->UI->setCursor(native_cursors_list[i].type);
                    break;
                }
            }

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

void NativeJSwindow::registerObject(JSContext *cx)
{
    JSObject *windowObj;
    windowObj = JS_DefineObject(cx, JS_GetGlobalObject(cx), "window",
        &window_class , NULL, 0);
    JS_DefineFunctions(cx, windowObj, window_funcs);
    JS_DefineProperties(cx, windowObj, window_props);

    jsval val = STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("binary")));

    if (JS_SetProperty(cx, windowObj, "title", &val) == JS_FALSE) {
        printf("cant set\n");
    }
}

