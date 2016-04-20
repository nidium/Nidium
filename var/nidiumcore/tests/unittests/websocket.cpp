/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <Net/WebSocket.h>

TEST(Websocket, Simple)
{
    ape_global * g_ape;
    ape_socket *socket;

    g_ape = APE_init();
    socket = APE_socket_new(APE_SOCKET_PT_UDP, 0, g_ape);
    Nidium::Net::WebSocketListener wsl(8080);

    APE_loop_run(g_ape);
    APE_destroy(g_ape);
}

