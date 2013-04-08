#include "NativeJSSocket.h"
#include "NativeJS.h"

/* only use on connected clients */
#define NATIVE_SOCKET_JSOBJECT(socket) ((JSObject *)socket->ctx)

static void Socket_Finalize(JSFreeOp *fop, JSObject *obj);
static void Socket_Finalize_client(JSFreeOp *fop, JSObject *obj);
static JSBool native_socket_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp);
static JSBool native_socket_connect(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_socket_listen(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_socket_write(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_socket_close(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_socket_client_write(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_socket_client_close(JSContext *cx,
    unsigned argc, jsval *vp);

static JSClass Socket_class = {
    "Socket", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Socket_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass socket_client_class = {
    "SocketClient", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Socket_Finalize_client,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSFunctionSpec socket_client_funcs[] = {
    JS_FN("write", native_socket_client_write, 1, 0),
    JS_FN("disconnect", native_socket_client_close, 0, 0),  /* TODO: add force arg */
    JS_FS_END
};

static JSFunctionSpec socket_funcs[] = {
    JS_FN("listen", native_socket_listen, 0, 0),
    JS_FN("connect", native_socket_connect, 0, 0),
    JS_FN("write", native_socket_write, 1, 0),
    JS_FN("disconnect", native_socket_close, 0, 0), /* TODO: add force arg */
    JS_FS_END
};

static JSPropertySpec Socket_props[] = {
    {"binary", SOCKET_PROP_BINARY, 0, JSOP_NULLWRAPPER,
    JSOP_WRAPPER(native_socket_prop_set)},
    {"readline", SOCKET_PROP_READLINE, 0, JSOP_NULLWRAPPER,
    JSOP_WRAPPER(native_socket_prop_set)},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSBool native_socket_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
    switch(JSID_TO_INT(id)) {
        case SOCKET_PROP_BINARY:
        {
            NativeJSSocket *nsocket;
            if (JSVAL_IS_BOOLEAN(vp) &&
                (nsocket = (NativeJSSocket *)JS_GetPrivate(obj.get())) != NULL) {

                nsocket->flags = (JSVAL_TO_BOOLEAN(vp) == JS_TRUE ?
                    nsocket->flags | NATIVE_SOCKET_ISBINARY :
                    nsocket->flags & ~NATIVE_SOCKET_ISBINARY);

            } else {
                vp.set(JSVAL_FALSE);
                return JS_TRUE;
            }
        }
        break;
        case SOCKET_PROP_READLINE:
        {
            NativeJSSocket *nsocket;
            if (JSVAL_IS_BOOLEAN(vp) &&
                (nsocket = (NativeJSSocket *)JS_GetPrivate(obj.get())) != NULL) {

                nsocket->flags = (JSVAL_TO_BOOLEAN(vp) == JS_TRUE ?
                    nsocket->flags | NATIVE_SOCKET_READLINE :
                    nsocket->flags & ~NATIVE_SOCKET_READLINE);

                if (JSVAL_TO_BOOLEAN(vp) == JS_TRUE &&
                    nsocket->lineBuffer.data == NULL) {

                    nsocket->lineBuffer.data = (char *)malloc(sizeof(char)
                        * SOCKET_LINEBUFFER_MAX);
                    nsocket->lineBuffer.pos = 0;
                }
            } else {
                vp.set(JSVAL_FALSE);
                return JS_TRUE;
            }
        }
        break;        
        default:
            break;
    }
    return JS_TRUE;
}

static void native_socket_wrapper_onconnected(ape_socket *s, ape_global *ape)
{
    JSContext *cx;
    jsval onconnect, rval;
    NativeJSSocket *nsocket = (NativeJSSocket *)s->ctx;

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->cx;

    if (JS_GetProperty(cx, nsocket->jsobject, "onconnect", &onconnect) &&
        JS_TypeOfValue(cx, onconnect) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, nsocket->jsobject, onconnect,
            0, NULL, &rval);
    }
}


static void native_socket_wrapper_onaccept(ape_socket *socket_server,
    ape_socket *socket_client, ape_global *ape)
{
    JSContext *cx;
    JSObject *jclient;
    NativeJSSocket *nsocket = (NativeJSSocket *)socket_server->ctx;
    jsval onaccept, rval, arg;

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->cx;

    jclient = JS_NewObject(cx, &socket_client_class, NULL, NULL);
    socket_client->ctx = jclient;

    NativeJSObj(cx)->rootObjectUntilShutdown(jclient);

    JS_SetPrivate(jclient, socket_client);

    JS_DefineFunctions(cx, jclient, socket_client_funcs);

    arg = OBJECT_TO_JSVAL(jclient);

    if (JS_GetProperty(cx, nsocket->jsobject, "onaccept", &onaccept) &&
        JS_TypeOfValue(cx, onaccept) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, nsocket->jsobject, onaccept,
            1, &arg, &rval);
    }
}

inline static void native_socket_readcb(NativeJSSocket *nsocket, char *data, size_t len)
{
    jsval onread, rval, jdata;
    JSContext *cx = nsocket->cx;
    JSString *tstr = JS_NewStringCopyN(cx, data, len);
    JSString *jstr = tstr;

    if (nsocket->lineBuffer.pos && nsocket->flags & NATIVE_SOCKET_READLINE) {
        JSString *left = JS_NewStringCopyN(cx, nsocket->lineBuffer.data,
            nsocket->lineBuffer.pos);

        jstr = JS_ConcatStrings(cx, left, tstr);
        nsocket->lineBuffer.pos = 0;
    }

    jdata = STRING_TO_JSVAL(jstr);

    if (JS_GetProperty(cx, nsocket->jsobject, "onread", &onread) &&
        JS_TypeOfValue(cx, onread) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, nsocket->jsobject, onread,
            1, &jdata, &rval);
    }    
}

static void native_socket_wrapper_client_read(ape_socket *socket_client,
    ape_global *ape)
{
    JSContext *cx;
    jsval onread, rval, jparams[2];
    ape_socket *socket_server = socket_client->parent;
    NativeJSSocket *nsocket = (NativeJSSocket *)socket_server->ctx;

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->cx;

    jparams[0] = OBJECT_TO_JSVAL(NATIVE_SOCKET_JSOBJECT(socket_client));

    if (nsocket->flags & NATIVE_SOCKET_ISBINARY) {
        JSObject *arrayBuffer = JS_NewArrayBuffer(cx, socket_client->data_in.used);
        uint8_t *data = JS_GetArrayBufferData(arrayBuffer);
        memcpy(data, socket_client->data_in.data, socket_client->data_in.used);

        jparams[1] = OBJECT_TO_JSVAL(arrayBuffer);

    } else if (nsocket->flags & NATIVE_SOCKET_READLINE) {
        char *pBuf = (char *)socket_client->data_in.data;
        size_t len = socket_client->data_in.used;
        char *eol;

        while (len > 0 && (eol = (char *)memchr(pBuf, '\n', len)) != NULL) {
            size_t pLen = eol - pBuf;
            len -= pLen;
            if (len-- > 0) {
                native_socket_readcb(nsocket, pBuf, pLen);
                pBuf = eol+1;
            }
        }

        if (len && len+nsocket->lineBuffer.pos <= SOCKET_LINEBUFFER_MAX) {
            memcpy(nsocket->lineBuffer.data+nsocket->lineBuffer.pos, pBuf, len);
            nsocket->lineBuffer.pos += len;
        } else if (len) {
            nsocket->lineBuffer.pos = 0;
        }

        return;

    } else {
        JSString *jstr = JS_NewStringCopyN(cx, (char *)socket_client->data_in.data,
            socket_client->data_in.used);

        if (jstr == NULL) {
            printf("JS_NewStringCopyN Failed\n");
            return;
        }
        jparams[1] = STRING_TO_JSVAL(jstr);        
    }

    if (JS_GetProperty(cx, nsocket->jsobject, "onread", &onread) &&
        JS_TypeOfValue(cx, onread) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, nsocket->jsobject, onread,
            2, jparams, &rval);
    }

}

static void native_socket_wrapper_read(ape_socket *s, ape_global *ape)
{
    JSContext *cx;
    jsval onread, rval, jdata;

    NativeJSSocket *nsocket = (NativeJSSocket *)s->ctx;

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->cx;

    if (nsocket->flags & NATIVE_SOCKET_ISBINARY) {
        JSObject *arrayBuffer = JS_NewArrayBuffer(cx, s->data_in.used);
        uint8_t *data = JS_GetArrayBufferData(arrayBuffer);
        memcpy(data, s->data_in.data, s->data_in.used);

        jdata = OBJECT_TO_JSVAL(arrayBuffer);

    } else if (nsocket->flags & NATIVE_SOCKET_READLINE) {
        char *pBuf = (char *)s->data_in.data;
        size_t len = s->data_in.used;
        char *eol;

        while (len > 0 && (eol = (char *)memchr(pBuf, '\n', len)) != NULL) {
            size_t pLen = eol - pBuf;
            len -= pLen;
            if (len-- > 0) {
                native_socket_readcb(nsocket, pBuf, pLen);
                pBuf = eol+1;
            }
        }

        if (len && len+nsocket->lineBuffer.pos <= SOCKET_LINEBUFFER_MAX) {
            memcpy(nsocket->lineBuffer.data+nsocket->lineBuffer.pos, pBuf, len);
            nsocket->lineBuffer.pos += len;
        } else if (len) {
            nsocket->lineBuffer.pos = 0;
        }

        return;

    } else {
        JSString *jstr = JS_NewStringCopyN(cx, (char *)s->data_in.data,
            s->data_in.used);

        if (jstr == NULL) {
            printf("JS_NewStringCopyN Failed\n");
            return;
        }
        jdata = STRING_TO_JSVAL(jstr);        
    }

    if (JS_GetProperty(cx, nsocket->jsobject, "onread", &onread) &&
        JS_TypeOfValue(cx, onread) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, nsocket->jsobject, onread,
            1, &jdata, &rval);
    }
}

static void native_socket_wrapper_client_disconnect(ape_socket *socket_client,
    ape_global *ape)
{
    JSContext *cx;
    jsval ondisconnect, rval, jparams[1];
    NativeJSSocket *nsocket;

    ape_socket *socket_server = socket_client->parent;
    if (socket_server == NULL) { /* the server has disconnected */
        return;
    }

    nsocket = (NativeJSSocket *)socket_server->ctx;

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->cx;

    jparams[0] = OBJECT_TO_JSVAL(NATIVE_SOCKET_JSOBJECT(socket_client));

    if (JS_GetProperty(cx, nsocket->jsobject, "ondisconnect", &ondisconnect) &&
        JS_TypeOfValue(cx, ondisconnect) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, nsocket->jsobject, ondisconnect,
            1, jparams, &rval);
    }

    NativeJSObj(cx)->unrootObject((JSObject *)socket_client->ctx);
    
    JS_SetPrivate((JSObject *)socket_client->ctx, NULL);
    socket_client->ctx = NULL;
}

static void native_socket_wrapper_disconnect(ape_socket *s, ape_global *ape)
{
    JSContext *cx;
    NativeJSSocket *nsocket = (NativeJSSocket *)s->ctx;
    jsval ondisconnect, rval;

    printf("Disconnected\n");

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        printf("Not callable\n");
        return;
    }

    cx = nsocket->cx;

    nsocket->dettach();

    if (JS_GetProperty(cx, nsocket->jsobject, "ondisconnect", &ondisconnect) &&
        JS_TypeOfValue(cx, ondisconnect) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, nsocket->jsobject, ondisconnect,
            0, NULL, &rval);
    }
}

