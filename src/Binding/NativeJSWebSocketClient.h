/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejswebsocketclient_h__
#define nativejswebsocketclient_h__

#include <cstddef>

#include "Core/NativeMessages.h"
#include "Net/NativeWebSocketClient.h"
#include "JSExposer.h"

class NativeJSWebSocket : public Nidium::Binding::JSExposer<NativeJSWebSocket>,
                                public NativeMessages
{
public:
    NativeJSWebSocket(JS::HandleObject obj, JSContext *cx,
        const char *host, unsigned short port, const char *path, bool ssl = false);
    ~NativeJSWebSocket();
    bool start();
    static void registerObject(JSContext *cx);
    void onMessage(const NativeSharedMessages::Message &msg);

    NativeWebSocketClient *ws() const {
        return m_WebSocketClient;
    }

private:
    NativeWebSocketClient *m_WebSocketClient;
};

#endif

