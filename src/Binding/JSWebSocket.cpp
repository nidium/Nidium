/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "JSWebSocket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Net/HTTP.h"
#include "JSUtils.h"

using Nidium::Net::WebSocketListener;
using Nidium::Net::WebSocketClientConnection;

namespace Nidium {
namespace Binding {

// {{{ Preamble
#define SET_PROP(where, name, val) JS_DefineProperty(cx, where, \
    (const char *)name, val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | \
        JSPROP_ENUMERATE)

static void WebSocketServer_Finalize(JSFreeOp *fop, JSObject *obj);
static void WebSocket_Finalize_client(JSFreeOp *fop, JSObject *obj);
static bool nidium_websocketclient_send(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_websocketclient_close(JSContext *cx, unsigned argc, JS::Value *vp);

static JSClass WebSocketServer_class = {
    "WebSocketServer", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, WebSocketServer_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *JSExposer<JSWebSocketServer>::jsclass = &WebSocketServer_class;

static JSFunctionSpec wsclient_funcs[] = {
    JS_FN("send", nidium_websocketclient_send, 1, NATIVE_JS_FNPROPS),
    JS_FN("close", nidium_websocketclient_close, 0, NATIVE_JS_FNPROPS),
    JS_FS_END
};

static JSClass WebSocketServer_client_class = {
    "WebSocketServerClient", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, WebSocket_Finalize_client,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};
// }}}

// {{{ JSWebSocketServer
JSWebSocketServer::JSWebSocketServer(JS::HandleObject obj, JSContext *cx,
    const char *host,
    unsigned short port) : JSExposer<JSWebSocketServer>(obj, cx)
{
    m_WebSocketServer = new WebSocketListener(port, host);
    m_WebSocketServer->addListener(this);
}

JSObject *JSWebSocketServer::createClient(WebSocketClientConnection *client)
{

    JS::RootedObject jclient(m_Cx, JS_NewObject(m_Cx, &WebSocketServer_client_class, JS::NullPtr(), JS::NullPtr()));

    JS_DefineFunctions(m_Cx, jclient, wsclient_funcs);

    JS_SetPrivate(jclient, client);

    NidiumJSObj(m_Cx)->rootObjectUntilShutdown(jclient);
    client->setData(jclient);

    return jclient;
}

void JSWebSocketServer::onMessage(const Core::SharedMessages::Message &msg)
{
    JSContext *cx = m_Cx;

    JS::RootedValue oncallback(cx);
    JS::RootedValue rval(cx);

    switch (msg.event()) {
        case NIDIUM_EVENT(WebSocketListener, SERVER_FRAME):
        {
            JS::AutoValueArray<2> arg(cx);

            const char *data = (const char *)msg.args[2].toPtr();
            int len = msg.args[3].toInt();
            bool binary = msg.args[4].toBool();

            JS::RootedObject jclient(cx, (JSObject *)((WebSocketClientConnection *)msg.args[1].toPtr())->getData());

            if (!jclient.get()) {
                return;
            }
            JS::RootedObject obj(m_Cx, this->getJSObject());
            if (JS_GetProperty(m_Cx, obj, "onmessage", &oncallback) &&
                JS_TypeOfValue(m_Cx, oncallback) == JSTYPE_FUNCTION) {

                JS::RootedValue jdata(cx);
                JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, NULL, JS::NullPtr(), JS::NullPtr()));

                JSUtils::strToJsval(m_Cx, data, len, &jdata, !binary ? "utf8" : NULL);
                NIDIUM_JSOBJ_SET_PROP(event, "data", jdata);

                arg[0].setObjectOrNull(jclient);
                arg[1].setObjectOrNull(event);

                JS_CallFunctionValue(m_Cx, obj, oncallback, arg, &rval);
            }

            break;
        }
        case NIDIUM_EVENT(WebSocketListener, SERVER_CONNECT):
        {
            JS::AutoValueArray<1> arg(cx);

            JSObject *jclient = this->createClient(
                (WebSocketClientConnection *)msg.args[1].toPtr());

            arg[0].setObject(*jclient);

            JS::RootedObject obj(cx, this->getJSObject());

            JSOBJ_CALLFUNCNAME(obj, "onopen", arg);

            break;
        }
        case NIDIUM_EVENT(WebSocketListener, SERVER_CLOSE):
        {
            WebSocketClientConnection *client = (WebSocketClientConnection *)msg.args[1].toPtr();
            JS::AutoValueArray<1> arg(cx);
            JS::RootedObject jclient(cx, (JSObject *)client->getData());
            JS::RootedObject obj(cx, this->getJSObject());

            arg[0].setObject(*jclient);
            JSOBJ_CALLFUNCNAME(obj, "onclose", arg);

            JS_SetPrivate(jclient, NULL);
            NidiumJSObj(m_Cx)->unrootObject(jclient);

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
static bool nidium_websocketclient_send(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(WebSocketClientConnection,
        &WebSocketServer_client_class);

    NIDIUM_JS_CHECK_ARGS("send", 1);

    if (args[0].isString()) {
        JSAutoByteString cdata;
        JS::RootedString str(cx, args[0].toString());
        cdata.encodeUtf8(cx, str);

        CppObj->write((unsigned char *)cdata.ptr(),
            strlen(cdata.ptr()), false, APE_DATA_COPY);

        args.rval().setInt32(0);

    } else if (args[0].isObject()) {
        JSObject *objdata = args[0].toObjectOrNull();

        if (!objdata || !JS_IsArrayBufferObject(objdata)) {
            JS_ReportError(cx, "write() invalid data (must be either a string or an ArrayBuffer)");
            return false;
        }
        uint32_t len = JS_GetArrayBufferByteLength(objdata);
        uint8_t *data = JS_GetArrayBufferData(objdata);

        CppObj->write((unsigned char *)data, len, true, APE_DATA_COPY);

        args.rval().setInt32(0);

    } else {
        JS_ReportError(cx, "write() invalid data (must be either a string or an ArrayBuffer)");
        return false;
    }

    return true;
}

static bool nidium_websocketclient_close(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(WebSocketClientConnection,
        &WebSocketServer_client_class);

    CppObj->close();

    return true;
}

static void WebSocket_Finalize_client(JSFreeOp *fop, JSObject *obj)
{

}
// }}}

// {{{ WebSocketServer implementation
static bool nidium_WebSocketServer_constructor(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JS::RootedString url(cx);
    JS::RootedString protocol(cx);

    if (!args.isConstructing()) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &WebSocketServer_class, args));

    if (!JS_ConvertArguments(cx, args, "S/S", url.address(), protocol.address())) {
        return false;
    }

    JSAutoByteString curl(cx, url);

    char *durl = strdup(curl.ptr());
    char *host = (char *)calloc(curl.length(), sizeof(char));
    char *path = (char *)calloc(curl.length(), sizeof(char));

    u_short port = 80;
    u_short default_port = 80;
    bool isSSL = false;

    const char *prefix = NULL;
    if (strncasecmp(curl.ptr(), CONST_STR_LEN("wss://")) == 0) {
        prefix = "wss://";
        isSSL = true;
        default_port = 443;
    } else if (strncasecmp(curl.ptr(), CONST_STR_LEN("ws://")) == 0) {
        prefix = "ws://";
    } else {
        /* No prefix provided. Assuming 'default url' => no SSL, port 80 */
        prefix = "";
    }

    if (Net::HTTP::ParseURI(durl, curl.length(), host,
        &port, path, prefix, default_port) == -1) {
        JS_ReportError(cx, "Invalid WebSocketServer URI : %s", durl);
        free(path);
        free(host);
        free(durl);
        return false;
    }

    JSWebSocketServer *wss = new JSWebSocketServer(ret, cx, host, port);

    free(path);
    free(host);
    free(durl);

    if (!wss->start()) {
        JS_ReportError(cx, "WebSocketServer: failed to bind on %s",
            curl.ptr());
        delete wss;
        return false;
    }

    JS_SetPrivate(ret, wss);

    args.rval().setObjectOrNull(ret);
    /*
        Server is listening at this point. Don't collect.
    */
    NidiumJSObj(cx)->rootObjectUntilShutdown(ret);

    return true;
}

static void WebSocketServer_Finalize(JSFreeOp *fop, JSObject *obj)
{
    JSWebSocketServer *wss = (JSWebSocketServer *)JS_GetPrivate(obj);

    if (wss != NULL) {
        delete wss;
    }
}
// }}}

// {{{ Registration
NIDIUM_JS_OBJECT_EXPOSE(WebSocketServer)
// }}}

} // namespace Binding
} // namespace Nidium

