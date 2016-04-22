/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "JSWebSocketClient.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "Net/HTTP.h"
#include "JSUtils.h"

using Nidium::Net::WebSocketClient;

namespace Nidium {
namespace Binding {

// {{{ Preamble
static void WebSocket_Finalize(JSFreeOp *fop, JSObject *obj);
static bool nidium_websocket_send(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_websocket_close(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_websocket_ping(JSContext *cx, unsigned argc, JS::Value *vp);

static JSClass WebSocket_class = {
    "WebSocket", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, WebSocket_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *JSExposer<JSWebSocket>::jsclass = &WebSocket_class;

static JSFunctionSpec ws_funcs[] = {
    JS_FN("send", nidium_websocket_send, 1, NATIVE_JS_FNPROPS),
    JS_FN("close", nidium_websocket_close, 0, NATIVE_JS_FNPROPS),
    JS_FN("ping", nidium_websocket_ping, 0, NATIVE_JS_FNPROPS),
    JS_FS_END
};
// }}}

// {{{ JSWebSocket
JSWebSocket::JSWebSocket(JS::HandleObject obj, JSContext *cx,
    const char *host,
    unsigned short port, const char *path, bool ssl) : JSExposer<JSWebSocket>(obj, cx)
{
    m_WebSocketClient = new WebSocketClient(port, path, host);
    bool ret = m_WebSocketClient->connect(ssl, (ape_global *)JS_GetContextPrivate(cx));

    if (!ret) {
        JS_ReportWarning(cx, "Failed to connect to WS endpoint\n");
        return;
    }

    m_WebSocketClient->addListener(this);
}

void JSWebSocket::onMessage(const Core::SharedMessages::Message &msg)
{
    JSContext *cx = m_Cx;

    JS::RootedValue oncallback(cx);
    JS::RootedValue rval(cx);

    switch (msg.event()) {
        case NIDIUM_EVENT(WebSocketClient, CLIENT_FRAME):
        {
            JS::AutoValueArray<1> arg(cx);

            const char *data = (const char *)msg.args[2].toPtr();
            int len = msg.args[3].toInt();
            bool binary = msg.args[4].toBool();

            JS::RootedObject obj(m_Cx, this->getJSObject());
            if (JS_GetProperty(m_Cx, obj, "onmessage", &oncallback) &&
                JS_TypeOfValue(m_Cx, oncallback) == JSTYPE_FUNCTION) {

                JS::RootedValue jdata(cx);
                JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, NULL, JS::NullPtr(), JS::NullPtr()));

                JSUtils::strToJsval(m_Cx, data, len, &jdata, !binary ? "utf8" : NULL);
                NIDIUM_JSOBJ_SET_PROP(event, "data", jdata);

                arg[0].setObjectOrNull(event);

                JS_CallFunctionValue(m_Cx, obj, oncallback, arg, &rval);
            }

            break;
        }
        case NIDIUM_EVENT(WebSocketClient, CLIENT_CONNECT):
        {
            JS::RootedObject obj(cx, this->getJSObject());

            JSOBJ_CALLFUNCNAME(obj, "onopen", JS::HandleValueArray::empty());

            break;
        }
        case NIDIUM_EVENT(WebSocketClient, CLIENT_CLOSE):
        {
            JS::RootedObject obj(cx, this->getJSObject());

            JSOBJ_CALLFUNCNAME(obj, "onclose", JS::HandleValueArray::empty());

            NidiumJSObj(cx)->unrootObject(obj);

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

static bool nidium_websocket_send(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSWebSocket, &WebSocket_class);

    NIDIUM_JS_CHECK_ARGS("send", 1);

    if (args[0].isString()) {
        JSAutoByteString cdata;
        JS::RootedString str(cx, args[0].toString());
        cdata.encodeUtf8(cx, str);

        CppObj->ws()->write((unsigned char *)cdata.ptr(),
            strlen(cdata.ptr()), false);

        args.rval().setInt32(0);

    } else if (args[0].isObject()) {
        JSObject *objdata = args[0].toObjectOrNull();

        if (!objdata || !JS_IsArrayBufferObject(objdata)) {
            JS_ReportError(cx, "write() invalid data (must be either a string or an ArrayBuffer)");
            return false;
        }
        uint32_t len = JS_GetArrayBufferByteLength(objdata);
        uint8_t *data = JS_GetArrayBufferData(objdata);

        CppObj->ws()->write((unsigned char *)data, len, true);

        args.rval().setInt32(0);

    } else {
        JS_ReportError(cx, "write() invalid data (must be either a string or an ArrayBuffer)");
        return false;
    }

    return true;
}

static bool nidium_websocket_close(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSWebSocket, &WebSocket_class);

    CppObj->ws()->close();

    return true;
}

static bool nidium_websocket_ping(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSWebSocket, &WebSocket_class);

    CppObj->ws()->ping();

    return true;
}

static bool nidium_WebSocket_constructor(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JS::RootedString url(cx);
    JS::RootedString protocol(cx);

    if (!args.isConstructing()) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &WebSocket_class, args));

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

    JSWebSocket *wss = new JSWebSocket(ret, cx, host, port, path, isSSL);

    free(path);
    free(host);
    free(durl);

    JS_SetPrivate(ret, wss);

    args.rval().setObjectOrNull(ret);

    NidiumJSObj(cx)->rootObjectUntilShutdown(ret);

    return true;
}

static void WebSocket_Finalize(JSFreeOp *fop, JSObject *obj)
{
    JSWebSocket *wss = (JSWebSocket *)JS_GetPrivate(obj);

    if (wss != NULL) {
        delete wss;
    }
}

// }}}

// {{{ Registration
void JSWebSocket::registerObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS_InitClass(cx, global, JS::NullPtr(), &WebSocket_class,
        nidium_WebSocket_constructor,
        1, NULL, ws_funcs, NULL, NULL);
}
// }}}

} // namespace Binding
} // namespace Nidium

