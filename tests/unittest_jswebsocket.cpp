#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeJSWebSocket.h>

TEST(NativeJSWebsocket, Simple)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    JSObject *globalObj = JS_GetGlobalObject(njs.cx);
    jsval rval;
    bool success;

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globalObj, "WebSocketServer", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);
    success = JS_GetProperty(njs.cx, globalObj, "WebSocketServerClient", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    NativeJSWebSocketServer::registerObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globalObj, "WebSocketServer", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globalObj, "WebSocketServerClient", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

//    NativeJSWebSocketServer wss(obj, njs.cx, "localhost", 8888);;

//    wss.start();
    ape_running = 0;
    events_loop(g_ape);
    //@TODO: wss.onMessage;
    native_netlib_destroy(g_ape);
}

#if 0
TEST(NativeJSWebsocket, Init)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    JSObject *globalObj = JS_GetGlobalObject(njs.cx);
    NativeJSWebSocketServer nws(globalObj, njs.cx, "0.0.0.0", 8888);

    EXPECT_TRUE(nws.getJSObject() == globalObj);
    EXPECT_TRUE(nws.getJSContext() == njs.cx);
}

#endif

