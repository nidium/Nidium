#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJSFileIO.h>

TEST(NativeJSFileIO, Simple)
{
    JSObject *globalObj;
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    jsval rval;
    bool success;

    globalObj = JS_GetGlobalObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globalObj, "File", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    NativeJSFileIO::registerObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globalObj, "File", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    //@TODO: GetFileFromJSObject
    //@TODO: generateJSObjecc
    //@TODO: handleError
    //@TODO: callbackForMessage
    //@TODO: getFile
    //@TODO: setFile
    //@TODO: onMessage();

    native_netlib_destroy(g_ape);
}

#if 0
TEST(NativeJSFileIO, Init)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    JSObject * globalObj = JS_GetGlobalObject(njs.cx);

    //NativePath::registerScheme(SCHEME_DEFINE("file://", NativeFileStream, false), true);

    NativeJSFileIO file(globalObj, njs.cx);

    EXPECT_TRUE(file.getJSObject() == globalObj);
    EXPECT_TRUE(file.getJSContext() == njs.cx);

    EXPECT_TRUE(file.m_Encoding == NULL);

    native_netlib_destroy(g_ape);
}
#endif

