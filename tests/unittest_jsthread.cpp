#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJSThread.h>

TEST(NativeJSThread, Simple)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    bool success;

    JS::RootedObject globObjnjs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedValue rval(njs.cx, JSVAL_VOID);
    success = JS_GetProperty(njs.cx, globalObj, "Thread", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    NativeJSThread::registerObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globalObj, "Thread", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    native_netlib_destroy(g_ape);
}

TEST(NativeJSThread, Init)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);

    JS::RootedObject globObjnjs.cx, JS::CurrentGlobalOrNull(njs.cx));
    NativeJSThread nt(globalObj, njs.cx);

    EXPECT_TRUE(nt.getJSObject() == globalObj);
    EXPECT_TRUE(nt.getJSContext() == njs.cx);

    EXPECT_TRUE(nt.jsFunction == NULL);
    EXPECT_TRUE(nt.jsRuntime == NULL);
    EXPECT_TRUE(nt.jsCx == NULL);
    EXPECT_TRUE(nt.jsObject == NULL);
    EXPECT_TRUE(nt.njs == NULL);
    EXPECT_TRUE(nt.markedStop == false);

    native_netlib_destroy(g_ape);
}

