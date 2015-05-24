#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJSProcess.h>

TEST(NativeJSProcess, Simple)
{
    JSObject *globalObj;
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    jsval rval;
    bool success;
    char * args[] = {"nidium"};

    globalObj = JS_GetGlobalObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globalObj, "process", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    NativeJSProcess::registerObject(njs.cx, (char**)args, 1);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globalObj, "process", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    jsval argv = JSVAL_VOID;
    JSObject *obj = JSVAL_TO_OBJECT(rval);
    JS_GetProperty(njs.cx, obj, "argv", &argv);
    EXPECT_TRUE(JSVAL_IS_VOID(argv) == false);

    NativeJSProcess *jproc = NULL;;
    jproc = (NativeJSProcess*)JS_GetPrivate(obj);
    EXPECT_TRUE(jproc != NULL);

    jsval el = JSVAL_VOID;
    JSObject *argObj = JSVAL_TO_OBJECT(argv);
    JS_GetElement(njs.cx, argObj, 0, &el);
    JSString *jstr = JSVAL_TO_STRING(el);
    char * cstr = JS_EncodeString(njs.cx, jstr);
    EXPECT_TRUE(strcmp(cstr, "nidium") == 0);
    free(cstr);

    native_netlib_destroy(g_ape);
}

TEST(NativeJSProcess, Init)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    JSObject * globalObj = JS_GetGlobalObject(njs.cx);

    NativeJSProcess np(globalObj, njs.cx);

    EXPECT_TRUE(np.getJSObject() == globalObj);
    EXPECT_TRUE(np.getJSContext() == njs.cx);

    EXPECT_TRUE(strcmp(np.getJSObjectName(), "process") == 0);
    EXPECT_TRUE(np.jsclass != NULL);

    native_netlib_destroy(g_ape);
}

