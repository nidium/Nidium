/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <ape_netlib.h>
#include <Binding/JSThread.h>

TEST(JSThread, Simple)
{
    ape_global * g_ape = native_netlib_init();
    Nidium::Binding::NidiumJS njs(g_ape);
    bool success;

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedValue rval(njs.cx, JSVAL_VOID);
    success = JS_GetProperty(njs.cx, globObj, "Thread", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    Nidium::Binding::JSThread::RegisterObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globObj, "Thread", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    native_netlib_destroy(g_ape);
}

TEST(JSThread, Init)
{
    ape_global * g_ape = native_netlib_init();
    Nidium::Binding::NidiumJS njs(g_ape);

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    Nidium::Binding::JSThread nt(globObj, njs.cx);

    EXPECT_TRUE(nt.getJSObject() == globObj);
    EXPECT_TRUE(nt.getJSContext() == njs.cx);

    EXPECT_TRUE(nt.jsFunction == NULL);
    EXPECT_TRUE(nt.jsRuntime == NULL);
    EXPECT_TRUE(nt.jsCx == NULL);
    EXPECT_TRUE(nt.jsObject == NULL);
    EXPECT_TRUE(nt.njs == NULL);
    EXPECT_TRUE(nt.markedStop == false);

    native_netlib_destroy(g_ape);
}

