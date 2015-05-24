#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeHTTPListener.h>
#include <NativeJS.h>

TEST(NativeHTTPListener, Simple)
{
    ape_global *g_ape = native_netlib_init();
    NativeJS g_native(g_ape);

    NativeHTTPListener nl(8111);
    EXPECT_EQ(nl.getPort(), 8111);
    EXPECT_TRUE(strcmp(nl.getIP(), "0.0.0.0") == 0);
    EXPECT_TRUE(nl.getSocket() != NULL);
    nl.start(false);
    nl.stop();

    ape_running = 0;
    events_loop(g_ape);

    native_netlib_destroy(g_ape);
}

