/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSHTTP.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "Binding/JSUtils.h"

using Nidium::Net::HTTP;
using Nidium::Net::HTTPRequest;

namespace Nidium {
namespace Binding {

// {{{ Preamble
#define SET_PROP(where, name, val) JS_DefineProperty(cx, where, \
    (const char *)name, val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | \
        JSPROP_ENUMERATE)

static bool nidium_http_request(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_http_stop(JSContext *cx, unsigned argc, JS::Value *vp);
static void Http_Finalize(JSFreeOp *fop, JSObject *obj);

static JSClass HTTP_class = {
    "HTTP", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(2),
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Http_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *JSExposer<JSHTTP>::jsclass = &HTTP_class;

static JSFunctionSpec http_funcs[] = {
    JS_FN("request", nidium_http_request, 2, NIDIUM_JS_FNPROPS),
    JS_FN("stop", nidium_http_stop, 0, NIDIUM_JS_FNPROPS),
    JS_FS_END
};
// }}}

// {{{ Implementation
static void getOptionsAndCallback(JSContext *cx, JS::CallArgs *args,
    int argsOffset, JS::MutableHandleObject options, JS::MutableHandleValue callback)
{
    JS::RootedValue arg1(cx, args->get(argsOffset));
    JS::RootedValue arg2(cx, args->get(argsOffset + 1));

    bool arg1Callable = arg1.isObject() &&
            JS_ObjectIsCallable(cx, arg1.toObjectOrNull());
    bool arg2Callable = arg2.isObject() &&
            JS_ObjectIsCallable(cx, arg2.toObjectOrNull());

    callback.set(JS::NullHandleValue);

    if (arg1Callable) {
        // Only callback providen
        callback.set(arg1);
    } else if (arg1.isObject()) {
        // Options and maybe a callback
        options.set(arg1.toObjectOrNull());
        if (arg2Callable) {
            callback.set(arg2);
        }
    }
}

static bool nidium_HTTP_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString url(cx);
    HTTP *nhttp;
    JSHTTP *jshttp;

    NIDIUM_JS_CONSTRUCTOR_PROLOGUE()

    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &HTTP_class, args));

    if (!JS_ConvertArguments(cx, args, "S", url.address())) {
        return false;
    }

    JSAutoByteString curl(cx, url);

    nhttp = new HTTP((ape_global *)JS_GetContextPrivate(cx));

    jshttp = new JSHTTP(ret, cx, curl.ptr());

    nhttp->setPrivate(jshttp);
    jshttp->m_HTTP = nhttp;
    jshttp->m_JSObj = ret;

    JS_SetPrivate(ret, jshttp);

    args.rval().setObjectOrNull(ret);

    JS_DefineFunctions(cx, ret, http_funcs);

    HTTPRequest *req = new HTTPRequest(jshttp->m_URL);
    jshttp->m_HTTPRequest = req;

    if (!req->isValid()) {
        JS_ReportError(cx, "Invalid URL");

        jshttp->m_HTTPRequest = nullptr;
        delete req;

        return false;
    }

    // Shorthand arguments
    if (argc > 1) {
        JS::RootedObject options(cx);
        JS::RootedValue callback(cx);

        getOptionsAndCallback(cx, &args, 1, &options, &callback);

        jshttp->parseOptions(cx, options);

        if (!callback.isNull()) {
            // Have a callback, directly execute the request
            if (!jshttp->request(cx, options, callback)) {
                return false;
            }
        }
    }

    return true;
}

static bool nidium_http_request(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(JSHTTP, &HTTP_class);

    JS::RootedObject options(cx);
    JS::RootedValue callback(cx);

    callback.set(JS::NullValue());

    if (argc > 0 && args[0].isObject()) {
        options = args[0].toObjectOrNull();
    }

    if (!CppObj->request(cx, options)) {
        return false;
    }

    args.rval().setObjectOrNull(thisobj);

    return true;
}

