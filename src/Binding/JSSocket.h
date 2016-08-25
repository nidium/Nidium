/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jssocket_h__
#define binding_jssocket_h__

#include <ape_netlib.h>

#include "Binding/JSExposer.h"
#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

#define SOCKET_LINEBUFFER_MAX 8192



class JSSocket : public ClassMapper<JSSocket>
{
public:
    static void RegisterObject(JSContext *cx);
    JSSocket(const char *host,
             unsigned short port);
    virtual ~JSSocket();

    static JSSocket *Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj);
    static JSFunctionSpec *ListMethods();
    static JSPropertySpec *ListProperties();

    enum SocketType
    {
        kSocketType_Binary          = 1 << 0,
        kSocketType_Readline        = 1 << 1,
        kSocketType_Server          = 1 << 2,
        kSocketType_ConnectedClient = 1 << 3
    };
    int write(unsigned char *data,
              size_t len,
              ape_socket_data_autorelease data_type);

    void disconnect();
    void shutdown();

    void dettach();
    bool isAttached();

    bool isJSCallable();

    void onRead(const char *data, size_t len);

    JSSocket *getParentServer() const
    {
        return m_ParentServer;
    }

    void setParentServer(JSSocket *parent)
    {
        m_ParentServer = parent;
        m_Flags |= JSSocket::kSocketType_ConnectedClient;
    }

    int getFlags() const
    {
        return m_ParentServer ? m_ParentServer->m_Flags : m_Flags;
    }

    const char *getEncoding() const
    {
        return m_ParentServer ? m_ParentServer->m_Encoding : m_Encoding;
    }

    uint8_t getFrameDelimiter() const
    {
        return m_ParentServer ? m_ParentServer->m_FrameDelimiter
                              : m_FrameDelimiter;
    }

    bool isClientFromOwnServer() const
    {
        return (m_ParentServer != NULL);
    }

    JSObject *getReceiverJSObject() const
    {
        return m_ParentServer ? m_ParentServer->getJSObject()
                              : this->getJSObject();
    }

    char *m_Host;
    unsigned short m_Port;
    ape_socket *m_Socket;
    int m_Flags;

    char *m_Encoding;

    struct
    {
        char *data;
        size_t pos;
    } m_LineBuffer;

    uint8_t m_FrameDelimiter;

    JSSocket *m_ParentServer;

    int m_TCPTimeout;

protected:
    NIDIUM_DECL_JSCALL(listen);
    NIDIUM_DECL_JSCALL(connect);
    NIDIUM_DECL_JSCALL(write);
    NIDIUM_DECL_JSCALL(disconnect);
    NIDIUM_DECL_JSCALL(sendTo);

    NIDIUM_DECL_JSGETTERSETTER(binary);
    NIDIUM_DECL_JSGETTERSETTER(readline);
    NIDIUM_DECL_JSGETTERSETTER(encoding);
    NIDIUM_DECL_JSGETTERSETTER(timeout);
private:
    void readFrame(const char *buf, size_t len);
};

class JSSocketClientConnection : public JSSocket,
                                 public ClassMapper<JSSocketClientConnection>
{
public:
    NIDIUM_CLASSMAPPER_FIX_MULTIPLE_BASE(JSSocketClientConnection);

    JSSocketClientConnection(const char *host, unsigned short port) :
        JSSocket(host, port)
    {

    }

    static void RegisterObject(JSContext *cx);
    virtual ~JSSocketClientConnection();

    static JSSocketClientConnection *Constructor(JSContext *cx,
        JS::CallArgs &args,
        JS::HandleObject obj);

    static JSFunctionSpec *ListMethods();
protected:
    NIDIUM_DECL_JSCALL(write);
    NIDIUM_DECL_JSCALL(disconnect);
    NIDIUM_DECL_JSCALL(sendFile);
};

} // namespace Binding
} // namespace Nidium

#endif
