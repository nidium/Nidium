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
#include <string.h>

#define REQUEST_HEADER(header) ape_array_lookup(m_HttpState.headers.list, \
    CONST_STR_LEN(header "\0"))

static void native_on_ws_frame(websocket_state *state,
    const unsigned char *data, ssize_t length);

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
        NativeHTTPListener *httpserver, ape_socket *socket) :
    NativeHTTPClientConnection(httpserver, socket), m_Handshaked(false)
{
    ape_ws_init(&m_WSState);
    m_WSState.socket = socket;
    m_WSState.on_frame = native_on_ws_frame;
}

NativeWebSocketClientConnection::~NativeWebSocketClientConnection()
{
    if (m_SocketClient->ctx == this) {
        m_SocketClient->ctx = NULL;
        APE_socket_shutdown_now(m_SocketClient);
    }
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
    if (strcasecmp(to, "websocket") != 0) {
        this->close();
        return;
    }
    printf("upgraded\n");
    const buffer *ws_key = REQUEST_HEADER("Sec-WebSocket-Key");

    if (ws_key == NULL || ws_key->used < 2) {
        this->close();
        return;
    }

    char *ws_computed_key = ape_ws_compute_key((const char *)ws_key->data,
        ws_key->used-1);

    APE_socket_write(m_SocketClient,
        (void *)CONST_STR_LEN(WEBSOCKET_HARDCODED_HEADERS), APE_DATA_STATIC);
    APE_socket_write(m_SocketClient,
        (void*)CONST_STR_LEN("Sec-WebSocket-Accept: "), APE_DATA_STATIC);
    APE_socket_write(m_SocketClient,
        ws_computed_key, strlen(ws_computed_key), APE_DATA_STATIC);
    APE_socket_write(m_SocketClient,
        (void *)CONST_STR_LEN("\r\nSec-WebSocket-Origin: 127.0.0.1\r\n\r\n"), APE_DATA_STATIC);

    m_Handshaked = true;   
}

void NativeWebSocketClientConnection::onContent(const char *data, size_t len)
{
    ape_ws_process_frame(&m_WSState, data, len);
}

void NativeWebSocketClientConnection::onFrame(const char *data, size_t len)
{
    printf("Ws got a frame of length : %ld\n", len);
    printf("Frame : %s\n", data);
}

void NativeWebSocketClientConnection::close()
{
    if (!m_Handshaked) {
        APE_socket_shutdown_now(m_SocketClient);
        return;
    }
    ape_ws_close(&m_WSState);
}

void NativeWebSocketClientConnection::write(const char *data,
    size_t len, ape_socket_data_autorelease type)
{
    ape_ws_write(m_SocketClient, (unsigned char *)data, len, type);
}

static void native_on_ws_frame(websocket_state *state,
    const unsigned char *data, ssize_t length)
{
    ape_socket *sock = state->socket;
    if (sock == NULL) {
        return;
    }

    NativeWebSocketClientConnection *con = (NativeWebSocketClientConnection *)sock->ctx;

    if (con == NULL) {
        return;
    }

    con->onFrame((const char *)data, length);
}
