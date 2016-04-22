/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "JSHTTP.h"
#include "JSUtils.h"

using Nidium::Net::HTTP;
using Nidium::Net::HTTPRequest;

namespace Nidium {
namespace Binding {

// {{{ Preamble
#define SET_PROP(where, name, val) JS_DefineProperty(cx, where, \
    (const char *)name, val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | \
        JSPROP_ENUMERATE)

static bool nidium_http_request(JSContext *cx, unsigned argc, JS::Value *vp);
static void Http_Finalize(JSFreeOp *fop, JSObject *obj);

static JSClass HTTP_class = {
    "Http", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Http_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *JSExposer<JSHTTP>::jsclass = &HTTP_class;

static JSFunctionSpec http_funcs[] = {
    JS_FN("request", nidium_http_request, 2, NIDIUM_JS_FNPROPS),
    JS_FS_END
};
// }}}

// {{{ Implementation
static bool nidium_HTTP_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString url(cx);
    HTTP *nhttp;
    JSHTTP *jshttp;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!args.isConstructing()) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &HTTP_class, args));

    if (!JS_ConvertArguments(cx, args, "S", url.address())) {
        return false;
    }

    JSAutoByteString curl(cx, url);

    nhttp = new HTTP((ape_global *)JS_GetContextPrivate(cx));

    jshttp = new JSHTTP(ret, cx, curl.ptr());

    nhttp->setPrivate(jshttp);
    jshttp->refHttp = nhttp;
    jshttp->jsobj = ret;

    JS_SetPrivate(ret, jshttp);

    args.rval().setObjectOrNull(ret);
    JS_DefineFunctions(cx, ret, http_funcs);

    return true;
}

static bool nidium_http_request(JSContext *cx, unsigned argc, JS::Value *vp)
{
    HTTP *nhttp;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx,  JS_THIS_OBJECT(cx, vp));
    JSHTTP *jshttp;
    JS::RootedObject options(cx);
    JS::RootedValue curopt(cx);
    JS::RootedValue callback(cx);
    HTTPRequest *req;

    NIDIUM_JS_CHECK_ARGS("request", 2);

    NIDIUM_JS_INIT_OPT();

    if (JS_InstanceOf(cx, caller, &HTTP_class, &args) == false) {
        return true;
    }

    args.rval().setObjectOrNull(caller);

    if (!JS_ConvertArguments(cx, args, "o", options.address())) {
        return false;
    }
    JS::RootedValue args1(cx, args.array()[1]);
    if (!JS_ConvertValue(cx, args1, JSTYPE_FUNCTION, &callback)) {
        return false;
    }

    if ((jshttp = (JSHTTP *)JS_GetPrivate(caller)) == NULL) {
        return true;
    }

    nhttp = jshttp->refHttp;

    if (!nhttp->canDoRequest()) {
        JS_ReportError(cx, "A request is already pending.");
        return false;
    }

    if ((req = nhttp->getRequest()) == NULL) {
        req = new HTTPRequest(jshttp->m_URL);
    } else {
        req->recycle();
    }

    if (!req->isValid()) {
        JS_ReportError(cx, "Invalid URL");
        if (nhttp->getRequest() == NULL) {
            delete req;
        }
        return false;
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "method", String) {
        JS::RootedString method(cx, __curopt.toString());
        JSAutoByteString cmethod(cx, method);
        if (strcmp("POST", cmethod.ptr()) == 0) {
            req->method = HTTPRequest::HTTP_POST;
        } else if (strcmp("HEAD", cmethod.ptr()) == 0) {
            req->method = HTTPRequest::HTTP_HEAD;
        } else if (strcmp("PUT", cmethod.ptr()) == 0) {
            req->method = HTTPRequest::HTTP_PUT;
        } else if (strcmp("DELETE", cmethod.ptr()) == 0) {
            req->method = HTTPRequest::HTTP_DELETE;
        }  else {
            req->method = HTTPRequest::HTTP_GET;
        }
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "headers", Object) {
        if (!__curopt.isPrimitive()) {

            JS::RootedObject headers(cx, __curopt.toObjectOrNull());
            JS::AutoIdArray ida(cx, JS_Enumerate(cx, headers));

            for (size_t i = 0; i < ida.length(); i++) {
                JS::RootedId id(cx, ida[i]);
                JS::RootedValue idval(cx);
                JS_IdToValue(cx, id, &idval);

                JS::RootedString idstr(cx, idval.toString());

                if (idstr == NULL) {
                    continue;
                }

                JSAutoByteString cidstr(cx, idstr);

                JS_GetPropertyById(cx, headers, id, &idval);

                idstr = idval.toString() ;
                if (idstr == NULL) {

                    continue;
                }
                JSAutoByteString cvalstr(cx, idstr);

                req->setHeader(cidstr.ptr(), cvalstr.ptr());
            }
        }
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "data", String) {
        /* TODO: handle ArrayBuffer */
        JS::RootedString data(cx, __curopt.toString());
        if (data != NULL) {
            char *hdata = JS_EncodeStringToUTF8(cx, data);
            req->setData(hdata, strlen(hdata));
            req->setDataReleaser(js_free);

            if (req->method != HTTPRequest::HTTP_PUT) {
                req->method = HTTPRequest::HTTP_POST;
            }

            char num[16];
            sprintf(num, "%zu", req->getDataLength());
            req->setHeader("Content-Length", num);
        }
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "timeout", Number) {
        nhttp->m_TimeoutTimer = __curopt.toInt32();
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "maxredirect", Number) {

        uint32_t max = 8;
        max = __curopt.toInt32();

        nhttp->setMaxRedirect(max);

    }

    NIDIUM_JS_GET_OPT_TYPE(options, "followlocation", Boolean) {
        nhttp->setFollowLocation(__curopt.toBoolean());
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "eval", Boolean) {
        jshttp->m_Eval = __curopt.toBoolean();
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "path", String) {

        JSAutoByteString cstr(cx, __curopt.toString());

        req->setPath(cstr.ptr());

    }

    jshttp->request = callback;
    JS_SetReservedSlot(caller, 0, callback);

    NidiumJSObj(cx)->rootObjectUntilShutdown(caller);

    //printf("Request : %s\n", req->getHeadersData()->data);

    if (!nhttp->request(req, jshttp)) {
        JS_ReportError(cx, "Failed to exec request");
        return false;
    }

    return true;
}

