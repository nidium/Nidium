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

#ifndef nativejssocket_h__
#define nativejssocket_h__

#include "NativeJSExposer.h"
#include <native_netlib.h>

enum {
    NATIVE_SOCKET_ISBINARY          = 1 << 0,
    NATIVE_SOCKET_READLINE          = 1 << 1,
    NATIVE_SOCKET_ISSERVER          = 1 << 2,
    NATIVE_SOCKET_ISCONNECTEDCLIENT = 1 << 3
};


#define SOCKET_LINEBUFFER_MAX 8192

class NativeJSSocket : public NativeJSExposer<NativeJSSocket>
{
public:
    static void registerObject(JSContext *cx);
    NativeJSSocket(JSObject *obj, JSContext *cx,
        const char *host, unsigned short port);
    ~NativeJSSocket();

    int write(unsigned char *data, size_t len,
        ape_socket_data_autorelease data_type);

    void disconnect();
    void shutdown();

    void dettach();
    bool isAttached();

    bool isJSCallable();

    void onRead();

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
