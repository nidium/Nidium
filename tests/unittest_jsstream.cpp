#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJSStream.h>
#include <NativeFileStream.h>

TEST(NativeJSStream, Simple)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    bool success;

    JS::RootedObject globObjnjs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedValue rval(njs.cx, JSVAL_VOID);
    success = JS_GetProperty(njs.cx, globalObj, "Stream", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    NativeJSStream::registerObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globalObj, "Stream", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    native_netlib_destroy(g_ape);
}

