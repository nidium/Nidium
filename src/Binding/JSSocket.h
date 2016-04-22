/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jssocket_h__
#define binding_jssocket_h__

#include <ape_netlib.h>

#include "JSExposer.h"

namespace Nidium {
namespace Binding {

enum {
    SOCKET_ISBINARY          = 1 << 0,
    SOCKET_READLINE          = 1 << 1,
    SOCKET_ISSERVER          = 1 << 2,
    SOCKET_ISCONNECTEDCLIENT = 1 << 3
};

#define SOCKET_LINEBUFFER_MAX 8192

class JSSocket : public JSExposer<JSSocket>
{
public:
    static void registerObject(JSContext *cx);
    JSSocket(JS::HandleObject obj, JSContext *cx,
        const char *host, unsigned short port);
    ~JSSocket();

    int write(unsigned char *data, size_t len,
        ape_socket_data_autorelease data_type);

    void disconnect();
    void shutdown();

    void dettach();
    bool isAttached();

    bool isJSCallable();

    void onRead(const char *data, size_t len);

    JSSocket *getParentServer() const {
        return m_ParentServer;
    }

    void setParentServer(JSSocket *parent) {
        m_ParentServer = parent;
        flags |= SOCKET_ISCONNECTEDCLIENT;
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

    JSSocket *m_ParentServer;

    int m_TCPTimeout;

private:
    void readFrame(const char *buf, size_t len);
};

} // namespace Binding
} // namespace Nidium

#endif

