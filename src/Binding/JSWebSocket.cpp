/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSWebSocket.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifndef _MSC_VER
#include <strings.h>
#include <unistd.h>
#endif

#include "Net/HTTP.h"
#include "Binding/JSUtils.h"

using Nidium::Net::WebSocketServer;
using Nidium::Net::WebSocketClientConnection;

namespace Nidium {
namespace Binding {

// {{{ Preamble
#define SET_PROP(where, name, val)                                    \
    JS_DefineProperty(cx, where, (const char *)name, val, NULL, NULL, \
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)


// }}}

// {{{ JSWebSocketServer
JSWebSocketServer::JSWebSocketServer(const char *host,
                                     unsigned short port)
{
    m_WebSocketServer = new WebSocketServer(port, host);
    m_WebSocketServer->addListener(this);
}

JSWebSocketClientConnection *
    JSWebSocketServer::createClient(WebSocketClientConnection *client)
{

    JSWebSocketClientConnection *wscc = new JSWebSocketClientConnection(client);
    JSWebSocketClientConnection::CreateObject(m_Cx, wscc);

    wscc->root();

    client->setData(wscc);

    return wscc;
}

void JSWebSocketServer::onMessage(const Core::SharedMessages::Message &msg)
{
    JSContext *cx = m_Cx;

    JS::RootedValue oncallback(cx);
    JS::RootedValue val(cx);

    switch (msg.event()) {
        case NIDIUM_EVENT(WebSocketServer,
                          WebSocketServer::kEvents_ServerFrame): {
            JS::AutoValueArray<2> arg(cx);

            const char *data = static_cast<const char *>(msg.m_Args[2].toPtr());
            int len          = msg.m_Args[3].toInt();
            bool binary      = msg.m_Args[4].toBool();

            WebSocketClientConnection *client
                = static_cast<WebSocketClientConnection *>(
                    msg.m_Args[1].toPtr());

            JSWebSocketClientConnection *wscc =
                (JSWebSocketClientConnection *)client->getData();

            if (!wscc) {
                return;
            }
            JS::RootedObject obj(m_Cx, this->getJSObject());
            if (JS_GetProperty(m_Cx, obj, "onmessage", &oncallback)
                && JS_TypeOfValue(m_Cx, oncallback) == JSTYPE_FUNCTION) {

                JS::RootedValue jdata(cx);
                JS::RootedObject event(m_Cx, JS_NewPlainObject(m_Cx));

                JSUtils::StrToJsval(m_Cx, data, len, &jdata,
                                    !binary ? "utf8" : NULL);
                NIDIUM_JSOBJ_SET_PROP(event, "data", jdata);

                arg[0].setObjectOrNull(wscc->getJSObject());
                arg[1].setObjectOrNull(event);

                JS_CallFunctionValue(m_Cx, obj, oncallback, arg, &val);
            }

            break;
        }
        case NIDIUM_EVENT(WebSocketServer,
                          WebSocketServer::kEvents_ServerConnect): {
            JS::AutoValueArray<1> arg(cx);

            JSWebSocketClientConnection *wscc
                = this->createClient(static_cast<WebSocketClientConnection *>(
                    msg.m_Args[1].toPtr()));

            arg[0].setObjectOrNull(wscc->getJSObject());

            JS::RootedObject obj(cx, this->getJSObject());

            JSOBJ_CALLFUNCNAME(obj, "onopen", arg);

            break;
        }
        case NIDIUM_EVENT(WebSocketServer,
                          WebSocketServer::kEvents_ServerClose): {
            WebSocketClientConnection *client
                = static_cast<WebSocketClientConnection *>(
                    msg.m_Args[1].toPtr());

            JSWebSocketClientConnection *wscc =
                (JSWebSocketClientConnection *)client->getData();

            JS::AutoValueArray<1> arg(cx);
            JS::RootedObject jclient(cx, wscc->getJSObject());
            JS::RootedObject obj(cx, this->getJSObject());

            arg[0].setObject(*jclient);
            JSOBJ_CALLFUNCNAME(obj, "onclose", arg);

            wscc->unroot();

            break;
        }
        default:
            break;
    }
}

bool JSWebSocketServer::start()
{
    if (!m_WebSocketServer) return false;

    return m_WebSocketServer->start();
}

JSWebSocketServer::~JSWebSocketServer()
{
    delete m_WebSocketServer;
}

// }}}

// {{{ WebSocketServerClient implementation
bool JSWebSocketClientConnection::JS_send(JSContext *cx, JS::CallArgs &args)
{

    if (args[0].isString()) {
        JSAutoByteString cdata;
        JS::RootedString str(cx, args[0].toString());
        cdata.encodeUtf8(cx, str);

        m_WebSocketClientConnection->write(
                    reinterpret_cast<unsigned char *>(cdata.ptr()),
                    strlen(cdata.ptr()), false, APE_DATA_COPY);

        args.rval().setInt32(0);

    } else if (args[0].isObject()) {
        JSObject *objdata = args[0].toObjectOrNull();

        if (!objdata || !JS_IsArrayBufferObject(objdata)) {
            JS_ReportError(cx,
                           "write() invalid data (must be either a string or "
                           "an ArrayBuffer)");
            return false;
        }
        uint32_t len  = JS_GetArrayBufferByteLength(objdata);

        JS::AutoCheckCannotGC nogc;
        bool shared;

        uint8_t *data = JS_GetArrayBufferData(objdata, &shared, nogc);

        m_WebSocketClientConnection->write(static_cast<unsigned char *>(data),
                        len, true,
                        APE_DATA_COPY);

        args.rval().setInt32(0);

    } else {
        JS_ReportError(
            cx,
            "write() invalid data (must be either a string or an ArrayBuffer)");
        return false;
    }

    return true;
}

bool JSWebSocketClientConnection::JS_close(JSContext *cx, JS::CallArgs &args)
{
    m_WebSocketClientConnection->close();

    return true;
}


// }}}

// {{{ WebSocketServer implementation
JSWebSocketServer *JSWebSocketServer::Constructor(JSContext *cx,
    JS::CallArgs &args,
    JS::HandleObject obj)
{
    JS::RootedString url(cx);
    JS::RootedString protocol(cx);

    if (!JS_ConvertArguments(cx, args, "S/S", url.address(),
                             protocol.address())) {
        return nullptr;
    }

    JSAutoByteString curl(cx, url);

    char *durl = strdup(curl.ptr());
    char *host = static_cast<char *>(calloc(curl.length(), sizeof(char)));
    char *path = static_cast<char *>(calloc(curl.length(), sizeof(char)));

    u_short port         = 80;
    u_short default_port = 80;
    bool isSSL           = false;

    const char *prefix = NULL;
    if (strncasecmp(curl.ptr(), CONST_STR_LEN("wss://")) == 0) {
        prefix       = "wss://";
        isSSL        = true;
        default_port = 443;
    } else if (strncasecmp(curl.ptr(), CONST_STR_LEN("ws://")) == 0) {
        prefix = "ws://";
    } else {
        /* No prefix provided. Assuming 'default url' => no SSL, port 80 */
        prefix = "";
    }

    if (Net::HTTP::ParseURI(durl, curl.length(), host, &port, path, prefix,
                            default_port)
        == -1) {
        JS_ReportError(cx, "Invalid WebSocketServer URI : %s", durl);
        free(path);
        free(host);
        free(durl);
        return nullptr;
    }

    JSWebSocketServer *wss = new JSWebSocketServer(host, port);

    free(path);
    free(host);
    free(durl);

    if (!wss->start()) {
        JS_ReportError(cx, "WebSocketServer: failed to bind on %s", curl.ptr());
        delete wss;
        return nullptr;
    }

    /* Workaround so we can call root() within ::Constructor */
    wss->m_Instance = obj;
    /*
        Server is listening at this point. Don't collect.
    */
    wss->root();

    return wss;
}

JSFunctionSpec * JSWebSocketClientConnection::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSWebSocketClientConnection, send, 1),
        CLASSMAPPER_FN(JSWebSocketClientConnection, close, 0),
        JS_FS_END
    };

    return funcs;
}

void JSWebSocketServer::RegisterObject(JSContext *cx)
{
    JSWebSocketServer::ExposeClass<1>(cx, "WebSocketServer");
    JSWebSocketClientConnection::ExposeClass(cx, "WebSocketClientConnection");
}

// }}}
} // namespace Binding
} // namespace Nidium
