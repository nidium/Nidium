/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativewebsocketclient_h__
#define nativewebsocketclient_h__

#include <ape_websocket.h>

#include "Core/Events.h"
#include "HTTPParser.h"

class NativeWebSocketClient : public Nidium::Core::Events, public Nidium::Net::HTTPParser
{
public:
    static const uint8_t EventID = 5;

    enum Events {
        CLIENT_CONNECT,
        CLIENT_FRAME,
        CLIENT_CLOSE
    };

    NativeWebSocketClient(uint16_t port, const char *url,
        const char *ip);
    bool connect(bool ssl, ape_global *ape);
    void write(uint8_t *data, size_t len, bool binary = false);
    void close();
    void ping();

    ~NativeWebSocketClient();

    void onConnected();
    void onDataHandshake(const uint8_t *data, size_t len);
    void onDataWS(const uint8_t *data, size_t len);
    void onFrame(const char *data, size_t len, bool binary);
    void onClose();

    void printInfo() {
        printf("(Socket : %d)\n", m_Socket->s.fd);
    }

    virtual void HTTPHeaderEnded();
    virtual void HTTPRequestEnded();
    virtual void HTTPOnData(size_t offset, size_t len);

private:
    char m_HandShakeKey[32];
    char *m_ComputedKey;
    ape_socket *m_Socket;
    char *m_Host;
    char *m_URL;
    uint16_t m_Port;
    bool m_SSL;

    websocket_state m_WSState;
};

#endif

