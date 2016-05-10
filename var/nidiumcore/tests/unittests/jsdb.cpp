/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <ape_netlib.h>
#include <Binding/Nidiumcore.h>
#include <Binding/JSDB.h>

NIDIUMJS_FIXTURE(JSDB)

TEST_F(JSDB, Simple)
{
    bool success;

    JS::RootedObject globObj(njs->m_Cx, JS::CurrentGlobalOrNull(njs->m_Cx));
    JS::RootedValue rval(njs->m_Cx, JSVAL_VOID);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);
/*
    success = JS_GetProperty(njs->m_Cx, globObj, "db", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    Nidium::Binding::JSConsole::RegisterObject(njs->m_Cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs->m_Cx, globObj, "db", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);
    //TODO insert
*/
}

