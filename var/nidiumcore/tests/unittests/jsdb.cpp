/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <ape_netlib.h>
#include <Binding/NidiumJS.h>
#include <Binding/JSDB.h>

NIDIUMJS_FIXTURE(JSDB)

TEST_F(JSDB, Simple)
{
    bool success;

    JS::RootedObject globObj(njs->cx, JS::CurrentGlobalOrNull(njs->cx));
    JS::RootedValue rval(njs->cx, JSVAL_VOID);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);
/*
    success = JS_GetProperty(njs->cx, globObj, "db", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    Nidium::Binding::JSConsole::RegisterObject(njs->cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs->cx, globObj, "db", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);
    //TODO insert
*/
}