static void Http_Finalize(JSFreeOp *fop, JSObject *obj)
{
    JSHTTP *jshttp = (JSHTTP *)JS_GetPrivate(obj);

    if (jshttp != NULL) {
        delete jshttp;
    }
}
// }}}

// {{{ JSHTTP
void JSHTTP::onError(HTTP::HTTPError err)
{
    JSContext *cx = m_Cx;
    JS::RootedValue rval(cx);
    JS::RootedValue onerror_callback(cx);
    JS::RootedValue jevent(cx);
    JS::RootedObject obj(cx, jsobj);

    if (!JS_GetProperty(m_Cx, obj, "onerror", &onerror_callback) ||
            JS_TypeOfValue(m_Cx, onerror_callback) != JSTYPE_FUNCTION) {

        return;
    }

    JS::AutoValueArray<1> event(cx);
    JS::RootedObject evobj(cx, JS_NewObject(m_Cx, NULL, JS::NullPtr(), JS::NullPtr()));
    event[0].setObject(*evobj);

    switch(err) {
        case HTTP::ERROR_RESPONSE:
            NIDIUM_JSOBJ_SET_PROP_CSTR(evobj, "error", "http_invalid_response");
            break;
        case HTTP::ERROR_DISCONNECTED:
            NIDIUM_JSOBJ_SET_PROP_CSTR(evobj, "error", "http_server_disconnected");
            break;
        case HTTP::ERROR_SOCKET:
            NIDIUM_JSOBJ_SET_PROP_CSTR(evobj, "error", "http_connection_error");
            break;
        case HTTP::ERROR_TIMEOUT:
            NIDIUM_JSOBJ_SET_PROP_CSTR(evobj, "error", "http_timedout");
            break;
        case HTTP::ERROR_HTTPCODE:
            NIDIUM_JSOBJ_SET_PROP_CSTR(evobj, "error", "http_response_code");
            break;
        case HTTP::ERROR_REDIRECTMAX:
            NIDIUM_JSOBJ_SET_PROP_CSTR(evobj, "error", "http_max_redirect_exceeded");
            break;
        default:
            break;
    }

    JS_CallFunctionValue(cx, obj, onerror_callback, event, &rval);

    NidiumJSObj(cx)->unrootObject(this->jsobj);
}

void JSHTTP::onProgress(size_t offset, size_t len,
    HTTP::HTTPData *h, HTTP::DataType type)
{
    JSContext *cx = m_Cx;
    JS::RootedValue rval(cx);
    JS::RootedValue ondata_callback(cx);
    JS::RootedValue jdata(cx);
    JS::AutoValueArray<1> jevent(cx);
    JS::RootedObject obj(cx, jsobj);

    if (!JS_GetProperty(cx, obj, "ondata", &ondata_callback) ||
            JS_TypeOfValue(cx, ondata_callback) != JSTYPE_FUNCTION) {
        return;
    }

    JS::RootedObject event(cx, JS_NewObject(cx, NULL, JS::NullPtr(), JS::NullPtr()));

    NIDIUM_JSOBJ_SET_PROP(event, "total", (double)h->contentlength);
    NIDIUM_JSOBJ_SET_PROP(event, "read", (double)(offset + len));

    switch(type) {
        case HTTP::DATA_JSON:
        case HTTP::DATA_STRING:

            NIDIUM_JSOBJ_SET_PROP_CSTR(event, "type", "string");

            jdata.setString(JS_NewStringCopyN(cx,
                (const char *)&h->data->data[offset], len));

            break;
        default:
        {
            JS::RootedObject arr(cx, JS_NewArrayBuffer(cx, len));
            uint8_t *data = JS_GetArrayBufferData(arr);

            memcpy(data, &h->data->data[offset], len);

            NIDIUM_JSOBJ_SET_PROP_CSTR(event, "type", "binary");

            jdata.setObject(*arr);

            break;
        }
    }

    NIDIUM_JSOBJ_SET_PROP(event, "data", jdata);

    jevent[0].setObject(*event);

    JS_CallFunctionValue(cx, obj, ondata_callback, jevent, &rval);
}

