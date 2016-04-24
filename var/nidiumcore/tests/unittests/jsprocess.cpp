/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <ape_netlib.h>
#include <Binding/JSProcess.h>

TEST(JSProcess, Simple)
{
    ape_global * g_ape = native_netlib_init();
    Nidium::Binding::NidiumJS njs(g_ape);
    bool success;
    const char * args[] = {"nidium"};

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedValue rval(njs.cx, JSVAL_VOID);
    success = JS_GetProperty(njs.cx, globObj, "process", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    JSProcess::RegisterObject(njs.cx, (char**)args, 1);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globObj, "process", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    JS::RootedValue argv(njs.cx, JSVAL_VOID);
    JS::RootedObject obj(njs.cx, JSVAL_TO_OBJECT(rval));
    JS_GetProperty(njs.cx, obj, "argv", &argv);
    EXPECT_TRUE(JSVAL_IS_VOID(argv) == false);

    JSProcess *jproc = NULL;
    jproc = (Nidium::Binding::JSProcess*)JS_GetPrivate(obj);
    EXPECT_TRUE(jproc != NULL);

    JS::RootedValue el(njs.cx, JSVAL_VOID);
    JS::RootedObject argObj(njs.cx, JSVAL_TO_OBJECT(argv));
    JS_GetElement(njs.cx, argObj, 0, &el);
    JS::RootedString jstr(njs.cx, JSVAL_TO_STRING(el));
    char * cstr = JS_EncodeString(njs.cx, jstr);
    EXPECT_TRUE(strcmp(cstr, "nidium") == 0);
    free(cstr);

    native_netlib_destroy(g_ape);
}

TEST(JSProcess, Init)
{
    ape_global * g_ape = native_netlib_init();
    Nidium::Binding::NidiumJS njs(g_ape);

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    Nidium::Binding::JSProcess np(globObj, njs.cx);

    EXPECT_TRUE(np.getJSObject() == globObj);
    EXPECT_TRUE(np.getJSContext() == njs.cx);

    EXPECT_TRUE(strcmp(np.GetJSObjectName(), "process") == 0);
    EXPECT_TRUE(np.jsclass != NULL);

    native_netlib_destroy(g_ape);
}

