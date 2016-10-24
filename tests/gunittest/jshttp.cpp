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
#include <Binding/JSHTTP.h>

NIDIUMJS_FIXTURE(JSHTTP)

TEST_F(JSHTTP, Simple)
{
    bool success;

    JS::RootedObject globObj(njs->m_Cx, JS::CurrentGlobalOrNull(njs->m_Cx));
    JS::RootedValue rval(njs->m_Cx, JSVAL_VOID);

    Nidium::Binding::JSHTTP::RegisterObject(njs->m_Cx);

    success = JS_GetProperty(njs->m_Cx, globObj, "HTTP", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);
}

