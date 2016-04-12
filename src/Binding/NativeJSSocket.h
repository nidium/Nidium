/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejssocket_h__
#define nativejssocket_h__

#include <native_netlib.h>

#include "JSExposer.h"

enum {
    NATIVE_SOCKET_ISBINARY          = 1 << 0,
    NATIVE_SOCKET_READLINE          = 1 << 1,
    NATIVE_SOCKET_ISSERVER          = 1 << 2,
    NATIVE_SOCKET_ISCONNECTEDCLIENT = 1 << 3
};

#define SOCKET_LINEBUFFER_MAX 8192

class NativeJSSocket : public Nidium::Binding::JSExposer<NativeJSSocket>
{
public:
    static void registerObject(JSContext *cx);
    NativeJSSocket(JS::HandleObject obj, JSContext *cx,
        const char *host, unsigned short port);
    ~NativeJSSocket();

    int write(unsigned char *data, size_t len,
        ape_socket_data_autorelease data_type);

    void disconnect();
    void shutdown();

    void dettach();
    bool isAttached();

    bool isJSCallable();

    void onRead(const char *data, size_t len);

    NativeJSSocket *getParentServer() const {
        return m_ParentServer;
    }

    void setParentServer(NativeJSSocket *parent) {
        m_ParentServer = parent;
        flags |= NATIVE_SOCKET_ISCONNECTEDCLIENT;
    }

    int getFlags() const {
        return m_ParentServer ? m_ParentServer->flags : flags;
    }

    const char *getEncoding() const {
        return m_ParentServer ? m_ParentServer->m_Encoding : m_Encoding;
    }

    uint8_t getFrameDelimiter() const {
        return m_ParentServer ? m_ParentServer->m_FrameDelimiter : m_FrameDelimiter;
    }

    bool isClientFromOwnServer() const {
        return (m_ParentServer != NULL);
    }

    JSObject *getReceiverJSObject() const {
        return m_ParentServer ? m_ParentServer->getJSObject() : this->getJSObject();
    }

    char *host;
    unsigned short port;
    ape_socket *socket;
    int flags;

    char *m_Encoding;

    struct {
        char *data;
        size_t pos;
    } lineBuffer;

    uint8_t m_FrameDelimiter;

    NativeJSSocket *m_ParentServer;

    int m_TCPTimeout;

private:
    void readFrame(const char *buf, size_t len);
};

#endif

