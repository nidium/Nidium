/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jswebsocket_h__
#define binding_jswebsocket_h__

#include "Core/Messages.h"
#include "Net/WebSocket.h"
#include "Binding/JSExposer.h"

namespace Nidium {
namespace Binding {

class JSWebSocketServer : public JSExposer<JSWebSocketServer>,
                                public Nidium::Core::Messages
{
public:
    JSWebSocketServer(JS::HandleObject obj, JSContext *cx,
        const char *host, unsigned short port);
    ~JSWebSocketServer();
    bool start();
    static void RegisterObject(JSContext *cx);
    void onMessage(const Nidium::Core::SharedMessages::Message &msg);
private:
    Nidium::Net::WebSocketServer *m_WebSocketServer;

    JSObject *createClient(Nidium::Net::WebSocketClientConnection *client);
};

} // namespace Binding
} // namespace Nidium

#endif

