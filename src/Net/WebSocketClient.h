/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef net_websocketclient_h__
#define net_websocketclient_h__

#include <ape_websocket.h>

#include "Core/Events.h"
#include "Net/HTTPParser.h"

namespace Nidium {
namespace Net {

class WebSocketClient : public Nidium::Core::Events, public HTTPParser
{
public:
    static const uint8_t EventID = 5;

    enum Events
    {
        kEvents_ClientConnect,
        kEvents_ClientFrame,
        kEvents_ClientClose
    };

    WebSocketClient(uint16_t port, const char *url, const char *ip);
    bool connect(bool ssl, ape_global *ape);
    void write(uint8_t *data, size_t len, bool binary = false);
    void close();
    void ping();

    ~WebSocketClient();

    void onConnected();
    void onDataHandshake(const uint8_t *data, size_t len);
    void onDataWS(const uint8_t *data, size_t len);
    void onFrame(const char *data, size_t len, bool binary);
    void onClose();

    void printInfo()
    {
        ndm_logf(NDM_LOG_DEBUG, "WebSocketClient", "Socket : %d", m_Socket->s.fd);
    }

    virtual void HTTPHeaderEnded();
    virtual void HTTPRequestEnded();
    virtual void HTTPOnData(const char *data, size_t len);

private:
    char m_HandShakeKey[32];
    char *m_ComputedKey;
    ape_socket *m_Socket;
    char *m_Host;
    char *m_URL;
    uint16_t m_Port;
    bool m_SSL;
    bool m_HandShakeDone = false;

    websocket_state m_WSState;
};

} // namespace Net
} // namespace Nidium

#endif
