/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSHTTPServer.h"

#include <stdbool.h>
#include <unistd.h>

#include "Binding/JSUtils.h"

using Nidium::Net::HTTPServer;
using Nidium::Net::HTTPClientConnection;

namespace Nidium {
namespace Binding {

// {{{ Preamble
static void HTTPServer_Finalize(JSFreeOp *fop, JSObject *obj);

static bool nidium_httpresponse_write(JSContext *cx,
    unsigned argc, JS::Value *vp);
static bool nidium_httpresponse_end(JSContext *cx,
    unsigned argc, JS::Value *vp);
static bool nidium_httpresponse_writeHead(JSContext *cx,
    unsigned argc, JS::Value *vp);

static JSClass HTTPServer_class = {
    "HTTPListener", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, HTTPServer_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *JSExposer<JSHTTPServer>::jsclass = &HTTPServer_class;

static JSClass HTTPRequest_class = {
    "HTTPRequest", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JSCLASS_NO_OPTIONAL_MEMBERS
};

/*
    TODO: write is for response
*/
static JSFunctionSpec HTTPResponse_funcs[] = {
    JS_FN("write", nidium_httpresponse_write, 1, NIDIUM_JS_FNPROPS),
    JS_FN("end", nidium_httpresponse_end, 0, NIDIUM_JS_FNPROPS),
    JS_FN("writeHead", nidium_httpresponse_writeHead, 1, NIDIUM_JS_FNPROPS),
    JS_FS_END
};

#if 0
static JSPropertySpec HTTPRequest_props[] = {
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};
#endif
// }}}

// {{{ JSHTTPResponse
JSHTTPResponse::JSHTTPResponse(JSContext *cx, uint16_t code) :
        HTTPResponse(code),
        JSObjectMapper(cx, "HTTPResponse")
{
    JS_DefineFunctions(cx, m_JSObj, HTTPResponse_funcs);
}
// }}}

// {{{ JSHTTPServer
JSHTTPServer::JSHTTPServer(JS::HandleObject obj, JSContext *cx,
    uint16_t port, const char *ip) :
    JSExposer<JSHTTPServer>(obj, cx),
    HTTPServer(port, ip)
{

}

JSHTTPServer::~JSHTTPServer()
{

}

void JSHTTPServer::onClientDisconnect(HTTPClientConnection *client)
{

}

void JSHTTPServer::onData(HTTPClientConnection *client,
    const char *buf, size_t len)
{
    // on progress
}

bool JSHTTPServer::onEnd(HTTPClientConnection *client)
{
    buffer *k, *v;

    JS::RootedValue rval(m_Cx);
    JS::RootedValue oncallback(m_Cx);

    JSHTTPClientConnection *subclient = static_cast<JSHTTPClientConnection *>(client);

    JS::RootedObject objrequest(m_Cx, JS_NewObject(m_Cx, &HTTPRequest_class, JS::NullPtr(), JS::NullPtr()));
    JS::RootedObject headers(m_Cx, JS_NewObject(m_Cx, NULL, JS::NullPtr(), JS::NullPtr()));

    if (client->getHTTPState()->headers.list) {
        APE_A_FOREACH(client->getHTTPState()->headers.list, k, v) {
            JS::RootedString jstr(m_Cx, JS_NewStringCopyN(m_Cx, (char *)v->data,
                v->used-1));
            NIDIUM_JSOBJ_SET_PROP_FLAGS(headers, k->data,
                jstr, JSPROP_ENUMERATE);
        }
    }

    buffer *url = client->getHTTPState()->url;
    if (url) {
        JS::RootedString jsurl(m_Cx, JS_NewStringCopyN(m_Cx, (char *)url->data, url->used));
        NIDIUM_JSOBJ_SET_PROP(objrequest, "url", jsurl);
    }

    buffer *data = client->getHTTPState()->data;
    if (client->getHTTPState()->parser.method == HTTP_POST) {
        JS::RootedValue strdata(m_Cx);
        if (data == NULL || data->used == 0) {
            strdata.setObjectOrNull(&JS_GetEmptyStringValue(m_Cx).toObject());
        } else {
            JSUtils::StrToJsval(m_Cx, (const char *)data->data,
                data->used, &strdata, "utf8");
        }

        NIDIUM_JSOBJ_SET_PROP(objrequest, "data", strdata);
    }

    JS::RootedValue method(m_Cx);
    JS::RootedValue head(m_Cx, OBJECT_TO_JSVAL(headers));
    JS::RootedValue cli(m_Cx, OBJECT_TO_JSVAL(subclient->getJSObject()));
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
    JS::RootedObject obj(m_Cx, m_JSObject);
    if (JS_GetProperty(m_Cx, obj, "onrequest", &oncallback) &&
        JS_TypeOfValue(m_Cx, oncallback) == JSTYPE_FUNCTION) {

        JS::AutoValueArray<2> arg(m_Cx);
        arg[0].setObjectOrNull(objrequest);
        arg[1].setObjectOrNull(static_cast<JSHTTPResponse*>(client->getResponse())->getJSObject());
        JS_CallFunctionValue(m_Cx, obj, oncallback, arg, &rval);
    }

    return false;
}
// }}}

// {{{ Implementation
static bool nidium_HTTPServer_constructor(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    uint16_t port;
    JS::RootedString ip_bind(cx);
    bool reuseport = false;
    JSHTTPServer *listener;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!args.isConstructing()) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &HTTPServer_class, args));