static bool nidium_http_stop(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(JSHTTP, &HTTP_class);

    CppObj->m_HTTP->stopRequest();

    args.rval().setObjectOrNull(thisobj);

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
void JSHTTP::fireJSEvent(const char *name, JS::MutableHandleValue ev)
{
    JSExposer::fireJSEvent(name, ev);

    if (m_JSCallback.isNull()) return;

    JS::RootedObject thisobj(m_Cx, m_JSObj);
    JS::RootedValue requestCallback(m_Cx, m_JSCallback);
    JS::RootedValue jserror(m_Cx);
    JS::AutoValueArray<1> args(m_Cx);
    JS::RootedValue rval(m_Cx);

    args[0].set(ev);

    JS_CallFunctionValue(m_Cx, thisobj, requestCallback, args, &rval);

    if (JS_IsExceptionPending(m_Cx)) {
        if (!JS_ReportPendingException(m_Cx)) {
            JS_ClearPendingException(m_Cx);
        }
    }
}

bool JSHTTP::request(JSContext *cx, JS::HandleObject options, JS::HandleValue callback)
{
    HTTPRequest *req = m_HTTPRequest;

    if (!m_HTTP->canDoRequest()) {
        JS_ReportError(cx, "A request is already pending.");
        return false;
    }

    if (m_HTTP->getRequest() != nullptr) {
        req->recycle();
    }

    if (options) {
        this->parseOptions(cx, options);
    }

    if (!callback.isNull()) {
        m_JSCallback = callback;
        JS_SetReservedSlot(m_JSObj, 0, callback);
    }

    NidiumJSObj(cx)->rootObjectUntilShutdown(m_JSObj);

    if (!m_HTTP->request(req, this)) {
        JS_ReportError(cx, "Failed to exec request");
        return false;
    }

    return true;
}

void JSHTTP::parseOptions(JSContext *cx, JS::HandleObject options)
{
    NIDIUM_JS_INIT_OPT();

    NIDIUM_JS_GET_OPT_TYPE(options, "method", String) {
        JS::RootedString method(cx, __curopt.toString());
        JSAutoByteString cmethod(cx, method);
        if (strcasecmp("POST", cmethod.ptr()) == 0) {
            m_HTTPRequest->m_Method = HTTPRequest::kHTTPMethod_Post;
        } else if (strcasecmp("HEAD", cmethod.ptr()) == 0) {
            m_HTTPRequest->m_Method = HTTPRequest::kHTTPMethod_Head;
        } else if (strcasecmp("PUT", cmethod.ptr()) == 0) {
            m_HTTPRequest->m_Method = HTTPRequest::kHTTPMethod_Put;
        } else if (strcasecmp("DELETE", cmethod.ptr()) == 0) {
            m_HTTPRequest->m_Method = HTTPRequest::kHTTPMethod_Delete;
        }  else {
            m_HTTPRequest->m_Method = HTTPRequest::kHTTPMethod_Get;
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

                m_HTTPRequest->setHeader(cidstr.ptr(), cvalstr.ptr());
            }
        }
    }

    NIDIUM_JS_GET_OPT(options, "data") {
        JS::RootedObject obj(cx, __curopt.toObjectOrNull());
        char *data;
        size_t dataLen;

        if (__curopt.isObject() && JS_IsArrayBufferObject(obj)) {
            data = reinterpret_cast<char *>(JS_GetArrayBufferData(obj));
            dataLen = JS_GetArrayBufferByteLength(obj);

            // Since the data may not be used right away
            // we need to root them until they request is done
            JS_SetReservedSlot(m_JSObj, 1, __curopt);
        } else {
            JS::RootedString str(cx, JS::ToString(cx, __curopt));

            data = JS_EncodeStringToUTF8(cx, str);
            dataLen = strlen(data);

            m_HTTPRequest->setDataReleaser(js_free);
        }

        m_HTTPRequest->setData(data, dataLen);

        if (m_HTTPRequest->m_Method != HTTPRequest::kHTTPMethod_Put) {
            m_HTTPRequest->m_Method = HTTPRequest::kHTTPMethod_Post;
        }

        char num[64];
        snprintf(num, 64, "%zu", m_HTTPRequest->getDataLength());
        m_HTTPRequest->setHeader("Content-Length", num);
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "timeout", Number) {
        m_HTTP->m_TimeoutTimer = __curopt.toInt32();
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "maxRedirects", Number) {
        uint32_t max = 8;
        max = __curopt.toInt32();

        m_HTTP->setMaxRedirect(max);
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "followLocation", Boolean) {
        m_HTTP->setFollowLocation(__curopt.toBoolean());
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "eval", Boolean) {
        m_Eval = __curopt.toBoolean();
    }

    NIDIUM_JS_GET_OPT_TYPE(options, "path", String) {
        JSAutoByteString cstr(cx, __curopt.toString());
        m_HTTPRequest->setPath(cstr.ptr());
    }
}

