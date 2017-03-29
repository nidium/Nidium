/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef _MSC_VER
#include <unistd.h>
#include <sys/socket.h>
#endif

#include "Net/HTTPServer.h"
#include "Binding/NidiumJS.h"

using Nidium::Core::Utils;

namespace Nidium {
namespace Net {

// {{{ Preamble
#define GET_HTTP_OR_FAIL(obj)                                     \
    static_cast<HTTPServer *>(obj->ctx);                          \
    HTTPServer *___http___ = static_cast<HTTPServer *>(obj->ctx); \
    if (___http___ == NULL) return;

static struct
{
    uint16_t code;
    const char *desc;
} HTTPCodes[] = { { 100, "Continue" },
                  { 101, "Switching Protocols" },
                  { 200, "OK" },
                  { 201, "Created" },
                  { 202, "Accepted" },
                  { 203, "Non-Authoritative Information" },
                  { 204, "No Content" },
                  { 205, "Reset Content" },
                  { 206, "Partial Content" },
                  { 300, "Multiple Choices" },
                  { 301, "Moved Permanently" },
                  { 302, "Found" },
                  { 303, "See Other" },
                  { 304, "Not Modified" },
                  { 305, "Use Proxy" },
                  { 307, "Temporary Redirect" },
                  { 400, "Bad Request" },
                  { 401, "Unauthorized" },
                  { 402, "Payment Required" },
                  { 403, "Forbidden" },
                  { 404, "Not Found" },
                  { 405, "Method Not Allowed" },
                  { 406, "Not Acceptable" },
                  { 407, "Proxy Authentication Required" },
                  { 408, "Request Time-out" },
                  { 409, "Conflict" },
                  { 410, "Gone" },
                  { 411, "Length Required" },
                  { 412, "Precondition Failed" },
                  { 413, "Request Entity Too Large" },
                  { 414, "Request-URI Too Large" },
                  { 415, "Unsupported Media Type" },
                  { 416, "Requested range not satisfiable" },
                  { 417, "Expectation Failed" },
                  { 500, "Internal Server Error" },
                  { 501, "Not Implemented" },
                  { 502, "Bad Gateway" },
                  { 503, "Service Unavailable" },
                  { 504, "Gateway Time-out" },
                  { 505, "HTTP Version not support" },
                  { 0, NULL } };

// }}}

// {{{ HTTP parser callbacks

static int message_begin_cb(http_parser *p);
static int headers_complete_cb(http_parser *p);
static int message_complete_cb(http_parser *p);
static int header_field_cb(http_parser *p, const char *buf, size_t len);
static int header_value_cb(http_parser *p, const char *buf, size_t len);
static int request_url_cb(http_parser *p, const char *buf, size_t len);
static int body_cb(http_parser *p, const char *buf, size_t len);

static http_parser_settings settings
    = {/*.on_message_begin    = */ message_begin_cb,
       /*.on_url              = */ request_url_cb,
       /*.on_status           = */ NULL,
       /*.on_header_field     = */ header_field_cb,
       /*.on_header_value     = */ header_value_cb,
       /*.on_headers_complete = */ headers_complete_cb,
       /*.on_body             = */ body_cb,
       /*.on_message_complete = */ message_complete_cb
};

static int message_begin_cb(http_parser *p)
{
    HTTPClientConnection *con                 = static_cast<HTTPClientConnection *>(p->data);
    HTTPClientConnection::HTTPData *http_data = con->getHTTPState();

    /*
        Resets the headers (in the case of keepalive)
    */
    if (http_data->headers.list) {
        ape_array_destroy(http_data->headers.list);
        http_data->headers.list = NULL;
        http_data->headers.tkey = NULL;
        http_data->headers.tval = NULL;
    }

    return 0;
}

static int header_field_cb(http_parser *p, const char *buf, size_t len)
{
    HTTPClientConnection *con                 = static_cast<HTTPClientConnection *>(p->data);
    HTTPClientConnection::HTTPData *http_data = con->getHTTPState();

    switch (http_data->headers.prevstate) {
        case HTTPClientConnection::PSTATE_NOTHING:
            http_data->headers.list = ape_array_new(16);
        /* fall through */
        case HTTPClientConnection::PSTATE_VALUE:
            http_data->headers.tkey = buffer_new(16);
            if (http_data->headers.tval != NULL) {
                buffer_append_char(http_data->headers.tval, '\0');
            }
            break;
        default:
            break;
    }

    http_data->headers.prevstate = HTTPClientConnection::PSTATE_FIELD;

    if (len != 0) {
        buffer_append_data_tolower(http_data->headers.tkey,
                                   reinterpret_cast<const unsigned char *>(buf),
                                   len);
    }

    return 0;
}

static int header_value_cb(http_parser *p, const char *buf, size_t len)
{
    HTTPClientConnection *con                 = static_cast<HTTPClientConnection *>(p->data);
    HTTPClientConnection::HTTPData *http_data = con->getHTTPState();

    switch (http_data->headers.prevstate) {
        case HTTPClientConnection::PSTATE_NOTHING:
            return -1;
        case HTTPClientConnection::PSTATE_FIELD:
            http_data->headers.tval = buffer_new(64);
            buffer_append_char(http_data->headers.tkey, '\0');
            ape_array_add_b(http_data->headers.list, http_data->headers.tkey,
                            http_data->headers.tval);
            break;
        default:
            break;
    }

    http_data->headers.prevstate = HTTPClientConnection::PSTATE_VALUE;

    if (len != 0) {
        buffer_append_data(http_data->headers.tval,
                           reinterpret_cast<const unsigned char *>(buf), len);
    }
    return 0;
}

static int headers_complete_cb(http_parser *p)
{
    HTTPClientConnection *con                 = static_cast<HTTPClientConnection *>(p->data);
    HTTPClientConnection::HTTPData *http_data = con->getHTTPState();

    if (http_data->headers.tval != NULL) {
        buffer_append_char(http_data->headers.tval, '\0');
    }

    if (p->content_length >= UINT32_MAX) {
        http_data->contentlength = 0;
        con->onHeaderEnded();
        return 0;
    }

    if (p->content_length > HTTP_MAX_CL) {
        return -1;
    }

    /* /!\ TODO: what happend if there is no content-length? */
    // if (p->content_length) http_data->data = buffer_new(p->content_length);

    http_data->contentlength = p->content_length;
    con->onHeaderEnded();

    return 0;
}

static int message_complete_cb(http_parser *p)
{
    HTTPClientConnection *client = static_cast<HTTPClientConnection *>(p->data);

    client->_createResponse();
    client->increaseRequestsCount();

    if (client->getHTTPServer()->onEnd(client)) {
        client->close();
    } else {
        /* Further reading possible */
        client->resetData();
    }

    HTTPClientConnection::HTTPData *http_data = client->getHTTPState();

    http_data->headers.prevstate = HTTPClientConnection::PSTATE_NOTHING;

    return 0;
}

static int body_cb(http_parser *p, const char *buf, size_t len)
{
    HTTPClientConnection *client = static_cast<HTTPClientConnection *>(p->data);

    if (client->getHTTPState()->data == NULL) {
        client->getHTTPState()->data = buffer_new(2048);
    }

    if (len != 0) {
        buffer_append_data(client->getHTTPState()->data,
                           reinterpret_cast<const unsigned char *>(buf), len);
    }

    client->getHTTPServer()->onData(client, buf, len);

    return 0;
}

static int request_url_cb(http_parser *p, const char *buf, size_t len)
{
    HTTPClientConnection *client = static_cast<HTTPClientConnection *>(p->data);
    if (client->getHTTPState()->url == NULL) {
        client->getHTTPState()->url = buffer_new(512);
    }

    if (len != 0) {
        buffer_append_data(client->getHTTPState()->url,
                           reinterpret_cast<const unsigned char *>(buf), len);
    }

    // ndm_logf(NDM_LOG_DEBUG, "HTTPServer", "Request URL cb %.*s", (int)len, buf);
    return 0;
}

// }}}

// {{{ Socket callbacks

static void nidium_socket_onaccept(ape_socket *socket_server,
                                   ape_socket *socket_client,
                                   ape_global *ape,
                                   void *socket_arg)
{
    HTTPServer *http = GET_HTTP_OR_FAIL(socket_server);

    HTTPClientConnection *con = http->onClientConnect(socket_client, ape);
    socket_client->ctx = con;

    if (con) {
        http->m_ClientConnections.insert({con, con});
    }
}

static void nidium_socket_client_read(ape_socket *socket_client,
                                      const uint8_t *data,
                                      size_t len,
                                      ape_global *ape,
                                      void *socket_arg)
{
    HTTPClientConnection *con
        = static_cast<HTTPClientConnection *>(socket_client->ctx);
    if (!con) {
        return;
    }

    con->onRead(reinterpret_cast<const char *>(data), len, ape);
}

static void nidium_socket_client_read_after_upgrade(ape_socket *socket_client,
                                                    const uint8_t *data,
                                                    size_t len,
                                                    ape_global *ape,
                                                    void *socket_arg)
{
    HTTPClientConnection *con
        = static_cast<HTTPClientConnection *>(socket_client->ctx);
    if (!con) {
        return;
    }
    con->onContent(reinterpret_cast<const char *>(data), len);
}

static void nidium_socket_client_disconnect(ape_socket *socket_client,
                                            ape_global *ape,
                                            void *socket_arg)
{
    HTTPClientConnection *con
        = static_cast<HTTPClientConnection *>(socket_client->ctx);
    if (!con) {
        return;
    }
    con->getHTTPServer()->onClientDisconnect(con);
    con->onDisconnect(ape);

    socket_client->ctx = NULL;

    delete con;
}

// }}}

// {{{ HTTPServer Implementation
HTTPServer::HTTPServer(uint16_t port, const char *ip, bool secure)
{
    ape_global *ape = Binding::NidiumJS::GetNet();
    m_Socket        = APE_socket_new(secure ?
                      APE_SOCKET_PT_SSL : APE_SOCKET_PT_TCP, 0, ape);

    m_IP   = strdup(ip);
    m_Port = port;
}

bool HTTPServer::start(bool reuseport, int timeout)
{
    if (!m_Socket) {
        return false;
    }

    m_Socket->callbacks.on_connect    = nidium_socket_onaccept;
    m_Socket->callbacks.on_read       = nidium_socket_client_read;
    m_Socket->callbacks.on_disconnect = nidium_socket_client_disconnect;
    m_Socket->callbacks.on_message    = NULL; // no udp on http
    m_Socket->callbacks.on_drain      = NULL;
    m_Socket->callbacks.arg           = NULL;
    m_Socket->ctx                     = this;

    if (timeout) {
        APE_socket_setTimeout(m_Socket, timeout);
    }

    return (APE_socket_listen(m_Socket, m_Port, m_IP, 1,
                              static_cast<int>(reuseport))
            != -1);
}

void HTTPServer::stop()
{
    shutdownClients();

    if (m_Socket && m_Socket->ctx == this) {
        m_Socket->ctx = NULL;
        APE_socket_shutdown_now(m_Socket);
    }
}

void HTTPServer::shutdownClients()
{
    for (auto &pair : m_ClientConnections) {
        HTTPClientConnection *con = pair.second;

        con->dettach();
    }

    m_ClientConnections.clear();
}

HTTPServer::~HTTPServer()
{
    this->stop();
    free(m_IP);
}

HTTPClientConnection *HTTPServer::onClientConnect(ape_socket *client, ape_global *ape)
{
    HTTPClientConnection *con = new HTTPClientConnection(this, client);

    this->onClientConnect(con);

    return con;
}

int HTTPClientConnection_checktimeout(void *arg)
{
    HTTPClientConnection *con = static_cast<HTTPClientConnection *>(arg);
    uint64_t timeout          = con->getTimeoutAfterMs();
    /*
        Never timeout if set to 0
    */
    if (timeout && Utils::GetTick(true) - con->getLastActivity() > timeout) {
        con->close();
    }

    return 1000;
}

// }}}

// {{{ HTTPClientConnection Implementation

HTTPClientConnection::HTTPClientConnection(HTTPServer *httpserver,
                                           ape_socket *socket)
    : m_Ctx(NULL), m_SocketClient(socket), m_HTTPServer(httpserver),
      m_Response(NULL), m_RequestsCount(0), m_MaxRequestsCount(0)
{
    m_HttpState.headers.prevstate = PSTATE_NOTHING;

    m_HttpState.headers.list = NULL;
    m_HttpState.headers.tkey = NULL;
    m_HttpState.headers.tval = NULL;
    m_HttpState.ended        = 0;
    m_HttpState.data         = NULL;
    m_HttpState.url          = NULL;

    m_ClientTimeoutMs = 10000;

    http_parser_init(&m_HttpState.parser, HTTP_REQUEST);
    m_HttpState.parser.data = this;

    ape_timer_t *timer = APE_timer_create(
        socket->ape, 1000, HTTPClientConnection_checktimeout, this);

    m_TimeoutTimer = APE_timer_getid(timer);

    m_LastAcitivty = Utils::GetTick(true);
}

void HTTPClientConnection::onRead(const char *data, size_t len, ape_global *ape)
{
#define REQUEST_HEADER(header) \
    ape_array_lookup(m_HttpState.headers.list, CONST_STR_LEN(header "\0"))

    m_LastAcitivty = Utils::GetTick(true);

    int nparsed
        = http_parser_execute(&m_HttpState.parser, &settings, data, len);

    if (m_HttpState.parser.upgrade) {
        buffer *upgrade_header = REQUEST_HEADER("upgrade");

        if (upgrade_header) {
            this->onUpgrade((const char *)upgrade_header->data);
            if (nparsed < len) {
                /* Non-http data in the current packet (after the header) */
                this->onContent(&data[nparsed], len - nparsed);
            }

            /* Change the callback for the next on_read calls */
            m_SocketClient->callbacks.on_read
                = nidium_socket_client_read_after_upgrade;
        }
    } else if (nparsed != len) {
        ndm_logf(NDM_LOG_ERROR, "HTTPServer", "Http error : %s",
                 http_errno_description(HTTP_PARSER_ERRNO(&m_HttpState.parser)));
    }
#undef REQUEST_HEADER
}

void HTTPClientConnection::close()
{
    if (m_SocketClient) {
        APE_socket_shutdown(m_SocketClient);
    }
}

void HTTPClientConnection::write(char *buf, size_t len)
{
    APE_socket_write(m_SocketClient, buf, len, APE_DATA_COPY);
}

const char *HTTPClientConnection::getHeader(const char *key)
{
    buffer *ret
        = ape_array_lookup_cstr(getHTTPState()->headers.list, key, strlen(key));
    return ret ? reinterpret_cast<const char *>(ret->data) : NULL;
}

HTTPResponse *HTTPClientConnection::onCreateResponse()
{
    return new HTTPResponse();
}

void HTTPClientConnection::dettach()
{
    m_HTTPServer = nullptr;

    if (m_SocketClient) {
        m_SocketClient->ctx = NULL;

        this->close();
    }
}

HTTPClientConnection::~HTTPClientConnection()
{
    if (m_TimeoutTimer) {
        ape_global *ape = Binding::NidiumJS::GetNet();

        APE_timer_clearbyid(ape, m_TimeoutTimer, 1);

        m_TimeoutTimer = 0;
    }

    if (m_HttpState.headers.list) {
        ape_array_destroy(m_HttpState.headers.list);
    }

    if (m_HttpState.data) {
        buffer_destroy(m_HttpState.data);
    }

    if (m_HttpState.url) {
        buffer_destroy(m_HttpState.url);
    }

    if (m_Response) {
        delete m_Response;
    }

    if (m_HTTPServer) {
        m_HTTPServer->m_ClientConnections.erase(this);
    }

    if (m_SocketClient) {
        m_SocketClient->ctx = NULL;
        this->close();
    }
};

// }}}

// {{{ HTTPResponse

HTTPResponse::HTTPResponse(uint16_t code)
    : m_Headers(ape_array_new(8)), m_Statuscode(code), m_Content(NULL),
      m_Headers_str(NULL), m_HeaderSent(false), m_Chunked(false)
{
    this->setHeader("Server", "nidium/" NIDIUM_VERSION_STR);
}

HTTPResponse::~HTTPResponse()
{
    if (m_Headers) {
        ape_array_destroy(m_Headers);
    }

    if (m_Headers_str) {
        buffer_destroy(m_Headers_str);
    }
    if (m_Content) {
        buffer_destroy(m_Content);
    }
}

void HTTPResponse::sendHeaders(bool chunked)
{
    if (!m_Con || m_Con->getSocket() == NULL || m_HeaderSent) {
        return;
    }

    if (chunked) {
        m_Chunked = true;
    }

    /* "No content requests must not be chunked" */
    if (m_Statuscode == 204) {
        m_Chunked = false;
    }

    const buffer &headers = this->getHeadersString();

    APE_socket_write(m_Con->getSocket(), headers.data, headers.used,
                     APE_DATA_AUTORELEASE);

    m_HeaderSent = true;

    this->dataOwnershipTransfered(true);
}

void HTTPResponse::sendChunk(char *buf,
                             size_t len,
                             ape_socket_data_autorelease datatype,
                             bool willEnd)
{
    if (!m_Con || m_Con->getSocket() == NULL || !len) {
        return;
    }

    if (!m_Chunked && willEnd) {
        this->setData(buf, len);
        this->send(APE_DATA_COPY);
        /*
            Hack :
            Avoid the destructor to release the data for us
            (ownership is not transfered by sendChunk)
        */
        m_Content->data = NULL;
        return;
    }

    m_Chunked = true;

    if (!m_HeaderSent) {
        this->sendHeaders();
    }

    char tmpbuf[64];
    int tmplen = sprintf(tmpbuf, "%zx\r\n", len);

    PACK_TCP(m_Con->getSocket()->s.fd);

    APE_socket_write(m_Con->getSocket(), tmpbuf, tmplen, APE_DATA_STATIC);
    APE_socket_write(m_Con->getSocket(), buf, len, datatype);
    APE_socket_write(m_Con->getSocket(), (char *)CONST_STR_LEN("\r\n"),
                     APE_DATA_STATIC);

    FLUSH_TCP(m_Con->getSocket()->s.fd);
}

void HTTPResponse::send(ape_socket_data_autorelease datatype)
{
    if (!m_Con || m_Con->getSocket() == NULL) {
        return;
    }

    PACK_TCP(m_Con->getSocket()->s.fd);

    const buffer *data = this->getDataBuffer();

    if (!m_HeaderSent) {
        const buffer &headers = this->getHeadersString();

        APE_socket_write(m_Con->getSocket(), headers.data, headers.used,
                         APE_DATA_AUTORELEASE);

        m_HeaderSent = true;
    }
    if (data && data->used) {
        APE_socket_write(m_Con->getSocket(), data->data, data->used, datatype);
    }
    FLUSH_TCP(m_Con->getSocket()->s.fd);

    /*
        If we are not in a APE_DATA_AUTORELEASE,
        only transfert the headers ownership
    */
    this->dataOwnershipTransfered(!(datatype == APE_DATA_AUTORELEASE));

    if (m_Con->shouldCloseConnection()) {
        /*
            TODO: temporise to (try to) avoid active close?
        */
        m_Con->close();
    }
}

void HTTPResponse::end()
{
    if (!m_Con || m_Con->getSocket() == NULL) {
        return;
    }

    if (!m_HeaderSent) {
        this->sendHeaders();
    }

    if (m_Chunked) {
        APE_socket_write(m_Con->getSocket(), (char *)CONST_STR_LEN("0\r\n\r\n"),
                         APE_DATA_STATIC);
    }

    if (m_Con->shouldCloseConnection()) {
        /*
            TODO: temporise to (try to) avoid active close?
        */
        m_Con->close();
    }
}

void HTTPResponse::setHeader(const char *key, const char *val)
{
    ape_array_add_camelkey_n(m_Headers, key, strlen(key), val, strlen(val));
}

void HTTPResponse::removeHeader(const char *key)
{
    ape_array_delete(m_Headers, key, strlen(key));
}

void HTTPResponse::setData(char *buf, size_t len)
{
    if (m_Content) {
        return;
    }
    m_Content       = buffer_new(0);
    m_Content->data = (unsigned char *)buf;
    m_Content->size = m_Content->used = len;
}

const buffer &HTTPResponse::getHeadersString()
{
    if (m_Headers_str == NULL) {
        m_Headers_str = buffer_new(512);
    } else {
        m_Headers_str->used = 0;
    }

    char tmpbuf[32];
    sprintf(tmpbuf, "HTTP/1.1 %u ", m_Statuscode);

    buffer_append_string(m_Headers_str, tmpbuf);
    buffer_append_string(m_Headers_str, this->getStatusDesc());

    char httpdate[256];
    Utils::HTTPTime(httpdate);

    this->setHeader("Date", httpdate);

    buffer_append_string_n(m_Headers_str, CONST_STR_LEN("\r\n"));

    if (!m_Chunked) {
        if (m_Content && m_Content->used) {
            sprintf(tmpbuf, "%zu", m_Content->used);
            this->setHeader("Content-Length", tmpbuf);
        } else {
            this->setHeader("Content-Length", "0");
        }
    } else {
        this->setHeader("Transfer-Encoding", "chunked");
        this->removeHeader("Content-Length");
    }

    if (m_Con && m_Con->shouldCloseConnection()) {
        this->setHeader("Connection", "close");
    }

    buffer *k, *v;
    if (this->getHeaders()) {
        APE_A_FOREACH(this->getHeaders(), k, v)
        {
            buffer_append_string_n(m_Headers_str,
                                   reinterpret_cast<char *>(k->data), k->used);
            buffer_append_string_n(m_Headers_str, ": ", 2);
            buffer_append_string_n(m_Headers_str,
                                   reinterpret_cast<char *>(v->data), v->used);
            buffer_append_string_n(m_Headers_str, CONST_STR_LEN("\r\n"));
        }
    } else {
        buffer_append_string_n(m_Headers_str, CONST_STR_LEN("\r\n"));
    }
    buffer_append_string_n(m_Headers_str, CONST_STR_LEN("\r\n"));

    return *m_Headers_str;
}

const buffer *HTTPResponse::getDataBuffer()
{
    return m_Content;
}

const char *HTTPResponse::getStatusDesc() const
{
    for (int i = 0; HTTPCodes[i].desc != NULL; i++) {
        if (HTTPCodes[i].code == m_Statuscode) {
            return HTTPCodes[i].desc;
        }
    }

    return "Unknown";
}

void HTTPResponse::dataOwnershipTransfered(bool onlyHeaders)
{
    /*
        Free the buffer object. We're only giving the data away.
    */
    free(m_Headers_str);
    m_Headers_str = NULL;

    if (!onlyHeaders) {
        free(m_Content);
        m_Content = NULL;
    }
}

// }}}

} // namespace Net
} // namespace Nidium
