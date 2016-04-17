/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <JS/NativeJSDebug.h>

TEST(NativeJSDebug, Simple)
{
    ape_global * g_ape = APE_init();
    NativeJS njs(g_ape);
    bool success;


    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedValue rval(njs.cx, JSVAL_VOID);
    success = JS_GetProperty(njs.cx, globObj, "Debug", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    NativeJSDebug::registerObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globObj, "Debug", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    APE_destroy(g_ape);
}

TEST(NativeJSDebug, Init)
{
    ape_global * g_ape = APE_init();
    NativeJS njs(g_ape);

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    NativeJSDebug nd(globObj, njs.cx);

    EXPECT_TRUE(nd.getJSObject() == globObj);
    EXPECT_TRUE(nd.getJSContext() == njs.cx);

    EXPECT_TRUE(strcmp(nd.getJSObjectName(), "Debug") == 0);

    APE_destroy(g_ape);
}

