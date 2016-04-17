/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jshttpserver_h__
#define binding_jshttpserver_h__

#include "JSExposer.h"
#include "Net/NativeHTTPListener.h"

namespace Nidium {
namespace Binding {

class JSHTTPClientConnection;

class JSHTTPResponse : public NativeHTTPResponse,
                             public Nidium::Binding::JSObjectMapper<JSHTTPResponse>
{
public:
    friend JSHTTPClientConnection;
protected:
    explicit JSHTTPResponse(JSContext *cx, uint16_t code = 200);
};

class JSHTTPClientConnection : public NativeHTTPClientConnection,
                                     public Nidium::Binding::JSObjectMapper<JSHTTPClientConnection>
{
public:
    JSHTTPClientConnection(JSContext *cx, NativeHTTPListener *httpserver,
        ape_socket *socket) :
        NativeHTTPClientConnection(httpserver, socket),
        JSObjectMapper(cx, "HTTPClientConnection") {}
    virtual NativeHTTPResponse *onCreateResponse() {
        return new JSHTTPResponse(m_JSCx);
    }
};


class JSHTTPServer :    public Nidium::Binding::JSExposer<JSHTTPServer>,
                                public NativeHTTPListener
{
public:
    JSHTTPServer(JS::HandleObject obj, JSContext *cx,
        uint16_t port, const char *ip = "0.0.0.0");
    virtual ~JSHTTPServer();
    virtual void onClientConnect(ape_socket *client, ape_global *ape) {
        JSHTTPClientConnection *conn;
        client->ctx = conn = new JSHTTPClientConnection(m_Cx, this, client);

        JS::RootedObject obj(m_Cx, conn->getJSObject());

        NIDIUM_JSOBJ_SET_PROP_CSTR(obj, "ip", APE_socket_ipv4(client));

        NativeHTTPListener::onClientConnect((NativeHTTPClientConnection *)client->ctx);
    }
    virtual void onClientDisconnect(NativeHTTPClientConnection *client);
    virtual void onData(NativeHTTPClientConnection *client, const char *buf, size_t len);
    virtual bool onEnd(NativeHTTPClientConnection *client);

    static void registerObject(JSContext *cx);
private:
};

} // namespace Binding
} // namespace Nidium

#endif

