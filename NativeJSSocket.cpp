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
#include "NativeJSUtils.h"

enum {
    SOCKET_PROP_BINARY,
    SOCKET_PROP_READLINE,
    SOCKET_PROP_ENCODING,
    SOCKET_PROP_TIMEOUT
};

/* only use on connected clients */
#define NATIVE_SOCKET_JSOBJECT(socket) (((NativeJSSocket *)socket)->getJSObject())

static void Socket_Finalize(JSFreeOp *fop, JSObject *obj);
static void Socket_Finalize_client(JSFreeOp *fop, JSObject *obj);
static bool native_socket_prop_set(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp);
static bool native_socket_connect(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_socket_listen(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_socket_write(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_socket_close(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_socket_sendto(JSContext *cx, unsigned argc, JS::Value *vp);

static bool native_socket_client_write(JSContext *cx,
    unsigned argc, JS::Value *vp);
static bool native_socket_client_sendFile(JSContext *cx,
    unsigned argc, JS::Value *vp);
static bool native_socket_client_close(JSContext *cx,
    unsigned argc, JS::Value *vp);

static JSClass Socket_class = {
    "Socket", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Socket_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *NativeJSExposer<NativeJSSocket>::jsclass = &Socket_class;

static JSClass socket_client_class = {
    "SocketClient", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Socket_Finalize_client,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSFunctionSpec socket_client_funcs[] = {
    JS_FN("sendFile", native_socket_client_sendFile, 1, 0),
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
    {"binary",   JSPROP_NATIVE_ACCESSORS, NATIVE_JS_STUBGETTER(), NATIVE_JS_SETTER(SOCKET_PROP_BINARY, native_socket_prop_set)},
    {"readline", JSPROP_NATIVE_ACCESSORS, NATIVE_JS_STUBGETTER(), NATIVE_JS_SETTER(SOCKET_PROP_READLINE, native_socket_prop_set)},
    {"encoding", JSPROP_NATIVE_ACCESSORS, NATIVE_JS_STUBGETTER(), NATIVE_JS_SETTER(SOCKET_PROP_ENCODING, native_socket_prop_set)},
    {"timeout",  JSPROP_NATIVE_ACCESSORS, NATIVE_JS_STUBGETTER(), NATIVE_JS_SETTER(SOCKET_PROP_TIMEOUT, native_socket_prop_set)},
    JS_PS_END
};

static bool native_socket_prop_set(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp)
{
    NativeJSSocket *nsocket = (NativeJSSocket *)JS_GetPrivate(obj);
    
    if (nsocket == NULL) {
        JS_ReportError(cx, "Invalid socket object");
        return false;
    }

    switch(id) {
        case SOCKET_PROP_BINARY:
        {
            if (vp.isBoolean()) {
                nsocket->flags = (vp.toBoolean() == true ?
                    nsocket->flags | NATIVE_SOCKET_ISBINARY :
                    nsocket->flags & ~NATIVE_SOCKET_ISBINARY);

            } else {
                vp.set(JSVAL_FALSE);
                return true;
            }
        }
        break;
        case SOCKET_PROP_READLINE:
        {
            bool isactive = ((vp.isBoolean() && vp.toBoolean() == true) || vp.isInt32());

            if (isactive) {

                nsocket->flags |= NATIVE_SOCKET_READLINE;

                if (nsocket->lineBuffer.data == NULL) {

                    nsocket->lineBuffer.data = (char *)malloc(sizeof(char)
                        * SOCKET_LINEBUFFER_MAX);
                    nsocket->lineBuffer.pos = 0;
                }

                /*
                    Default delimiter is line feed.
                */
                nsocket->m_FrameDelimiter = vp.isBoolean() ? '\n' : vp.toInt32() & 0xFF;

            } else {
                nsocket->flags &= ~NATIVE_SOCKET_READLINE;
                
                vp.set(JSVAL_FALSE);
                return true;
            }
        }
        break;
        case SOCKET_PROP_ENCODING:
        {
            if (vp.isString()) {
                JSAutoByteString enc(cx, vp.toString());
                nsocket->m_Encoding = strdup(enc.ptr());
            }
        }
        break;
        case SOCKET_PROP_TIMEOUT:
        {
            if (vp.isNumber()) {
                nsocket->m_TCPTimeout = APE_ABS(vp.toInt32());

                if (nsocket->socket &&
                    !APE_socket_setTimeout(nsocket->socket, nsocket->m_TCPTimeout)) {

                        JS_ReportWarning(cx, "Couldn't set TCP timeout on socket");
                }
            }
        }
        break;
        default:
            break;
    }
    return true;
}

static void native_socket_wrapper_onconnected(ape_socket *s, ape_global *ape,
    void *socket_arg)
{
    JSContext *cx;

    NativeJSSocket *nsocket = (NativeJSSocket *)s->ctx;

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->getJSContext();
    JS::RootedValue onconnect(cx);
    JS::RootedValue rval(cx);

    JS::RootedObject obj(cx, nsocket->getJSObject());
    if (JS_GetProperty(cx, obj, "onconnect", &onconnect) &&
        JS_TypeOfValue(cx, onconnect) == JSTYPE_FUNCTION) {

        PACK_TCP(s->s.fd);
        JS_CallFunctionValue(cx, obj, onconnect, JS::HandleValueArray::empty(), &rval);
        FLUSH_TCP(s->s.fd);
    }
}


static void native_socket_wrapper_onaccept(ape_socket *socket_server,
    ape_socket *socket_client, ape_global *ape, void *socket_arg)
{
    JSContext *m_Cx;
 
    NativeJSSocket *nsocket = (NativeJSSocket *)socket_server->ctx;

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    m_Cx = nsocket->getJSContext();

    JS::RootedValue onaccept(m_Cx);
    JS::RootedValue rval(m_Cx);
    JS::AutoValueArray<1> params(m_Cx);
    JS::RootedObject jclient(m_Cx, JS_NewObject(m_Cx, &socket_client_class, JS::NullPtr(), JS::NullPtr()));

    NativeJSObj(m_Cx)->rootObjectUntilShutdown(jclient);

    NativeJSSocket *sobj = new NativeJSSocket(jclient,
        nsocket->getJSContext(), APE_socket_ipv4(socket_client), 0);

    sobj->m_ParentServer = nsocket;
    sobj->socket = socket_client;

    if (sobj->getFlags() & NATIVE_SOCKET_READLINE) {
        sobj->lineBuffer.data = (char *)malloc(sizeof(char)
            * SOCKET_LINEBUFFER_MAX);
        sobj->lineBuffer.pos = 0;
    }

    socket_client->ctx = sobj;

    JS_SetPrivate(jclient, sobj);

    JS_DefineFunctions(m_Cx, jclient, socket_client_funcs);

    JSOBJ_SET_PROP_CSTR(jclient, "ip", APE_socket_ipv4(socket_client));

    params[0].setObject(*jclient);

    JS::RootedObject socketjs(m_Cx, nsocket->getJSObject());

    if (JS_GetProperty(m_Cx, socketjs, "onaccept", &onaccept) &&
        JS_TypeOfValue(m_Cx, onaccept) == JSTYPE_FUNCTION) {

        PACK_TCP(socket_client->s.fd);
        JS::RootedValue onacceptVal(m_Cx, onaccept);
        JS_CallFunctionValue(m_Cx, socketjs, onacceptVal, params, &rval);
        FLUSH_TCP(socket_client->s.fd);
    }
}

void NativeJSSocket::readFrame(const char *buf, size_t len)
{
    JS::RootedValue onread(m_Cx);
    JS::RootedValue rval(m_Cx);
    JS::AutoValueArray<2> jdata(m_Cx);
    JS::RootedString tstr(m_Cx, NativeJSUtils::newStringWithEncoding(m_Cx, buf, len, this->getEncoding()));
    JS::RootedString jstr(m_Cx);
    jstr = tstr;

    if (this->lineBuffer.pos && (this->getFlags() & NATIVE_SOCKET_READLINE)) {
        JS::RootedString left(m_Cx, NativeJSUtils::newStringWithEncoding(m_Cx, this->lineBuffer.data,
            this->lineBuffer.pos, this->getEncoding()));

        jstr = JS_ConcatStrings(m_Cx, left, tstr);
        this->lineBuffer.pos = 0;
    }

    if (isClientFromOwnServer()) {
        jdata[0].setObjectOrNull(this->getJSObject());
        jdata[1].setString(jstr);
    } else {
        jdata[0].setString(jstr);
    }

    JS::RootedObject obj(m_Cx, getReceiverJSObject());
    if (JS_GetProperty(m_Cx, obj, "onread", &onread) &&
        JS_TypeOfValue(m_Cx, onread) == JSTYPE_FUNCTION) {
        PACK_TCP(socket->s.fd);
        JS_CallFunctionValue(m_Cx, obj, onread, jdata, &rval);
        FLUSH_TCP(socket->s.fd);
    }  
}

static void native_socket_wrapper_client_ondrain(ape_socket *socket_server,
    ape_global *ape, void *socket_arg)
{
    JSContext *cx;
    NativeJSSocket *nsocket = (NativeJSSocket *)socket_server->ctx;

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->getJSContext();

    JS::RootedValue ondrain(cx);
    JS::RootedValue rval(cx);
    JS::RootedObject obj(cx, nsocket->getJSObject());

    if (JS_GetProperty(cx, obj, "ondrain", &ondrain) &&
        JS_TypeOfValue(cx, ondrain) == JSTYPE_FUNCTION) {
        JS_CallFunctionValue(cx, obj, ondrain, JS::HandleValueArray::empty(), &rval);
    }
}

static void native_socket_wrapper_client_onmessage(ape_socket *socket_server,
    ape_global *ape, const unsigned char *packet, size_t len,
    struct sockaddr_in *addr, void *socket_arg)
{
    JSContext *cx;
    NativeJSSocket *nsocket = (NativeJSSocket *)socket_server->ctx;


    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->getJSContext();
    JS::AutoValueArray<2> jparams(cx);;
    JS::RootedValue onmessage(cx);
    JS::RootedValue rval(cx);

    if (nsocket->flags & NATIVE_SOCKET_ISBINARY) {
        JS::RootedObject arrayBuffer(cx, JS_NewArrayBuffer(cx, len));
        uint8_t *data = JS_GetArrayBufferData(arrayBuffer);
        memcpy(data, packet, len);

        jparams[0].setObject(*arrayBuffer);

    } else {
        JS::RootedString jstr(cx, NativeJSUtils::newStringWithEncoding(cx, (char *)packet, len, nsocket->m_Encoding));

        jparams[0].setString(jstr);
    }

    JS::RootedObject obj(cx, nsocket->getJSObject());
    if (JS_GetProperty(cx, obj, "onmessage", &onmessage) &&
        JS_TypeOfValue(cx, onmessage) == JSTYPE_FUNCTION) {

        JS::RootedObject remote(cx, JS_NewObject(cx, NULL, JS::NullPtr(), JS::NullPtr()));

        /*
            TODO: inet_ntoa is not reentrant
        */
        char *cip = inet_ntoa(addr->sin_addr);
        JS::RootedValue jip(cx, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, cip)));

        JS_SetProperty(cx, remote, "ip", jip);
        JS::RootedValue jport(cx, INT_TO_JSVAL(ntohs(addr->sin_port)));
        
        JS_SetProperty(cx, remote, "port", jport);

        jparams[1].setObject(*remote);

        JS_CallFunctionValue(cx, obj, onmessage, jparams, &rval);
    }
}

void NativeJSSocket::onRead()
{
    JS::RootedValue onread(m_Cx);
    JS::RootedValue rval(m_Cx);

    if (!isJSCallable()) {
        return;
    }

    JS::AutoValueArray<2> jparams(m_Cx);
    int dataPosition = 0;

    if (isClientFromOwnServer()) {
        dataPosition = 1;
        JS::RootedObject obj(m_Cx, this->getJSObject());
        jparams[0].setObjectOrNull(obj);
    } else {
        dataPosition = 0;
    }

    if (this->getFlags() & NATIVE_SOCKET_ISBINARY) {
        JS::RootedObject arrayBuffer(m_Cx, JS_NewArrayBuffer(m_Cx, socket->data_in.used));
        uint8_t *data = JS_GetArrayBufferData(arrayBuffer);
        memcpy(data, socket->data_in.data, socket->data_in.used);

        jparams[dataPosition].setObject(*arrayBuffer);

    } else if (this->getFlags() & NATIVE_SOCKET_READLINE) {
        char *pBuf = (char *)socket->data_in.data;
        size_t len = socket->data_in.used;
        char *eol;

        while (len > 0 && (eol = (char *)memchr(pBuf,
            this->getFrameDelimiter(), len)) != NULL) {

            size_t pLen = eol - pBuf;
            len -= pLen;
            if (len-- > 0) {
                this->readFrame(pBuf, pLen);
                pBuf = eol+1;
            }
        }

        if (len && len+this->lineBuffer.pos <= SOCKET_LINEBUFFER_MAX) {
            memcpy(this->lineBuffer.data+this->lineBuffer.pos, pBuf, len);
            this->lineBuffer.pos += len;
        } else if (len) {
            this->lineBuffer.pos = 0;
        }

        return;
    } else {
        JS::RootedString jstr(m_Cx, NativeJSUtils::newStringWithEncoding(m_Cx,
            (char *)socket->data_in.data,
            socket->data_in.used, this->getEncoding()));

        jparams[dataPosition].setString(jstr);
    }

    JS::RootedObject obj(m_Cx, getReceiverJSObject());
    if (JS_GetProperty(m_Cx, obj, "onread", &onread) &&
        JS_TypeOfValue(m_Cx, onread) == JSTYPE_FUNCTION) {
        PACK_TCP(socket->s.fd);
        JS_CallFunctionValue(m_Cx, obj, onread, jparams, &rval);
        FLUSH_TCP(socket->s.fd);
    } 
}

static void native_socket_wrapper_client_read(ape_socket *socket_client,
    ape_global *ape, void *socket_arg)
{
    NativeJSSocket *client = (NativeJSSocket *)socket_client->ctx;

    if (client == NULL) {
        return;
    }

    client->onRead();
}

static void native_socket_wrapper_read(ape_socket *s, ape_global *ape,
    void *socket_arg)
{
    NativeJSSocket *nsocket = (NativeJSSocket *)s->ctx;

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    nsocket->onRead();
}

static void native_socket_wrapper_client_disconnect(ape_socket *socket_client,
    ape_global *ape, void *socket_arg)
{
    JSContext *cx;

    NativeJSSocket *csocket = (NativeJSSocket *)socket_client->ctx;
    if (!csocket || !csocket->isClientFromOwnServer()) {
        return;
    }

    NativeJSSocket *ssocket = csocket->getParentServer();

    if (ssocket == NULL || !ssocket->isJSCallable()) {
        return;
    }

    cx = ssocket->getJSContext();

    JS::RootedValue ondisconnect(cx);
    JS::RootedValue rval(cx);

    JS::AutoValueArray<1> jparams(cx);
    jparams[1].setObject(*csocket->getJSObject());

    csocket->dettach();

    JS::RootedObject obj(cx, ssocket->getJSObject());
    if (JS_GetProperty(cx, obj, "ondisconnect", &ondisconnect) &&
        JS_TypeOfValue(cx, ondisconnect) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, obj, ondisconnect, jparams, &rval);
    }

    NativeJSObj(cx)->unrootObject(csocket->getJSObject());
}

static void native_socket_wrapper_disconnect(ape_socket *s, ape_global *ape,
    void *socket_arg)
{
    JSContext *cx;
    NativeJSSocket *nsocket = (NativeJSSocket *)s->ctx;

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->getJSContext();

    JS::RootedValue ondisconnect(cx);
    JS::RootedValue rval(cx);

    nsocket->dettach();

    JS::RootedObject obj(cx, nsocket->getJSObject());
    if (JS_GetProperty(cx, obj, "ondisconnect", &ondisconnect) &&
        JS_TypeOfValue(cx, ondisconnect) == JSTYPE_FUNCTION) {
        JS_CallFunctionValue(cx, obj, ondisconnect, JS::HandleValueArray::empty(), &rval);
    }

    NativeJSObj(cx)->unrootObject(nsocket->getJSObject());
}

static bool native_Socket_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString host(cx);

    unsigned int port;
    NativeJSSocket *nsocket;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    
    if (!args.isConstructing()) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &Socket_class, args));

    if (!JS_ConvertArguments(cx, args, "Su", host.address(), &port)) {
        return false;
    }

    JSAutoByteString chost(cx, host);

    nsocket = new NativeJSSocket(ret, cx, chost.ptr(), port);

    JS_SetPrivate(ret, nsocket);

    args.rval().setObjectOrNull(ret);

    JS_DefineFunctions(cx, ret, socket_funcs);
    JS_DefineProperties(cx, ret, Socket_props);

    JS::RootedValue isBinary(cx, JSVAL_FALSE);

    JS_SetProperty(cx, ret, "binary", isBinary);

    return true;
}

