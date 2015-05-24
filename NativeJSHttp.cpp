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

static JSBool native_http_request(JSContext *cx, unsigned argc, jsval *vp);
static void Http_Finalize(JSFreeOp *fop, JSObject *obj);

static JSClass Http_class = {
    "Http", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Http_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
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

static JSBool native_Http_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *url;
    NativeHTTP *nhttp;
    NativeJSHttp *jshttp;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!JS_IsConstructing(cx, vp)) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    JSObject *ret = JS_NewObjectForConstructor(cx, &Http_class, vp);

    if (!JS_ConvertArguments(cx, argc, args.array(), "S", &url)) {
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

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));
    JS_DefineFunctions(cx, ret, http_funcs);

    return true;
}


static JSBool native_http_request(JSContext *cx, unsigned argc, jsval *vp)
{
#define GET_OPT(name) if (JS_GetProperty(cx, options, name, &curopt) && curopt != JSVAL_VOID && curopt != JSVAL_NULL)
    jsval callback;
    NativeHTTP *nhttp;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject *caller = &args.thisv().toObject();
    NativeJSHttp *jshttp;
    JSObject *options = NULL;
    jsval curopt;
    NativeHTTPRequest *req;

    NATIVE_CHECK_ARGS("request", 2);

    if (JS_InstanceOf(cx, caller, &Http_class, args.array()) == false) {
        return true;
    }

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(caller));

    if (!JS_ConvertArguments(cx, 1, args.array(), "o", &options)) {
        return false;
    }

    if (!JS_ConvertValue(cx, args.array()[1], JSTYPE_FUNCTION, &callback)) {
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

    GET_OPT("method") {
        JSString *method = JSVAL_TO_STRING(curopt);
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

    GET_OPT("headers") {
        if (!JSVAL_IS_PRIMITIVE(curopt)) {
            JSObject *headers = JSVAL_TO_OBJECT(curopt);
            JS::AutoIdArray ida(cx, JS_Enumerate(cx, headers));

            for (size_t i = 0; i < ida.length(); i++) {
                JS::Rooted<jsid> id(cx, ida[i]);
                jsval idval;
                JS_IdToValue(cx, id.get(), &idval);

                JSString *idstr = JS_ValueToString(cx, idval);

                if (idstr == NULL) {
                    continue;
                }

                JSAutoByteString cidstr(cx, idstr);

                JS_GetPropertyById(cx, headers, id.get(), &idval);
                idstr = JS_ValueToString(cx, idval);
                if (idstr == NULL) {

                    continue;
                }
                JSAutoByteString cvalstr(cx, idstr);

                req->setHeader(cidstr.ptr(), cvalstr.ptr());
            }
        }
    }

    GET_OPT("data") {
        /* TODO: handle ArrayBuffer */
        JSString *data = JS_ValueToString(cx, curopt);
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

    GET_OPT("timeout") {
        if (JSVAL_IS_NUMBER(curopt)) {
            JS_ValueToECMAUint32(cx, curopt, &nhttp->m_Timeout);
        }
    }

    GET_OPT("maxredirect") {
        uint32_t max = 8;
        if (JSVAL_IS_NUMBER(curopt)) {
            JS_ValueToECMAUint32(cx, curopt, &max);

            nhttp->setMaxRedirect(max);
        }        
    }

    GET_OPT("eval") {
        if (curopt.isBoolean()) {
            jshttp->m_Eval = curopt.toBoolean();
        }
    }

    GET_OPT("path") {
        if (curopt.isString()) {
            JSAutoByteString cstr(cx, curopt.toString());

            req->setPath(cstr.ptr());
        }
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
    jsval onerror_callback, jevent, rval;
    JSObject *event;
    JSContext *cx = m_Cx;

    if (!JS_GetProperty(m_Cx, jsobj, "onerror", &onerror_callback) ||
            JS_TypeOfValue(m_Cx, onerror_callback) != JSTYPE_FUNCTION) {

        return;
    }

    event = JS_NewObject(m_Cx, NULL, NULL, NULL);

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

    jevent = OBJECT_TO_JSVAL(event);

    JS_CallFunctionValue(cx, jsobj, onerror_callback,
        1, &jevent, &rval);    

    NativeJSObj(cx)->unrootObject(this->jsobj);
}

void NativeJSHttp::onProgress(size_t offset, size_t len,
    NativeHTTP::HTTPData *h, NativeHTTP::DataType type)
{
    JSObject *event;
    jsval jdata, jevent, ondata_callback, rval;
    JSContext *cx = m_Cx;

    if (!JS_GetProperty(cx, jsobj, "ondata", &ondata_callback) ||
            JS_TypeOfValue(cx, ondata_callback) != JSTYPE_FUNCTION) {
        return;
    }

    event = JS_NewObject(cx, NULL, NULL, NULL);

    SET_PROP(event, "total", DOUBLE_TO_JSVAL(h->contentlength));
    SET_PROP(event, "read", DOUBLE_TO_JSVAL(offset + len));

    switch(type) {
        case NativeHTTP::DATA_JSON:
        case NativeHTTP::DATA_STRING:
            SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("string"))));
            jdata = STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                (const char *)&h->data->data[offset], len));            
            break;
        default:
        {
            JSObject *arr = JS_NewArrayBuffer(cx, len);
            uint8_t *data = JS_GetArrayBufferData(arr);

            memcpy(data, &h->data->data[offset], len);
            
            SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("binary"))));
            jdata = OBJECT_TO_JSVAL(arr);
            break;
        }
    }
    
    SET_PROP(event, "data", jdata);

    jevent = OBJECT_TO_JSVAL(event);

    JS_CallFunctionValue(cx, jsobj, ondata_callback,
        1, &jevent, &rval);
}

