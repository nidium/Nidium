/*
    NativeJS Core Library
    Copyright (C) 2013 Anthony Catel <paraboul@gmail.com>
    Copyright (C) 2013 Nicolas Trani <n.trani@weelya.com>

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

#include "NativeJSHttp.h"
#include "NativeJSUtils.h"
//#include "ape_http_parser.h"

#include "NativeJS.h"

#define SET_PROP(where, name, val) JS_DefineProperty(cx, where, \
    (const char *)name, val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | \
        JSPROP_ENUMERATE)

static bool native_http_request(JSContext *cx, unsigned argc, JS::Value *vp);
static void Http_Finalize(JSFreeOp *fop, JSObject *obj);

static JSClass Http_class = {
    "Http", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Http_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *NativeJSExposer<NativeJSHttp>::jsclass = &Http_class;


static JSFunctionSpec http_funcs[] = {
    JS_FN("request", native_http_request, 2, 0),
    JS_FS_END
};

static void Http_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeHTTP *nhttp = (NativeHTTP *)JS_GetPrivate(obj);
    if (nhttp != NULL) {
        NativeJSHttp *jshttp = (NativeJSHttp *)nhttp->getPrivate();
        if (jshttp != NULL) {
            delete jshttp;
        }
    }
}

static bool native_Http_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString url(cx);
    NativeHTTP *nhttp;
    NativeJSHttp *jshttp;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!args.isConstructing()) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &Http_class, vp));

    if (!JS_ConvertArguments(cx, args, "S", &url)) {
        return false;
    }

    JSAutoByteString curl(cx, url);

    nhttp = new NativeHTTP((ape_global *)JS_GetContextPrivate(cx));

    jshttp = new NativeJSHttp(ret, cx, curl.ptr());

    nhttp->setPrivate(jshttp);
    jshttp->refHttp = nhttp;
    jshttp->jsobj = ret;

    /* TODO: store jshttp intead of nhttp */
    JS_SetPrivate(ret, nhttp);

    args.rval().setObjectOrNull(ret);
    JS_DefineFunctions(cx, ret, http_funcs);

    return true;
}


