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
static JSBool native_websocketclient_send(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_websocketclient_close(JSContext *cx, unsigned argc, jsval *vp);

static JSClass WebSocketServer_class = {
    "WebSocketServer", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, WebSocketServer_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

template<>
JSClass *NativeJSExposer<NativeJSWebSocketServer>::jsclass = &WebSocketServer_class;

static JSFunctionSpec wsclient_funcs[] = {
    JS_FN("send", native_websocketclient_send, 1, 0),
    JS_FN("close", native_websocketclient_close, 0, 0),
    JS_FS_END
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

static JSBool native_websocketclient_send(JSContext *cx, unsigned argc, jsval *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeWebSocketClientConnection,
        &WebSocketServer_client_class);

    NATIVE_CHECK_ARGS("send", 1);

    if (args[0].isString()) {
        JSAutoByteString cdata;

        cdata.encodeUtf8(cx, args[0].toString());

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

static JSBool native_websocketclient_close(JSContext *cx, unsigned argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, &args.thisv().toObject());

    return true;
}

static JSBool native_WebSocketServer_constructor(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSString *localhost, *protocol = NULL;
    if (!JS_IsConstructing(cx, vp)) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    JSObject *ret = JS_NewObjectForConstructor(cx, &WebSocketServer_class, vp);

    if (!JS_ConvertArguments(cx, argc, args.array(), "S/S",
        &localhost, &protocol)) {
        return false;
    }

    JSAutoByteString clocalhost(cx, localhost);

    uint16_t port;
    char *url = strdup(clocalhost.ptr());
    char *host = (char *)malloc(clocalhost.length());
    char *path = (char *)malloc(clocalhost.length());

    if (NativeHTTP::ParseURI(url, clocalhost.length(), host,
        &port, path, "ws://") == -1) {
        JS_ReportError(cx, "Invalid WebSocketServer URI : %s", url);
        free(path);
        free(host);
        free(url);
        return false;
    }

    NativeJSWebSocketServer *wss = new NativeJSWebSocketServer(ret, cx, host, port);

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

    args.rval().setObjectOrNull(ret);

    /*
        Server is listening at this point. Don't collect.
    */
    NativeJSObj(cx)->rootObjectUntilShutdown(ret);

    return true;
}

NativeJSWebSocketServer::NativeJSWebSocketServer(JSObject *obj, JSContext *cx,
    const char *host,
    unsigned short port) : NativeJSExposer<NativeJSWebSocketServer>(obj, cx)
{
    m_WebSocketServer = new NativeWebSocketListener(port, host);
    m_WebSocketServer->addListener(this);
}

NativeJSWebSocketServer::~NativeJSWebSocketServer()
{
    delete m_WebSocketServer;
}

JSObject *NativeJSWebSocketServer::createClient(NativeWebSocketClientConnection *client)
{
    JSObject *jclient;

    jclient = JS_NewObject(m_Cx, &WebSocketServer_client_class, NULL, NULL);

    JS_DefineFunctions(m_Cx, jclient, wsclient_funcs);

    JS_SetPrivate(jclient, client);

    NativeJSObj(m_Cx)->rootObjectUntilShutdown(jclient);
    client->setData(jclient);

    return jclient;
}

void NativeJSWebSocketServer::onMessage(const NativeSharedMessages::Message &msg)
{
    JSContext *cx = m_Cx;

    JS::RootedValue oncallback(cx);
    JS::RootedValue rval(cx);

    switch (msg.event()) {
        case NATIVE_EVENT(NativeWebSocketListener, SERVER_FRAME):
        {
            jsval arg[2];
            JS::AutoArrayRooter argRooter(cx, 2, arg);

            const char *data = (const char *)msg.args[2].toPtr();
            int len = msg.args[3].toInt();
            bool binary = msg.args[4].toBool();
            
            JSObject *jclient = (JSObject *)((NativeWebSocketClientConnection *)msg.args[1].toPtr())->getData();

            if (!jclient) {
                return;
            }

            if (JS_GetProperty(m_Cx, this->getJSObject(), "onmessage", oncallback.address()) &&
                JS_TypeOfValue(m_Cx, oncallback) == JSTYPE_FUNCTION) {

                JS::RootedValue jdata(cx);

                JSObject *event = JS_NewObject(m_Cx, NULL, NULL, NULL);           
                NativeJSUtils::strToJsval(m_Cx, data, len, &jdata, !binary ? "utf8" : NULL);
                SET_PROP(event, "data", jdata);

                arg[0].setObjectOrNull(jclient);
                arg[1].setObjectOrNull(event);

                JS_CallFunctionValue(m_Cx, this->getJSObject(), oncallback,
                    2, arg, rval.address());
            }

            break;
        }
        case NATIVE_EVENT(NativeWebSocketListener, SERVER_CONNECT):
        {
            JSObject *jclient = this->createClient(
                (NativeWebSocketClientConnection *)msg.args[1].toPtr());

            JS::Value arg[1] = { OBJECT_TO_JSVAL(jclient) };
            JS::AutoArrayRooter rooter(cx, 1, arg);

            JSOBJ_CALLFUNCNAME(this->getJSObject(), "onopen", 1, arg);

            break;
        }
        case NATIVE_EVENT(NativeWebSocketListener, SERVER_CLOSE):
        {
            NativeWebSocketClientConnection *client = (NativeWebSocketClientConnection *)msg.args[1].toPtr();
            JSObject *jclient = (JSObject *)client->getData();

            JS::Value arg[1] = { OBJECT_TO_JSVAL(jclient) };
            JS::AutoArrayRooter rooter(cx, 1, arg);

            JSOBJ_CALLFUNCNAME(this->getJSObject(), "onclose", 1, arg);

            JS_SetPrivate(jclient, NULL);
            NativeJSObj(m_Cx)->unrootObject(jclient);

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
