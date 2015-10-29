#include "NativeWebSocketClient.h"
#include <ape_base64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    NativeHTTPParser(), m_Port(port), m_SSL(false), m_Socket(NULL)
{
    m_Host = strdup(host);
    m_URL  = strdup(url);

    uint64_t r64 = NativeUtils::rand64();
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
    ape_ws_init(&m_WSState);
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
    ret = APE_socket_write(m_Socket, (unsigned char *)CONST_STR_LEN("Connection: Upgrade\r\nPragma: no-cache\r\nUpgrade: websocket\r\nOrigin: file://\r\nSec-WebSocket-Version: 13\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/46.0.2490.71 Safari/537.36\r\nSec-WebSocket-Key: "), APE_DATA_STATIC);
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
    printf("GOT A WS FRAME \\o/ : %s\n", data);
}

void NativeWebSocketClient::onClose()
{
    m_Socket = NULL;
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
    char data[32];
    sprintf(data, "%s", "hello");

    uint32_t key = 123456;
    ape_ws_write(m_Socket, (unsigned char *)data, 5, 0, APE_DATA_STATIC, &key);

    printf("Request ended\n");
}

void NativeWebSocketClient::HTTPOnData(size_t offset, size_t len)
{

}