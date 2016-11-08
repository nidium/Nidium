/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSSocket.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Binding/JSUtils.h"

namespace Nidium {
namespace Binding {

JSSocketBase::JSSocketBase(JSContext *cx, const char *host,
                   unsigned short port)
    : m_Socket(NULL), m_Flags(0),
      m_FrameDelimiter('\n'), m_ParentServer(NULL), m_TCPTimeout(0), m_Cx(cx)
{
    m_Host = strdup(host);
    m_Port = port;

    m_LineBuffer.pos  = 0;
    m_LineBuffer.data = NULL;

    m_Encoding = NULL;
}

void JSSocketBase::readFrame(const char *buf, size_t len)
{
    JS::RootedValue onread(m_Cx);
    JS::RootedValue rval(m_Cx);
    JS::AutoValueArray<2> jdata(m_Cx);
    JS::RootedString tstr(m_Cx, JSUtils::NewStringWithEncoding(
                                    m_Cx, buf, len, this->getEncoding()));
    JS::RootedString jstr(m_Cx);
    jstr = tstr;

    if (m_LineBuffer.pos
        && (this->getFlags() & JSSocket::kSocketType_Readline)) {
        JS::RootedString left(m_Cx, JSUtils::NewStringWithEncoding(
                                        m_Cx, m_LineBuffer.data,
                                        m_LineBuffer.pos, this->getEncoding()));

        jstr             = JS_ConcatStrings(m_Cx, left, tstr);
        m_LineBuffer.pos = 0;
    }

    if (isClientFromOwnServer()) {
        jdata[0].setObjectOrNull(this->getjsobj());
        jdata[1].setString(jstr);
    } else {
        jdata[0].setString(jstr);
    }

    JS::RootedObject obj(m_Cx, getReceiverJSObject());
    if (JS_GetProperty(m_Cx, obj, "onread", &onread)
        && JS_TypeOfValue(m_Cx, onread) == JSTYPE_FUNCTION) {
        PACK_TCP(m_Socket->s.fd);
        JS_CallFunctionValue(m_Cx, obj, onread, jdata, &rval);
        FLUSH_TCP(m_Socket->s.fd);
    }
}

bool JSSocketBase::isAttached()
{
    return (m_Socket != NULL);
}

bool JSSocketBase::isJSCallable()
{
    if (m_ParentServer && !m_ParentServer->getjsobj()) {
        return false;
    }
    return (this->getjsobj() != NULL);
}

void JSSocketBase::dettach()
{
    if (isAttached()) {
        m_Socket->ctx = NULL;
        m_Socket      = NULL;
    }
}

int JSSocketBase::write(unsigned char *data,
                    size_t len,
                    ape_socket_data_autorelease data_type)
{
    if (!m_Socket || !m_Socket->ctx) {
        return 0;
    }

    return APE_socket_write(m_Socket, data, len, data_type);
}

void JSSocketBase::disconnect()
{
    APE_socket_shutdown_now(m_Socket);
}

void JSSocketBase::onRead(const char *data, size_t len)
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
        JS::RootedObject obj(m_Cx, this->getjsobj());
        jparams[0].setObjectOrNull(obj);
    } else {
        dataPosition = 0;
    }

    if (this->getFlags() & JSSocket::kSocketType_Binary) {
        JS::RootedObject arrayBuffer(m_Cx,
            JSUtils::NewArrayBufferWithCopiedContents(m_Cx, len, data));

        jparams[dataPosition].setObject(*arrayBuffer);

    } else if (this->getFlags() & JSSocket::kSocketType_Readline) {
        const char *pBuf = data;
        size_t tlen      = len;
        char *eol;

        // TODO: new style cast
        while (
            tlen > 0
            && (eol = (char *)(memchr(pBuf, this->getFrameDelimiter(), tlen)))
                   != NULL) {

            size_t pLen = eol - pBuf;
            tlen -= pLen;
            if (tlen-- > 0) {
                this->readFrame(pBuf, pLen);
                pBuf = eol + 1;
            }
        }

        if (tlen && tlen + m_LineBuffer.pos <= SOCKET_LINEBUFFER_MAX) {
            memcpy(m_LineBuffer.data + m_LineBuffer.pos, pBuf, tlen);
            m_LineBuffer.pos += tlen;
        } else if (tlen) {
            m_LineBuffer.pos = 0;
        }

        return;
    } else {
        JS::RootedString jstr(m_Cx, JSUtils::NewStringWithEncoding(
                                        m_Cx, data, len, this->getEncoding()));

        jparams[dataPosition].setString(jstr);
    }

