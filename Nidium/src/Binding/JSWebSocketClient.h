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
#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

class JSWebSocket : public ClassMapper<JSWebSocket>, public Nidium::Core::Messages
{
public:
    JSWebSocket(JSContext *cx, const char *host,
                unsigned short port,
                const char *path,
                bool ssl = false);
    virtual ~JSWebSocket();
    bool start();
    static void RegisterObject(JSContext *cx);
    void onMessage(const Nidium::Core::SharedMessages::Message &msg);

    Nidium::Net::WebSocketClient *ws() const
    {
        return m_WebSocketClient;
    }
    static JSWebSocket *Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj);

    static JSFunctionSpec *ListMethods();
protected:
    NIDIUM_DECL_JSCALL(send);
    NIDIUM_DECL_JSCALL(close);
    NIDIUM_DECL_JSCALL(ping);
private:
    Nidium::Net::WebSocketClient *m_WebSocketClient;
};

} // namespace Binding
} // namespace Nidium

#endif