static JSBool native_Socket_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *host;
    unsigned int port;
    NativeJSSocket *nsocket;
    jsval isBinary = JSVAL_FALSE;

    if (!JS_IsConstructing(cx, vp)) {
        JS_ReportError(cx, "Bad constructor");
        return JS_FALSE;
    }

    JSObject *ret = JS_NewObjectForConstructor(cx, &Socket_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "Su",
        &host, &port)) {
        return JS_TRUE;
    }

    nsocket = new NativeJSSocket(JS_EncodeString(cx, host), port);

    JS_SetPrivate(ret, nsocket);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    JS_DefineFunctions(cx, ret, socket_funcs);
    JS_DefineProperties(cx, ret, Socket_props);

    JS_SetProperty(cx, ret, "binary", &isBinary);

    return JS_TRUE;
}

static JSBool native_socket_listen(JSContext *cx, unsigned argc, jsval *vp)
{
    ape_socket *socket;
    ape_global *net = (ape_global *)JS_GetContextPrivate(cx);
    JSObject *caller = JS_THIS_OBJECT(cx, vp);

    if (JS_InstanceOf(cx, caller, &Socket_class, JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }

    NativeJSSocket *nsocket = (NativeJSSocket *)JS_GetPrivate(caller);

    if (nsocket == NULL || nsocket->isAttached()) {
        return JS_TRUE;
    }

    if ((socket = APE_socket_new(APE_SOCKET_PT_TCP, 0, net)) == NULL) {
        JS_ReportError(cx, "Failed to create socket");
        return JS_FALSE;
    }

    socket->callbacks.on_connect    = native_socket_wrapper_onaccept;
    socket->callbacks.on_read       = native_socket_wrapper_client_read;
    socket->callbacks.on_disconnect = native_socket_wrapper_client_disconnect;

    socket->ctx = nsocket;

    nsocket->socket     = socket;
    nsocket->cx         = cx;
    nsocket->jsobject   = caller;

    if (APE_socket_listen(socket, nsocket->port, nsocket->host) == -1) {
        JS_ReportError(cx, "Can't listen on socket (%s:%d)", nsocket->host,
            nsocket->port);
        /* TODO: close() leak */
        return JS_FALSE;
    }

    NativeJSObj(cx)->rootObjectUntilShutdown(caller);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(caller));

    return JS_TRUE;
}