    JS::RootedObject obj(m_Cx, getReceiverJSObject());
    if (JS_GetProperty(m_Cx, obj, "onread", &onread)
        && JS_TypeOfValue(m_Cx, onread) == JSTYPE_FUNCTION) {
        PACK_TCP(m_Socket->s.fd);
        JS_CallFunctionValue(m_Cx, obj, onread, jparams, &rval);
        FLUSH_TCP(m_Socket->s.fd);
    }
}

void JSSocketBase::shutdown()
{
    if (!m_Socket || !m_Socket->ctx) {
        return;
    }
    APE_socket_shutdown(m_Socket);
}

JSSocketBase::~JSSocketBase()
{
    if (isAttached()) {
        m_Socket->ctx = NULL;
        this->disconnect();
    }
    free(m_Host);
    if (m_LineBuffer.data) {
        free(m_LineBuffer.data);
    }

    if (m_Encoding) {
        free(m_Encoding);
    }
}


// }}}

// {{{ Socket server/client common callbacks
static void nidium_socket_wrapper_client_onmessage(ape_socket *socket_server,
                                                   ape_global *ape,
                                                   const unsigned char *packet,
                                                   size_t len,
                                                   struct sockaddr_in *addr,
                                                   void *socket_arg)
{
    JSContext *cx;
    JSSocket *nsocket = static_cast<JSSocket *>(socket_server->ctx);

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->getJSContext();
    JS::AutoValueArray<2> jparams(cx);
    JS::RootedValue onmessage(cx);
    JS::RootedValue rval(cx);

    if (nsocket->m_Flags & JSSocket::kSocketType_Binary) {
        JS::RootedObject arrayBuffer(cx,
            JSUtils::NewArrayBufferWithCopiedContents(cx, len, packet));

        jparams[0].setObject(*arrayBuffer);

    } else {
        // TODO: new style cast
        JS::RootedString jstr(
            cx, JSUtils::NewStringWithEncoding(cx, (char *)(packet), len,
                                               nsocket->m_Encoding));

        jparams[0].setString(jstr);
    }

    JS::RootedObject obj(cx, nsocket->getJSObject());
    if (JS_GetProperty(cx, obj, "onmessage", &onmessage)
        && JS_TypeOfValue(cx, onmessage) == JSTYPE_FUNCTION) {

        JS::RootedObject remote(cx, JS_NewPlainObject(cx));

        /*
            TODO: inet_ntoa is not reentrant
        */
        char *cip = inet_ntoa(addr->sin_addr);
        JS::RootedString jip(cx, JS_NewStringCopyZ(cx, cip));
        JS::RootedValue vip(cx, JS::StringValue(jip));

        JS_SetProperty(cx, remote, "ip", vip);
        JS::RootedValue jport(cx, JS::Int32Value(ntohs(addr->sin_port)));

        JS_SetProperty(cx, remote, "port", jport);

        jparams[1].setObject(*remote);

        JS_CallFunctionValue(cx, obj, onmessage, jparams, &rval);
    }
}
// }}}

