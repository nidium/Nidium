/*
    NativeJS Core Library
    Copyright (C) 2014 Anthony Catel <paraboul@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "NativeJSWebSocket.h"
#include "NativeHTTP.h"

#include <NativeJSUtils.h>

#define SET_PROP(where, name, val) JS_DefineProperty(cx, where, \
    (const char *)name, val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | \
        JSPROP_ENUMERATE)

static void WebSocketServer_Finalize(JSFreeOp *fop, JSObject *obj);
static void WebSocket_Finalize_client(JSFreeOp *fop, JSObject *obj);

static JSClass WebSocketServer_class = {
    "WebSocketServer", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, WebSocketServer_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass WebSocketServer_client_class = {
    "WebSocketServerClient", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, WebSocket_Finalize_client,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static void WebSocket_Finalize_client(JSFreeOp *fop, JSObject *obj)
{
    printf("Ws client finalized\n");
}

static void WebSocketServer_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSWebSocketServer *wss = (NativeJSWebSocketServer *)JS_GetPrivate(obj);

    if (wss != NULL) {
        delete wss;
        printf("Delete websocket server...\n");
    }    
}

static JSBool native_WebSocketServer_constructor(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JSString *localhost, *protocol = NULL;
    if (!JS_IsConstructing(cx, vp)) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    JSObject *ret = JS_NewObjectForConstructor(cx, &WebSocketServer_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S/S",
        &localhost, &protocol)) {
        return false;
    }

    JSAutoByteString clocalhost(cx, localhost);

    char *url = strdup(clocalhost.ptr());
    char *host = (char *)malloc(clocalhost.length());
    char *path = (char *)malloc(clocalhost.length());
    uint16_t port;

    if (NativeHTTP::ParseURI(url, clocalhost.length(), host,
        &port, path, "ws://") == -1) {
        JS_ReportError(cx, "Invalid WebSocketServer URI : %s", url);
        free(path);
        free(host);
        free(url);
        return false;
    }

    NativeJSWebSocketServer *wss = new NativeJSWebSocketServer(host, port);
    wss->jsobj = ret;
    wss->cx = cx;

    free(path);
    free(host);
    free(url);

    if (!wss->start()) {
        JS_ReportError(cx, "WebSocketServer: failed to bind on %s",
            clocalhost.ptr());
        delete wss;
        return false;
    }

    JS_SetPrivate(ret, wss);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    /*
        Server is listening at this point. Don't collect.
    */
    NativeJSObj(cx)->rootObjectUntilShutdown(ret);

    return true;
}

NativeJSWebSocketServer::NativeJSWebSocketServer(const char *host,
    unsigned short port)
{
    m_WebSocketServer = new NativeWebSocketListener(port, host);
    m_WebSocketServer->setListener(this);
}

NativeJSWebSocketServer::~NativeJSWebSocketServer()
{
    delete m_WebSocketServer;
}

JSObject *NativeJSWebSocketServer::createClient(NativeWebSocketClientConnection *client)
{
    JSObject *jclient;

    jclient = JS_NewObject(cx, &WebSocketServer_client_class, NULL, NULL);

    JS_SetPrivate(jclient, client);

    NativeJSObj(cx)->rootObjectUntilShutdown(jclient);
    client->setData(jclient);

    return jclient;
}

void NativeJSWebSocketServer::onMessage(const NativeSharedMessages::Message &msg)
{
    jsval oncallback, rval;
    switch (msg.event()) {
        case NATIVEWEBSOCKET_SERVER_FRAME:
        {
            jsval arg[2];
            const char *data = (const char *)msg.args[1].toPtr();
            int len = msg.args[2].toInt();
            bool binary = msg.args[3].toBool();
            
            JSObject *jclient = (JSObject *)((NativeWebSocketClientConnection *)msg.args[0].toPtr())->getData();

            if (!jclient) {
                return;
            }

            if (JS_GetProperty(cx, this->getJSObject(), "onmessage", &oncallback) &&
                JS_TypeOfValue(cx, oncallback) == JSTYPE_FUNCTION) {

                jsval jdata;

                JSObject *event = JS_NewObject(cx, NULL, NULL, NULL);           
                NativeJSUtils::strToJsval(cx, data, len, &jdata, !binary ? "utf8" : NULL);
                SET_PROP(event, "data", jdata);

                arg[0].setObjectOrNull(jclient);
                arg[1].setObjectOrNull(event);

                JS_CallFunctionValue(cx, this->getJSObject(), oncallback,
                    2, arg, &rval);
            }

            break;
        }
        case NATIVEWEBSOCKET_SERVER_CONNECT:
        {
            jsval arg;
            JSObject *jclient = this->createClient(
                (NativeWebSocketClientConnection *)msg.args[0].toPtr());

            arg.setObjectOrNull(jclient);

            if (JS_GetProperty(cx, this->getJSObject(), "onopen", &oncallback) &&
                JS_TypeOfValue(cx, oncallback) == JSTYPE_FUNCTION) {

                JS_CallFunctionValue(cx, this->getJSObject(), oncallback,
                    1, &arg, &rval);
            }

            break;
        }
        default:
            break;
    }
}

bool NativeJSWebSocketServer::start()
{
    if (!m_WebSocketServer) return false;

    return m_WebSocketServer->start();
}

NATIVE_OBJECT_EXPOSE(WebSocketServer)
