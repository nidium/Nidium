/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jshttpserver_h__
#define binding_jshttpserver_h__

#include "Net/HTTPServer.h"
#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

class JSHTTPClientConnection;

// {{{ JSHTTPResponse
class JSHTTPResponse : public Nidium::Net::HTTPResponse,
                       public ClassMapper<JSHTTPResponse>
{
public:
    friend JSHTTPClientConnection;

    static JSFunctionSpec *ListMethods();
    virtual ~JSHTTPResponse(){}

protected:
    explicit JSHTTPResponse(uint16_t code = 200);

    NIDIUM_DECL_JSCALL(write);
    NIDIUM_DECL_JSCALL(writeHead);
    NIDIUM_DECL_JSCALL(end);
};
// }}}

// {{{ JSHTTPClientConnection
class JSHTTPClientConnection : public Nidium::Net::HTTPClientConnection,
                               public ClassMapper<JSHTTPClientConnection>
{
public:
    JSHTTPClientConnection(Nidium::Net::HTTPServer *httpserver,
                           ape_socket *socket)
        : Nidium::Net::HTTPClientConnection(httpserver, socket)
    {
    }
    virtual Nidium::Net::HTTPResponse *onCreateResponse()
    {
        JSHTTPResponse *resp = new JSHTTPResponse();
        JSHTTPResponse::CreateObject(m_Cx, resp);

        /*
            HTTPResponse is delete by ~HTTPClientConnection()
        */
        resp->root();

        return resp;
    }
};
// }}}

// {{{ JSHTTPServer
class JSHTTPServer : public ClassMapper<JSHTTPServer>,
                     public Nidium::Net::HTTPServer
{
public:
    static JSHTTPServer *Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj);

    JSHTTPServer(uint16_t port,
                 const char *ip = "0.0.0.0");
    virtual ~JSHTTPServer();
    virtual void onClientConnect(ape_socket *client, ape_global *ape);
    virtual void onClientDisconnect(Nidium::Net::HTTPClientConnection *client);
    virtual void onData(Nidium::Net::HTTPClientConnection *client,
                        const char *buf,
                        size_t len);
    virtual bool onEnd(Nidium::Net::HTTPClientConnection *client);

    static void RegisterObject(JSContext *cx);
};
// }}}

} // namespace Binding
} // namespace Nidium

#endif