static JSBool native_socket_connect(JSContext *cx, unsigned argc, jsval *vp)
{
    ape_socket *socket;
    ape_global *net = (ape_global *)JS_GetContextPrivate(cx);
    JSObject *caller = JS_THIS_OBJECT(cx, vp);

    if (JS_InstanceOf(cx, caller, &Socket_class, JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }

    NativeJSSocket *nsocket = (NativeJSSocket *)JS_GetPrivate(caller);

    if (nsocket == NULL || nsocket->isAttached()) {
        return JS_TRUE;
    }

    if ((socket = APE_socket_new(APE_SOCKET_PT_TCP, 0, net)) == NULL) {
        JS_ReportError(cx, "Failed to create socket");
        return JS_FALSE;
    }

    socket->callbacks.on_connected  = native_socket_wrapper_onconnected;
    socket->callbacks.on_read       = native_socket_wrapper_read;
    socket->callbacks.on_disconnect = native_socket_wrapper_disconnect;

    socket->ctx = nsocket;

    nsocket->socket   = socket;
    nsocket->cx       = cx;
    nsocket->jsobject = caller;

    if (APE_socket_connect(socket, nsocket->port, nsocket->host) == -1) {
        JS_ReportError(cx, "Can't listen on socket (%s:%d)", nsocket->host,
            nsocket->port);
        return JS_FALSE;
    }

    NativeJSObj(cx)->rootObjectUntilShutdown(caller);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(caller));

    return JS_TRUE;
}

