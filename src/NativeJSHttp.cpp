#include "NativeJSHttp.h"
#include "ape_http_parser.h"

#define HTTP_PREFIX "http://"
#define SOCKET_WRITE_STATIC(data) APE_socket_write(s, \
    (unsigned char *)CONST_STR_LEN(data), APE_DATA_STATIC)

#define SOCKET_WRITE_OWN(data) APE_socket_write(s, (unsigned char *)data, \
    strlen(data), APE_DATA_OWN)

static JSBool native_http_request(JSContext *cx, unsigned argc, jsval *vp);

static JSClass http_class = {
    "Http", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSFunctionSpec http_funcs[] = {
    JS_FN("request", native_http_request, 1, 0),
    JS_FS_END
};

static int parse_uri(char *url, char *host,
    u_short *port, char *file)
{
    char *p;
    const char *p2;
    int len;

    len = strlen(HTTP_PREFIX);
    if (strncasecmp(url, HTTP_PREFIX, len)) {
        return -1;
    }

    url += len;

    /* We might overrun */
    strncpy(host, url, 1023);

    p = strchr(host, '/');
    if (p != NULL) {
        *p = '\0';
        p2 = p + 1;
    } else {
        p2 = NULL;
    }
    if (file != NULL) {
        /* Generate request file */
        if (p2 == NULL)
            p2 = "";
        sprintf(file, "/%s", p2);
    }

    p = strchr(host, ':');

    if (p != NULL) {
        *p = '\0';
        *port = atoi(p + 1);

        if (*port == 0)
            return -1;
    } else
        *port = 80;

    return 0;
}


static int native_http_callback(void **ctx, callback_type type,
        int value, uint32_t step)
{
    NativeJSHttp *nhttp = (NativeJSHttp *)ctx[0];

    switch(type) {
        case HTTP_HEADER_KEYC:
            buffer_append_char(nhttp->http.headers.tkey, (unsigned char)value);
            break;
        case HTTP_HEADER_VALC:
            buffer_append_char(nhttp->http.headers.tval, (unsigned char)value);
            break;
        case HTTP_CL_VAL:
            nhttp->http.data = buffer_new(value);
            break;
        case HTTP_HEADER_VAL:
            buffer_append_char(nhttp->http.headers.tkey, '\0');
            buffer_append_char(nhttp->http.headers.tval, '\0');
            ape_array_add_b(nhttp->http.headers.list,
                    nhttp->http.headers.tkey, nhttp->http.headers.tval);
            nhttp->http.headers.tkey = buffer_new(16);
            nhttp->http.headers.tval = buffer_new(64);
            
            break;
        case HTTP_BODY_CHAR:
            if (nhttp->http.data == NULL) {
                nhttp->http.data = buffer_new(2048);
            }

            buffer_append_char(nhttp->http.data, (unsigned char)value);
            break;
        case HTTP_READY:
            buffer_destroy(nhttp->http.headers.tkey);
            buffer_destroy(nhttp->http.headers.tval);
            nhttp->requestEnded();
            break;
        default:break;
    }

    return 1;
}

static void native_http_connected(ape_socket *s, ape_global *ape)
{
    NativeJSHttp *nhttp = (NativeJSHttp *)s->ctx;

    if (nhttp == NULL) {
        return;
    }

    nhttp->http.headers.list = ape_array_new(16);
    nhttp->http.headers.tkey = buffer_new(16);
    nhttp->http.headers.tval = buffer_new(64);
    nhttp->http.data = NULL;

    nhttp->http.parser.ctx[0] = nhttp;

    SOCKET_WRITE_STATIC("GET ");
    SOCKET_WRITE_OWN(nhttp->path);
    SOCKET_WRITE_STATIC(" HTTP/1.0\nHost: ");
    SOCKET_WRITE_OWN(nhttp->host);
    SOCKET_WRITE_STATIC("\nUser-Agent: Nativestudio/1.0\nConnection: close\n\n");
}

static void native_http_read(ape_socket *s, ape_global *ape)
{
    unsigned int i;
    NativeJSHttp *nhttp = (NativeJSHttp *)s->ctx;

    for (i = 0; i < s->data_in.used; i++) {

        if (!parse_http_char(&nhttp->http.parser,
            s->data_in.data[i])) {
            printf("Failed at %d %c\n", i, s->data_in.data[i]);
            printf("next %s\n", &s->data_in.data[i]);
            // TODO : graceful shutdown
            shutdown(s->s.fd, 2);
            break;
        }

    //printf("%c", socket_client->data_in.data[i]);
    }
}

static JSBool native_Http_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *url;
    char host[8096];
    char path[8096];
    u_short port;
    NativeJSHttp *nhttp;

    JSObject *ret = JS_NewObjectForConstructor(cx, &http_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S",
        &url)) {
        return JS_TRUE;
    }

    JSAutoByteString curl(cx, url);

    if (strlen(curl.ptr()) > 8094) {
        return JS_FALSE;
    }

    if (parse_uri(curl.ptr(), host, &port, path) == -1) {
        return JS_TRUE;
    }

    nhttp = new NativeJSHttp();

    nhttp->host = strdup(host);
    nhttp->path = strdup(path);
    nhttp->port = port;
    nhttp->cx   = cx;

    JS_SetPrivate(ret, nhttp);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));
    JS_DefineFunctions(cx, ret, http_funcs);

    return JS_TRUE;
}

