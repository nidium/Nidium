/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jswebsocket_h__
#define binding_jswebsocket_h__

#include "Core/Messages.h"
#include "Net/WebSocket.h"
#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

class JSWebSocketClientConnection;

class JSWebSocketServer : public ClassMapper<JSWebSocketServer>,
                          public Nidium::Core::Messages
{
public:
    JSWebSocketServer(const char *host,
                      unsigned short port);
    virtual ~JSWebSocketServer();
    bool start();
    static void RegisterObject(JSContext *cx);
    void onMessage(const Core::SharedMessages::Message &msg);
    static JSWebSocketServer *Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj);
protected:
private:
    Net::WebSocketServer *m_WebSocketServer;

    JSWebSocketClientConnection *
        createClient(Net::WebSocketClientConnection *client);
};

class JSWebSocketClientConnection:
    public ClassMapper<JSWebSocketClientConnection>
{
public:
    JSWebSocketClientConnection(Net::WebSocketClientConnection *client) :
         m_WebSocketClientConnection(client) {}

    virtual ~JSWebSocketClientConnection(){};
    static JSFunctionSpec *ListMethods();
protected:
    NIDIUM_DECL_JSCALL(send);
    NIDIUM_DECL_JSCALL(close);
private:
    Net::WebSocketClientConnection *m_WebSocketClientConnection;
};

} // namespace Binding
} // namespace Nidium

#endif