void NativeJSHttp::onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType type)
{
    buffer *k, *v;
    JSObject *headers, *event;
    jsval rval, jevent, jdata = JSVAL_NULL;
    JSContext *cx = m_Cx;

    JSAutoRequest ar(m_Cx);

    event = JS_NewObject(m_Cx, NULL, NULL, NULL);
    headers = JS_NewObject(m_Cx, NULL, NULL, NULL);

    APE_A_FOREACH(h->headers.list, k, v) {
        JSString *jstr = JS_NewStringCopyN(m_Cx, (char *)v->data,
            v->used-1);
        JSOBJ_SET_PROP_FLAGS(headers, k->data,
            STRING_TO_JSVAL(jstr), JSPROP_ENUMERATE);
    }
    
    SET_PROP(event, "headers", OBJECT_TO_JSVAL(headers));
    SET_PROP(event, "statusCode", INT_TO_JSVAL(h->parser.status_code));

    if (h->data == NULL) {
        SET_PROP(event, "data", JSVAL_NULL);

        jevent = OBJECT_TO_JSVAL(event);
        SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
            CONST_STR_LEN("null"))));

        JS_CallFunctionValue(cx, jsobj, request,
            1, &jevent, &rval);


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
            JSString *str;
            const jschar *chars;
            size_t clen;
            SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("json"))));

            str = JS_NewStringCopyN(cx, (const char *)h->data->data,
                h->data->used);
            if (str == NULL) {
                printf("Cant encode json string\n");
                break;
            }
            chars = JS_GetStringCharsZAndLength(cx, str, &clen);

            if (JS_ParseJSON(cx, chars, clen, &jdata) == false) {
                jdata = JSVAL_NULL;
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
            JSObject *arr = JS_NewArrayBuffer(cx, h->data->used);
            uint8_t *data = JS_GetArrayBufferData(arr);

            memcpy(data, h->data->data, h->data->used);
            
            SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("binary"))));
            jdata = OBJECT_TO_JSVAL(arr);
            break;
        }
    }

    SET_PROP(event, "data", jdata);

    jevent = OBJECT_TO_JSVAL(event);

    JS_CallFunctionValue(cx, jsobj, request,
        1, &jevent, &rval);

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