static bool native_http_request(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativeHTTP *nhttp;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx,  &args.thisv().toObject());
    NativeJSHttp *jshttp;
    JS::RootedObject options(cx);
    JS::RootedValue curopt(cx);
    JS::RootedValue callback(cx);
    NativeHTTPRequest *req;

    NATIVE_CHECK_ARGS("request", 2);

    JS_INITOPT();

    if (JS_InstanceOf(cx, caller, &Http_class, &args) == false) {
        return true;
    }

    args.rval().setObjectOrNull(caller);

    if (!JS_ConvertArguments(cx, args, "o", &options)) {
        return false;
    }
    JS::RootedValue args1(cx, args.array()[1]);
    if (!JS_ConvertValue(cx, args1, JSTYPE_FUNCTION, &callback)) {
        return false;
    }

    if ((nhttp = (NativeHTTP *)JS_GetPrivate(caller)) == NULL) {
        return true;
    }

    if (!nhttp->canDoRequest()) {
        JS_ReportError(cx, "A request is already pending.");
        return false;
    }

    jshttp = (NativeJSHttp *)nhttp->getPrivate();

    if ((req = nhttp->getRequest()) == NULL) {
        req = new NativeHTTPRequest(jshttp->m_URL);
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

    JSGET_OPT_TYPE(options, "method", String) {
        JS::RootedString method(cx, __curopt.toString());
        JSAutoByteString cmethod(cx, method);
        if (strcmp("POST", cmethod.ptr()) == 0) {
            req->method = NativeHTTPRequest::NATIVE_HTTP_POST;
        } else if (strcmp("HEAD", cmethod.ptr()) == 0) {
            req->method = NativeHTTPRequest::NATIVE_HTTP_HEAD;
        } else if (strcmp("PUT", cmethod.ptr()) == 0) {
            req->method = NativeHTTPRequest::NATIVE_HTTP_PUT;
        } else if (strcmp("DELETE", cmethod.ptr()) == 0) {
            req->method = NativeHTTPRequest::NATIVE_HTTP_DELETE;
        }  else {
            req->method = NativeHTTPRequest::NATIVE_HTTP_GET;
        }
    }

    JSGET_OPT_TYPE(options, "headers", Object) {
        if (!__curopt.isPrimitive()) {

            JS::RootedObject headers(cx, __curopt.toObjectOrNull());
            JS::AutoIdArray ida(cx, JS_Enumerate(cx, headers));

            for (size_t i = 0; i < ida.length(); i++) {
                JS::RootedId id(cx, ida[i]);
                JS::RootedValue idval(cx);
                JS_IdToValue(cx, *id.get(), &idval);

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

    JSGET_OPT_TYPE(options, "data", String) {
        /* TODO: handle ArrayBuffer */
        JS::RootedString data(cx, __curopt.toString());
        if (data != NULL) {
            char *hdata = JS_EncodeStringToUTF8(cx, data);
            req->setData(hdata, strlen(hdata));
            req->setDataReleaser(js_free);

            if (req->method != NativeHTTPRequest::NATIVE_HTTP_PUT) {
                req->method = NativeHTTPRequest::NATIVE_HTTP_POST;
            }

            char num[16];
            sprintf(num, "%zu", req->getDataLength());
            req->setHeader("Content-Length", num);
        }
    }

    JSGET_OPT_TYPE(options, "timeout", Number) {
        nhttp->m_TimeoutTimer = __curopt.toInt32();
    }

    JSGET_OPT_TYPE(options, "maxredirect", Number) {

        uint32_t max = 8;
        max = __curopt.toInt32();

        nhttp->setMaxRedirect(max);
      
    }

    JSGET_OPT_TYPE(options, "eval", Boolean) {
        jshttp->m_Eval = __curopt.toBoolean();
    }

    JSGET_OPT_TYPE(options, "path", String) {

        JSAutoByteString cstr(cx, __curopt.toString());

        req->setPath(cstr.ptr());

    }

    jshttp->request = callback;
    JS_SetReservedSlot(caller, 0, callback);

    NativeJSObj(cx)->rootObjectUntilShutdown(caller);

    //printf("Request : %s\n", req->getHeadersData()->data);

    if (!nhttp->request(req, jshttp)) {
        JS_ReportError(cx, "Failed to exec request");
        return false;
    }

    return true;
}

void NativeJSHttp::onError(NativeHTTP::HTTPError err)
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

    JS::AutoValueArray<2> event(cx);
    event[0] = JS_NewObject(m_Cx, NULL, JS::NullPtr(), JS::NullPtr());

    switch(err) {
        case NativeHTTP::ERROR_RESPONSE:
            SET_PROP(event, "error", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("http_invalid_response"))));            
            break;
        case NativeHTTP::ERROR_DISCONNECTED:
            SET_PROP(event, "error", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("http_server_disconnected"))));      
            break;
        case NativeHTTP::ERROR_SOCKET:
            SET_PROP(event, "error", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("http_connection_error"))));      
            break;
        case NativeHTTP::ERROR_TIMEOUT:
            SET_PROP(event, "error", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("http_timedout"))));      
            break;
        case NativeHTTP::ERROR_HTTPCODE:
            SET_PROP(event, "error", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("http_response_code"))));      
            break;
        case NativeHTTP::ERROR_REDIRECTMAX:
            SET_PROP(event, "error", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("http_max_redirect_exceeded"))));      
            break;
        default:
            break;
    }

    jevent.setObject(event);

    JS_CallFunctionValue(cx, obj, onerror_callback, event, &rval);

    NativeJSObj(cx)->unrootObject(this->jsobj);
}

