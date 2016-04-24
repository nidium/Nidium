/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <ape_netlib.h>
#include <Binding/JSFileIO.h>

TEST(JSFileIO, Simple)
{
    ape_global * g_ape = APE_init();
    Nidium::Binding::NidiumJS njs(g_ape);
    bool success;

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedValue rval(njs.cx, JSVAL_VOID);
    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globObj, "File", &rval);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

    Nidium::Binding::JSFileIO::RegisterObject(njs.cx);

    rval = JSVAL_VOID;
    success = JS_GetProperty(njs.cx, globObj, "File", &rval);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

    APE_destroy(g_ape);
}

TEST(JSFileIO, Init)
{
    ape_global * g_ape = APE_init();
    Nidium::Binding::NidiumJS njs(g_ape);

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    //Nidium::Core::Path::RegisterScheme(SCHEME_DEFINE("file://", Nidium::IO::FileStream, false), true);

    Nidium::Binding::JSFileIO file(globObj, njs.cx);

    EXPECT_TRUE(file.getJSObject() == globObj);
    EXPECT_TRUE(file.getJSContext() == njs.cx);

    EXPECT_TRUE(file.m_Encoding == NULL);

    APE_destroy(g_ape);
}

