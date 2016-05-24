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
#include <Binding/JSGlobal.h>

using namespace JS;

NIDIUMJS_FIXTURE(JSGlobal)

TEST_F(JSGlobal, Simple)
{
    bool success;

    JS::RootedObject globObj(njs->m_Cx, JS::CurrentGlobalOrNull(njs->m_Cx));
    JS::RootedValue rval(njs->m_Cx, JSVAL_VOID);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);
    success = JS_GetProperty(njs->m_Cx, globObj, "load", &rval);
    EXPECT_TRUE(JSTYPE_FUNCTION == JS_TypeOfValue(njs->m_Cx, rval));
    success = JS_GetProperty(njs->m_Cx, globObj, "setTimeout", &rval);
    EXPECT_TRUE(JSTYPE_FUNCTION == JS_TypeOfValue(njs->m_Cx, rval));
    success = JS_GetProperty(njs->m_Cx, globObj, "setInterval", &rval);
    EXPECT_TRUE(JSTYPE_FUNCTION == JS_TypeOfValue(njs->m_Cx, rval));
    success = JS_GetProperty(njs->m_Cx, globObj, "clearTimeout", &rval);
    EXPECT_TRUE(JSTYPE_FUNCTION == JS_TypeOfValue(njs->m_Cx, rval));
    success = JS_GetProperty(njs->m_Cx, globObj, "clearInterval", &rval);
    EXPECT_TRUE(JSTYPE_FUNCTION == JS_TypeOfValue(njs->m_Cx, rval));
    success = JS_GetProperty(njs->m_Cx, globObj, "btoa", &rval);
    EXPECT_TRUE(JSTYPE_FUNCTION == JS_TypeOfValue(njs->m_Cx, rval));
/*
fixme:
    success = JS_GetProperty(njs->m_Cx, globObj, "__filename", &rval);
    EXPECT_TRUE(JSTYPE_STRING == JS_TypeOfValue(njs->m_Cx, rval));
    //todo check value
    success = JS_GetProperty(njs->m_Cx, globObj, "__dirname", &rval);
    EXPECT_TRUE(JSTYPE_STRING == JS_TypeOfValue(njs->m_Cx, rval));
    //todo check value
    success = JS_GetProperty(njs->m_Cx, globObj, "global", &rval);
    EXPECT_TRUE(JSTYPE_OBJECT == JS_TypeOfValue(njs->m_Cx, rval));
    //todo check value
*/
}

