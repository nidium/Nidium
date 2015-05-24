#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJSStream.h>
#include <NativeFileStream.h>

TEST(NativeJSStream, Simple)
{
    JSObject *globalObj;
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    jsval rval;
    bool success;

    globalObj = JS_GetGlobalObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globalObj, "Stream", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    NativeJSStream::registerObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globalObj, "Stream", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);
    //@TODO: getstream()
    //@TODO: onMessage();

    native_netlib_destroy(g_ape);
}

#if 0
TEST(NativeJSStream, Init)
{
    const char * url = "file:///tmp/dummy.txt";
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    JSObject * globalObj = JS_GetGlobalObject(njs.cx);

    NativePath::registerScheme(SCHEME_DEFINE("file://", NativeFileStream, false), true);

    NativeJSStream nt(globalObj, njs.cx, g_ape, url);

    EXPECT_TRUE(nt.getJSObject() == globalObj);
    EXPECT_TRUE(nt.getJSContext() == njs.cx);

    EXPECT_TRUE(nt.getStream() != NULL);

    native_netlib_destroy(g_ape);
}
#endif

