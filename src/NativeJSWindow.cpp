#include "NativeJSWindow.h"
#include "NativeJS.h"
#include "NativeUIInterface.h"
#include "NativeSkia.h"

#define CONST_STR_LEN(x) x, x ? sizeof(x) - 1 : 0

static JSBool native_window_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp);

static JSBool native_window_openFileDialog(JSContext *cx, unsigned argc, jsval *vp);

enum {
    WINDOW_PROP_TITLE,
    WINDOW_PROP_CURSOR,
    WINDOW_PROP_TITLEBAR_COLOR,
    WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETX,
    WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETY,
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
    {"titleBarColor", WINDOW_PROP_TITLEBAR_COLOR, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_NULLWRAPPER,
        JSOP_WRAPPER(native_window_prop_set)},
    {"titleBarControlsOffsetX", WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETX, JSPROP_PERMANENT | JSPROP_ENUMERATE,
        JSOP_NULLWRAPPER,
        JSOP_WRAPPER(native_window_prop_set)},
    {"titleBarControlsOffsetY", WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETY, JSPROP_PERMANENT | JSPROP_ENUMERATE,
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
        case WINDOW_PROP_TITLEBAR_COLOR:
        {
            if (!JSVAL_IS_STRING(vp)) {
                return JS_TRUE;
            }
            JSAutoByteString color(cx, JSVAL_TO_STRING(vp));
            uint32_t icolor = NativeSkia::parseColor(color.ptr());

            NativeJSObj(cx)->UI->setTitleBarRGBAColor(
                (icolor & 0x00FF0000) >> 16,
                (icolor & 0x0000FF00) >> 8,
                 icolor & 0x000000FF,
                 icolor >> 24);

            break;
        }
        case WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETX:
        {
            double dval, oval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &dval);
            jsval offsety;

            if (JS_GetProperty(cx, obj.get(), "titleBarControlsOffsetY", &offsety) == JS_FALSE) {
                offsety = DOUBLE_TO_JSVAL(0);
            }

            JS_ValueToNumber(cx, offsety, &oval);

            NativeJSObj(cx)->UI->setWindowControlsOffset(dval, oval);
            break;
        }
        case WINDOW_PROP_TITLEBAR_CONTROLS_OFFSETY:
        {
            double dval, oval;
            if (!JSVAL_IS_NUMBER(vp)) {
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, vp, &dval);
            jsval offsetx;
            if (JS_GetProperty(cx, obj.get(), "titleBarControlsOffsetX", &offsetx) == JS_FALSE) {
                offsetx = DOUBLE_TO_JSVAL(0);
            }
            JS_ValueToNumber(cx, offsetx, &oval);

            NativeJSObj(cx)->UI->setWindowControlsOffset(oval, dval);
            break;
        }
        default:
            break;
    }
    return JS_TRUE;
}

struct _nativeopenfile
{
    JSContext *cx;
    jsval cb;
};

static void native_window_openfilecb(void *_nof, const char *lst[], uint32_t len)
{
    struct _nativeopenfile *nof = (struct _nativeopenfile *)_nof;
    jsval rval;

    jsval cb = nof->cb;
    JSObject *arr = JS_NewArrayObject(nof->cx, len, NULL);

    for (int i = 0; i < len; i++) {
        jsval val = STRING_TO_JSVAL(JS_NewStringCopyZ(nof->cx, lst[i]));
        JS_SetElement(nof->cx, arr, i, &val);        
    }

    jsval jarr = OBJECT_TO_JSVAL(arr);

    JS_CallFunctionValue(nof->cx, JS_GetGlobalObject(nof->cx), cb, 1, &jarr, &rval);

    JS_RemoveValueRoot(nof->cx, &nof->cb);
    free(nof);

}

static JSBool native_window_openFileDialog(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *types;
    jsval callback;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ov", &types, &callback)) {
        return JS_TRUE;
    }

    if (!JSVAL_IS_NULL(OBJECT_TO_JSVAL(types)) && !JS_IsArrayObject(cx, types)) {
        JS_ReportError(cx, "First parameter must be an array or null");
        return JS_FALSE;
    }
    uint32_t len = 0;

    char **ctypes = NULL;

    if (!JSVAL_IS_NULL(OBJECT_TO_JSVAL(types))) {
        JS_GetArrayLength(cx, types, &len);

        ctypes = (char **)malloc(sizeof(char *) * (len+1));
        memset(ctypes, 0, sizeof(char *) * (len+1));
        int j = 0;
        for (int i = 0; i < len; i++) {
            jsval val;
            JS_GetElement(cx, types, i, &val);

            if (JSVAL_IS_STRING(val)) {
                JSString *str = JSVAL_TO_STRING(val);
                ctypes[j] = JS_EncodeString(cx, str);
                j++;
            }
        }
        ctypes[j] = NULL;
    }

    struct _nativeopenfile *nof = (struct _nativeopenfile *)malloc(sizeof(*nof));
    nof->cb = callback;
    nof->cx = cx;

    JS_AddValueRoot(cx, &nof->cb);

    NativeJSObj(cx)->UI->openFileDialog((const char **)ctypes, native_window_openfilecb, nof);

    if (ctypes) {
        for (int i = 0; i < len; i++) {
            JS_free(cx, ctypes[i]);
        }
        free(ctypes);
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
    JS_FN("openFileDialog", native_window_openFileDialog, 2, 0),
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
                CONST_STR_LEN("Native")));

    JS_SetProperty(cx, windowObj, "title", &val);

    val = DOUBLE_TO_JSVAL(0);
    JS_SetProperty(cx, windowObj, "titleBarControlsOffsetX", &val);

    val = DOUBLE_TO_JSVAL(0);
    JS_SetProperty(cx, windowObj, "titleBarControlsOffsetY", &val);
}