void JSHTTP::onError(HTTP::HTTPError err)
{
    this->onError(err, HTTP::HTTPErrorDescription[err]);
}

void JSHTTP::onError(const char *error)
{
    this->onError(HTTP::_ERROR_END_ + 1, error);
}

void JSHTTP::onError(int code, const char *error)
{
    JS::RootedObject eventObject(m_Cx,
            JSEvents::CreateErrorEventObject(m_Cx, code, error));
    JS::RootedValue eventValue(m_Cx, OBJECT_TO_JSVAL(eventObject));

    this->fireJSEvent("error", &eventValue);

    JS_SetReservedSlot(m_JSObj, 0, JS::NullValue());
    JS_SetReservedSlot(m_JSObj, 1, JS::NullValue());
    NidiumJSObj(m_Cx)->unrootObject(m_JSObj);
}

void JSHTTP::onProgress(size_t offset, size_t len,
    HTTP::HTTPData *h, HTTP::DataType type)
{
    JS::RootedObject eventObject(m_Cx, JSEvents::CreateEventObject(m_Cx));
    JSObjectBuilder eventBuilder(m_Cx, eventObject);
    JS::RootedValue eventValue(m_Cx, eventBuilder.jsval());

    eventBuilder.set("total", (double)h->m_ContentLength);
    eventBuilder.set("read", (double)(offset + len));

    switch(type) {
        case HTTP::DATA_JSON:
        case HTTP::DATA_STRING:
            eventBuilder.set("type", "string");
            eventBuilder.set("data",
                    (const char *)&h->m_Data->data[offset]);
            break;
        default:
        {
            JS::RootedValue arrVal(m_Cx);
            JS::RootedObject arr(m_Cx, JS_NewArrayBuffer(m_Cx, len));
            uint8_t *data = JS_GetArrayBufferData(arr);

            arrVal.setObjectOrNull(arr);
            memcpy(data, &h->m_Data->data[offset], len);

            eventBuilder.set("type", "binary");
            eventBuilder.set("data", arrVal);
            break;
        }
    }

    JSExposer::fireJSEvent("progress", &eventValue);
}

void JSHTTP::headersToJSObject(JS::MutableHandleObject obj)
{
    buffer *k, *v;
    ape_array_t *headers = m_HTTP->m_HTTP.m_Headers.list;

    if (headers == nullptr) {
        return;
    }

    obj.set(JS_NewObject(m_Cx, NULL, JS::NullPtr(), JS::NullPtr()));

    APE_A_FOREACH(headers, k, v) {
        JS::RootedString jstr(m_Cx,
                JS_NewStringCopyN(m_Cx, reinterpret_cast<char *>(v->data),
                v->used-1));

        NIDIUM_JSOBJ_SET_PROP_FLAGS(obj, k->data, jstr, JSPROP_ENUMERATE);
    }
}

void JSHTTP::onHeader()
{
    JS::RootedObject eventObject(m_Cx, JSEvents::CreateEventObject(m_Cx));
    JSObjectBuilder eventBuilder(m_Cx, eventObject);
    JS::RootedValue eventValue(m_Cx, eventBuilder.jsval());

    JS::RootedValue headersVal(m_Cx);
    JS::RootedObject headers(m_Cx);

    this->headersToJSObject(&headers);

    headersVal.setObjectOrNull(headers);

    eventBuilder.set("headers", headersVal);
    eventBuilder.set("statusCode", m_HTTP->m_HTTP.parser.status_code);

    JSExposer::fireJSEvent("headers", &eventValue);
}