// {{{ Socket server callbacks
static void nidium_socket_wrapper_onaccept(ape_socket *socket_server,
                                           ape_socket *socket_client,
                                           ape_global *ape,
                                           void *socket_arg)
{
    JSContext *m_Cx;

    JSSocket *nsocket = static_cast<JSSocket *>(socket_server->ctx);

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    m_Cx = nsocket->getJSContext();

    JS::RootedValue onaccept(m_Cx);
    JS::RootedValue rval(m_Cx);
    JS::AutoValueArray<1> params(m_Cx);

    JSSocketClientConnection *sobj =
        new JSSocketClientConnection(m_Cx, APE_socket_ipv4(socket_client), 0);

    socket_client->ctx = sobj;

    JS::RootedObject jclient(m_Cx,
        JSSocketClientConnection::CreateObject(m_Cx, sobj));

    sobj->root();

    sobj->m_ParentServer = nsocket;
    sobj->m_Socket       = socket_client;

    if (sobj->getFlags() & JSSocket::kSocketType_Readline) {
        sobj->m_LineBuffer.data
            = static_cast<char *>(malloc(sizeof(char) * SOCKET_LINEBUFFER_MAX));
        sobj->m_LineBuffer.pos = 0;
    }


    NIDIUM_JSOBJ_SET_PROP_CSTR(jclient, "ip", APE_socket_ipv4(socket_client));

    params[0].setObject(*jclient);

    if (APE_SOCKET_IS_LZ4(socket_server, tx)) {
        APE_socket_enable_lz4(socket_client,
                              APE_LZ4_COMPRESS_TX | APE_LZ4_COMPRESS_RX);
    }

    JS::RootedObject socketjs(m_Cx, nsocket->getJSObject());

    if (JS_GetProperty(m_Cx, socketjs, "onaccept", &onaccept)
        && JS_TypeOfValue(m_Cx, onaccept) == JSTYPE_FUNCTION) {

        PACK_TCP(socket_client->s.fd);
        JS::RootedValue onacceptVal(m_Cx, onaccept);
        JS_CallFunctionValue(m_Cx, socketjs, onacceptVal, params, &rval);
        FLUSH_TCP(socket_client->s.fd);
    }
}

static void nidium_socket_wrapper_client_read(ape_socket *socket_client,
                                              const uint8_t *data,
                                              size_t len,
                                              ape_global *ape,
                                              void *socket_arg)
{
    JSSocket *client = static_cast<JSSocket *>(socket_client->ctx);

    if (client == NULL) {
        return;
    }

    client->onRead(reinterpret_cast<const char *>(data), len);
}

static void nidium_socket_wrapper_client_disconnect(ape_socket *socket_client,
                                                    ape_global *ape,
                                                    void *socket_arg)
{
    JSContext *cx;

    JSSocketClientConnection *csocket =
        static_cast<JSSocketClientConnection *>(socket_client->ctx);

    if (!csocket || !csocket->isClientFromOwnServer()) {
        return;
    }

    JSSocket *ssocket =
        reinterpret_cast<JSSocket *>(csocket->getParentServer());

    if (ssocket == NULL || !ssocket->isJSCallable()) {
        return;
    }

    cx = ssocket->getJSContext();

    JS::RootedValue ondisconnect(cx);
    JS::RootedValue rval(cx);

    JS::AutoValueArray<1> jparams(cx);

    jparams[0].setObject(*csocket->getJSObject());

    csocket->dettach();

    JS::RootedObject obj(cx, ssocket->getJSObject());
    if (JS_GetProperty(cx, obj, "ondisconnect", &ondisconnect)
        && JS_TypeOfValue(cx, ondisconnect) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, obj, ondisconnect, jparams, &rval);
    }

    csocket->unroot();
}
// }}}


bool JSSocket::JSSetter_binary(JSContext *cx, JS::MutableHandleValue vp)
{
    if (vp.isBoolean()) {
        m_Flags = (vp.toBoolean() == true
                   ? m_Flags | kSocketType_Binary
                   : m_Flags & ~kSocketType_Binary);

    }

    return true;
}

bool JSSocket::JSGetter_binary(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setBoolean(m_Flags & kSocketType_Binary);

    return true;
}

bool JSSocket::JSSetter_readline(JSContext *cx, JS::MutableHandleValue vp)
{
    bool isactive
        = ((vp.isBoolean() && vp.toBoolean() == true) || vp.isInt32());

    if (isactive) {

        m_Flags |= kSocketType_Readline;

        if (m_LineBuffer.data == NULL) {

            m_LineBuffer.data = static_cast<char *>(
                malloc(sizeof(char) * SOCKET_LINEBUFFER_MAX));
            m_LineBuffer.pos = 0;
        }

        /*
            Default delimiter is line feed.
        */
        m_FrameDelimiter
            = vp.isBoolean() ? '\n' : vp.toInt32() & 0xFF;

    } else {
        m_Flags &= ~kSocketType_Readline;
    }

    return true;
}

