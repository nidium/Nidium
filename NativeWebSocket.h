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

#include "NativeHTTPListener.h"
#include <ape_websocket.h>

class NativeWebSocketListener : public NativeHTTPListener
{
public:
    NativeWebSocketListener(uint16_t port, const char *ip = "0.0.0.0");
    virtual void onClientConnect(ape_socket *client, ape_global *ape);  
};

class NativeWebSocketClientConnection : public NativeHTTPClientConnection
{
public:
    NativeWebSocketClientConnection(NativeHTTPListener *httpserver,
        ape_socket *socket);
    ~NativeWebSocketClientConnection();

    virtual void onFrame(const char *data, size_t len);

    /* TODO: support "buffering" detection + ondrain()
        (need ape_websocket.c modification)
    */
    void write(const char *data, size_t len,
        ape_socket_data_autorelease type = APE_DATA_COPY);

    virtual void onHeaderEnded();
    virtual void onDisconnect(ape_global *ape);
    virtual void onUpgrade(const char *to);
    virtual void onContent(const char *data, size_t len);

    virtual void close();
private:
    websocket_state m_WSState;
    bool m_Handshaked;
};

#endif