static JSBool native_socket_client_write(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JSString *data;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    ape_socket *socket_client;

    if (JS_InstanceOf(cx, caller, &socket_client_class,
        JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }

    if ((socket_client = (ape_socket *)JS_GetPrivate(caller)) == NULL) {
        return JS_TRUE;
    }

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &data)) {
        return JS_TRUE;
    }

    JSAutoByteString cdata(cx, data);

    APE_socket_write(socket_client, (unsigned char *)cdata.ptr(),
        strlen(cdata.ptr()), APE_DATA_COPY);

    return JS_TRUE;
}

static JSBool native_socket_write(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *data;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);

    if (JS_InstanceOf(cx, caller, &Socket_class, JS_ARGV(cx, vp)) == JS_FALSE) {
        printf("Bad caller\n");
        return JS_TRUE;
    }

    NativeJSSocket *nsocket = (NativeJSSocket *)JS_GetPrivate(caller);

    if (nsocket == NULL || !nsocket->isAttached() ||
        !JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &data)) {
        return JS_TRUE;
    }
    /* TODO array buffer */
    /* TODO: Use APE_DATA_AUTORELEASE and specicy a free func (JS_free)) */
    JSAutoByteString cdata(cx, data);

    nsocket->write((unsigned char*)cdata.ptr(),
        strlen(cdata.ptr()), APE_DATA_COPY);

    return JS_TRUE;
}

