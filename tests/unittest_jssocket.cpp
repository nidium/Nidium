#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJSSocket.h>

TEST(NativeJSSocket, Simple)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    bool success;

    JS::RootedObject globObjnjs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedValue rval(njs.cx, JSVAL_VOID);
    success = JS_GetProperty(njs.cx, globalObj, "Socket", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    NativeJSSocket::registerObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globalObj, "Socket", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    native_netlib_destroy(g_ape);
}

TEST(NativeJSSocket, Init)
{
    const char * host = "127.0.0.1";
    const uint16_t port = 1212;
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);

    JS::RootedObject globObjnjs.cx, JS::CurrentGlobalOrNull(njs.cx));
    NativeJSSocket ns(globalObj, njs.cx, host, port);

    EXPECT_TRUE(ns.getJSObject() == globalObj);
    EXPECT_TRUE(ns.getJSContext() == njs.cx);

    EXPECT_TRUE(ns.socket == NULL);
    EXPECT_EQ(ns.flags, 0);
    EXPECT_TRUE(ns.m_ParentServer == NULL);
    EXPECT_TRUE(strcmp(ns.host, host) == 0);
    EXPECT_EQ(ns.port, port);
    EXPECT_EQ(ns.lineBuffer.pos, 0);
    EXPECT_TRUE(ns.lineBuffer.data == NULL);
    EXPECT_TRUE(ns.m_Encoding == NULL);

    native_netlib_destroy(g_ape);
}