bool JSSocket::JSGetter_readline(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setBoolean(m_Flags & kSocketType_Readline);

    return true;
}

bool JSSocket::JSSetter_encoding(JSContext *cx, JS::MutableHandleValue vp)
{
    if (vp.isString()) {
        JSAutoByteString enc(cx, vp.toString());
        if (m_Encoding) {
            free(m_Encoding);
        }
        m_Encoding = strdup(enc.ptr());
    }

    return true;
}

bool JSSocket::JSGetter_encoding(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setString(JS_NewStringCopyZ(
                cx, m_Encoding ? m_Encoding : "ascii"));

    return true;
}

bool JSSocket::JSSetter_timeout(JSContext *cx, JS::MutableHandleValue vp)
{

    uint32_t timeout;

    if (!JS::ToUint32(cx, vp, &timeout)) {
        return true;
    }

    m_TCPTimeout = APE_ABS(timeout);

    if (m_Socket
        && !APE_socket_setTimeout(m_Socket,
                                  m_TCPTimeout)) {

        JS_ReportWarning(cx, "Couldn't set TCP timeout on socket");
    }

    return true;
}

bool JSSocket::JSGetter_timeout(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(m_TCPTimeout);

    return true;
}


JSSocket *JSSocket::Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj)
{
    JS::RootedString host(cx);

    unsigned int port;
    JSSocket *nsocket;

    if (!JS_ConvertArguments(cx, args, "Su", host.address(), &port)) {
        return nullptr;
    }

    JSAutoByteString chost(cx, host);

    nsocket = new JSSocket(cx, chost.ptr(), port);

    return nsocket;
}

bool JSSocket::JS_listen(JSContext *cx, JS::CallArgs &args)
{
    ape_socket *socket;
    ape_socket_proto protocol = APE_SOCKET_PT_TCP;
    bool isLZ4                = false;

    ape_global *net = static_cast<ape_global *>(JS_GetContextPrivate(cx));

    if (this->isAttached()) {
        return true;
    }

    if (args.length() > 0 && args[0].isString()) {
        JS::RootedString farg(cx, args[0].toString());

        JSAutoByteString cproto(cx, farg);

        if (strncasecmp("udp", cproto.ptr(), 3) == 0) {
            protocol = APE_SOCKET_PT_UDP;
        } else if (strncasecmp("tcp-lz4", cproto.ptr(), 7) == 0) {
            isLZ4 = true;
        }
    }

    if ((socket = APE_socket_new(protocol, 0, net)) == NULL) {
        JS_ReportError(cx, "Failed to create socket");
        return false;
    }

    socket->callbacks.on_connect    = nidium_socket_wrapper_onaccept;
    socket->callbacks.on_read       = nidium_socket_wrapper_client_read;
    socket->callbacks.on_disconnect = nidium_socket_wrapper_client_disconnect;
    socket->callbacks.on_message    = nidium_socket_wrapper_client_onmessage;
    /* TODO: need a drain for client socket */
    // socket->callbacks.on_drain      = nidium_socket_wrapper_client_ondrain;
    socket->callbacks.on_drain = NULL;
    socket->ctx                = this;

    m_Socket = socket;

    if (m_TCPTimeout) {
        if (!APE_socket_setTimeout(socket, m_TCPTimeout)) {
            JS_ReportWarning(cx, "Couldn't set TCP timeout on socket\n");
        }
    }

    if (APE_socket_listen(socket, m_Port, m_Host, 0, 0) == -1) {
        JS_ReportError(cx, "Can't listen on socket (%s:%d)", m_Host,
                       m_Port);
        /* TODO: close() leak */
        return false;
    }

    if (isLZ4) {
        APE_socket_enable_lz4(socket,
                              APE_LZ4_COMPRESS_TX | APE_LZ4_COMPRESS_RX);
    }

    this->root();

    args.rval().setObjectOrNull(m_Instance);

    m_Flags |= JSSocket::kSocketType_Server;

    return true;
}

