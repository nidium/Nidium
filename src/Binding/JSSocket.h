/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jssocket_h__
#define binding_jssocket_h__

#include <ape_netlib.h>
#include <ape_socket.h>

#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

#define SOCKET_LINEBUFFER_MAX 8192

class JSSocketBase
{
protected:
    JSSocketBase(JSContext *cx, const char *host,
             unsigned short port);
    virtual ~JSSocketBase();

public:
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

    JSSocketBase *getParentServer() const
    {
        return m_ParentServer;
    }

    void setParentServer(JSSocketBase *parent)
    {
        m_ParentServer = parent;
        m_Flags |= JSSocketBase::kSocketType_ConnectedClient;
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
        return m_ParentServer ? m_ParentServer->getjsobj()
                              : this->getjsobj();
    }

    virtual JSObject *getjsobj() const=0;
    void readFrame(const char *buf, size_t len);

    /*
        These need to be public because we don't forward APE_socket callbacks
        to the class directly.

        We will need to implement a C++ APE_Socket wrapper in order to fix this
    */
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

    JSSocketBase *m_ParentServer;

    int m_TCPTimeout;

    JSContext *m_Cx;
};

class JSSocket : public JSSocketBase,
                 public ClassMapper<JSSocket>
{
public:
    using ClassMapper<JSSocket>::m_Cx;

    JSSocket(JSContext *cx, const char *host, unsigned short port) :
        JSSocketBase(cx, host, port)
    {

    }

    JSObject *getjsobj() const
    {
        return m_Instance;
    }

    virtual ~JSSocket(){};

    static void RegisterObject(JSContext *cx);
    static JSSocket *Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj);
    static JSFunctionSpec *ListMethods();
    static JSPropertySpec *ListProperties();




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

};

class JSSocketClientConnection : public JSSocketBase,
                                 public ClassMapper<JSSocketClientConnection>
{
public:
    using ClassMapper<JSSocketClientConnection>::m_Cx;

    JSSocketClientConnection(JSContext *cx,
        const char *host, unsigned short port) :
        JSSocketBase(cx, host, port)
    {
    }

    JSObject *getjsobj() const
    {
        return m_Instance;
    }

    virtual ~JSSocketClientConnection(){};

    static JSFunctionSpec *ListMethods();
protected:
#ifndef _MSC_VER
    NIDIUM_DECL_JSCALL(sendFile);
#endif
    NIDIUM_DECL_JSCALL(write);
    NIDIUM_DECL_JSCALL(disconnect);
};

} // namespace Binding
} // namespace Nidium

#endif
