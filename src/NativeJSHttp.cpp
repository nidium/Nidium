#include "NativeJSHttp.h"

#define HTTP_PREFIX "http://"

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

static void native_http_connected(ape_socket *s, ape_global *ape)
{
	printf("Connected to http server\n");
}

static JSBool native_Http_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
	JSString *url;
	char host[1024];
	char path[8096];
	u_short port;
	NativeJSHttp *nhttp;

	JSObject *ret = JS_NewObjectForConstructor(cx, &http_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S",
        &url)) {
        return JS_TRUE;
    }

    JSAutoByteString curl(cx, url);

    if (parse_uri(curl.ptr(), host, &port, path) == -1) {
    	return JS_TRUE;
    }

    nhttp = new NativeJSHttp();

    nhttp->host = strdup(host);
    nhttp->path = strdup(path);
    nhttp->port = port;

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

    socket->ctx = nhttp;

    if (APE_socket_connect(socket, nhttp->port, nhttp->host) == -1) {
        printf("[Socket] Cant connect (0)\n");
        return JS_TRUE;
    }

	return JS_TRUE;
}

NativeJSHttp::NativeJSHttp()
	: host(NULL), path(NULL), port(0), request(JSVAL_NULL)
{

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