bool JSSocket::JS_write(JSContext *cx, JS::CallArgs &args)
{
    if (!this->isAttached()) {

        JS_ReportWarning(cx, "socket.write() Invalid socket (not connected)");
        args.rval().setInt32(-1);
        return true;
    }

    if (args[0].isString()) {
        JSAutoByteString cdata;
        JS::RootedString str(cx, args[0].toString());
        cdata.encodeUtf8(cx, str);

        int ret = this->write(reinterpret_cast<unsigned char *>(cdata.ptr()),
                                strlen(cdata.ptr()), APE_DATA_COPY);

        args.rval().setInt32(ret);

    } else if (args[0].isObject()) {
        JSObject *objdata = args[0].toObjectOrNull();

        if (!objdata || !JS_IsArrayBufferObject(objdata)) {
            JS_ReportError(cx,
                           "write() invalid data (must be either a string or "
                           "an ArrayBuffer)");
            return false;
        }
        uint32_t len  = JS_GetArrayBufferByteLength(objdata);

        bool shared;
        JS::AutoCheckCannotGC nogc;

        uint8_t *data = JS_GetArrayBufferData(objdata, &shared, nogc);

        int ret = this->write(data, len, APE_DATA_COPY);

        args.rval().setInt32(ret);

    } else {
        JS_ReportError(
            cx,
            "write() invalid data (must be either a string or an ArrayBuffer)");
        return false;
    }

    return true;
}

bool JSSocket::JS_disconnect(JSContext *cx, JS::CallArgs &args)
{
    if (!this->isAttached()) {
        JS_ReportWarning(cx, "socket.close() Invalid socket (not connected)");
        args.rval().setInt32(-1);
        return true;
    }

    this->shutdown();

    return true;
}

bool JSSocket::JS_sendTo(JSContext *cx, JS::CallArgs &args)
{
    if (!this->isAttached()) {
        return true;
    }

    if (!(m_Flags & JSSocket::kSocketType_Server)) {
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

        ape_socket_write_udp(this->m_Socket, cdata.ptr(), cdata.length(),
                             cip.ptr(), static_cast<uint16_t>(port));
    } else if (args[2].isObject()) {
        JSObject *objdata = args[2].toObjectOrNull();

        if (!objdata || !JS_IsArrayBufferObject(objdata)) {
            JS_ReportError(cx,
                           "sendTo() invalid data (must be either a string or "
                           "an ArrayBuffer)");
            return false;
        }
        uint32_t len  = JS_GetArrayBufferByteLength(objdata);

        bool shared;
        JS::AutoCheckCannotGC nogc;
        uint8_t *data = JS_GetArrayBufferData(objdata, &shared, nogc);

        ape_socket_write_udp(this->m_Socket, reinterpret_cast<char *>(data),
                             len, cip.ptr(), static_cast<uint16_t>(port));

    } else {
        JS_ReportError(cx,
                       "sendTo() invalid data (must be either a string or an "
                       "ArrayBuffer)");
        return false;
    }

    return true;
}
// }}}

// {{{ Socket client callbacks
static void nidium_socket_wrapper_onconnected(ape_socket *s,
                                              ape_global *ape,
                                              void *socket_arg)
{
    JSContext *cx;

    JSSocket *nsocket = static_cast<JSSocket *>(s->ctx);

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->getJSContext();
    JS::RootedValue onconnect(cx);
    JS::RootedValue rval(cx);

    JS::RootedObject obj(cx, nsocket->getJSObject());

    if (JS_GetProperty(cx, obj, "onconnect", &onconnect)
        && JS_TypeOfValue(cx, onconnect) == JSTYPE_FUNCTION) {

        PACK_TCP(s->s.fd);
        JS_CallFunctionValue(cx, obj, onconnect, JS::HandleValueArray::empty(),
                             &rval);
        FLUSH_TCP(s->s.fd);
    }
}

