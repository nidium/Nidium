/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJSHTTPListener.h>

TEST(NativeJSHTTPListener, Simple)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    bool success;

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedValue rval(njs.cx, JSVAL_VOID);
    success = JS_GetProperty(njs.cx, globObj, "HTTPListener", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    NativeJSHTTPListener::registerObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globObj, "HTTPListener", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    native_netlib_destroy(g_ape);
}

TEST(NativeJSListener, Connection)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    ape_socket *socket = APE_socket_new(APE_SOCKET_PT_TCP, 0, g_ape);
    NativeHTTPListener listener(8080, "0.0.0.0");
    NativeJSHTTPClientConnection conn(njs.cx, &listener, socket);
    EXPECT_EQ(conn.getHTTPListener(), &listener);

    native_netlib_destroy(g_ape);
}

TEST(NativeJSHTTPListener, Listener)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    NativeJSHTTPListener lis(globObj, njs.cx, 8080, "127.0.0.1");

    EXPECT_TRUE(lis.getJSObject() == globObj);
    EXPECT_TRUE(lis.getJSContext() == njs.cx);

    EXPECT_EQ(lis.getPort(), 8080);
    EXPECT_TRUE(strcmp(lis.getIP(), "127.0.0.1") == 0);

    native_netlib_destroy(g_ape);
}

