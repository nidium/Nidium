/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJS.h>
#include <NativeJSModules.h>

TEST(NativeJSModules, ModulesSimple)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    NativeJSModules modules(njs.cx);
    EXPECT_TRUE(modules.main == NULL);
    EXPECT_TRUE(strcmp(modules.m_TopDir, "/") == 0);
    modules.setPath("/tmp/");
    EXPECT_TRUE(strcmp(modules.m_TopDir, "/tmp/") == 0);
    modules.init();
    EXPECT_TRUE(modules.main != NULL);

    native_netlib_destroy(g_ape);
}

TEST(NativeJSModules, ModuleSimple)
{
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    NativeJSModules modules(njs.cx);
    NativeJSModule module(njs.cx, &modules, NULL, "dummy");
    EXPECT_TRUE(module.absoluteDir == NULL);
    EXPECT_TRUE(module.filePath == NULL);
    EXPECT_TRUE(strcmp(module.name, "dummy") == 0);
    EXPECT_TRUE(module.m_ModuleType == NativeJSModule::NONE);
    EXPECT_TRUE(module.m_Cached == false);
    EXPECT_TRUE(module.exports == NULL);
    EXPECT_TRUE(module.parent == NULL);
    EXPECT_TRUE(module.modules == &modules);

    native_netlib_destroy(g_ape);
}

