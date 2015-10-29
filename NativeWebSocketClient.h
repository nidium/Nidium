/*
    NativeJS Core Library
    Copyright (C) 2015 Anthony Catel <paraboul@gmail.com>

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

#ifndef nativewebsocketclient_h__
#define nativewebsocketclient_h__

#include <ape_websocket.h>
#include "NativeEvents.h"
#include "NativeHTTPParser.h"

class NativeWebSocketClient : public NativeEvents, public NativeHTTPParser
{
public:
    static const uint8_t EventID = 5;

    enum Events {
        CLIENT_CONNECT,
        CLIENT_FRAME,
        CLIENT_CLOSE
    };

    NativeWebSocketClient(uint16_t port, const char *url,
        const char *ip = "0.0.0.0");
    bool connect(bool ssl, ape_global *ape);
    void write(uint8_t *data, size_t len, bool binary = false);

    ~NativeWebSocketClient();

    void onConnected();
    void onDataHandshake(const uint8_t *data, size_t len);
    void onDataWS(const uint8_t *data, size_t len);
    void onFrame(const char *data, size_t len, bool binary);
    void onClose();

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