static void nidium_socket_wrapper_read(ape_socket *s,
                                       const uint8_t *data,
                                       size_t len,
                                       ape_global *ape,
                                       void *socket_arg)
{
    JSSocket *nsocket = static_cast<JSSocket *>(s->ctx);

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    nsocket->onRead(reinterpret_cast<const char *>(data), len);
}

static void nidium_socket_wrapper_disconnect(ape_socket *s,
                                             ape_global *ape,
                                             void *socket_arg)
{
    JSContext *cx;
    JSSocket *nsocket = static_cast<JSSocket *>(s->ctx);

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->getJSContext();

    JS::RootedValue ondisconnect(cx);
    JS::RootedValue rval(cx);

    nsocket->dettach();

    JS::RootedObject obj(cx, nsocket->getJSObject());
    if (JS_GetProperty(cx, obj, "ondisconnect", &ondisconnect)
        && JS_TypeOfValue(cx, ondisconnect) == JSTYPE_FUNCTION) {
        JS_CallFunctionValue(cx, obj, ondisconnect,
                             JS::HandleValueArray::empty(), &rval);
    }

    nsocket->unroot();
}

static void nidium_socket_wrapper_client_ondrain(ape_socket *socket_server,
                                                 ape_global *ape,
                                                 void *socket_arg)
{
    JSContext *cx;
    JSSocket *nsocket = static_cast<JSSocket *>(socket_server->ctx);

    if (nsocket == NULL || !nsocket->isJSCallable()) {
        return;
    }

    cx = nsocket->getJSContext();

    JS::RootedValue ondrain(cx);
    JS::RootedValue rval(cx);
    JS::RootedObject obj(cx, nsocket->getJSObject());

    if (JS_GetProperty(cx, obj, "ondrain", &ondrain)
        && JS_TypeOfValue(cx, ondrain) == JSTYPE_FUNCTION) {
        JS_CallFunctionValue(cx, obj, ondrain, JS::HandleValueArray::empty(),
                             &rval);
    }
}
// }}}

// {{{ Socket client implementation
bool JSSocket::JS_connect(JSContext *cx, JS::CallArgs &args)
{
    ape_socket *socket;
    ape_socket_proto protocol = APE_SOCKET_PT_TCP;
    uint16_t localport        = 0;
    bool isLZ4                = false;

    ape_global *net = static_cast<ape_global *>(JS_GetContextPrivate(cx));

    if (this->isAttached()) {
        return false;
    }

    if (args.length() > 0 && args[0].isString()) {
        JS::RootedString farg(cx, args[0].toString());

        JSAutoByteString cproto(cx, farg);

        if (strncasecmp("udp", cproto.ptr(), 3) == 0) {
            protocol = APE_SOCKET_PT_UDP;
        } else if (strncasecmp("ssl", cproto.ptr(), 3) == 0) {
            protocol = APE_SOCKET_PT_SSL;
        } else if (strncasecmp("unix", cproto.ptr(), 4) == 0) {
            protocol = APE_SOCKET_PT_UNIX;
        } else if (strncasecmp("tcp-lz4", cproto.ptr(), 7) == 0) {
            isLZ4 = true;
        }

        localport = (args.length() > 1 && args[1].isNumber()
                         ? static_cast<uint16_t>(args[1].toInt32())
                         : 0);
    }

    if ((socket = APE_socket_new(protocol, 0, net)) == NULL) {
        JS_ReportError(cx, "Failed to create socket");
        return false;
    }

    socket->callbacks.on_connected  = nidium_socket_wrapper_onconnected;
    socket->callbacks.on_read       = nidium_socket_wrapper_read;
    socket->callbacks.on_disconnect = nidium_socket_wrapper_disconnect;
    socket->callbacks.on_message    = nidium_socket_wrapper_client_onmessage;
    socket->callbacks.on_drain      = nidium_socket_wrapper_client_ondrain;

    socket->ctx = this;

    m_Socket = socket;

    if (m_TCPTimeout) {
        if (!APE_socket_setTimeout(socket, m_TCPTimeout)) {
            JS_ReportWarning(cx, "Couldn't set TCP timeout on socket\n");
        }
    }

    if (isLZ4) {
        APE_socket_enable_lz4(socket,
                              APE_LZ4_COMPRESS_TX | APE_LZ4_COMPRESS_RX);
    }

    if (APE_socket_connect(socket, m_Port, m_Host, localport)
        == -1) {
        JS_ReportError(cx, "Can't connect on socket (%s:%d)", m_Host,
                       m_Port);
        return false;
    }

    this->root();

    args.rval().setObjectOrNull(m_Instance);

    return true;
}

