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
#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

class JSWebSocketServer : public ClassMapper<JSWebSocketServer>,
                          public Nidium::Core::Messages
{
public:
    JSWebSocketServer(const char *host,
                      unsigned short port);
    ~JSWebSocketServer();
    bool start();
    static void RegisterObject(JSContext *cx);
    void onMessage(const Nidium::Core::SharedMessages::Message &msg);
    static JSWebSocketServer *Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj);
    static JSFunctionSpec *ListMethods();
protected:
private:
    Nidium::Net::WebSocketServer *m_WebSocketServer;

    JSObject *createClient(Nidium::Net::WebSocketClientConnection *client);
};

} // namespace Binding
} // namespace Nidium

#endif
