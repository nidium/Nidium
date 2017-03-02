/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef net_httpserver_h__
#define net_httpserver_h__

#include <stdio.h>

#include <http_parser.h>

#ifdef _MSC_VER
#include <port/windows.h>
#endif

#include <ape_netlib.h>
#include <ape_array.h>
#include <ape_socket.h>
#include <unordered_map>

#include "Core/Messages.h"
#include "Core/Events.h"

#define HTTP_MAX_CL (1024ULL * 1024ULL * 1024ULL * 2ULL)
#define HTTP_DEFAULT_TIMEOUT 15000

namespace Nidium {
namespace Net {

class HTTPClientConnection;
class HTTPResponse;

// {{{ HTTPServer
class HTTPServer : public Nidium::Core::Events
{
public:
    static const uint8_t EventID = 3;

    enum Events
    {
        kEvents_None = 1
    };

    HTTPServer(uint16_t port, const char *ip = "0.0.0.0", bool secure = false);
    virtual ~HTTPServer();

    /*
        @param reuseport true to allow the server to re use the listening port
        @param timeout <seconds> to set a TCP keepalive timeout
    */
    bool start(bool reuseport = false, int timeout = 0);
    void stop();

    ape_socket *getSocket() const
    {
        return m_Socket;
    }

    /*
        Subclasses can override this in order to set
        their own HTTPClientConnection subclass.

        HTTPClientConnection is then automatically
        deleted when the socket disconnect.

    */
    virtual HTTPClientConnection *onClientConnect(ape_socket *client,
        ape_global *ape);

    /*
        Callbacks for subclasses
    */
    virtual void onClientConnect(HTTPClientConnection *client){};
    virtual void onClientDisconnect(HTTPClientConnection *client){};
    virtual void
    onData(HTTPClientConnection *client, const char *buf, size_t len){};

    /* return true to close the connection */
    virtual bool onEnd(HTTPClientConnection *client)
    {
        return true;
    };

    uint16_t getPort() const
    {
        return m_Port;
    }

    const char *getIP() const
    {
        return m_IP;
    }

    void shutdownClients();


    std::unordered_map<HTTPClientConnection *, HTTPClientConnection *> m_ClientConnections;
private:

    ape_socket *m_Socket;
    char *m_IP;
    uint16_t m_Port;

};
// }}}

//  {{{ HTTPResponse
/*
    TODO: add APE_socket_sendfile() support
*/
class HTTPResponse
{
public:
    friend HTTPClientConnection;

    virtual ~HTTPResponse();

    void setHeader(const char *key, const char *val);
    void removeHeader(const char *key);

    /*
        Give ownership
        Use dataOwnershipTransfered if it's transfered
    */
    void setData(char *buf, size_t len);

    ape_array_t *getHeaders() const
    {
        return m_Headers;
    }

    void setStatusCode(uint16_t code)
    {
        m_Statuscode = code;
    }

    const buffer &getHeadersString();
    const buffer *getDataBuffer();
    const char *getStatusDesc() const;

    /*
        Send a chunk of data
    */
    void sendChunk(char *buf,
                   size_t len,
                   ape_socket_data_autorelease datatype = APE_DATA_AUTORELEASE,
                   bool willEnd = false);

    /*
        Send the request.
        |datatype| defines how data ownership is managed
    */
    void send(ape_socket_data_autorelease datatype = APE_DATA_AUTORELEASE);
    void sendHeaders(bool chunked = false);
    void end();
    bool isHeadersAlreadySent() const
    {
        return m_HeaderSent;
    }

    /*
        We want zerocopy.
        This is used to tell the object that the data
        has been transfered to another owner.
    */
    void dataOwnershipTransfered(bool onlyHeaders = false);

private:
    ape_array_t *m_Headers;
    uint16_t m_Statuscode;
    buffer *m_Content;
    buffer *m_Headers_str;
    bool m_HeaderSent;
    bool m_Chunked;

    HTTPClientConnection *m_Con;

protected:
    explicit HTTPResponse(uint16_t code = 200);
};
// }}}

// {{{ HTTPClientConnection
class HTTPClientConnection
{
public:
    HTTPClientConnection(HTTPServer *httpserver, ape_socket *socket);
    virtual ~HTTPClientConnection();

    enum PrevState
    {
        PSTATE_NOTHING,
        PSTATE_FIELD,
        PSTATE_VALUE
    };
    struct HTTPData
    {
        http_parser parser;
        struct
        {
            ape_array_t *list;
            buffer *tkey;
            buffer *tval;
            PrevState prevstate;
        } headers;
        int ended;
        uint64_t contentlength;
        buffer *data;
        buffer *url;
    };

    struct HTTPData *getHTTPState()
    {
        return &m_HttpState;
    }

    ape_socket *getSocket() const
    {
        return m_SocketClient;
    }

    HTTPServer *getHTTPServer() const
    {
        return m_HTTPServer;
    }

    buffer *getData() const
    {
        return m_HttpState.data;
    }

    buffer *getURL() const
    {
        return m_HttpState.url;
    }

    /* Get a header value from the client request */
    const char *getHeader(const char *key);

    void resetData()
    {
        if (m_HttpState.data != NULL) {
            m_HttpState.data->used = 0;
        }
        if (m_HttpState.url != NULL) {
            m_HttpState.url->used = 0;
        }
    }

    void onRead(const char *data, size_t len, ape_global *ape);
    void write(char *buf, size_t len);
    void setContext(void *arg)
    {
        m_Ctx = arg;
    }
    void *getContext() const
    {
        return m_Ctx;
    }

    HTTPResponse *getResponse()
    {
        return m_Response;
    }

    void setTimeout(int val)
    {
        m_ClientTimeoutMs = val;
    }

    uint64_t getTimeoutAfterMs() const
    {
        return m_ClientTimeoutMs;
    }

    uint64_t getLastActivity() const
    {
        return m_LastAcitivty;
    }

    void increaseRequestsCount()
    {
        m_RequestsCount++;
    }

    bool shouldCloseConnection()
    {
        const char *header_connection = getHeader("connection");

        return ((m_MaxRequestsCount && m_RequestsCount >= m_MaxRequestsCount)
                || (header_connection
                    && strcasecmp(header_connection, "close") == 0));
    }

    void setMaxRequestsCount(uint64_t n)
    {
        m_MaxRequestsCount = n;
    }

    void dettach();

    virtual HTTPResponse *onCreateResponse();

    virtual void onHeaderEnded(){};
    virtual void onDisconnect(ape_global *ape){};
    virtual void onUpgrade(const char *to){};
    virtual void onContent(const char *data, size_t len){};

    virtual void close();

    void _createResponse()
    {
        HTTPResponse *resp = onCreateResponse();
        if (m_Response && resp != m_Response) {
            delete m_Response;
        }
        m_Response        = resp;
        m_Response->m_Con = this;
    }

    void *m_Ctx;

protected:
    struct HTTPData m_HttpState;
    ape_socket *m_SocketClient;
    HTTPServer *m_HTTPServer;
    HTTPResponse *m_Response;
    uint64_t m_TimeoutTimer;
    uint64_t m_LastAcitivty;
    int m_ClientTimeoutMs;
    uint64_t m_RequestsCount;
    uint64_t m_MaxRequestsCount;
};
// }}}

} // namespace Net
} // namespace Nidium

#endif