bool JSSocketClientConnection::JS_sendFile(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString file(cx);

    if (!this->isAttached()) {
        JS_ReportWarning(cx,
                         "socket.sendFile() Invalid socket (not connected)");
        args.rval().setInt32(-1);
        return true;
    }
    if (!JS_ConvertArguments(cx, args, "S", file.address())) {
        return false;
    }

    JSAutoByteString cfile(cx, file);

    APE_sendfile(this->m_Socket, cfile.ptr());

    return true;
}

bool JSSocketClientConnection::JS_write(JSContext *cx, JS::CallArgs &args)
{

    if (!this->isAttached()) {

        JS_ReportWarning(cx, "socket.write() Invalid socket (not connected)");
        args.rval().setInt32(-1);
        return true;
    }

    if (args[0].isString()) {

        JSAutoByteString cdata;
        JS::RootedString str(cx, args[0].toString());
        cdata.encodeUtf8(cx, str);

        int ret = this->write(reinterpret_cast<unsigned char *>(cdata.ptr()),
                                strlen(cdata.ptr()), APE_DATA_COPY);

        args.rval().setInt32(ret);

    } else if (args[0].isObject()) {
        JSObject *objdata = args[0].toObjectOrNull();

        if (!objdata || !JS_IsArrayBufferObject(objdata)) {
            JS_ReportError(cx,
                           "write() invalid data (must be either a string or "
                           "an ArrayBuffer)");
            return false;
        }
        uint32_t len  = JS_GetArrayBufferByteLength(objdata);

        bool shared;
        JS::AutoCheckCannotGC nogc;
        uint8_t *data = JS_GetArrayBufferData(objdata, &shared, nogc);

        int ret = this->write(data, len, APE_DATA_COPY);

        args.rval().setInt32(ret);

    } else {
        JS_ReportError(
            cx,
            "write() invalid data (must be either a string or an ArrayBuffer)");
        return false;
    }

    return true;
}

bool JSSocketClientConnection::JS_disconnect(JSContext *cx, JS::CallArgs &args)
{
    if (!this->isAttached()) {
        JS_ReportWarning(cx, "socket.close() Invalid socket (not connected)");
        args.rval().setInt32(-1);
        return true;
    }

    this->shutdown();

    return true;
}
// }}}



JSFunctionSpec *JSSocket::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSSocket, listen, 0),
        CLASSMAPPER_FN(JSSocket, connect, 0),
        CLASSMAPPER_FN(JSSocket, write, 1),
        CLASSMAPPER_FN(JSSocket, disconnect, 0),
        CLASSMAPPER_FN(JSSocket, sendTo, 3),

        JS_FS_END
    };

    return funcs;
}

JSPropertySpec *JSSocket::ListProperties()
{
    static JSPropertySpec props[] = {
        CLASSMAPPER_PROP_GS(JSSocket, binary),
        CLASSMAPPER_PROP_GS(JSSocket, readline),
        CLASSMAPPER_PROP_GS(JSSocket, encoding),
        CLASSMAPPER_PROP_GS(JSSocket, timeout),

        JS_PS_END
    };

    return props;
}

void JSSocket::RegisterObject(JSContext *cx)
{
    JSSocket::ExposeClass<2>(cx, "Socket");
    JSSocketClientConnection::ExposeClass(cx, "SocketClientConnection");
}


JSFunctionSpec *JSSocketClientConnection::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSSocketClientConnection, sendFile, 1),
        CLASSMAPPER_FN(JSSocketClientConnection, write, 1),
        CLASSMAPPER_FN(JSSocketClientConnection, disconnect, 0),
        JS_FS_END
    };

    return funcs;
}


} // namespace Binding
} // namespace Nidium
