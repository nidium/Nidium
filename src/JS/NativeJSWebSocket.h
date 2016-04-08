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

#ifndef nativejswebsocket_h__
#define nativejswebsocket_h__

#include "NativeWebSocket.h"
#include "NativeMessages.h"
#include "NativeJSExposer.h"

class NativeJSWebSocketServer : public NativeJSExposer<NativeJSWebSocketServer>,
                                public NativeMessages
{
public:
    NativeJSWebSocketServer(JS::HandleObject obj, JSContext *cx,
        const char *host, unsigned short port);
    ~NativeJSWebSocketServer();
    bool start();
    static void registerObject(JSContext *cx);
    void onMessage(const NativeSharedMessages::Message &msg);
private:
    NativeWebSocketListener *m_WebSocketServer;

    JSObject *createClient(NativeWebSocketClientConnection *client);
};

#endif

