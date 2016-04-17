/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <JS/NativeJSStream.h>
#include <IO/NativeFileStream.h>

TEST(NativeJSStream, Simple)
{
    ape_global * g_ape = APE_init();
    NativeJS njs(g_ape);
    bool success;

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedValue rval(njs.cx, JSVAL_VOID);
    success = JS_GetProperty(njs.cx, globObj, "Stream", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    NativeJSStream::registerObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globObj, "Stream", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    APE_destroy(g_ape);
}

