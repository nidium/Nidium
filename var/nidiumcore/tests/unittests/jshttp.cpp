/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <ape_netlib.h>
#include <Binding/JSHTTP.h>

TEST(JSHTTP, Simple)
{
    ape_global * g_ape = APE_init();
    Nidium::Binding::NidiumJS njs(g_ape);
    bool success;

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedValue rval(njs.cx, JSVAL_VOID);
    success = JS_GetProperty(njs.cx, globObj, "Http", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    Nidium::Binding::JSHTTP::RegisterObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globObj, "Http", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    APE_destroy(g_ape);
}

TEST(JSHTTP, init)
{
    ape_global * g_ape = APE_init();
    Nidium::Binding::NidiumJS njs(g_ape);
    char * url = strdup("http://nidium.com:80/new.html");

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    Nidium::Binding::JSHTTP ht(globObj, njs.cx, url);

    EXPECT_TRUE(ht.getJSObject() == globObj);
    EXPECT_TRUE(ht.getJSContext() == njs.cx);

    EXPECT_TRUE(ht.request == JSVAL_NULL);
    EXPECT_TRUE(ht.refHttp == NULL);
    EXPECT_TRUE(ht.m_Eval == true);
    EXPECT_TRUE(strcmp(ht.m_URL, url) == 0);

    free(url);
    APE_destroy(g_ape);
}

