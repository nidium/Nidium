/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <ape_netlib.h>
#include <Binding/JSDebug.h>

NIDIUMJS_FIXTURE(JSDebug)

TEST_F(JSDebug, Simple)
{
    bool success;


    JS::RootedObject globObj(njs->cx, JS::CurrentGlobalOrNull(njs->cx));
    JS::RootedValue rval(njs->cx, JSVAL_VOID);
    success = JS_GetProperty(njs->cx, globObj, "Debug", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    Nidium::Binding::JSDebug::RegisterObject(njs->cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs->cx, globObj, "Debug", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);
}

TEST_F(JSDebug, Init)
{
    JS::RootedObject globObj(njs->cx, JS::CurrentGlobalOrNull(njs->cx));
    Nidium::Binding::JSDebug nd(globObj, njs->cx);

    EXPECT_TRUE(nd.getJSObject() == globObj);
    EXPECT_TRUE(nd.getJSContext() == njs->cx);

    EXPECT_TRUE(strcmp(nd.GetJSObjectName(), "Debug") == 0);
}

