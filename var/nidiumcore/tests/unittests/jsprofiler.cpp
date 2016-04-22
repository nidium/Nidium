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
#include <Binding/JSProfiler.h>


TEST(JSProfiler, Init)
{
    ape_global * g_ape = native_netlib_init();
    Nidium::Binding::NidiumJS njs(g_ape);
    Nidium::Binding::Profile::RegisterObject(njs.cx);

    Nidium::Binding::Profiler *np = Nidium::Binding::Profiler::getInstance(njs.cx);

    np->start("dummy");
    np->stop();

    JS::RootedObject obj(njs.cx, np->toJSObject());
    EXPECT_TRUE(obj != NULL);

    obj = np->getJSObject();
    EXPECT_TRUE(obj != NULL);

    JS::RootedValue rval(njs.cx, JSVAL_VOID);
    JS_GetProperty(njs.cx, obj, "toJSObject", &rval);
    EXPECT_TRUE(!JSVAL_IS_VOID(rval));

    rval = JSVAL_VOID;
    JS_GetProperty(njs.cx, obj, "dump", &rval);
    EXPECT_TRUE(!JSVAL_IS_VOID(rval));

    native_netlib_destroy(g_ape);
}

TEST(JSProfiler, InitEntry)
{
    char * sigDummy = strdup("object");
    char * sig;
    ape_global * g_ape = native_netlib_init();
    Nidium::Binding::NidiumJS njs(g_ape);
    Nidium::Binding::ProfileEntry pe("file.js", "madFunc", 12, sigDummy, NULL);
    EXPECT_EQ(pe.getTotalTime(), 0);
    EXPECT_EQ(pe.getTotalTSC(), 0);
    EXPECT_EQ(pe.getTotalCall(), 0);
    EXPECT_EQ(pe.getLine(), 12);
    EXPECT_TRUE(strcmp(pe.getScript(), "file.js") == 0);
    EXPECT_TRUE(strcmp(pe.getFunction(), "madFunc") == 0);
    EXPECT_TRUE(strcmp(pe.getSignature(), sigDummy) == 0);
    EXPECT_TRUE(pe.getParent() == NULL);
    sig = pe.generateSignature("di.js", "sub", 121);
    EXPECT_TRUE(strcmp(sig, "di.js:sub@121") == 0);
    pe.enter();
    pe.exit();
    EXPECT_TRUE(pe.getTotalTime()> 0);
    EXPECT_TRUE(pe.getTotalTSC() > 0);
    EXPECT_TRUE(pe.getTotalCall()> 0);

    JS::RootedObject obj(njs.cx, pe.toJSObject(njs.cx));
    EXPECT_TRUE(obj.get() != NULL);

    free(sig);
    free(sigDummy);
    native_netlib_destroy(g_ape);
}

TEST(JSProfiler, InitChild)
{
    char * sigDummy = strdup("object");
    ape_global * g_ape = native_netlib_init();
    Nidium::Binding::NidiumJS njs(g_ape);
    Nidium::Binding::ProfileEntry pe("file.js", "madFunc", 12, sigDummy, NULL);
    Nidium::Binding::ProfileChildEntry ce(&pe, 13);
    EXPECT_TRUE(ce.getEntry() == &pe);
    EXPECT_EQ(ce.getTotalTime(), 0);
    EXPECT_EQ(ce.getTotalTSC(), 0);
    EXPECT_EQ(ce.getTotalCall(), 0);
    EXPECT_EQ(ce.getCallLine(), 13);
    ce.update(14, 16);
    EXPECT_EQ(ce.getTotalTime(), 14);
    EXPECT_EQ(ce.getTotalTSC(), 16);
    EXPECT_EQ(ce.getTotalCall(), 1);

    JS::RootedObject obj(njs.cx, pe.toJSObject(njs.cx));
    EXPECT_TRUE(obj.get() != NULL);

    free(sigDummy);
    native_netlib_destroy(g_ape);
}

