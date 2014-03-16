/*
    NativeJS Core Library
    Copyright (C) 2014 Anthony Catel <paraboul@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "NativeWebSocket.h"
#include <stdio.h>
#include <stdlib.h>

NativeWebSocketListener::NativeWebSocketListener(uint16_t port, const char *ip) :
    NativeHTTPListener(port, ip)
{

}

void NativeWebSocketListener::onClientConnect(ape_socket *client, ape_global *ape)
{
    client->ctx = new NativeWebSocketClientConnection(this, client);
}

//////////
//////////

NativeWebSocketClientConnection::NativeWebSocketClientConnection(
        NativeHTTPListener *httpserver,
        ape_socket *socket) :
    NativeHTTPClientConnection(httpserver, socket)
{

}

void NativeWebSocketClientConnection::onHeaderEnded()
{
    printf("WS header ended\n");
}

void NativeWebSocketClientConnection::onDisconnect(ape_global *ape)
{
    printf("Ws disconnected\n");
}

void NativeWebSocketClientConnection::onUpgrade(const char *to)
{
    printf("Ws upgrade to %s\n", to);
}

void NativeWebSocketClientConnection::onContent(const char *data, size_t len)
{
    printf("ws data of len %ld\n", len);
}

void NativeWebSocketClientConnection::close()
{

}