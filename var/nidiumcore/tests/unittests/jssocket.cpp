/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <ape_netlib.h>
#include <Binding/JSSocket.h>

TEST(JSSocket, Simple)
{
    ape_global * g_ape = APE_init();
    Nidium::Binding::NidiumJS njs(g_ape);
    bool success;

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedValue rval(njs.cx, JSVAL_VOID);
    success = JS_GetProperty(njs.cx, globObj, "Socket", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    Nidium::Binding::JSSocket::registerObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globObj, "Socket", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    APE_destroy(g_ape);
}

TEST(JSSocket, Init)
{
    const char * host = "127.0.0.1";
    const uint16_t port = 1212;
    ape_global * g_ape = APE_init();
    Nidium::Binding::NidiumJS njs(g_ape);

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    Nidium::Binding::JSSocket ns(globObj, njs.cx, host, port);

    EXPECT_TRUE(ns.getJSObject() == globObj);
    EXPECT_TRUE(ns.getJSContext() == njs.cx);

    EXPECT_TRUE(ns.socket == NULL);
    EXPECT_EQ(ns.flags, 0);
    EXPECT_TRUE(ns.m_ParentServer == NULL);
    EXPECT_TRUE(strcmp(ns.host, host) == 0);
    EXPECT_EQ(ns.port, port);
    EXPECT_EQ(ns.lineBuffer.pos, 0);
    EXPECT_TRUE(ns.lineBuffer.data == NULL);
    EXPECT_TRUE(ns.m_Encoding == NULL);

    APE_destroy(g_ape);
}

