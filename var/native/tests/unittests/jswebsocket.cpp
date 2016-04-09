/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeJSWebSocket.h>

TEST(NativeJSWebsocket, Simple)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    bool success;

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedValue rval(njs.cx, JSVAL_VOID);
    success = JS_GetProperty(njs.cx, globObj, "WebSocketServer", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);
    success = JS_GetProperty(njs.cx, globObj, "WebSocketServerClient", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    NativeJSWebSocketServer::registerObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globObj, "WebSocketServer", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globObj, "WebSocketServerClient", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    ape_running = 0;
    events_loop(g_ape);
    native_netlib_destroy(g_ape);
}

TEST(NativeJSWebsocket, Init)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    NativeJSWebSocketServer nws(globObj, njs.cx, "0.0.0.0", 8888);

    EXPECT_TRUE(nws.getJSObject() == globObj);
    EXPECT_TRUE(nws.getJSContext() == njs.cx);
}

