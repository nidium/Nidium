#include "NativeJSHttp.h"
#include "ape_http_parser.h"
#include "NativeSkImage.h"
#include "NativeJSImage.h"

static JSBool native_http_request(JSContext *cx, unsigned argc, jsval *vp);
static void Http_Finalize(JSFreeOp *fop, JSObject *obj);

static JSClass Http_class = {
    "Http", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Http_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSFunctionSpec http_funcs[] = {
    JS_FN("request", native_http_request, 1, 0),
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

    nhttp = new NativeHTTP(curl.ptr(), (ape_global *)JS_GetContextPrivate(cx));

    jshttp = new NativeJSHttp();
    nhttp->setPrivate(jshttp);
    jshttp->refHttp = nhttp;

    jshttp->cx = cx;

    /* TODO: store jshttp intead of nhttp */
    JS_SetPrivate(ret, nhttp);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));
    JS_DefineFunctions(cx, ret, http_funcs);

    return JS_TRUE;
}


static JSBool native_http_request(JSContext *cx, unsigned argc, jsval *vp)
{   
    jsval callback;
    NativeHTTP *nhttp;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    NativeJSHttp *jshttp;

    if (JS_InstanceOf(cx, caller, &Http_class, JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[0], JSTYPE_FUNCTION, &callback)) {
        return JS_TRUE;
    }

    if ((nhttp = (NativeHTTP *)JS_GetPrivate(caller)) == NULL) {
        return JS_TRUE;
    }

    jshttp = (NativeJSHttp *)nhttp->getPrivate();
    jshttp->request = callback;

    nhttp->request(jshttp);

    return JS_TRUE;
}

void NativeJSHttp::onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType type)
{
#define REQUEST_HEADER(header) ape_array_lookup(h->headers.list, \
    CONST_STR_LEN(header "\0"))
#define SET_PROP(where, name, val) JS_DefineProperty(cx, where, \
    (const char *)name, val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | \
        JSPROP_ENUMERATE)

    buffer *k, *v;
    JSObject *headers, *event;
    jsval rval, jevent, jdata = JSVAL_NULL;

    JSAutoRequest ar(cx);

    event = JS_NewObject(cx, NULL, NULL, NULL);
    headers = JS_NewObject(cx, NULL, NULL, NULL);

    APE_A_FOREACH(h->headers.list, k, v) {
        JSString *jstr = JS_NewStringCopyN(cx, (char *)&v->data[1],
            v->used-2);
        SET_PROP(headers, k->data, STRING_TO_JSVAL(jstr));
    }
    
    SET_PROP(event, "headers", OBJECT_TO_JSVAL(headers));


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
            SET_PROP(event, "type", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
                CONST_STR_LEN("null"))));
            jdata = JSVAL_NULL;
            break;
    }

    SET_PROP(event, "data", jdata);

    jevent = OBJECT_TO_JSVAL(event);

    JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), request,
        1, &jevent, &rval);

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