void JSHTTP::onRequest(HTTP::HTTPData *h, HTTP::DataType type)
{
    JSAutoRequest ar(m_Cx);

    JS::RootedObject eventObject(m_Cx, JSEvents::CreateEventObject(m_Cx));
    JSObjectBuilder eventBuilder(m_Cx, eventObject);
    JS::RootedValue eventValue(m_Cx, eventBuilder.jsval());
    JS::RootedValue jsdata(m_Cx);

    JS::RootedValue headersVal(m_Cx);
    JS::RootedObject headers(m_Cx);

    this->headersToJSObject(&headers);

    headersVal.setObjectOrNull(headers);

    eventBuilder.set("headers", headersVal);
    eventBuilder.set("statusCode", h->parser.status_code);

    if (h->m_Data == NULL) {
        JS::AutoValueArray<1> args(m_Cx);

        eventBuilder.set("data", JS::NullHandleValue);
        eventBuilder.set("type", JS::NullHandleValue);

        this->fireJSEvent("response", &eventValue);

        NidiumJSObj(m_Cx)->unrootObject(m_JSObj);
        JS_SetReservedSlot(m_JSObj, 0, JS::NullValue());
        JS_SetReservedSlot(m_JSObj, 1, JS::NullValue());
        return;
    }

    if (!m_Eval) {
        type = HTTP::DATA_STRING;
    }

    switch(type) {
        case HTTP::DATA_STRING:
            eventBuilder.set("type", "string");

            JSUtils::StrToJsval(m_Cx, reinterpret_cast<const char *>(h->m_Data->data),
                    h->m_Data->used, &jsdata, "utf8");
            break;
        case HTTP::DATA_JSON:
        {
            const jschar *chars;
            size_t clen;

            eventBuilder.set("type", "json");

            JS::RootedString str(m_Cx,
                    JS_NewStringCopyN(m_Cx, reinterpret_cast<const char *>(h->m_Data->data), h->m_Data->used));

            if (str == NULL) {
                this->onError("Can't encode JSON string\n");
                return;
            }

            chars = JS_GetStringCharsZAndLength(m_Cx, str, &clen);

            if (!JS_ParseJSON(m_Cx, chars, clen, &jsdata)) {
                char *err;
                asprintf(&err, "Cant parse JSON of size %ld :\n = %.*s\n = \n",
                        static_cast<unsigned long>(h->m_Data->used),
                        static_cast<int>(h->m_Data->used), h->m_Data->data);

                this->onError(err);

                free(err);

                return;
            }

            break;
        }
#if 0
        case HTTP::DATA_IMAGE:
        {
            Image *nimg;
            SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("image"))));

            nimg = new Image(h->m_Data->data, h->m_Data->used);
            jdata = OBJECT_TO_JSVAL(NidiumJSImage::BuildImageObject(cx, nimg));

            break;
        }
        case HTTP::DATA_AUDIO:
        {
            JSObject *arr = JS_NewArrayBuffer(cx, h->m_Data->used);
            uint8_t *data = JS_GetArrayBufferData(arr);

            memcpy(data, h->m_Data->data, h->m_Data->used);

            SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("audio"))));

            jdata = OBJECT_TO_JSVAL(arr);

            break;
        }
#endif
        default:
        {
            JS::RootedObject arr(m_Cx, JS_NewArrayBuffer(m_Cx, h->m_Data->used));
            uint8_t *data = JS_GetArrayBufferData(arr);

            memcpy(data, h->m_Data->data, h->m_Data->used);

            eventBuilder.set("type", "binary");
            jsdata.setObjectOrNull(arr);

            break;
        }
    }

    eventBuilder.set("data", jsdata);

    this->fireJSEvent("response", &eventValue);

    NidiumJSObj(m_Cx)->unrootObject(m_JSObj);

    JS_SetReservedSlot(m_JSObj, 0, JS::NullValue());
    JS_SetReservedSlot(m_JSObj, 1, JS::NullValue());
}

JSHTTP::JSHTTP(JS::HandleObject obj, JSContext *cx, char *url) :
    JSExposer<JSHTTP>(obj, cx), m_JSCallback(JS::NullValue()), m_JSObj(nullptr)
{
    m_URL = strdup(url);
}

JSHTTP::~JSHTTP()
{
    if (m_HTTP) {
        delete m_HTTP;
    }
    free(m_URL);
}
// }}}

// {{{ Registration
NIDIUM_JS_OBJECT_EXPOSE(HTTP)
// }}}

} // namespace Binding
} // namespace Nidium

