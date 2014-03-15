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
static JSBool native_socket_sendto(JSContext *cx, unsigned argc, jsval *vp);

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
    JS_FN("sendTo", native_socket_sendto, 3, 0),
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

static void native_socket_wrapper_onconnected(ape_socket *s, ape_global *ape,
    void *socket_arg)
{
    JSContext *cx;
    jsval onconnect, rval;
    NativeJSSocket *nsocket = (NativeJSSocket *)s->ctx;

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->cx;

    if (JS_GetProperty(cx, nsocket->getJSObject(), "onconnect", &onconnect) &&
        JS_TypeOfValue(cx, onconnect) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, nsocket->getJSObject(), onconnect,
            0, NULL, &rval);
    }
}


static void native_socket_wrapper_onaccept(ape_socket *socket_server,
    ape_socket *socket_client, ape_global *ape, void *socket_arg)
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

    if (JS_GetProperty(cx, nsocket->getJSObject(), "onaccept", &onaccept) &&
        JS_TypeOfValue(cx, onaccept) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, nsocket->getJSObject(), onaccept,
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

    if (JS_GetProperty(cx, nsocket->getJSObject(), "onread", &onread) &&
        JS_TypeOfValue(cx, onread) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, nsocket->getJSObject(), onread,
            1, &jdata, &rval);
    }    
}

static void native_socket_wrapper_client_ondrain(ape_socket *socket_server,
    ape_global *ape, void *socket_arg)
{
    JSContext *cx;
    NativeJSSocket *nsocket = (NativeJSSocket *)socket_server->ctx;
    jsval ondrain, rval;

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->cx;

    if (JS_GetProperty(cx, nsocket->getJSObject(), "ondrain", &ondrain) &&
        JS_TypeOfValue(cx, ondrain) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, nsocket->getJSObject(), ondrain,
            0, NULL, &rval);
    }
}

static void native_socket_wrapper_client_onmessage(ape_socket *socket_server,
    ape_global *ape, const unsigned char *packet, size_t len,
    struct sockaddr_in *addr, void *socket_arg)
{
    JSContext *cx;
    NativeJSSocket *nsocket = (NativeJSSocket *)socket_server->ctx;
    jsval onmessage, rval, jparams[2];

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->cx;

    if (nsocket->flags & NATIVE_SOCKET_ISBINARY) {
        JSObject *arrayBuffer = JS_NewArrayBuffer(cx, len);
        uint8_t *data = JS_GetArrayBufferData(arrayBuffer);
        memcpy(data, packet, len);

        jparams[0] = OBJECT_TO_JSVAL(arrayBuffer);

    } else {
        JSString *jstr = JS_NewStringCopyN(cx, (char *)packet, len);

        if (jstr == NULL) {
            printf("JS_NewStringCopyN Failed\n");
            return;
        }
        jparams[0] = STRING_TO_JSVAL(jstr);        
    }

    if (JS_GetProperty(cx, nsocket->getJSObject(), "onmessage", &onmessage) &&
        JS_TypeOfValue(cx, onmessage) == JSTYPE_FUNCTION) {

        JSObject *remote = JS_NewObject(cx, NULL, NULL, NULL);

        char *cip = inet_ntoa(addr->sin_addr);
        jsval jip = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, cip));
        jsval jport = INT_TO_JSVAL(ntohs(addr->sin_port));

        JS_SetProperty(cx, remote, "ip", &jip);
        JS_SetProperty(cx, remote, "port", &jport);
        jparams[1] = OBJECT_TO_JSVAL(remote);

        JS_CallFunctionValue(cx, nsocket->getJSObject(), onmessage,
            2, jparams, &rval);
    }
}

static void native_socket_wrapper_client_read(ape_socket *socket_client,
    ape_global *ape, void *socket_arg)
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

    if (JS_GetProperty(cx, nsocket->getJSObject(), "onread", &onread) &&
        JS_TypeOfValue(cx, onread) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, nsocket->getJSObject(), onread,
            2, jparams, &rval);
    }

}

