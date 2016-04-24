/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jswebsocketclient_h__
#define binding_jswebsocketclient_h__

#include <cstddef>

#include "Core/Messages.h"
#include "Net/WebSocketClient.h"
#include "JSExposer.h"

namespace Nidium {
namespace Binding {

class JSWebSocket : public JSExposer<JSWebSocket>, public Nidium::Core::Messages
{
public:
    JSWebSocket(JS::HandleObject obj, JSContext *cx,
        const char *host, unsigned short port, const char *path, bool ssl = false);
    ~JSWebSocket();
    bool start();
    static void RegisterObject(JSContext *cx);
    void onMessage(const Nidium::Core::SharedMessages::Message &msg);

    Nidium::Net::WebSocketClient *ws() const {
        return m_WebSocketClient;
    }

private:
    Nidium::Net::WebSocketClient *m_WebSocketClient;
};

} // namespace Binding
} // namespace Nidium

#endif

