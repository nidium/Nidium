/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <JS/NativeJSHttp.h>

TEST(NativeJSHTTP, Simple)
{
    ape_global * g_ape = APE_init();
    NativeJS njs(g_ape);
    bool success;

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedValue rval(njs.cx, JSVAL_VOID);
    success = JS_GetProperty(njs.cx, globObj, "Http", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    NativeJSHttp::registerObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globObj, "Http", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    APE_destroy(g_ape);
}

TEST(NativeJSHTTP, init)
{
    ape_global * g_ape = APE_init();
    NativeJS njs(g_ape);
    char * url = strdup("http://nidium.com:80/new.html");

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    NativeJSHttp ht(globObj, njs.cx, url);

    EXPECT_TRUE(ht.getJSObject() == globObj);
    EXPECT_TRUE(ht.getJSContext() == njs.cx);

    EXPECT_TRUE(ht.request == JSVAL_NULL);
    EXPECT_TRUE(ht.refHttp == NULL);
    EXPECT_TRUE(ht.m_Eval == true);
    EXPECT_TRUE(strcmp(ht.m_URL, url) == 0);

    free(url);
    APE_destroy(g_ape);
}

