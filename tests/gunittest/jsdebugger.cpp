/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <ape_netlib.h>
#include <Binding/JSDebugger.h>
#include <Core/Context.h>

NIDIUMJS_FIXTURE(JSDebugger)

TEST_F(JSDebugger, Simple)
{
    bool success;

    JS::RootedObject globObj(njs->m_Cx, JS::CurrentGlobalOrNull(njs->m_Cx));
    JS::RootedValue rval(njs->m_Cx, JSVAL_VOID);

    Nidium::Binding::JSDebuggerCompartment::RegisterObject(njs->m_Cx);

    success = JS_GetProperty(njs->m_Cx, globObj, "DebuggerCompartement", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);
}

