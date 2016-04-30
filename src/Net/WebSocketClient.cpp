/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "WebSocketClient.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ape_base64.h>

using Nidium::Core::Args;

namespace Nidium {
namespace Net {

// {{{ Callbacks
static void nidium_ws_connected(ape_socket *s,
    ape_global *ape, void *arg)
{
    static_cast<WebSocketClient *>(arg)->onConnected();
}

static void nidium_ws_read_handshake(ape_socket *s,
    const uint8_t *data, size_t len, ape_global *ape, void *arg)
{
    static_cast<WebSocketClient *>(arg)->onDataHandshake(data, len);
}

static void nidium_ws_read_ws(ape_socket *s,
    const uint8_t *data, size_t len, ape_global *ape, void *arg)
{
    static_cast<WebSocketClient *>(arg)->onDataWS(data, len);
}

static void nidium_ws_disconnect(ape_socket *s,
    ape_global *ape, void *arg)
{
    static_cast<WebSocketClient *>(arg)->onClose();
}

static void nidium_on_ws_client_frame(websocket_state *state,
    const unsigned char *data, ssize_t length, int binary)
{
    ape_socket *sock = state->socket;
    if (sock == NULL) {
        return;
    }

    WebSocketClient *con = static_cast<WebSocketClient *>(sock->ctx);

    if (con == NULL) {
        return;
    }

    con->onFrame(reinterpret_cast<const char *>(data), length, (bool)binary);
}
// }}}

// {{{ WebSocketClient
WebSocketClient::WebSocketClient(uint16_t port, const char *url,
    const char *host) :
    HTTPParser(), m_Socket(NULL), m_Port(port), m_SSL(false)
{
    m_Host = strdup(host);
    m_URL  = strdup(url);

    uint64_t r64 = Core::Utils::RandInt<uint64_t>();
    base64_encode_b_safe(reinterpret_cast<unsigned char *>(&r64), m_HandShakeKey, sizeof(uint64_t), 0);

    m_ComputedKey = ape_ws_compute_key(m_HandShakeKey, strlen(m_HandShakeKey));
}

bool WebSocketClient::connect(bool ssl, ape_global *ape)
{
    if (m_Socket) {
        return false;
    }

    m_Socket = APE_socket_new(ssl ? APE_SOCKET_PT_SSL : APE_SOCKET_PT_TCP, 0, ape);

    if (m_Socket == NULL) {
        return false;
    }

    m_SSL = ssl;
    if (APE_socket_connect(m_Socket, m_Port, m_Host, 0) == -1) {
        return false;
    }

    m_Socket->callbacks.on_connected  = nidium_ws_connected;
    m_Socket->callbacks.on_read       = nidium_ws_read_handshake;
    m_Socket->callbacks.on_disconnect = nidium_ws_disconnect;
    m_Socket->callbacks.arg = this;

    m_Socket->ctx = this;

    return true;
}

void WebSocketClient::HTTPHeaderEnded()
{
    const char *swa = this->HTTPGetHeader("Sec-WebSocket-Accept");

    /* Check handshake key integrity */
    if (swa == NULL || strcmp(swa, m_ComputedKey) != 0) {
        APE_socket_shutdown_now(m_Socket);
        return;
    }
}

void WebSocketClient::HTTPRequestEnded()
{
    m_Socket->callbacks.on_read = nidium_ws_read_ws;

    Args args;
    args[0].set(this);

    this->fireEvent<WebSocketClient>(WebSocketClient::CLIENT_CONNECT, args);
}

void WebSocketClient::write(uint8_t *data, size_t len, bool binary)
{
    if (!m_Socket) return;

    ape_ws_write(&m_WSState, data, len, binary, APE_DATA_COPY);
}

void WebSocketClient::close()
{
    if (!m_Socket) return;

    ape_ws_close(&m_WSState);
}

void WebSocketClient::ping()
{
    if (!m_Socket) return;

    ape_ws_ping(&m_WSState);
}

void WebSocketClient::HTTPOnData(size_t offset, size_t len)
{

}


WebSocketClient::~WebSocketClient()
{
    free(m_Host);
    free(m_URL);
    free(m_ComputedKey);
    if (m_Socket) {
        //ape_ws_close(websocket_state *state);
        APE_socket_remove_callbacks(m_Socket);
        APE_socket_shutdown_now(m_Socket);
    }
}
// }}}

// {{{ WebSocketClient events
void WebSocketClient::onConnected()
{
    ape_ws_init(&m_WSState, 1);
    m_WSState.socket = m_Socket;
    m_WSState.on_frame = nidium_on_ws_client_frame;

    /*
        Write http header
    */

    PACK_TCP(m_Socket->s.fd);

    //TODO: new style cast
    APE_socket_write(m_Socket, (unsigned char *)CONST_STR_LEN("GET "), APE_DATA_STATIC);
    APE_socket_write(m_Socket, m_URL, strlen(m_URL), APE_DATA_OWN);
    APE_socket_write(m_Socket, (unsigned char *)CONST_STR_LEN(" HTTP/1.1\r\nHost: "), APE_DATA_STATIC);
    APE_socket_write(m_Socket, m_Host, strlen(m_Host), APE_DATA_OWN);

    if (m_Port != 80 && m_Port != 443) {
        char portstr[8];
        int ret = sprintf(portstr, ":%hu", m_Port);

        APE_socket_write(m_Socket, portstr, ret, APE_DATA_STATIC);

    }

    // TODO: new style cast
    APE_socket_write(m_Socket, (unsigned char *)CONST_STR_LEN("\r\n"), APE_DATA_STATIC);
    APE_socket_write(m_Socket, (unsigned char *)CONST_STR_LEN(
        "Connection: Upgrade\r\n"
        "Pragma: no-cache\r\n"
        "Upgrade: websocket\r\n"
        "Origin: file://\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "User-Agent: Mozilla/5.0 (Unknown arch) nidium/0.1 (nidium, like Gecko) nidium/0.1\r\n"
        "Sec-WebSocket-Key: "), APE_DATA_STATIC);

    /*
        Send the handshake key
    */
    APE_socket_write(m_Socket, m_HandShakeKey, strlen(m_HandShakeKey), APE_DATA_STATIC);
    //TODO: new style cast
    APE_socket_write(m_Socket, (unsigned char *)CONST_STR_LEN("\r\n\r\n"), APE_DATA_STATIC);

    FLUSH_TCP(m_Socket->s.fd);
}

void WebSocketClient::onDataHandshake(const uint8_t *data, size_t len)
{
    //TODO: new style cast
    this->HTTPParse((char *)(data), len);
}

void WebSocketClient::onDataWS(const uint8_t *data, size_t len)
{
    //TODO: new style cast
    ape_ws_process_frame(&m_WSState, (char *)(data), len);
}

void WebSocketClient::onFrame(const char *data, size_t len, bool binary)
{
    Args args;
    args[0].set(this);
    args[1].set((void *)(data)); // TODO: new style cast
    args[2].set(len);
    args[3].set(binary);

    this->fireEvent<WebSocketClient>(WebSocketClient::CLIENT_FRAME, args);
}

void WebSocketClient::onClose()
{
    m_Socket = NULL;

    Args args;
    args[0].set(this);

    this->fireEvent<WebSocketClient>(WebSocketClient::CLIENT_CLOSE, args);
}
// }}}

} // namespace Net
} // namespace Nidium

