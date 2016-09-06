/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <ape_netlib.h>
#include <Core/Context.h>
#include <Binding/JSSocket.h>

NIDIUMJS_FIXTURE(JSSocket)

TEST_F(JSSocket, Simple)
{
    bool success;

    JS::RootedObject globObj(njs->m_Cx, JS::CurrentGlobalOrNull(njs->m_Cx));
    JS::RootedValue rval(njs->m_Cx, JSVAL_VOID);

    Nidium::Binding::JSSocket::RegisterObject(njs->m_Cx);

    success = JS_GetProperty(njs->m_Cx, globObj, "Socket", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);
}

TEST_F(JSSocket, Init)
{
    const char * host = "127.0.0.1";
    const uint16_t port = 1212;

    JS::RootedObject globObj(njs->m_Cx, JS::CurrentGlobalOrNull(njs->m_Cx));
    Nidium::Binding::JSSocket ns(njs->m_Cx, host, port);

    EXPECT_TRUE(ns.getJSObject() == globObj);
    EXPECT_TRUE(ns.getJSContext() == njs->m_Cx);

    EXPECT_TRUE(ns.m_Socket == NULL);
    EXPECT_EQ(ns.m_Flags, 0);
    EXPECT_TRUE(ns.m_ParentServer == NULL);
    EXPECT_TRUE(strcmp(ns.m_Host, host) == 0);
    EXPECT_EQ(ns.m_Port, port);
    EXPECT_EQ(ns.m_LineBuffer.pos, 0);
    EXPECT_TRUE(ns.m_LineBuffer.data == NULL);
    EXPECT_TRUE(ns.m_Encoding == NULL);
}

