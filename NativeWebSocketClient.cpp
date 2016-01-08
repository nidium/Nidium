#include "NativeWebSocketClient.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ape_base64.h>

static void native_ws_connected(ape_socket *s,
    ape_global *ape, void *arg)
{
    ((NativeWebSocketClient *)arg)->onConnected();
}

static void native_ws_read_handshake(ape_socket *s,
    const uint8_t *data, size_t len, ape_global *ape, void *arg)
{
    ((NativeWebSocketClient *)arg)->onDataHandshake(data, len);
}

static void native_ws_read_ws(ape_socket *s,
    const uint8_t *data, size_t len, ape_global *ape, void *arg)
{
    ((NativeWebSocketClient *)arg)->onDataWS(data, len);
}

static void native_ws_disconnect(ape_socket *s,
    ape_global *ape, void *arg)
{
    ((NativeWebSocketClient *)arg)->onClose();
}

static void native_on_ws_client_frame(websocket_state *state,
    const unsigned char *data, ssize_t length, int binary)
{
    ape_socket *sock = state->socket;
    if (sock == NULL) {
        return;
    }

    NativeWebSocketClient *con =
        (NativeWebSocketClient *)sock->ctx;

    if (con == NULL) {
        return;
    }

    con->onFrame((const char *)data, length, (bool)binary);
}

NativeWebSocketClient::NativeWebSocketClient(uint16_t port, const char *url,
    const char *host) :
    NativeHTTPParser(), m_Socket(NULL), m_Port(port), m_SSL(false)
{
    m_Host = strdup(host);
    m_URL  = strdup(url);

    uint64_t r64 = NativeUtils::randInt<uint64_t>();
    base64_encode_b_safe((unsigned char *)&r64, m_HandShakeKey, sizeof(uint64_t), 0);

    m_ComputedKey = ape_ws_compute_key(m_HandShakeKey, strlen(m_HandShakeKey));
}

NativeWebSocketClient::~NativeWebSocketClient()
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

bool NativeWebSocketClient::connect(bool ssl, ape_global *ape)
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

    m_Socket->callbacks.on_connected  = native_ws_connected;
    m_Socket->callbacks.on_read       = native_ws_read_handshake;
    m_Socket->callbacks.on_disconnect = native_ws_disconnect;
    m_Socket->callbacks.arg = this;

    m_Socket->ctx = this;

    return true;
}

void NativeWebSocketClient::onConnected()
{
    ape_ws_init(&m_WSState, 1);
    m_WSState.socket = m_Socket;
    m_WSState.on_frame = native_on_ws_client_frame;

    /*
        Write http header
    */

    PACK_TCP(m_Socket->s.fd);

    int ret = 0;

    ret = APE_socket_write(m_Socket, (unsigned char *)CONST_STR_LEN("GET "), APE_DATA_STATIC);
    ret = APE_socket_write(m_Socket, m_URL, strlen(m_URL), APE_DATA_OWN);
    ret = APE_socket_write(m_Socket, (unsigned char *)CONST_STR_LEN(" HTTP/1.1\r\nHost: "), APE_DATA_STATIC);

    ret = APE_socket_write(m_Socket, m_Host, strlen(m_Host), APE_DATA_OWN);

    if (m_Port != 80 && m_Port != 443) {
        char portstr[8];
        int ret = sprintf(portstr, ":%hu", m_Port);

        ret = APE_socket_write(m_Socket, portstr, ret, APE_DATA_STATIC);

    }

    ret = APE_socket_write(m_Socket, (unsigned char *)CONST_STR_LEN("\r\n"), APE_DATA_STATIC);
    ret = APE_socket_write(m_Socket, (unsigned char *)CONST_STR_LEN(
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
    ret = APE_socket_write(m_Socket, m_HandShakeKey, strlen(m_HandShakeKey), APE_DATA_STATIC);
    ret = APE_socket_write(m_Socket, (unsigned char *)CONST_STR_LEN("\r\n\r\n"), APE_DATA_STATIC);

    FLUSH_TCP(m_Socket->s.fd);
}

void NativeWebSocketClient::onDataHandshake(const uint8_t *data, size_t len)
{
    this->HTTPParse((char *)data, len);
}

void NativeWebSocketClient::onDataWS(const uint8_t *data, size_t len)
{
    ape_ws_process_frame(&m_WSState, (char *)data, len);
}

void NativeWebSocketClient::onFrame(const char *data, size_t len, bool binary)
{
    NativeArgs args;
    args[0].set(this);
    args[1].set((void *)data);
    args[2].set(len);
    args[3].set(binary);

    this->fireEvent<NativeWebSocketClient>(NativeWebSocketClient::CLIENT_FRAME, args);
}

void NativeWebSocketClient::onClose()
{
    m_Socket = NULL;

    NativeArgs args;
    args[0].set(this);

    this->fireEvent<NativeWebSocketClient>(NativeWebSocketClient::CLIENT_CLOSE, args);
}

void NativeWebSocketClient::HTTPHeaderEnded()
{
    const char *swa = this->HTTPGetHeader("Sec-WebSocket-Accept");

    /* Check handshake key integrity */
    if (swa == NULL || strcmp(swa, m_ComputedKey) != 0) {
        APE_socket_shutdown_now(m_Socket);
        return;
    }
}

void NativeWebSocketClient::HTTPRequestEnded()
{
    m_Socket->callbacks.on_read = native_ws_read_ws;

    NativeArgs args;
    args[0].set(this);

    this->fireEvent<NativeWebSocketClient>(NativeWebSocketClient::CLIENT_CONNECT, args);
}

void NativeWebSocketClient::write(uint8_t *data, size_t len, bool binary)
{
    if (!m_Socket) return;

    uint32_t r32 = NativeUtils::randInt<uint32_t>();
    ape_ws_write(m_Socket, data, len, binary, APE_DATA_COPY, &r32);
}

void NativeWebSocketClient::close()
{
    if (!m_Socket) return;

    ape_ws_close(&m_WSState);
}

void NativeWebSocketClient::ping()
{
    if (!m_Socket) return;

    ape_ws_ping(&m_WSState);
}

void NativeWebSocketClient::HTTPOnData(size_t offset, size_t len)
{

}

