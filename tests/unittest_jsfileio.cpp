#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJSFileIO.h>

TEST(NativeJSFileIO, Simple)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    bool success;

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedValue rval(njs.cx, JSVAL_VOID);
    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globObj, "File", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    NativeJSFileIO::registerObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globObj, "File", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    native_netlib_destroy(g_ape);
}

TEST(NativeJSFileIO, Init)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    //NativePath::registerScheme(SCHEME_DEFINE("file://", NativeFileStream, false), true);

    NativeJSFileIO file(globObj, njs.cx);

    EXPECT_TRUE(file.getJSObject() == globObj);
    EXPECT_TRUE(file.getJSContext() == njs.cx);

    EXPECT_TRUE(file.m_Encoding == NULL);

    native_netlib_destroy(g_ape);
}