void JSHTTP::onRequest(HTTP::HTTPData *h, HTTP::DataType type)
{
    buffer *k, *v;

    JSContext *cx = m_Cx;

    JSAutoRequest ar(m_Cx);

    JS::RootedObject event(cx, JS_NewObject(m_Cx, NULL, JS::NullPtr(), JS::NullPtr()));
    JS::RootedObject headers(cx, JS_NewObject(m_Cx, NULL, JS::NullPtr(), JS::NullPtr()));
    JS::RootedObject obj(cx, jsobj);
    JS::RootedValue jdata(cx);
    JS::RootedValue rval(cx);
    JS::AutoValueArray<1> jevent(cx);

    jdata.setNull();

    APE_A_FOREACH(h->headers.list, k, v) {
        JS::RootedString jstr(m_Cx, JS_NewStringCopyN(m_Cx, (char *)v->data,
            v->used-1));
        NIDIUM_JSOBJ_SET_PROP_FLAGS(headers, k->data, jstr, JSPROP_ENUMERATE);
    }

    NIDIUM_JSOBJ_SET_PROP(event, "headers", headers);
    NIDIUM_JSOBJ_SET_PROP(event, "statusCode", h->parser.status_code);

    if (h->data == NULL) {
        NIDIUM_JSOBJ_SET_PROP(event, "data", JS::NullHandleValue);

        jevent[0].setObject(*event);
        NIDIUM_JSOBJ_SET_PROP_CSTR(event, "type", "null");

        JS::RootedValue req(cx, request);
        JS_CallFunctionValue(cx, obj, req, jevent, &rval);

        NidiumJSObj(cx)->unrootObject(this->jsobj);

        JS_SetReservedSlot(jsobj, 0, JSVAL_NULL);

        return;
    }

    if (!m_Eval) {
        type = HTTP::DATA_STRING;
    }

    switch(type) {
        case HTTP::DATA_STRING:
            NIDIUM_JSOBJ_SET_PROP_CSTR(event, "type", "string");

            JSUtils::strToJsval(cx, (const char *)h->data->data,
                h->data->used, &jdata, "utf8");
            break;
        case HTTP::DATA_JSON:
        {

            const jschar *chars;
            size_t clen;
            NIDIUM_JSOBJ_SET_PROP_CSTR(event, "type", "json");

            JS::RootedString str(cx, JS_NewStringCopyN(cx, (const char *)h->data->data,
                h->data->used));
            if (str == NULL) {
                printf("Cant encode json string\n");
                break;
            }
            chars = JS_GetStringCharsZAndLength(cx, str, &clen);

            if (JS_ParseJSON(cx, chars, clen, &jdata) == false) {
                jdata.setNull();
                printf("Cant parse JSON of size %ld :\n = %.*s\n = \n",
                    (unsigned long) h->data->used, (int)h->data->used, h->data->data);
            }

            break;
        }
#if 0
        case HTTP::DATA_IMAGE:
        {
            NativeSkImage *nimg;
            SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("image"))));

            nimg = new NativeSkImage(h->data->data, h->data->used);
            jdata = OBJECT_TO_JSVAL(NidiumJSImage::buildImageObject(cx, nimg));

            break;
        }
        case HTTP::DATA_AUDIO:
        {
            JSObject *arr = JS_NewArrayBuffer(cx, h->data->used);
            uint8_t *data = JS_GetArrayBufferData(arr);

            memcpy(data, h->data->data, h->data->used);

            SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("audio"))));

            jdata = OBJECT_TO_JSVAL(arr);

            break;
        }
#endif
        default:
        {
            JS::RootedObject arr(cx, JS_NewArrayBuffer(cx, h->data->used));
            uint8_t *data = JS_GetArrayBufferData(arr);

            memcpy(data, h->data->data, h->data->used);

            NIDIUM_JSOBJ_SET_PROP_CSTR(event, "type", "binary");

            jdata.setObject(*arr);

            break;
        }
    }

    NIDIUM_JSOBJ_SET_PROP(event, "data", jdata);

    jevent[0].setObject(*event);

    JS::RootedValue req(cx, request);
    JS_CallFunctionValue(cx, obj, req, jevent, &rval);

    NidiumJSObj(cx)->unrootObject(this->jsobj);
    JS_SetReservedSlot(jsobj, 0, JSVAL_NULL);
}

JSHTTP::JSHTTP(JS::HandleObject obj, JSContext *cx, char *url) :
    JSExposer<JSHTTP>(obj, cx),
    request(JSVAL_NULL), refHttp(NULL), m_Eval(true)
{
    m_URL = strdup(url);
}

JSHTTP::~JSHTTP()
{
    if (refHttp) {
        delete refHttp;
    }
    free(m_URL);
}
// }}}

// {{{ Registration
NIDIUM_JS_OBJECT_EXPOSE(HTTP)
// }}}

} // namespace Binding
} // namespace Nidium

