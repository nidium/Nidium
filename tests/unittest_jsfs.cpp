#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJS.h>
#include <NativeJSFS.h>

TEST(NativeJSFS, Simple)
{
    JSObject *globalObj;
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    jsval rval;
    bool success;

    globalObj = JS_GetGlobalObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globalObj, "fs", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    NativeJSFS::registerObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globalObj, "fs", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    rval = JSVAL_VOID;
    JSObject *obj = JSVAL_TO_OBJECT(rval);
    JS_GetProperty(njs.cx, obj, "readDir", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    native_netlib_destroy(g_ape);
}

#if 0
//@FIXME: uniform register JSObject * globalObj = JS_GetGlobalObject(njs.cx);
TEST(NativeJSFS, Init)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);

    native_netlib_destroy(g_ape);
}
#endif

