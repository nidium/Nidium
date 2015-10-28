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

static void native_ws_read(ape_socket *s,
    const uint8_t *data, size_t len, ape_global *ape, void *arg)
{
    ((NativeWebSocketClient *)arg)->onData(data, len);
}

static void native_ws_disconnect(ape_socket *s,
    ape_global *ape, void *arg)
{
    printf("Disconnected :'(\n");
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
    m_Port(port), m_SSL(false), m_Socket(NULL)
{
    m_Host = strdup(host);
    m_URL  = strdup(url);

    m_HandShakeKey = NativeUtils::rand64();
}

NativeWebSocketClient::~NativeWebSocketClient()
{
    free(m_Host);
    free(m_URL);
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
        printf("fail 1\n");
        return false;
    }

    m_SSL = ssl;
    if (APE_socket_connect(m_Socket, m_Port, m_Host, 0) == -1) {
        printf("Fail 2\n");
        return false;
    }

    m_Socket->callbacks.on_connected  = native_ws_connected;
    m_Socket->callbacks.on_read       = native_ws_read;
    m_Socket->callbacks.on_disconnect = native_ws_disconnect;
    m_Socket->callbacks.arg = this;
    
    m_Socket->ctx = this;

    return true;
}

void NativeWebSocketClient::onConnected()
{
    printf("COnnected !\n");

    char b64Key[32];
    ape_ws_init(&m_WSState);
    m_WSState.socket = m_Socket;
    m_WSState.on_frame = NULL;

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
    base64_encode_b_safe((unsigned char *)&m_HandShakeKey, b64Key, sizeof(uint64_t), 0);
    ret = APE_socket_write(m_Socket, b64Key, strlen(b64Key), APE_DATA_STATIC);
    ret = APE_socket_write(m_Socket, (unsigned char *)CONST_STR_LEN("\r\n\r\n"), APE_DATA_STATIC);

    FLUSH_TCP(m_Socket->s.fd);
}

void NativeWebSocketClient::onData(const uint8_t *data, size_t len)
{
    printf("Data : %.*s\n", (int)len, data);
    ape_ws_process_frame(&m_WSState, (char *)data, len);
}

void NativeWebSocketClient::onFrame(const char *data, size_t len, bool binary)
{

}

void NativeWebSocketClient::onClose()
{
    m_Socket = NULL;
}
