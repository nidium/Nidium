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
#include <Binding/JSThread.h>

NIDIUMJS_FIXTURE(JSThread)

TEST_F(JSThread, Simple)
{
    bool success;

    JS::RootedObject globObj(njs->m_Cx, JS::CurrentGlobalOrNull(njs->m_Cx));
    JS::RootedValue rval(njs->m_Cx, JSVAL_VOID);
    success = JS_GetProperty(njs->m_Cx, globObj, "Thread", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    Nidium::Binding::JSThread::RegisterObject(njs->m_Cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs->m_Cx, globObj, "Thread", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);
}

TEST_F(JSThread, Init)
{
    JS::RootedObject globObj(njs->m_Cx, JS::CurrentGlobalOrNull(njs->m_Cx));
    Nidium::Binding::JSThread nt(globObj, njs->m_Cx);

    EXPECT_TRUE(nt.getJSObject() == globObj);
    EXPECT_TRUE(nt.getJSContext() == njs->m_Cx);

    EXPECT_TRUE(nt.jsFunction == NULL);
    EXPECT_TRUE(nt.jsRuntime == NULL);
    EXPECT_TRUE(nt.jsCx == NULL);
    EXPECT_TRUE(nt.jsObject == NULL);
    EXPECT_TRUE(nt.njs == NULL);
    EXPECT_TRUE(nt.markedStop == false);
}

