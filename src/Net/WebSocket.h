/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef net_websocket_h__
#define net_websocket_h__

#include <ape_websocket.h>

#include "Net/HTTPServer.h"

namespace Nidium {
namespace Net {

#define WEBSOCKET_PING_INTERVAL 5000 /* ms */

class WebSocketServer : public HTTPServer
{
public:
    static const uint8_t EventID = 4;

    enum Events {
        SERVER_CONNECT = 1,
        SERVER_FRAME,
        SERVER_CLOSE
    };

    WebSocketServer(uint16_t port, const char *ip = "0.0.0.0");
    virtual void onClientConnect(ape_socket *client, ape_global *ape) override;

    virtual bool onEnd(HTTPClientConnection *client) override {
        return false;
    };
};

class WebSocketClientConnection : public HTTPClientConnection
{
public:
    WebSocketClientConnection(HTTPServer *httpserver, ape_socket *socket);
    ~WebSocketClientConnection();

    virtual void onFrame(const char *data, size_t len, bool binary);

    /* TODO: support "buffering" detection + ondrain()
        (need ape_websocket.c modification)
    */
    void write(unsigned char *data, size_t len,
        bool binary = false,
        ape_socket_data_autorelease type = APE_DATA_COPY);

    virtual void onHeaderEnded();
    virtual void onDisconnect(ape_global *ape);
    virtual void onUpgrade(const char *to);
    virtual void onContent(const char *data, size_t len);

    virtual void setData(void *data) {
        m_Data = data;
    }
    virtual void *getData() const {
        return m_Data;
    }
    virtual void close();
    void ping();

    static int PingTimer(void *arg);
private:
    websocket_state m_WSState;
    bool m_Handshaked;
    uint64_t m_PingTimer;

    void *m_Data;
};

} // namespace Net
} // namespace Nidium

#endif

