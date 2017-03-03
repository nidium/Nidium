/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Net/WebSocket.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#ifdef _MSC_VER
#include "Port/MSWindows.h"
#else
#include <strings.h>
#include <unistd.h>
#endif

#include "Binding/NidiumJS.h"

using Nidium::Core::Args;

namespace Nidium {
namespace Net {

// {{{ Preamble
#define REQUEST_HEADER(header) \
    ape_array_lookup(m_HttpState.headers.list, CONST_STR_LEN(header "\0"))
// }}}

// {{{ WebSocketServer
WebSocketServer::WebSocketServer(uint16_t port, const char *ip)
    : HTTPServer(port, ip)
{
}

HTTPClientConnection *WebSocketServer::onClientConnect(ape_socket *client,
    ape_global *ape)
{
    return new WebSocketClientConnection(this, client);
}
// }}}

// {{{ WebSocketClientConnection Implementation
static void nidium_on_ws_frame(websocket_state *state,
                               const unsigned char *data,
                               ssize_t length,
                               int binary)
{
    ape_socket *sock = state->socket;
    if (sock == NULL) {
        return;
    }

    WebSocketClientConnection *con
        = static_cast<WebSocketClientConnection *>(sock->ctx);

    if (con == NULL) {
        return;
    }

    con->onFrame(reinterpret_cast<const char *>(data), length,
                 static_cast<bool>(binary));
}

WebSocketClientConnection::WebSocketClientConnection(HTTPServer *httpserver,
                                                     ape_socket *socket)
    : HTTPClientConnection(httpserver, socket), m_Handshaked(false),
      m_PingTimer(0), m_Data(NULL)
{
    m_ClientTimeoutMs = 0; /* Disable HTTP timeout */
    ape_ws_init(&m_WSState, 0);
    m_WSState.socket   = socket;
    m_WSState.on_frame = nidium_on_ws_frame;
}

void WebSocketClientConnection::ping()
{
    if (!m_Handshaked) {
        return;
    }
    ape_ws_ping(&m_WSState);
}

int WebSocketClientConnection::PingTimer(void *arg)
{
    WebSocketClientConnection *con
        = static_cast<WebSocketClientConnection *>(arg);

    con->ping();

    return WEBSOCKET_PING_INTERVAL;
}

void WebSocketClientConnection::write(unsigned char *data,
                                      size_t len,
                                      bool binary,
                                      ape_socket_data_autorelease type)
{
    ape_ws_write(&m_WSState, static_cast<unsigned char *>(data), len,
                 static_cast<int>(binary), type);
}

void WebSocketClientConnection::close()
{
    if (!m_Handshaked) {
        APE_socket_shutdown_now(m_SocketClient);
        return;
    }
    ape_ws_close(&m_WSState);
}

WebSocketClientConnection::~WebSocketClientConnection()
{
    if (m_SocketClient->ctx == this) {
        m_SocketClient->ctx = NULL;
        APE_socket_shutdown_now(m_SocketClient);
    }

    if (m_PingTimer) {
        ape_global *ape = Binding::NidiumJS::GetNet();

        APE_timer_clearbyid(ape, m_PingTimer, 1);

        m_PingTimer = 0;
    }
}
// }}}

// {{{ WebSocketClientConnection Events
void WebSocketClientConnection::onHeaderEnded()
{
}

void WebSocketClientConnection::onDisconnect(ape_global *ape)
{
    Args args;
    args[0].set(this);

    if (m_PingTimer) {
        APE_timer_clearbyid(ape, m_PingTimer, 1);
        m_PingTimer = 0;
    }

    m_HTTPServer->fireEventSync<WebSocketServer>(
        WebSocketServer::kEvents_ServerClose, args);
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

    char *ws_computed_key = ape_ws_compute_key(
        reinterpret_cast<const char *>(ws_key->data), ws_key->used - 1);

    PACK_TCP(m_SocketClient->s.fd);
    APE_socket_write(m_SocketClient,
                     (void *)CONST_STR_LEN(WEBSOCKET_HARDCODED_HEADERS),
                     APE_DATA_STATIC);
    APE_socket_write(m_SocketClient,
                     (void *)CONST_STR_LEN("Sec-WebSocket-Accept: "),
                     APE_DATA_STATIC);
    APE_socket_write(m_SocketClient, ws_computed_key, strlen(ws_computed_key),
                     APE_DATA_AUTORELEASE);
    APE_socket_write(
        m_SocketClient,
        (void *)CONST_STR_LEN("\r\nSec-WebSocket-Origin: 127.0.0.1\r\n\r\n"),
        APE_DATA_STATIC);
    FLUSH_TCP(m_SocketClient->s.fd);

    m_Handshaked = true;

    Args args;
    args[0].set(this);

    ape_timer_t *timer
        = APE_timer_create(m_SocketClient->ape, WEBSOCKET_PING_INTERVAL,
                           WebSocketClientConnection::PingTimer, this);

    m_PingTimer = APE_timer_getid(timer);
    m_HTTPServer->fireEventSync<WebSocketServer>(
        WebSocketServer::kEvents_ServerConnect, args);
}

void WebSocketClientConnection::onContent(const char *data, size_t len)
{
    m_LastAcitivty = Core::Utils::GetTick(true);

    ape_ws_process_frame(&m_WSState, data, len);
}

void WebSocketClientConnection::onFrame(const char *data,
                                        size_t len,
                                        bool binary)
{
    Args args;
    args[0].set(this);
    args[1].set((void *)(data)); // TODO: new style cast
    args[2].set(len);
    args[3].set(binary);

    m_HTTPServer->fireEventSync<WebSocketServer>(
        WebSocketServer::kEvents_ServerFrame, args);
}

// }}}

} // namespace Net
} // namespace Nidium