static void native_socket_wrapper_read(ape_socket *s, ape_global *ape,
    void *socket_arg)
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

    if (JS_GetProperty(cx, nsocket->getJSObject(), "onread", &onread) &&
        JS_TypeOfValue(cx, onread) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, nsocket->getJSObject(), onread,
            1, &jdata, &rval);
    }
}

static void native_socket_wrapper_client_disconnect(ape_socket *socket_client,
    ape_global *ape, void *socket_arg)
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

    if (JS_GetProperty(cx, nsocket->getJSObject(), "ondisconnect", &ondisconnect) &&
        JS_TypeOfValue(cx, ondisconnect) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, nsocket->getJSObject(), ondisconnect,
            1, jparams, &rval);
    }

    NativeJSObj(cx)->unrootObject((JSObject *)socket_client->ctx);
    
    JS_SetPrivate((JSObject *)socket_client->ctx, NULL);
    socket_client->ctx = NULL;
}

static void native_socket_wrapper_disconnect(ape_socket *s, ape_global *ape,
    void *socket_arg)
{
    JSContext *cx;
    NativeJSSocket *nsocket = (NativeJSSocket *)s->ctx;
    jsval ondisconnect, rval;

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->cx;

    nsocket->dettach();

    if (JS_GetProperty(cx, nsocket->getJSObject(), "ondisconnect", &ondisconnect) &&
        JS_TypeOfValue(cx, ondisconnect) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, nsocket->getJSObject(), ondisconnect,
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
        return false;
    }

    JSObject *ret = JS_NewObjectForConstructor(cx, &Socket_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "Su",
        &host, &port)) {
        return false;
    }

    nsocket = new NativeJSSocket(JS_EncodeString(cx, host), port);

    JS_SetPrivate(ret, nsocket);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    JS_DefineFunctions(cx, ret, socket_funcs);
    JS_DefineProperties(cx, ret, Socket_props);

    JS_SetProperty(cx, ret, "binary", &isBinary);

    return true;
}

static JSBool native_socket_listen(JSContext *cx, unsigned argc, jsval *vp)
{
    ape_socket *socket;
    ape_socket_proto protocol = APE_SOCKET_PT_TCP;

    ape_global *net = (ape_global *)JS_GetContextPrivate(cx);

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    if (JS_InstanceOf(cx, caller, &Socket_class, args.array()) == JS_FALSE) {
        return false;
    }

    NativeJSSocket *nsocket = (NativeJSSocket *)JS_GetPrivate(caller);

    if (nsocket == NULL || nsocket->isAttached()) {
        return JS_TRUE;
    }

    if (args.length() > 0) {
        JSString *farg = args[0].toString();

        JSAutoByteString cproto(cx, farg);

        if (strncasecmp("udp", cproto.ptr(), 3) == 0) {
            protocol = APE_SOCKET_PT_UDP;
        }

    }

    if ((socket = APE_socket_new(protocol, 0, net)) == NULL) {
        JS_ReportError(cx, "Failed to create socket");
        return false;
    }

    socket->callbacks.on_connect    = native_socket_wrapper_onaccept;
    socket->callbacks.on_read       = native_socket_wrapper_client_read;
    socket->callbacks.on_disconnect = native_socket_wrapper_client_disconnect;
    socket->callbacks.on_message    = native_socket_wrapper_client_onmessage;
    socket->callbacks.on_drain      = native_socket_wrapper_client_ondrain;

    socket->ctx = nsocket;

    nsocket->socket     = socket;
    nsocket->cx         = cx;
    nsocket->jsobj      = caller;

    if (APE_socket_listen(socket, nsocket->port, nsocket->host) == -1) {
        JS_ReportError(cx, "Can't listen on socket (%s:%d)", nsocket->host,
            nsocket->port);
        /* TODO: close() leak */
        return JS_FALSE;
    }

    NativeJSObj(cx)->rootObjectUntilShutdown(caller);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(caller));

    nsocket->flags |= NATIVE_SOCKET_ISSERVER;

    return JS_TRUE;
}

