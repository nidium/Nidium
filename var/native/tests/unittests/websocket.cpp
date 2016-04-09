/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeWebSocket.h>

TEST(NativeWebsocket, Simple)
{
    ape_global * g_ape;
    ape_socket *socket;

    g_ape = native_netlib_init();
    ape_running = g_ape->is_running = 0;
    socket = APE_socket_new(APE_SOCKET_PT_UDP, 0, g_ape);
    NativeWebSocketListener wsl(8080);

    native_netlib_destroy(g_ape);
}

