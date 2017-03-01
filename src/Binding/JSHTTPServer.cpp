/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSHTTPServer.h"
#include "Binding/JSUtils.h"


#include <stdbool.h>
#include <unistd.h>


using Nidium::Net::HTTPServer;
using Nidium::Net::HTTPClientConnection;

namespace Nidium {
namespace Binding {


// {{{ JSHTTPResponse
JSHTTPResponse::JSHTTPResponse(uint16_t code)
    : HTTPResponse(code)
{

}
// }}}

// {{{ JSHTTPServer
JSHTTPServer::JSHTTPServer(uint16_t port,
                           const char *ip)
    : HTTPServer(port, ip)
{
}

JSHTTPServer::~JSHTTPServer()
{
}

HTTPClientConnection *JSHTTPServer::onClientConnect(ape_socket *client,
    ape_global *ape)
{
    JSHTTPClientConnection *conn;
    conn = new JSHTTPClientConnection(this, client);

    JS::RootedObject obj(m_Cx,
        JSHTTPClientConnection::CreateObject(m_Cx, conn));

    /*
        The object will be deleted during onClientDisconnect
    */
    conn->root();

    NIDIUM_JSOBJ_SET_PROP_CSTR(obj, "ip", APE_socket_ipv4(client));

    HTTPServer::onClientConnect(conn);

    return conn;
}

void JSHTTPServer::onClientDisconnect(HTTPClientConnection *client)
{

}

void JSHTTPServer::onData(HTTPClientConnection *client,
                          const char *buf,
                          size_t len)
{
    // on progress
}

bool JSHTTPServer::onEnd(HTTPClientConnection *client)
{
    buffer *k, *v;

    JS::RootedValue rval(m_Cx);
    JS::RootedValue oncallback(m_Cx);

    JSHTTPClientConnection *subclient
        = reinterpret_cast<JSHTTPClientConnection *>(client);

    JS::RootedObject objrequest(
        m_Cx,
        JS_NewPlainObject(m_Cx));
    JS::RootedObject headers(
        m_Cx, JS_NewPlainObject(m_Cx));

    if (client->getHTTPState()->headers.list) {
        APE_A_FOREACH(client->getHTTPState()->headers.list, k, v)
        {
            JS::RootedString jstr(
                m_Cx, JS_NewStringCopyN(m_Cx, (char *)v->data, v->used - 1));
            NIDIUM_JSOBJ_SET_PROP_FLAGS(headers, k->data, jstr,
                                        JSPROP_ENUMERATE);
        }
    }

    buffer *url = client->getHTTPState()->url;
    if (url) {
        JS::RootedString jsurl(
            m_Cx, JS_NewStringCopyN(m_Cx, (char *)url->data, url->used));
        NIDIUM_JSOBJ_SET_PROP(objrequest, "url", jsurl);
    }

    buffer *data = client->getHTTPState()->data;
    if (client->getHTTPState()->parser.method == HTTP_POST) {
        JS::RootedValue strdata(m_Cx);
        if (data == NULL || data->used == 0) {
            strdata.set(JS_GetEmptyStringValue(m_Cx));
        } else {
            JSUtils::StrToJsval(m_Cx, (const char *)data->data, data->used,
                                &strdata, "utf8");
        }

        NIDIUM_JSOBJ_SET_PROP(objrequest, "data", strdata);
    }

    JS::RootedValue method(m_Cx);
    JS::RootedObject cli(m_Cx, subclient->getJSObject());

    switch (client->getHTTPState()->parser.method) {
        case HTTP_POST:
            method.setString(JS_NewStringCopyN(m_Cx, CONST_STR_LEN("POST")));
            break;
        case HTTP_GET:
            method.setString(JS_NewStringCopyN(m_Cx, CONST_STR_LEN("GET")));
            break;
        case HTTP_PUT:
            method.setString(JS_NewStringCopyN(m_Cx, CONST_STR_LEN("PUT")));
            break;
        default:
            method.setString(JS_NewStringCopyN(m_Cx, CONST_STR_LEN("UNKOWN")));
            break;
    }

    NIDIUM_JSOBJ_SET_PROP(objrequest, "method", method);
    NIDIUM_JSOBJ_SET_PROP(objrequest, "headers", headers);
    NIDIUM_JSOBJ_SET_PROP(objrequest, "client", cli);
    JS::RootedObject obj(m_Cx, m_Instance);
    if (JS_GetProperty(m_Cx, obj, "onrequest", &oncallback)
        && JS_TypeOfValue(m_Cx, oncallback) == JSTYPE_FUNCTION) {

        JS::AutoValueArray<2> arg(m_Cx);
        arg[0].setObjectOrNull(objrequest);
        arg[1].setObjectOrNull(
            static_cast<JSHTTPResponse *>(client->getResponse())
                ->getJSObject());
        JS_CallFunctionValue(m_Cx, obj, oncallback, arg, &rval);
    }

    return false;
}
// }}}

// {{{ Implementation
JSHTTPServer *JSHTTPServer::Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj)
{
    uint16_t port;
    JS::RootedString ip_bind(cx);
    bool reuseport = false;
    JSHTTPServer *listener;
    JS::RootedObject options(cx);

    NIDIUM_JS_INIT_OPT();

    if (!JS_ConvertArguments(cx, args, "Sc/o", ip_bind.address(), &port,
                             options.address())) {
        return nullptr;
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "reusePort", Boolean)
    {
        reuseport = __curopt.toBoolean();
    }

    if (ip_bind) {
        JSAutoByteString cip(cx, ip_bind);
        listener = new JSHTTPServer(port, cip.ptr());
    } else {
        listener = new JSHTTPServer(port);
    }

    if (!listener->start((bool)reuseport)) {
        JS_ReportErrorUTF8(cx, "HTTPServer() couldn't listener on %d", port);
        delete listener;
        
        return nullptr;
    }

    /* Workaround so we can call root() within ::Constructor */
    listener->m_Instance = obj;

    listener->root();

    return listener;
}


bool JSHTTPResponse::JS_write(JSContext *cx, JS::CallArgs &args)
{
    if (args[0].isString()) {
        JSAutoByteString jsdata;
        JS::RootedString str(cx, args[0].toString());
        jsdata.encodeUtf8(cx, str);

        this->sendChunk(jsdata.ptr(), jsdata.length(), APE_DATA_COPY);

    } else if (args[0].isObject()) {
        JS::RootedObject objdata(cx, args[0].toObjectOrNull());
        if (!objdata || !JS_IsArrayBufferObject(objdata)) {
            JS_ReportErrorUTF8(cx,
                           "write() invalid data (must be either a string or "
                           "an ArrayBuffer)");
            return false;
        }
        uint32_t len  = JS_GetArrayBufferByteLength(objdata);

        JS::AutoCheckCannotGC nogc;
        bool shared;

        uint8_t *data = JS_GetArrayBufferData(objdata, &shared, nogc);

        this->sendChunk((char *)data, len, APE_DATA_COPY);
    } else {
        JS_ReportErrorUTF8(cx, "write() only accepts String or ArrayBuffer");
        return false;
    }

    return true;
}

bool JSHTTPResponse::JS_end(JSContext *cx, JS::CallArgs &args)
{
    if (args.length() > 0) {
        if (args[0].isString()) {
            JSAutoByteString jsdata;
            JS::RootedString str(cx, args[0].toString());
            jsdata.encodeUtf8(cx, str);

            this->sendChunk(jsdata.ptr(), jsdata.length(), APE_DATA_COPY, true);
        } else if (args[0].isObject()) {
            JS::RootedObject objdata(cx, args[0].toObjectOrNull());
            if (!objdata || !JS_IsArrayBufferObject(objdata)) {
                JS_ReportErrorUTF8(cx,
                               "end() invalid data (must be either a string or "
                               "an ArrayBuffer)");
                return false;
            }
            uint32_t len  = JS_GetArrayBufferByteLength(objdata);
            JS::AutoCheckCannotGC nogc;
            bool shared;

            uint8_t *data = JS_GetArrayBufferData(objdata, &shared, nogc);

            this->sendChunk((char *)data, len, APE_DATA_COPY, true);
        }
    }

    this->end();

    return true;
}

bool JSHTTPResponse::JS_writeHead(JSContext *cx, JS::CallArgs &args)
{
    uint16_t statuscode;

    JS::RootedObject headers(cx);

    if (!JS_ConvertArguments(cx, args, "c/o", &statuscode, headers.address())) {
        return false;
    }

    if (this->isHeadersAlreadySent()) {
        return true;
    }

    this->setStatusCode(statuscode);

    if (args.length() >= 2 && !args[1].isPrimitive()) {

        JS::Rooted<JS::IdVector> ida(cx, JS::IdVector(cx));
        JS_Enumerate(cx, headers, &ida);
        JS::RootedId id(cx);

        for (size_t i = 0; i < ida.length(); i++) {

            id = ida[i];

            if (!JSID_IS_STRING(id)) {
                continue;
            }
            JS::RootedString key(cx, JSID_TO_STRING(id));
            JS::RootedValue val(cx);

            if (!JS_GetPropertyById(cx, headers, id, &val)
                || !val.isString()) {
                continue;
            }

            JSAutoByteString ckey(cx, key);
            JSAutoByteString cval(cx, val.toString());

            this->setHeader(ckey.ptr(), cval.ptr());
        }
    }

    this->sendHeaders(true);

    return true;
}


// }}}

// {{{ Registration

JSFunctionSpec *JSHTTPResponse::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSHTTPResponse, write, 1),
        CLASSMAPPER_FN(JSHTTPResponse, writeHead, 1),
        CLASSMAPPER_FN(JSHTTPResponse, end, 0),
        JS_FS_END
    };

    return funcs;
}

void JSHTTPServer::RegisterObject(JSContext *cx)
{
    JSHTTPServer::ExposeClass<1>(cx, "HTTPServer");
    JSHTTPClientConnection::ExposeClass(cx, "HTTPServerClientConnection");
    JSHTTPResponse::ExposeClass(cx, "HTTPServerResponse");
}
// }}}

} // namespace Binding
} // namespace Nidium