static JSBool native_socket_connect(JSContext *cx, unsigned argc, jsval *vp)
{
    ape_socket *socket;
    ape_socket_proto protocol = APE_SOCKET_PT_TCP;
    uint16_t localport = 0;

    ape_global *net = (ape_global *)JS_GetContextPrivate(cx);

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    if (JS_InstanceOf(cx, caller, &Socket_class, args.array()) == JS_FALSE) {
        return false;
    }

    NativeJSSocket *nsocket = (NativeJSSocket *)JS_GetPrivate(caller);

    if (nsocket == NULL || nsocket->isAttached()) {
        return false;
    }

    if (args.length() > 0) {
        JSString *farg = args[0].toString();

        JSAutoByteString cproto(cx, farg);

        if (strncasecmp("udp", cproto.ptr(), 3) == 0) {
            protocol = APE_SOCKET_PT_UDP;
        }

        localport = (args.length() > 1 ? (uint16_t)args[1].toInt32() : 0);
    }

    if ((socket = APE_socket_new(protocol, 0, net)) == NULL) {
        JS_ReportError(cx, "Failed to create socket");
        return false;
    }

    socket->callbacks.on_connected  = native_socket_wrapper_onconnected;
    socket->callbacks.on_read       = native_socket_wrapper_read;
    socket->callbacks.on_disconnect = native_socket_wrapper_disconnect;
    socket->callbacks.on_message    = native_socket_wrapper_client_onmessage;
    socket->callbacks.on_drain      = native_socket_wrapper_client_ondrain;

    socket->ctx = nsocket;

    nsocket->socket   = socket;
    nsocket->cx       = cx;
    nsocket->jsobj    = caller;

    if (APE_socket_connect(socket, nsocket->port, nsocket->host, localport) == -1) {
        JS_ReportError(cx, "Can't connect on socket (%s:%d)", nsocket->host,
            nsocket->port);
        return false;
    }

    NativeJSObj(cx)->rootObjectUntilShutdown(caller);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(caller));

    return true;
}

static JSBool native_socket_client_write(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JSString *data;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    ape_socket *socket_client;

    NATIVE_CHECK_ARGS("write", 1);

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (JS_InstanceOf(cx, caller, &socket_client_class,
        args.array()) == JS_FALSE) {
        return false;
    }

    if ((socket_client = (ape_socket *)JS_GetPrivate(caller)) == NULL) {
        JS_ReportError(cx, "socket.write() Invalid socket");
        return false;
    }

    if (args[0].isString()) {

        JSAutoByteString cdata(cx, args[0].toString());

        int ret = APE_socket_write(socket_client, (unsigned char *)cdata.ptr(),
            strlen(cdata.ptr()), APE_DATA_COPY);

        args.rval().setInt32(ret);

    } else if (args[0].isObject()) {
        JSObject *objdata = args[0].toObjectOrNull();

        if (!objdata || !JS_IsArrayBufferObject(objdata)) {
            JS_ReportError(cx, "write() invalid data (must be either a string or an ArrayBuffer)");
            return false;            
        }
        uint32_t len = JS_GetArrayBufferByteLength(objdata);
        uint8_t *data = JS_GetArrayBufferData(objdata);

        int ret = APE_socket_write(socket_client, data, len, APE_DATA_COPY);

        args.rval().setInt32(ret);

    } else {
        JS_ReportError(cx, "write() invalid data (must be either a string or an ArrayBuffer)");
        return false;
    }

    return true;
}

