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

#ifndef nativewebsocket_h__
#define nativewebsocket_h__

#include <ape_websocket.h>

#include "NativeHTTPListener.h"

#define NATIVEWEBSOCKET_PING_INTERVAL 5000 /* ms */

class NativeWebSocketListener : public NativeHTTPListener
{
public:
    static const uint8_t EventID = 4;

    enum Events {
        SERVER_CONNECT = 1,
        SERVER_FRAME,
        SERVER_CLOSE
    };

    NativeWebSocketListener(uint16_t port, const char *ip = "0.0.0.0");
    virtual void onClientConnect(ape_socket *client, ape_global *ape);

    virtual bool onEnd(NativeHTTPClientConnection *client) override {
        return false;
    };
};

class NativeWebSocketClientConnection : public NativeHTTPClientConnection
{
public:
    NativeWebSocketClientConnection(NativeHTTPListener *httpserver,
        ape_socket *socket);
    ~NativeWebSocketClientConnection();

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

    static int pingTimer(void *arg);
private:
    websocket_state m_WSState;
    bool m_Handshaked;
    uint64_t m_PingTimer;

    void *m_Data;
};

#endif

