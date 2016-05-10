/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <ape_netlib.h>
#include <Binding/JSHTTPServer.h>

NIDIUMJS_FIXTURE(JSHTTPServer)

TEST_F(JSHTTPServer, Simple)
{
    bool success;

    JS::RootedObject globObj(njs->m_Cx, JS::CurrentGlobalOrNull(njs->m_Cx));
    JS::RootedValue rval(njs->m_Cx, JSVAL_VOID);
    success = JS_GetProperty(njs->m_Cx, globObj, "HTTPListener", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    Nidium::Binding::JSHTTPServer::RegisterObject(njs->m_Cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs->m_Cx, globObj, "HTTPListener", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);
}

/*
TEST_F(JSServer, Connection)
{
    ape_global * g_ape = APE_init();
    Nidium::Binding::Nidiumcore njs(g_ape);
    ape_socket *socket = APE_socket_new(APE_SOCKET_PT_TCP, 0, g_ape);
    JS::RootedValue args(cx);
    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &HTTPServer_class, args));
    Nidium::Binding::JSHTTPServer server(ret, 8080, "0.0.0.0");
    Nidium::Binding::JSHTTPClientConnection conn(njs->m_Cx, &server, socket);
    EXPECT_EQ(conn.getHTTPServer(), &server);

    APE_destroy(g_ape);
}
*/

TEST_F(JSHTTPServer, Server)
{
    JS::RootedObject globObj(njs->m_Cx, JS::CurrentGlobalOrNull(njs->m_Cx));
    Nidium::Binding::JSHTTPServer lis(globObj, njs->m_Cx, 8080, "127.0.0.1");

    EXPECT_TRUE(lis.getJSObject() == globObj);
    EXPECT_TRUE(lis.getJSContext() == njs->m_Cx);

    EXPECT_EQ(lis.getPort(), 8080);
    EXPECT_TRUE(strcmp(lis.getIP(), "127.0.0.1") == 0);
}

