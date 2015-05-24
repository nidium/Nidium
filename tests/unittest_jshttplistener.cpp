#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJSHTTPListener.h>

TEST(NativeJSHTTPListener, Simple)
{
    JSObject *globalObj;
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    jsval rval;
    bool success;

    globalObj = JS_GetGlobalObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globalObj, "HTTPListener", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    NativeJSHTTPListener::registerObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globalObj, "HTTPListener", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);
    //@TODO: onClientConnect
    //@TODO: onClientDisconnect
    //@TODO: onData
    //@TODO: onEnd

    native_netlib_destroy(g_ape);
}

#if 0
TEST(NativeJSListener, Response)
{
    //@TODO: NativeJSHTTPResponse response._createResponse();
}
#endif

TEST(NativeJSListener, Connection)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    ape_socket *socket = APE_socket_new(APE_SOCKET_PT_TCP, 0, g_ape);
    NativeHTTPListener listener(8080, "0.0.0.0");
    NativeJSHTTPClientConnection conn(njs.cx, &listener, socket);
    EXPECT_EQ(conn.getHTTPListener(), &listener);
    //TODO: onCreateResponse

    native_netlib_destroy(g_ape);
}

TEST(NativeJSHTTPListener, Listener)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    JSObject * globalObj = JS_GetGlobalObject(njs.cx);

    NativeJSHTTPListener lis(globalObj, njs.cx, 8080, "127.0.0.1");

    EXPECT_TRUE(lis.getJSObject() == globalObj);
    EXPECT_TRUE(lis.getJSContext() == njs.cx);

    EXPECT_EQ(lis.getPort(), 8080);
    EXPECT_TRUE(strcmp(lis.getIP(), "127.0.0.1") == 0);

    native_netlib_destroy(g_ape);
}