void NativeJSHttp::onProgress(size_t offset, size_t len,
    NativeHTTP::HTTPData *h, NativeHTTP::DataType type)
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

    SET_PROP(event, "total", DOUBLE_TO_JSVAL(h->contentlength));
    SET_PROP(event, "read", DOUBLE_TO_JSVAL(offset + len));

    switch(type) {
        case NativeHTTP::DATA_JSON:
        case NativeHTTP::DATA_STRING:
            SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("string"))));

            jdata.setString(JS_NewStringCopyN(cx,
                (const char *)&h->data->data[offset], len));
         
            break;
        default:
        {
            JS::RootedObject arr(cx, JS_NewArrayBuffer(cx, len));
            uint8_t *data = JS_GetArrayBufferData(arr);

            memcpy(data, &h->data->data[offset], len);
            
            SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("binary"))));

            jdata.setObject(*arr);

            break;
        }
    }
    
    SET_PROP(event, "data", jdata);

    jevent[1] = event;

    JS_CallFunctionValue(cx, obj, ondata_callback, jevent, &rval);
}

void NativeJSHttp::onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType type)
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
        JSOBJ_SET_PROP_FLAGS(headers, k->data,
            STRING_TO_JSVAL(jstr), JSPROP_ENUMERATE);
    }
    
    SET_PROP(event, "headers", OBJECT_TO_JSVAL(headers));
    SET_PROP(event, "statusCode", INT_TO_JSVAL(h->parser.status_code));

    if (h->data == NULL) {
        SET_PROP(event, "data", JSVAL_NULL);

        jevent[0] = OBJECT_TO_JSVAL(event);
        SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
            CONST_STR_LEN("null"))));
        JS::RootedValue req(cx, request);
        JS_CallFunctionValue(cx, obj, req, jevent, &rval);

        NativeJSObj(cx)->unrootObject(this->jsobj);

        JS_SetReservedSlot(jsobj, 0, JSVAL_NULL);

        return; 
    }

    if (!m_Eval) {
        type = NativeHTTP::DATA_STRING;
    }

    switch(type) {
        case NativeHTTP::DATA_STRING:
            SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("string"))));

            NativeJSUtils::strToJsval(cx, (const char *)h->data->data,
                h->data->used, &jdata, "utf8");
            break;
        case NativeHTTP::DATA_JSON:
        {

            const jschar *chars;
            size_t clen;
            SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("json"))));

            JS::RootedString str(cx, JS_NewStringCopyN(cx, (const char *)h->data->data,
                h->data->used));
            if (str == NULL) {
                printf("Cant encode json string\n");
                break;
            }
            chars = JS_GetStringCharsZAndLength(cx, str, &clen);

            if (JS_ParseJSON(cx, chars, clen, &jdata) == false) {
                jdata.setNull();
                printf("Cant parse JSON of size %ld :\n===%.*s\n===\n",
                    (unsigned long) h->data->used, (int)h->data->used, h->data->data);
            }

            break;
        }
#if 0
        case NativeHTTP::DATA_IMAGE:
        {
            NativeSkImage *nimg;
            SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("image"))));

            nimg = new NativeSkImage(h->data->data, h->data->used);
            jdata = OBJECT_TO_JSVAL(NativeJSImage::buildImageObject(cx, nimg));

            break;
        }
        case NativeHTTP::DATA_AUDIO:
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
            
            SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("binary"))));

            jdata.setObject(*arr);
 
            break;
        }
    }

    SET_PROP(event, "data", jdata);

    jevent[0] = event;
    JS::RootedValue req(cx, request);
    JS::RootedObject obj(cx, jsobj);
    JS_CallFunctionValue(cx, obj, req, jevent, &rval);

    NativeJSObj(cx)->unrootObject(this->jsobj);
    JS_SetReservedSlot(jsobj, 0, JSVAL_NULL);
}

NativeJSHttp::NativeJSHttp(JSObject *obj, JSContext *cx, char *url) :
    NativeJSExposer<NativeJSHttp>(obj, cx),
    request(JSVAL_NULL), refHttp(NULL), m_Eval(true)
{
    m_URL = strdup(url);
}

NativeJSHttp::~NativeJSHttp()
{
    if (refHttp) {
        delete refHttp;
    }
    free(m_URL);
}

NATIVE_OBJECT_EXPOSE(Http)