static JSBool native_socket_write(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *data;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);

    NATIVE_CHECK_ARGS("write", 1);

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (JS_InstanceOf(cx, caller, &Socket_class, args.array()) == JS_FALSE) {
        return false;
    }

    NativeJSSocket *nsocket = (NativeJSSocket *)JS_GetPrivate(caller);

    if (nsocket == NULL || !nsocket->isAttached()) {

        JS_ReportError(cx, "socket.write() Invalid socket");
        return false;
    }

    if (args[0].isString()) {
        JSAutoByteString cdata(cx, args[0].toString());

        int ret = nsocket->write((unsigned char*)cdata.ptr(),
            strlen(cdata.ptr()), APE_DATA_COPY);

        args.rval().setInt32(ret);

    } else if (args[0].isObject()) {
        JSObject *objdata = args[0].toObjectOrNull();

        if (!objdata || !JS_IsArrayBufferObject(objdata)) {
            JS_ReportError(cx, "write() invalid data (must be either a string or an ArrayBuffer)");
            return false;            
        }
        uint32_t len = JS_GetArrayBufferByteLength(objdata);
        uint8_t *data = JS_GetArrayBufferData(objdata);

        int ret = nsocket->write(data, len, APE_DATA_COPY);

        args.rval().setInt32(ret);

    } else {
        JS_ReportError(cx, "write() invalid data (must be either a string or an ArrayBuffer)");
        return false;
    }

    return JS_TRUE;
}

static JSBool native_socket_client_close(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    ape_socket *socket_client;

    if (JS_InstanceOf(cx, caller, &socket_client_class,
        JS_ARGV(cx, vp)) == JS_FALSE) {
        return false;
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
        return false;
    }

    NativeJSSocket *nsocket = (NativeJSSocket *)JS_GetPrivate(caller);

    if (nsocket == NULL || !nsocket->isAttached()) {
        return JS_TRUE;
    }

    nsocket->shutdown();

    return JS_TRUE;
}

static JSBool native_socket_sendto(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *caller = JS_THIS_OBJECT(cx, vp);

    NATIVE_CHECK_ARGS("sendto", 3);

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (JS_InstanceOf(cx, caller, &Socket_class, args.array()) == JS_FALSE) {
        return false;
    }

    NativeJSSocket *nsocket = (NativeJSSocket *)JS_GetPrivate(caller);

    if (nsocket == NULL || !nsocket->isAttached()) {
        return JS_TRUE;
    }

    if (!(nsocket->flags & NATIVE_SOCKET_ISSERVER)) {
        JS_ReportError(cx, "sendto() is only available on listening socket");
        return false;
    }

    JSString *ip = args[0].toString();
    unsigned int port = args[1].toInt32();

    JSAutoByteString cip(cx, ip);

    if (args[2].isString()) {
        JSAutoByteString cdata(cx, args[2].toString());

        ape_socket_write_udp(nsocket->socket, cdata.ptr(), cdata.length(), cip.ptr(), (uint16_t)port);
    } else if (args[2].isObject()) {
        JSObject *objdata = args[2].toObjectOrNull();

        if (!objdata || !JS_IsArrayBufferObject(objdata)) {
            JS_ReportError(cx, "sendTo() invalid data (must be either a string or an ArrayBuffer)");
            return false;
        }
        uint32_t len = JS_GetArrayBufferByteLength(objdata);
        uint8_t *data = JS_GetArrayBufferData(objdata);

        ape_socket_write_udp(nsocket->socket, (char *)data, len, cip.ptr(), (uint16_t)port);

    } else {
        JS_ReportError(cx, "sendTo() invalid data (must be either a string or an ArrayBuffer)");
        return false;
    }

    return true;
}

static void Socket_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSSocket *nsocket = (NativeJSSocket *)JS_GetPrivate(obj);

    if (nsocket != NULL) {
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
    : socket(NULL), flags(0)
{
    cx = NULL;

    this->jsobj = NULL;
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
    return (cx != NULL && this->getJSObject() != NULL);
}

void NativeJSSocket::dettach()
{
    if (isAttached()) {
        socket->ctx = NULL;
        socket = NULL;
    }
}

int NativeJSSocket::write(unsigned char *data, size_t len,
    ape_socket_data_autorelease data_type)
{
    return APE_socket_write(socket, data, len, data_type);
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