    if (!JS_ConvertArguments(cx, args, "c/bS", &port, &reuseport, ip_bind.address())) {
        return false;
    }

    if (ip_bind) {
        JSAutoByteString cip(cx, ip_bind);
        listener = new JSHTTPServer(ret, cx, port, cip.ptr());
    } else {
        listener = new JSHTTPServer(ret, cx, port);
    }

    if (!listener->start((bool)reuseport)) {
        JS_ReportError(cx, "HTTPServer() couldn't listener on %d", port);
        delete listener;
        return false;
    }

    JS_SetPrivate(ret, listener);
    args.rval().setObjectOrNull(ret);

    NidiumJSObj(cx)->rootObjectUntilShutdown(ret);

    return true;
}

#if 0
static bool nidium_HTTPRequest_class_constructor(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS_ReportError(cx, "Illegal constructor");
    return false;
}
#endif

static bool nidium_httpresponse_write(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    NIDIUM_JS_CHECK_ARGS("write", 1);

    JSHTTPResponse *resp = JSHTTPResponse::GetObject(caller);
    if (!resp) {
        return true;
    }

    if (args[0].isString()) {
        JSAutoByteString jsdata;
        JS::RootedString str(cx, args[0].toString());
        jsdata.encodeUtf8(cx, str);

        resp->sendChunk(jsdata.ptr(), jsdata.length(), APE_DATA_COPY);

    }  else if (args[0].isObject()) {
        JS::RootedObject objdata(cx, args[0].toObjectOrNull());
        if (!objdata || !JS_IsArrayBufferObject(objdata)) {
            JS_ReportError(cx, "write() invalid data (must be either a string or an ArrayBuffer)");
            return false;
        }
        uint32_t len = JS_GetArrayBufferByteLength(objdata);
        uint8_t *data = JS_GetArrayBufferData(objdata);

        resp->sendChunk((char *)data, len, APE_DATA_COPY);
    } else {
        JS_ReportError(cx, "write() only accepts String or ArrayBuffer");
        return false;
    }

    return true;
}

static bool nidium_httpresponse_end(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    JSHTTPResponse *resp = JSHTTPResponse::GetObject(caller);
    if (!resp) {
        return true;
    }

    if (args.length() > 0) {
        if (args[0].isString()) {
            JSAutoByteString jsdata;
            JS::RootedString str(cx, args[0].toString());
            jsdata.encodeUtf8(cx, str);

            resp->sendChunk(jsdata.ptr(), jsdata.length(), APE_DATA_COPY, true);
        } else if (args[0].isObject()) {
            JS::RootedObject objdata(cx, args[0].toObjectOrNull());
            if (!objdata || !JS_IsArrayBufferObject(objdata)) {
                JS_ReportError(cx, "end() invalid data (must be either a string or an ArrayBuffer)");
                return false;
            }
            uint32_t len = JS_GetArrayBufferByteLength(objdata);
            uint8_t *data = JS_GetArrayBufferData(objdata);

            resp->sendChunk((char *)data, len, APE_DATA_COPY, true);
        }
    }

    resp->end();

    return true;
}

static bool nidium_httpresponse_writeHead(JSContext *cx, unsigned argc, JS::Value *vp)
{
    uint16_t statuscode;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject headers(cx);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    if (!JS_ConvertArguments(cx, args, "c/o", &statuscode, headers.address())) {
        return false;
    }

    JSHTTPResponse *resp = JSHTTPResponse::GetObject(caller);
    if (!resp) {
        return true;
    }

    if (resp->isHeadersAlreadySent()) {
        return true;
    }

    resp->setStatusCode(statuscode);

    if (args.length() >= 2 && !args[1].isPrimitive()) {

        JS::RootedId idp(cx);

        JS::RootedObject iterator(cx, JS_NewPropertyIterator(cx, headers));

        while (JS_NextProperty(cx, iterator, idp.address()) && !JSID_IS_VOID(idp)) {
            if (!JSID_IS_STRING(idp)) {
                continue;
            }
            JS::RootedString key(cx, JSID_TO_STRING(idp));
            JS::RootedValue val(cx);

            if (!JS_GetPropertyById(cx, headers, idp, &val) || !val.isString()) {
                continue;
            }

            JSAutoByteString ckey(cx, key);
            JSAutoByteString cval(cx, val.toString());

            resp->setHeader(ckey.ptr(), cval.ptr());
        }
    }

    resp->sendHeaders(true);

    return true;
}

static void HTTPServer_Finalize(JSFreeOp *fop, JSObject *obj)
{
    JSHTTPServer *server = (JSHTTPServer *)JS_GetPrivate(obj);

    if (server != NULL) {
        delete server;
    }
}
// }}}

// {{{ Registration
void JSHTTPServer::RegisterObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS_InitClass(cx, global, JS::NullPtr(), &HTTPServer_class,
        nidium_HTTPServer_constructor,
        0, NULL, NULL, NULL, NULL);
#if 0
    //TODO: how to init a class from a NidiumJSObjectMapper derived class
    JS_InitClass(cx, global, NULL, &HTTPRequest_class,
                nidium_HTTPRequest_class_constructor,
                0, HTTPRequest_props, HTTPRequest_funcs, NULL, NULL);
#endif
}
// }}}

} // namespace Binding
} // namespace Nidium

