/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSWebSocketClient.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "Net/HTTP.h"
#include "Binding/JSUtils.h"

using Nidium::Net::WebSocketClient;

namespace Nidium {
namespace Binding {

// {{{ JSBinding
JSWebSocket::JSWebSocket(JSContext *cx,
                         const char *host,
                         unsigned short port,
                         const char *path,
                         bool ssl)
{
    m_WebSocketClient = new WebSocketClient(port, path, host);
    bool ret = m_WebSocketClient->connect(
        ssl, static_cast<ape_global *>(JS_GetContextPrivate(cx)));

    if (!ret) {
        JS_ReportWarningUTF8(cx, "Failed to connect to WS endpoint\n");
        return;
    }

    m_WebSocketClient->addListener(this);
}

void JSWebSocket::onMessage(const Core::SharedMessages::Message &msg)
{
    JSContext *cx = m_Cx;

    JS::RootedValue oncallback(cx);
    JS::RootedValue val(cx);

    switch (msg.event()) {
        case NIDIUM_EVENT(WebSocketClient,
                          WebSocketClient::kEvents_ClientFrame): {
            JS::AutoValueArray<1> arg(cx);

            const char *data = static_cast<const char *>(msg.m_Args[2].toPtr());
            int len          = msg.m_Args[3].toInt();
            bool binary      = msg.m_Args[4].toBool();

            JS::RootedObject obj(m_Cx, this->getJSObject());
            if (JS_GetProperty(m_Cx, obj, "onmessage", &oncallback)
                && JS_TypeOfValue(m_Cx, oncallback) == JSTYPE_FUNCTION) {

                JS::RootedValue jdata(cx);
                JS::RootedObject event(m_Cx, JS_NewPlainObject(m_Cx));

                JSUtils::StrToJsval(m_Cx, data, len, &jdata,
                                    !binary ? "utf8" : NULL);

                NIDIUM_JSOBJ_SET_PROP(event, "data", jdata);

                arg[0].setObjectOrNull(event);

                JS_CallFunctionValue(m_Cx, obj, oncallback, arg, &val);
            }

            break;
        }
        case NIDIUM_EVENT(WebSocketClient,
                          WebSocketClient::kEvents_ClientConnect): {
            JS::RootedObject obj(cx, this->getJSObject());

            JSOBJ_CALLFUNCNAME(obj, "onopen", JS::HandleValueArray::empty());

            break;
        }
        case NIDIUM_EVENT(WebSocketClient,
                          WebSocketClient::kEvents_ClientClose): {
            JS::RootedObject obj(cx, this->getJSObject());

            JSOBJ_CALLFUNCNAME(obj, "onclose", JS::HandleValueArray::empty());

            this->unroot();

            break;
        }
        default:
            break;
    }
}

JSWebSocket::~JSWebSocket()
{
    delete m_WebSocketClient;
}
// }}}

// {{{ Implementation

bool JSWebSocket::JS_send(JSContext *cx, JS::CallArgs &args)
{
    if (args[0].isString()) {
        JSAutoByteString cdata;
        JS::RootedString str(cx, args[0].toString());
        cdata.encodeUtf8(cx, str);

        this->ws()->write(reinterpret_cast<unsigned char *>(cdata.ptr()),
                            strlen(cdata.ptr()), false);

        args.rval().setInt32(0);

    } else if (args[0].isObject()) {
        JSObject *objdata = args[0].toObjectOrNull();

        if (!objdata || !JS_IsArrayBufferObject(objdata)) {
            JS_ReportErrorUTF8(cx,
                           "write() invalid data (must be either a string or "
                           "an ArrayBuffer)");
            return false;
        }
        uint32_t len  = JS_GetArrayBufferByteLength(objdata);

        bool shared;
        JS::AutoCheckCannotGC nogc;

        uint8_t *data = JS_GetArrayBufferData(objdata, &shared, nogc);

        this->ws()->write(static_cast<unsigned char *>(data), len, true);

        args.rval().setInt32(0);

    } else {
        JS_ReportErrorUTF8(
            cx,
            "write() invalid data (must be either a string or an ArrayBuffer)");
        return false;
    }

    return true;
}

bool JSWebSocket::JS_close(JSContext *cx, JS::CallArgs &args)
{
    this->ws()->close();

    return true;
}

bool JSWebSocket::JS_ping(JSContext *cx, JS::CallArgs &args)
{
    this->ws()->ping();

    return true;
}
JSWebSocket *JSWebSocket::Constructor(JSContext *cx, JS::CallArgs &args,
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
        JS_ReportErrorUTF8(cx, "Invalid WebSocketServer URI : %s", durl);
        free(path);
        free(host);
        free(durl);
        return nullptr;
    }

    JSWebSocket *wss = new JSWebSocket(cx, host, port, path, isSSL);
    
    /* Workaround so we can call root() within ::Constructor */
    wss->m_Instance = obj;
    
    wss->root();

    free(path);
    free(host);
    free(durl);

    return wss;
}
// }}}

JSFunctionSpec * JSWebSocket::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSWebSocket, send, 1),
        CLASSMAPPER_FN(JSWebSocket, close, 0),
        CLASSMAPPER_FN(JSWebSocket, ping, 0),
        JS_FS_END
    };

    return funcs;
}

void JSWebSocket::RegisterObject(JSContext *cx)
{
    JSWebSocket::ExposeClass<1>(cx, "WebSocket");
}

} // namespace Binding
} // namespace Nidium
