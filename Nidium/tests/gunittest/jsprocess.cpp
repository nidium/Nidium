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
#include <Binding/JSProcess.h>

NIDIUMJS_FIXTURE(JSProcess)

TEST_F(JSProcess, Simple)
{
    bool success;
    const char * args[] = {"nidium"};

    JS::RootedObject globObj(njs->m_Cx, JS::CurrentGlobalOrNull(njs->m_Cx));
    JS::RootedValue rval(njs->m_Cx, JSVAL_VOID);
    success = JS_GetProperty(njs->m_Cx, globObj, "process", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    Nidium::Binding::JSProcess::RegisterObject(njs->m_Cx, (char**)args, 1);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs->m_Cx, globObj, "process", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    JS::RootedObject obj(njs->m_Cx, rval.toObjectOrNull());

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs->m_Cx, obj, "cwd", &rval);
    EXPECT_TRUE(JSTYPE_FUNCTION == JS_TypeOfValue(njs->m_Cx, rval));

    JS::RootedValue argv(njs->m_Cx, JSVAL_VOID);
    JS_GetProperty(njs->m_Cx, obj, "argv", &argv);
    EXPECT_TRUE(JSVAL_IS_VOID(argv) == false);

    Nidium::Binding::JSProcess *jproc = NULL;
    jproc = (Nidium::Binding::JSProcess*)JS_GetPrivate(obj);
    EXPECT_TRUE(jproc != NULL);

    JS::RootedValue el(njs->m_Cx, JSVAL_VOID);
    JS::RootedObject argObj(njs->m_Cx, JSVAL_TO_OBJECT(argv));
    JS_GetElement(njs->m_Cx, argObj, 0, &el);
    JS::RootedString jstr(njs->m_Cx, JSVAL_TO_STRING(el));
    char * cstr = JS_EncodeString(njs->m_Cx, jstr);
    EXPECT_TRUE(strcmp(cstr, "nidium") == 0);
    free(cstr);
}

