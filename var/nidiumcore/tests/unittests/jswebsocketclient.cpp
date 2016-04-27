/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include <ape_netlib.h>

#include "unittest.h"

#include <Binding/JSWebSocketClient.h>

NIDIUMJS_FIXTURE(JSWebsocketClient)

TEST_F(JSWebsocketClient, Simple)
{
    bool success;

    JS::RootedObject globObj(njs->cx, JS::CurrentGlobalOrNull(njs->cx));
    JS::RootedValue rval(njs->cx, JSVAL_VOID);
    success = JS_GetProperty(njs->cx, globObj, "WebSocket", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);
    success = JS_GetProperty(njs->cx, globObj, "WebSocket", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    Nidium::Binding::JSWebSocket::RegisterObject(njs->cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs->cx, globObj, "WebSocket", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs->cx, globObj, "WebSocket", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    ape_running = 0;
    APE_loop_run(g_ape);
}

