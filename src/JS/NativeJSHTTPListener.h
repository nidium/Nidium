/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejshttplistener_h__
#define nativejshttplistener_h__

#include "Net/NativeHTTPListener.h"
#include "NativeJSExposer.h"

class NativeJSHTTPClientConnection;

class NativeJSHTTPResponse : public NativeHTTPResponse,
                             public NativeJSObjectMapper<NativeJSHTTPResponse>
{
public:
    friend NativeJSHTTPClientConnection;
protected:
    explicit NativeJSHTTPResponse(JSContext *cx, uint16_t code = 200);
};

class NativeJSHTTPClientConnection : public NativeHTTPClientConnection,
                                     public NativeJSObjectMapper<NativeJSHTTPClientConnection>
{
public:
    NativeJSHTTPClientConnection(JSContext *cx, NativeHTTPListener *httpserver,
        ape_socket *socket) :
        NativeHTTPClientConnection(httpserver, socket),
        NativeJSObjectMapper(cx, "HTTPClientConnection") {}
    virtual NativeHTTPResponse *onCreateResponse() {
        return new NativeJSHTTPResponse(m_JSCx);
    }
};

class NativeJSHTTPListener :    public NativeJSExposer<NativeJSHTTPListener>,
                                public NativeHTTPListener
{
public:
    NativeJSHTTPListener(JS::HandleObject obj, JSContext *cx,
        uint16_t port, const char *ip = "0.0.0.0");
    virtual ~NativeJSHTTPListener();
    virtual void onClientConnect(ape_socket *client, ape_global *ape) {
        NativeJSHTTPClientConnection *conn;
        client->ctx = conn = new NativeJSHTTPClientConnection(m_Cx, this, client);

        JS::RootedObject obj(m_Cx, conn->getJSObject());

        JSOBJ_SET_PROP_CSTR(obj, "ip", APE_socket_ipv4(client));

        NativeHTTPListener::onClientConnect((NativeHTTPClientConnection *)client->ctx);
    }
    virtual void onClientDisconnect(NativeHTTPClientConnection *client);
    virtual void onData(NativeHTTPClientConnection *client, const char *buf, size_t len);
    virtual bool onEnd(NativeHTTPClientConnection *client);

    static void registerObject(JSContext *cx);
private:
};

#endif

