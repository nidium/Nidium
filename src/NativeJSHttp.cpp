#include "NativeJSHttp.h"
//#include "ape_http_parser.h"
#include "NativeSkImage.h"
#include "NativeJSImage.h"
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

    JSObject *ret = JS_NewObjectForConstructor(cx, &Http_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &url)) {
        return JS_TRUE;
    }

    JSAutoByteString curl(cx, url);

    nhttp = new NativeHTTP(new NativeHTTPRequest(curl.ptr()),
        (ape_global *)JS_GetContextPrivate(cx));

    jshttp = new NativeJSHttp();
    nhttp->setPrivate(jshttp);
    jshttp->refHttp = nhttp;
    jshttp->jsobj = ret;

    jshttp->cx = cx;

    /* TODO: store jshttp intead of nhttp */
    JS_SetPrivate(ret, nhttp);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));
    JS_DefineFunctions(cx, ret, http_funcs);

    return JS_TRUE;
}


static JSBool native_http_request(JSContext *cx, unsigned argc, jsval *vp)
{
#define GET_OPT(name) if (JS_GetProperty(cx, options, name, &curopt) && curopt != JSVAL_VOID)
    jsval callback;
    NativeHTTP *nhttp;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    NativeJSHttp *jshttp;
    JSObject *options = NULL;
    jsval curopt;

    if (JS_InstanceOf(cx, caller, &Http_class, JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(caller));

    if (!JS_ConvertArguments(cx, 1, JS_ARGV(cx, vp), "o", &options)) {
        return JS_TRUE;
    }

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[1], JSTYPE_FUNCTION, &callback)) {
        return JS_TRUE;
    }

    if ((nhttp = (NativeHTTP *)JS_GetPrivate(caller)) == NULL) {
        return JS_TRUE;
    }

    NativeHTTPRequest *req = nhttp->getRequest();

    GET_OPT("method") {
        JSString *method = JSVAL_TO_STRING(curopt);
        JSAutoByteString cmethod(cx, method);
        if (strcmp("POST", cmethod.ptr()) == 0) {
            req->method = NativeHTTPRequest::NATIVE_HTTP_POST;
        } else {
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
        JSString *data = JS_ValueToString(cx, curopt);
        if (data != NULL) {
            char *hdata = JS_EncodeString(cx, data);
            req->setData(hdata, strlen(hdata));
            req->setDataReleaser(js_free);

            req->method = NativeHTTPRequest::NATIVE_HTTP_POST;
            char num[16];
            sprintf(num, "%ld", req->getDataLength());
            req->setHeader("Content-Length", num);
        }
    }

    GET_OPT("timeout") {
        if (JSVAL_IS_NUMBER(curopt)) {
            JS_ValueToECMAUint32(cx, curopt, &nhttp->timeout);
        }
    }

    jshttp = (NativeJSHttp *)nhttp->getPrivate();
    jshttp->request = callback;
    JS_SetReservedSlot(caller, 0, callback);

    NativeJSObj(cx)->rootObjectUntilShutdown(caller);

    printf("Request : %s\n", req->getHeadersData()->data);

    nhttp->request(jshttp);

    return JS_TRUE;
}

void NativeJSHttp::onError(NativeHTTP::HTTPError err)
{
    jsval onerror_callback, jevent, rval;
    JSObject *event;

    if (!JS_GetProperty(cx, jsobj, "onerror", &onerror_callback) ||
            JS_TypeOfValue(cx, onerror_callback) != JSTYPE_FUNCTION) {

        return;
    }

    event = JS_NewObject(cx, NULL, NULL, NULL);

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
    }

    jevent = OBJECT_TO_JSVAL(event);

    JS_CallFunctionValue(cx, jsobj, onerror_callback,
        1, &jevent, &rval);    

}

void NativeJSHttp::onProgress(size_t offset, size_t len,
    NativeHTTP::HTTPData *h, NativeHTTP::DataType type)
{
    JSObject *event;
    jsval jdata, jevent, ondata_callback, rval;

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

    JSAutoRequest ar(cx);

    event = JS_NewObject(cx, NULL, NULL, NULL);
    headers = JS_NewObject(cx, NULL, NULL, NULL);

    APE_A_FOREACH(h->headers.list, k, v) {
        JSString *jstr = JS_NewStringCopyN(cx, (char *)v->data,
            v->used-1);
        SET_PROP(headers, k->data, STRING_TO_JSVAL(jstr));
    }
    
    SET_PROP(event, "headers", OBJECT_TO_JSVAL(headers));

    if (h->data == NULL) {
        SET_PROP(event, "data", JSVAL_NULL);

        jevent = OBJECT_TO_JSVAL(event);
        SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
            CONST_STR_LEN("null"))));

        JS_CallFunctionValue(cx, jsobj, request,
            1, &jevent, &rval);

        NativeJSObj(cx)->unrootObject(this->jsobj);

        return; 
    }

    switch(type) {
        case NativeHTTP::DATA_STRING:
            SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("string"))));
            jdata = STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                (const char *)h->data->data, h->data->used));
            break;
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

            if (JS_ParseJSON(cx, chars, clen, &jdata) == JS_FALSE) {
                jdata = JSVAL_NULL;
                printf("Cant JSON parse\n");
            }

            break;
        }
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
}

NativeJSHttp::NativeJSHttp()
    : request(JSVAL_NULL), refHttp(NULL)
{
}

NativeJSHttp::~NativeJSHttp()
{
    if (refHttp) {
        delete refHttp;
    }
}

NATIVE_OBJECT_EXPOSE(Http)
