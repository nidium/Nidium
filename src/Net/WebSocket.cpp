/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "WebSocket.h"

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

#include "Binding/NidiumJS.h"

namespace Nidium {
namespace Net {

#define REQUEST_HEADER(header) ape_array_lookup(m_HttpState.headers.list, \
    CONST_STR_LEN(header "\0"))

static void nidium_on_ws_frame(websocket_state *state,
    const unsigned char *data, ssize_t length, int binary);

WebSocketListener::WebSocketListener(uint16_t port, const char *ip) :
    HTTPServer(port, ip)
{

}

void WebSocketListener::onClientConnect(ape_socket *client, ape_global *ape)
{
    client->ctx = new WebSocketClientConnection(this, client);
}

WebSocketClientConnection::WebSocketClientConnection(
        Nidium::Net::HTTPServer *httpserver, ape_socket *socket) :
    Nidium::Net::HTTPClientConnection(httpserver, socket), m_Handshaked(false),
    m_PingTimer(0), m_Data(NULL)
{
    m_ClientTimeoutMs = 0; /* Disable HTTP timeout */
    ape_ws_init(&m_WSState, 0);
    m_WSState.socket = socket;
    m_WSState.on_frame = nidium_on_ws_frame;
}

WebSocketClientConnection::~WebSocketClientConnection()
{
    if (m_SocketClient->ctx == this) {
        m_SocketClient->ctx = NULL;
        APE_socket_shutdown_now(m_SocketClient);
    }

    if (m_PingTimer) {
        ape_global *ape = Nidium::Binding::NidiumJS::getNet();

        APE_timer_clearbyid(ape, m_PingTimer, 1);

        m_PingTimer = 0;
    }
}

int WebSocketClientConnection::pingTimer(void *arg)
{
    WebSocketClientConnection *con = (WebSocketClientConnection *)arg;

    con->ping();

    return WEBSOCKET_PING_INTERVAL;
}

void WebSocketClientConnection::onHeaderEnded()
{

}

void WebSocketClientConnection::onDisconnect(ape_global *ape)
{
    Nidium::Core::Args args;
    args[0].set(this);

    if (m_PingTimer) {
        APE_timer_clearbyid(ape, m_PingTimer, 1);
        m_PingTimer = 0;
    }

    m_HTTPListener->fireEvent<WebSocketListener>(WebSocketListener::SERVER_CLOSE, args);
}

void WebSocketClientConnection::onUpgrade(const char *to)
{
    if (strcasecmp(to, "websocket") != 0) {
        this->close();
        return;
    }

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
        ws_computed_key, strlen(ws_computed_key), APE_DATA_AUTORELEASE);
    APE_socket_write(m_SocketClient,
        (void *)CONST_STR_LEN("\r\nSec-WebSocket-Origin: 127.0.0.1\r\n\r\n"),
        APE_DATA_STATIC);

    m_Handshaked = true;

    Nidium::Core::Args args;
    args[0].set(this);

    ape_timer_t *timer = APE_timer_create(m_SocketClient->ape, WEBSOCKET_PING_INTERVAL,
        WebSocketClientConnection::pingTimer, this);

    m_PingTimer = APE_timer_getid(timer);
    m_HTTPListener->fireEvent<WebSocketListener>(WebSocketListener::SERVER_CONNECT, args);

}

void WebSocketClientConnection::onContent(const char *data, size_t len)
{
    m_LastAcitivty = Nidium::Core::Utils::getTick(true);

    ape_ws_process_frame(&m_WSState, data, len);
}

void WebSocketClientConnection::onFrame(const char *data, size_t len,
    bool binary)
{
    Nidium::Core::Args args;
    args[0].set(this);
    args[1].set((void *)data);
    args[2].set(len);
    args[3].set(binary);

    m_HTTPListener->fireEvent<WebSocketListener>(WebSocketListener::SERVER_FRAME, args);
}

void WebSocketClientConnection::close()
{
    if (!m_Handshaked) {
        APE_socket_shutdown_now(m_SocketClient);
        return;
    }
    ape_ws_close(&m_WSState);
}

void WebSocketClientConnection::ping()
{
    if (!m_Handshaked) {
        return;
    }
    ape_ws_ping(&m_WSState);
}

void WebSocketClientConnection::write(unsigned char *data,
    size_t len, bool binary, ape_socket_data_autorelease type)
{
    ape_ws_write(&m_WSState, (unsigned char *)data, len,
        (int)binary, type);
}

static void nidium_on_ws_frame(websocket_state *state,
    const unsigned char *data, ssize_t length, int binary)
{
    ape_socket *sock = state->socket;
    if (sock == NULL) {
        return;
    }

    WebSocketClientConnection *con = (WebSocketClientConnection *)sock->ctx;

    if (con == NULL) {
        return;
    }

    con->onFrame((const char *)data, length, (bool)binary);
}

} // namespace Net
} // namespace Nidium

