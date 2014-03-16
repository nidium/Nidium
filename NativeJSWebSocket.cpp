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

static void WebSocketServer_Finalize(JSFreeOp *fop, JSObject *obj);

static JSClass WebSocketServer_class = {
    "WebSocketServer", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, WebSocketServer_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static void WebSocketServer_Finalize(JSFreeOp *fop, JSObject *obj)
{

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

    free(path);
    free(host);
    free(url);

    if (!wss->start()) {
        JS_ReportError(cx, "WebSocketServer: failed to bind on %s",
            clocalhost.ptr());
        delete wss;
        return false;
    }
    return true;
}

NativeJSWebSocketServer::NativeJSWebSocketServer(const char *host,
    unsigned short port)
{
    m_WebSocketServer = new NativeWebSocketListener(port, host);
    m_WebSocketServer->setListener(this);
}

void NativeJSWebSocketServer::onMessage(const NativeSharedMessages::Message &msg)
{
    
}

bool NativeJSWebSocketServer::start()
{
    if (!m_WebSocketServer) return false;

    return m_WebSocketServer->start();
}

NATIVE_OBJECT_EXPOSE(WebSocketServer)