static bool native_socket_listen(JSContext *cx, unsigned argc, JS::Value *vp)
{
    ape_socket *socket;
    ape_socket_proto protocol = APE_SOCKET_PT_TCP;

    ape_global *net = (ape_global *)JS_GetContextPrivate(cx);

    JSNATIVE_PROLOGUE_CLASS(NativeJSSocket, &Socket_class);

    if (CppObj->isAttached()) {
        return true;
    }

    if (args.length() > 0 && args[0].isString()) {
        JS::RootedString farg(cx, args[0].toString());

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
    /* TODO: need a drain for client socket */
    //socket->callbacks.on_drain      = native_socket_wrapper_client_ondrain;
    socket->callbacks.on_drain = NULL;
    socket->ctx = CppObj;

    CppObj->socket     = socket;

    if (CppObj->m_TCPTimeout) {
        if (!APE_socket_setTimeout(socket, CppObj->m_TCPTimeout)) {
            JS_ReportWarning(cx, "Couldn't set TCP timeout on socket\n");
        }
    }

    if (APE_socket_listen(socket, CppObj->port, CppObj->host, 0, 0) == -1) {
        JS_ReportError(cx, "Can't listen on socket (%s:%d)", CppObj->host,
            CppObj->port);
        /* TODO: close() leak */
        return false;
    }

    NativeJSObj(cx)->rootObjectUntilShutdown(thisobj);

    args.rval().setObjectOrNull(thisobj);

    CppObj->flags |= NATIVE_SOCKET_ISSERVER;

    return true;
}

static bool native_socket_connect(JSContext *cx, unsigned argc, JS::Value *vp)
{
    ape_socket *socket;
    ape_socket_proto protocol = APE_SOCKET_PT_TCP;
    uint16_t localport = 0;

    ape_global *net = (ape_global *)JS_GetContextPrivate(cx);

    JSNATIVE_PROLOGUE_CLASS(NativeJSSocket, &Socket_class);

    if (CppObj->isAttached()) {
        return false;
    }

    if (args.length() > 0 && args[0].isString()) {
        JS::RootedString farg(cx, args[0].toString());

        JSAutoByteString cproto(cx, farg);

        if (strncasecmp("udp", cproto.ptr(), 3) == 0) {
            protocol = APE_SOCKET_PT_UDP;
        } else if (strncasecmp("ssl", cproto.ptr(), 3) == 0) {
            protocol = APE_SOCKET_PT_SSL;
        }

        localport = (args.length() > 1 && args[1].isNumber() ? (uint16_t)args[1].toInt32() : 0);
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

    socket->ctx = CppObj;

    CppObj->socket   = socket;

    if (CppObj->m_TCPTimeout) {
        if (!APE_socket_setTimeout(socket, CppObj->m_TCPTimeout)) {
            JS_ReportWarning(cx, "Couldn't set TCP timeout on socket\n");
        }
    }

    if (APE_socket_connect(socket, CppObj->port, CppObj->host, localport) == -1) {
        JS_ReportError(cx, "Can't connect on socket (%s:%d)", CppObj->host,
            CppObj->port);
        return false;
    }

    NativeJSObj(cx)->rootObjectUntilShutdown(thisobj);

    args.rval().setObjectOrNull(thisobj);

    return true;
}

static bool native_socket_client_sendFile(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
	JS::RootedString file(cx);

    NATIVE_CHECK_ARGS("sendFile", 1);
    
    JSNATIVE_PROLOGUE_CLASS(NativeJSSocket, &socket_client_class);

    if (!CppObj->isAttached()) {
        JS_ReportWarning(cx, "socket.sendFile() Invalid socket (not connected)");
        args.rval().setInt32(-1);
        return true;
    }
    if (!JS_ConvertArguments(cx, args, "S", file.address())) {
        return false;
    }

    JSAutoByteString cfile(cx, file);

    APE_sendfile(CppObj->socket, cfile.ptr());

    return true;
}

static bool native_socket_client_write(JSContext *cx,
    unsigned argc, JS::Value *vp)
{

    NATIVE_CHECK_ARGS("write", 1);

    JSNATIVE_PROLOGUE_CLASS(NativeJSSocket, &socket_client_class);

    if (!CppObj->isAttached()) {

        JS_ReportWarning(cx, "socket.write() Invalid socket (not connected)");
        args.rval().setInt32(-1);
        return true;
    }

    if (args[0].isString()) {

        JSAutoByteString cdata;
        JS::RootedString str(cx, args[0].toString());
        cdata.encodeUtf8(cx, str);

        int ret = CppObj->write((unsigned char *)cdata.ptr(),
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

        int ret = CppObj->write(data, len, APE_DATA_COPY);

        args.rval().setInt32(ret);

    } else {
        JS_ReportError(cx, "write() invalid data (must be either a string or an ArrayBuffer)");
        return false;
    }

    return true;
}

static bool native_socket_write(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NATIVE_CHECK_ARGS("write", 1);

    JSNATIVE_PROLOGUE_CLASS(NativeJSSocket, &Socket_class);

    if (!CppObj->isAttached()) {

        JS_ReportWarning(cx, "socket.write() Invalid socket (not connected)");
        args.rval().setInt32(-1);
        return true;
    }

    if (args[0].isString()) {
        JSAutoByteString cdata;
        JS::RootedString str(cx, args[0].toString());
        cdata.encodeUtf8(cx, str);

        int ret = CppObj->write((unsigned char*)cdata.ptr(),
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

        int ret = CppObj->write(data, len, APE_DATA_COPY);

        args.rval().setInt32(ret);

    } else {
        JS_ReportError(cx, "write() invalid data (must be either a string or an ArrayBuffer)");
        return false;
    }

    return true;
}

static bool native_socket_client_close(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeJSSocket, &socket_client_class);

    if (!CppObj->isAttached()) {
        JS_ReportWarning(cx, "socket.close() Invalid socket (not connected)");
        args.rval().setInt32(-1);
        return true;
    }

    CppObj->shutdown();

    return true;
}

static bool native_socket_close(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeJSSocket, &Socket_class);

    if (!CppObj->isAttached()) {
        JS_ReportWarning(cx, "socket.close() Invalid socket (not connected)");
        args.rval().setInt32(-1);
        return true;
    }

    CppObj->shutdown();

    return true;
}

static bool native_socket_sendto(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, &args.thisv().toObject());

    NATIVE_CHECK_ARGS("sendto", 3);

    if (JS_InstanceOf(cx, caller, &Socket_class, &args) == false) {
        return false;
    }

    NativeJSSocket *nsocket = (NativeJSSocket *)JS_GetPrivate(caller);

    if (nsocket == NULL || !nsocket->isAttached()) {
        return true;
    }

    if (!(nsocket->flags & NATIVE_SOCKET_ISSERVER)) {
        JS_ReportError(cx, "sendto() is only available on listening socket");
        return false;
    }

    if (!args[0].isString()) {
        JS_ReportError(cx, "sendto() IP must be a string");
        return false;
    }

    JS::RootedString ip(cx, args[0].toString());
    unsigned int port = args[1].isNumber() ? args[1].toInt32() : 0;

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
    NativeJSSocket *nsocket = (NativeJSSocket *)JS_GetPrivate(obj);

    if (nsocket != NULL) {

        if (nsocket->socket) {
            nsocket->socket->ctx = NULL;
            APE_socket_shutdown_now(nsocket->socket);
        }

        delete nsocket;
    }
}

NativeJSSocket::NativeJSSocket(JS::HandleObject obj, JSContext *cx,
    const char *host, unsigned short port)
    :  NativeJSExposer<NativeJSSocket>(obj, cx),
    socket(NULL), flags(0), m_ParentServer(NULL), m_TCPTimeout(0)
{
    this->host = strdup(host);
    this->port = port;

    lineBuffer.pos = 0;
    lineBuffer.data = NULL;

    m_Encoding = NULL;
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

    if (m_Encoding) {
        free(m_Encoding);
    }
}

bool NativeJSSocket::isAttached()
{
    return (socket != NULL);
}

bool NativeJSSocket::isJSCallable()
{
    if (m_ParentServer && !m_ParentServer->getJSObject()) {
        return false;
    }
    return (this->getJSObject() != NULL);
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
    if (!socket || !socket->ctx) {
        return 0;
    }

    return APE_socket_write(socket, data, len, data_type);
}

void NativeJSSocket::disconnect()
{
    APE_socket_shutdown_now(socket);
}

void NativeJSSocket::shutdown()
{
    if (!socket || !socket->ctx) {
        return;
    }
    APE_socket_shutdown(socket);
}

NATIVE_OBJECT_EXPOSE(Socket)
