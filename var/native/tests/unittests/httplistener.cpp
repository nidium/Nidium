/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <Net/NativeHTTPListener.h>
#include <JS/NativeJS.h>

TEST(NativeHTTPListener, Simple)
{
    ape_global *g_ape = APE_init();
    NativeJS g_native(g_ape);

    NativeHTTPListener nl(8111);

    EXPECT_EQ(nl.getPort(), 8111);
    EXPECT_TRUE(strcmp(nl.getIP(), "0.0.0.0") == 0);
    EXPECT_TRUE(nl.getSocket() != NULL);

    nl.start(false);
    nl.stop();

    ape_running = 0;
    APE_loop_run(g_ape);

    APE_destroy(g_ape);
}

