/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <Net/HTTPServer.h>
#include <Binding/NidiumJS.h>

TEST(HTTPServer, Simple)
{
    ape_global *g_ape = APE_init();
    Nidium::Binding::NidiumJS g_native(g_ape);

    Nidium::Net::HTTPServer nl(8111);

    EXPECT_EQ(nl.getPort(), 8111);
    EXPECT_TRUE(strcmp(nl.getIP(), "0.0.0.0") == 0);
    EXPECT_TRUE(nl.getSocket() != NULL);

    nl.start(false);
    nl.stop();

    ape_running = 0;
    APE_loop_run(g_ape);

    APE_destroy(g_ape);
}

