/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jshttpserver_h__
#define binding_jshttpserver_h__

#include "JSExposer.h"
#include "Net/HTTPServer.h"

namespace Nidium {
namespace Binding {

class JSHTTPClientConnection;

class JSHTTPResponse : public Nidium::Net::HTTPResponse,
                             public Nidium::Binding::JSObjectMapper<JSHTTPResponse>
{
public:
    friend JSHTTPClientConnection;
protected:
    explicit JSHTTPResponse(JSContext *cx, uint16_t code = 200);
};

class JSHTTPClientConnection : public Nidium::Net::HTTPClientConnection,
                                     public Nidium::Binding::JSObjectMapper<JSHTTPClientConnection>
{
public:
    JSHTTPClientConnection(JSContext *cx, Nidium::Net::HTTPServer *httpserver,
        ape_socket *socket) :
        Nidium::Net::HTTPClientConnection(httpserver, socket),
        JSObjectMapper(cx, "HTTPClientConnection") {}
    virtual Nidium::Net::HTTPResponse *onCreateResponse() {
        return new JSHTTPResponse(m_JSCx);
    }
};


class JSHTTPServer :    public Nidium::Binding::JSExposer<JSHTTPServer>,
                        public Nidium::Net::HTTPServer
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

        HTTPServer::onClientConnect((Nidium::Net::HTTPClientConnection *)client->ctx);
    }
    virtual void onClientDisconnect(Nidium::Net::HTTPClientConnection *client);
    virtual void onData(Nidium::Net::HTTPClientConnection *client, const char *buf, size_t len);
    virtual bool onEnd(Nidium::Net::HTTPClientConnection *client);

    static void registerObject(JSContext *cx);
private:
};

} // namespace Binding
} // namespace Nidium

#endif