static JSBool native_http_request(JSContext *cx, unsigned argc, jsval *vp)
{   
    jsval callback;
    NativeJSHttp *nhttp;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    ape_global *net = (ape_global *)JS_GetContextPrivate(cx);
    ape_socket *socket;

    if (JS_InstanceOf(cx, caller, &http_class, JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[0], JSTYPE_FUNCTION, &callback)) {
        return JS_TRUE;
    }

    if ((nhttp = (NativeJSHttp *)JS_GetPrivate(caller)) == NULL) {
        return JS_TRUE;
    }

    if ((socket = APE_socket_new(APE_SOCKET_PT_TCP, 0, net)) == NULL) {
        printf("[Socket] Cant load socket (new)\n");
        /* TODO: add exception */
        return JS_TRUE;
    }

    nhttp->request = callback;

    JS_AddValueRoot(cx, &nhttp->request);

    socket->callbacks.on_connected  = native_http_connected;
    socket->callbacks.on_read       = native_http_read;

    socket->ctx = nhttp;

    if (APE_socket_connect(socket, nhttp->port, nhttp->host) == -1) {
        printf("[Socket] Cant connect (0)\n");
        return JS_TRUE;
    }

    return JS_TRUE;
}

void NativeJSHttp::requestEnded()
{
#define REQUEST_HEADER(header) ape_array_lookup(http.headers.list, \
    CONST_STR_LEN(header))
#define SET_PROP(where, name, val) JS_DefineProperty(cx, where, \
    (const char *)name, val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | \
        JSPROP_ENUMERATE)

    buffer *k, *v;
    JSObject *headers, *event;
    jsval rval, jevent;

    event = JS_NewObject(cx, NULL, NULL, NULL);
    headers = JS_NewObject(cx, NULL, NULL, NULL);

    APE_A_FOREACH(http.headers.list, k, v) {
        JSString *jstr = JS_NewStringCopyN(cx, (char *)&v->data[1],
            v->used-2);
        SET_PROP(headers, k->data, STRING_TO_JSVAL(jstr));
    }
    
    SET_PROP(event, "headers", OBJECT_TO_JSVAL(headers));

    jevent = OBJECT_TO_JSVAL(event);

    JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), request,
        1, &jevent, &rval);

}

NativeJSHttp::NativeJSHttp()
    : host(NULL), path(NULL), port(0), request(JSVAL_NULL)
{
    HTTP_PARSER_RESET(&http.parser);

    http.headers.list    = NULL;
    http.parser.callback = native_http_callback;
}

NativeJSHttp::~NativeJSHttp()
{
    free(host);
    free(path);
}

void NativeJSHttp::registerObject(JSContext *cx)
{
    JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, &http_class,
        native_Http_constructor,
        1, NULL, NULL, NULL, NULL);

}