static JSBool native_socket_client_close(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    ape_socket *socket_client;

    if (JS_InstanceOf(cx, caller, &socket_client_class,
        JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }

    if ((socket_client = (ape_socket *)JS_GetPrivate(caller)) == NULL) {
        return JS_TRUE;
    }

    APE_socket_shutdown((ape_socket *)JS_GetPrivate(caller));

    return JS_TRUE;
}

static JSBool native_socket_close(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *caller = JS_THIS_OBJECT(cx, vp);

    if (JS_InstanceOf(cx, caller, &Socket_class, JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }

    NativeJSSocket *nsocket = (NativeJSSocket *)JS_GetPrivate(caller);

    if (nsocket == NULL || !nsocket->isAttached()) {
        return JS_TRUE;
    }

    nsocket->shutdown();

    return JS_TRUE;
}

static void Socket_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSSocket *nsocket = (NativeJSSocket *)JS_GetPrivate(obj);

    if (nsocket != NULL) {
        printf("Finalizing socket\n");
        delete nsocket;
    }
}

static void Socket_Finalize_client(JSFreeOp *fop, JSObject *obj)
{
    ape_socket *socket = (ape_socket *)JS_GetPrivate(obj);

    if (socket != NULL) {
        socket->ctx = NULL;
        APE_socket_shutdown_now(socket);
    }
}

NativeJSSocket::NativeJSSocket(const char *host, unsigned short port)
    : jsobject(NULL), socket(NULL), flags(0)
{
    cx = NULL;

    this->host = strdup(host);
    this->port = port;

    lineBuffer.pos = 0;
    lineBuffer.data = NULL;
}

NativeJSSocket::~NativeJSSocket()
{
    if (isAttached()) {
        socket->ctx = NULL;
        this->disconnect();
    }
    free(host);
    if (lineBuffer.data) {
        free(lineBuffer.data);
    }
}

bool NativeJSSocket::isAttached()
{
    return (socket != NULL);
}

bool NativeJSSocket::isJSCallable()
{
    return (cx != NULL && jsobject != NULL);
}

void NativeJSSocket::dettach()
{
    if (isAttached()) {
        socket->ctx = NULL;
        socket = NULL;
    }
}

void NativeJSSocket::write(unsigned char *data, size_t len,
    ape_socket_data_autorelease data_type)
{
    APE_socket_write(socket, data, len, data_type);
}

void NativeJSSocket::disconnect()
{
    APE_socket_shutdown_now(socket);
}

void NativeJSSocket::shutdown()
{
    APE_socket_shutdown(socket);
}

NATIVE_OBJECT_EXPOSE(Socket)
