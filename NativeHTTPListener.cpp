/*
    NativeJS Core Library
    Copyright (C) 2014 Anthony Catel <paraboul@gmail.com>

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

#include "NativeHTTPListener.h"
#include "NativeJS.h"

#include <stdlib.h>
#include <stdint.h>

#define GET_HTTP_OR_FAIL(obj) \
    (NativeHTTPListener *)obj->ctx; \
    NativeHTTPListener *___http___ = (NativeHTTPListener *)obj->ctx; \
    if (___http___ == NULL) return;

static int message_begin_cb(http_parser *p);
static int headers_complete_cb(http_parser *p);
static int message_complete_cb(http_parser *p);
static int header_field_cb(http_parser *p, const char *buf, size_t len);
static int header_value_cb(http_parser *p, const char *buf, size_t len);
static int request_url_cb(http_parser *p, const char *buf, size_t len);
static int body_cb(http_parser *p, const char *buf, size_t len);


static http_parser_settings settings =
{
.on_message_begin = message_begin_cb,
.on_header_field = header_field_cb,
.on_header_value = header_value_cb,
.on_url = request_url_cb,
.on_body = body_cb,
.on_headers_complete = headers_complete_cb,
.on_message_complete = message_complete_cb
};


static struct {
    uint16_t code;
    const char *desc;
} NativeHTTP_Codes[] = {
    {100, "Continue"},
    {101, "Switching Protocols"},
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {203, "Non-Authoritative Information"},
    {204, "No Content"},
    {205, "Reset Content"},
    {206, "Partial Content"},
    {300, "Multiple Choices"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {303, "See Other"},
    {304, "Not Modified"},
    {305, "Use Proxy"},
    {307, "Temporary Redirect"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {402, "Payment Required"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {406, "Not Acceptable"},
    {407, "Proxy Authentication Required"},
    {408, "Request Time-out"},
    {409, "Conflict"},
    {410, "Gone"},
    {411, "Length Required"},
    {412, "Precondition Failed"},
    {413, "Request Entity Too Large"},
    {414, "Request-URI Too Large"},
    {415, "Unsupported Media Type"},
    {416, "Requested range not satisfiable"},
    {417, "Expectation Failed"},
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Time-out"},
    {505, "HTTP Version not support"},
    {0, NULL}
};


static int message_begin_cb(http_parser *p)
{
    NativeHTTPClientConnection *con = (NativeHTTPClientConnection *)p->data;
    NativeHTTPClientConnection::HTTPData *http_data = con->getHTTPState();

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
    NativeHTTPClientConnection *con = (NativeHTTPClientConnection *)p->data;
    NativeHTTPClientConnection::HTTPData *http_data = con->getHTTPState();

    switch (http_data->headers.prevstate) {
        case NativeHTTPClientConnection::PSTATE_NOTHING:
            http_data->headers.list = ape_array_new(16);
            /* fall through */
        case NativeHTTPClientConnection::PSTATE_VALUE:
            http_data->headers.tkey = buffer_new(16);
            if (http_data->headers.tval != NULL) {
                buffer_append_char(http_data->headers.tval, '\0');
            }
            break;
        default:
            break;
    }

    http_data->headers.prevstate = NativeHTTPClientConnection::PSTATE_FIELD;

    if (len != 0) {
        buffer_append_data_tolower(http_data->headers.tkey,
            (const unsigned char *)buf, len);
    }

    return 0;
}

static int header_value_cb(http_parser *p, const char *buf, size_t len)
{
    NativeHTTPClientConnection *con = (NativeHTTPClientConnection *)p->data;
    NativeHTTPClientConnection::HTTPData *http_data = con->getHTTPState();

    switch (http_data->headers.prevstate) {
        case NativeHTTPClientConnection::PSTATE_NOTHING:
            return -1;
        case NativeHTTPClientConnection::PSTATE_FIELD:
            http_data->headers.tval = buffer_new(64);
            buffer_append_char(http_data->headers.tkey, '\0');
            ape_array_add_b(http_data->headers.list,
                    http_data->headers.tkey, http_data->headers.tval);
            break;
        default:
            break;
    }

    http_data->headers.prevstate = NativeHTTPClientConnection::PSTATE_VALUE;

    if (len != 0) {
        buffer_append_data(http_data->headers.tval,
            (const unsigned char *)buf, len);
    }
    return 0;
}

static int headers_complete_cb(http_parser *p)
{
    NativeHTTPClientConnection *con = (NativeHTTPClientConnection *)p->data;
    NativeHTTPClientConnection::HTTPData *http_data = con->getHTTPState();

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
    //if (p->content_length) http_data->data = buffer_new(p->content_length);

    http_data->contentlength = p->content_length;
    con->onHeaderEnded();

    return 0;
}

static int message_complete_cb(http_parser *p)
{
    NativeHTTPClientConnection *client = (NativeHTTPClientConnection *)p->data;

    client->_createResponse();

    if (client->getHTTPListener()->onEnd(client)) {
        client->close();
    } else {
        /* Further reading possible */
        client->resetData();
    }

    NativeHTTPClientConnection::HTTPData *http_data = client->getHTTPState();

    http_data->headers.prevstate = NativeHTTPClientConnection::PSTATE_NOTHING;

    return 0;
}

static int body_cb(http_parser *p, const char *buf, size_t len)
{
    NativeHTTPClientConnection *client = (NativeHTTPClientConnection *)p->data;

    if (client->getHTTPState()->data == NULL) {
        client->getHTTPState()->data = buffer_new(2048);
    }

    if (len != 0) {
        buffer_append_data(client->getHTTPState()->data,
            (const unsigned char *)buf, len);
    }

    client->getHTTPListener()->onData(client, buf, len);

    return 0;
}

static int request_url_cb(http_parser *p, const char *buf, size_t len)
{
    NativeHTTPClientConnection *client = (NativeHTTPClientConnection *)p->data;
    if (client->getHTTPState()->url == NULL) {
        client->getHTTPState()->url = buffer_new(512);
    }

    if (len != 0) {
        buffer_append_data(client->getHTTPState()->url,
            (const unsigned char *)buf, len);
    }

    //printf("Request URL cb %.*s\n", (int)len, buf);
    return 0;
}

static void native_socket_onaccept(ape_socket *socket_server,
    ape_socket *socket_client, ape_global *ape, void *socket_arg)
{
    NativeHTTPListener *http = GET_HTTP_OR_FAIL(socket_server);

    http->onClientConnect(socket_client, ape);
}

static void native_socket_client_read(ape_socket *socket_client,
    ape_global *ape, void *socket_arg)
{
    NativeHTTPClientConnection *con = (NativeHTTPClientConnection *)socket_client->ctx;
    if (!con) {
        return;
    }

    con->onRead(&socket_client->data_in, ape);
}

static void native_socket_client_read_after_upgrade(ape_socket *socket_client,
    ape_global *ape, void *socket_arg)
{
    NativeHTTPClientConnection *con = (NativeHTTPClientConnection *)socket_client->ctx;
    if (!con) {
        return;
    }
    con->onContent((const char *)socket_client->data_in.data,
        socket_client->data_in.used);  
}

static void native_socket_client_disconnect(ape_socket *socket_client,
    ape_global *ape, void *socket_arg)
{
    NativeHTTPClientConnection *con = (NativeHTTPClientConnection *)socket_client->ctx;
    if (!con) {
        return;
    }
    con->getHTTPListener()->onClientDisconnect(con);
    con->onDisconnect(ape);

    socket_client->ctx = NULL;

    delete con;
}

////////////////////
////////////////////

NativeHTTPListener::NativeHTTPListener(uint16_t port, const char *ip)
{
    ape_global *ape = NativeJS::getNet();
    m_Socket = APE_socket_new(APE_SOCKET_PT_TCP, 0, ape);

    m_IP = strdup(ip);
    m_Port = port;
}

bool NativeHTTPListener::start(bool reuseport)
{
    if (!m_Socket) {
        return false;
    }

    m_Socket->callbacks.on_connect    = native_socket_onaccept;
    m_Socket->callbacks.on_read       = native_socket_client_read;
    m_Socket->callbacks.on_disconnect = native_socket_client_disconnect;
    m_Socket->callbacks.on_message    = NULL; // no udp on http
    m_Socket->callbacks.on_drain      = NULL;
    m_Socket->callbacks.arg           = NULL;
    m_Socket->ctx = this;

    return (APE_socket_listen(m_Socket, m_Port, m_IP, 1, (int)reuseport) != -1);
}

void NativeHTTPListener::stop()
{
    if (m_Socket && m_Socket->ctx == this) {
        m_Socket->ctx = NULL;
        APE_socket_shutdown_now(m_Socket);
    }
}


NativeHTTPListener::~NativeHTTPListener()
{
    this->stop();
    free(m_IP);
}


////////////////////
////////////////////

void NativeHTTPListener::onClientConnect(ape_socket *client, ape_global *ape)
{
    client->ctx = new NativeHTTPClientConnection(this, client);

    this->onClientConnect((NativeHTTPClientConnection *)client->ctx);
}


//////////////
//////////////

int NativeHTTPClientConnection_checktimeout(void *arg)
{
    NativeHTTPClientConnection *con = (NativeHTTPClientConnection *)arg;
    uint64_t timeout = con->getTimeoutAfterMs();
    /*
        Never timeout if set to 0
    */
    if (timeout && NativeUtils::getTick(true) - con->getLastActivity() > timeout) {
        con->close();
    }

    return 1000;
}

NativeHTTPClientConnection::NativeHTTPClientConnection(NativeHTTPListener *httpserver,
    ape_socket *socket) :
    m_Ctx(NULL), m_SocketClient(socket), m_HTTPListener(httpserver), m_Response(NULL)
{
    m_HttpState.headers.prevstate = PSTATE_NOTHING;

    m_HttpState.headers.list = NULL;
    m_HttpState.headers.tkey = NULL;
    m_HttpState.headers.tval = NULL;
    m_HttpState.ended = 0;
    m_HttpState.data  = NULL;
    m_HttpState.url  = NULL;

    m_ClientTimeoutMs = 10000;

    http_parser_init(&m_HttpState.parser, HTTP_REQUEST);
    m_HttpState.parser.data = this;

    ape_global *net = socket->ape;

    ape_timer *timer = add_timer(&socket->ape->timersng, 1000,
        NativeHTTPClientConnection_checktimeout, this);

    m_TimeoutTimer = timer->identifier;

    m_LastAcitivty = NativeUtils::getTick(true);
}

void NativeHTTPClientConnection::onRead(buffer *buf, ape_global *ape)
{
#define REQUEST_HEADER(header) ape_array_lookup(m_HttpState.headers.list, \
    CONST_STR_LEN(header "\0"))

    m_LastAcitivty = NativeUtils::getTick(true);

    int nparsed = http_parser_execute(&m_HttpState.parser, &settings,
        (const char *)buf->data, (size_t)buf->used);

    if (m_HttpState.parser.upgrade) {
        buffer *upgrade_header = REQUEST_HEADER("upgrade");

        if (upgrade_header) {
            this->onUpgrade((const char *)upgrade_header->data);
            if (nparsed < buf->used) {
                /* Non-http data in the current packet (after the header) */
                this->onContent((const char *)&buf->data[nparsed], buf->used - nparsed);
            }

            /* Change the callback for the next on_read calls */
            m_SocketClient->callbacks.on_read = native_socket_client_read_after_upgrade;
        }
    } else if (nparsed != buf->used) {
        printf("Http error : %s\n", http_errno_description(HTTP_PARSER_ERRNO(&m_HttpState.parser)));
    }
#undef REQUEST_HEADER    
}

void NativeHTTPClientConnection::close()
{
    if (m_SocketClient) {
        APE_socket_shutdown(m_SocketClient);
    }
}

NativeHTTPClientConnection::~NativeHTTPClientConnection()
{
    if (m_TimeoutTimer) {
        ape_global *ape = NativeJS::getNet();
        clear_timer_by_id(&ape->timersng, m_TimeoutTimer, 1);
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
};

void NativeHTTPClientConnection::write(char *buf, size_t len)
{
    APE_socket_write(m_SocketClient, buf, len, APE_DATA_COPY);
}

const char *NativeHTTPClientConnection::getHeader(const char *key)
{
    buffer *ret = ape_array_lookup_cstr(getHTTPState()->headers.list,
        key, strlen(key));
    return ret ? (const char *)ret->data : NULL;
}

NativeHTTPResponse *NativeHTTPClientConnection::onCreateResponse()
{
    return new NativeHTTPResponse();
}


//////

NativeHTTPResponse::NativeHTTPResponse(uint16_t code) :
    m_Headers(ape_array_new(8)), m_Statuscode(code), m_ContentLength(0),
    m_Content(NULL), m_Headers_str(NULL), m_HeaderSent(false), m_Chunked(false)
{
    this->setHeader("Server", "nidium/1.0");
}

NativeHTTPResponse::~NativeHTTPResponse()
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

void NativeHTTPResponse::sendHeaders(bool chunked)
{
    if (!m_Con || m_Con->getSocket() == NULL || m_HeaderSent) {
        return;
    }

    if (chunked) {
        m_Chunked = true;
    }

    const buffer &headers = this->getHeadersString();

    APE_socket_write(m_Con->getSocket(), headers.data,
        headers.used, APE_DATA_AUTORELEASE);

    m_HeaderSent = true;

    this->dataOwnershipTransfered(true);
}


void NativeHTTPResponse::sendChunk(char *buf, size_t len,
    ape_socket_data_autorelease datatype, bool willEnd)
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
    int tmplen = sprintf(tmpbuf, "%lx\r\n", len);

    PACK_TCP(m_Con->getSocket()->s.fd);

    APE_socket_write(m_Con->getSocket(), tmpbuf, tmplen, APE_DATA_STATIC);
    APE_socket_write(m_Con->getSocket(), buf, len, datatype);
    APE_socket_write(m_Con->getSocket(), (char *)CONST_STR_LEN("\r\n"), APE_DATA_STATIC);

    FLUSH_TCP(m_Con->getSocket()->s.fd);
}

void NativeHTTPResponse::send(ape_socket_data_autorelease datatype)
{
    if (!m_Con || m_Con->getSocket() == NULL) {
        return;
    }

    PACK_TCP(m_Con->getSocket()->s.fd);

    const buffer *data = this->getDataBuffer();

    if (!m_HeaderSent) {
        const buffer &headers = this->getHeadersString();

        APE_socket_write(m_Con->getSocket(), headers.data,
            headers.used, APE_DATA_AUTORELEASE);

        m_HeaderSent = true;
    }
    if (data && data->used) {
        APE_socket_write(m_Con->getSocket(), data->data,
            data->used, datatype);
    }
    FLUSH_TCP(m_Con->getSocket()->s.fd);
    
    /*
        If we are not in a APE_DATA_AUTORELEASE,
        only transfert the headers ownership
    */
    this->dataOwnershipTransfered(!(datatype == APE_DATA_AUTORELEASE));
}

void NativeHTTPResponse::end()
{
    if (!m_Con || m_Con->getSocket() == NULL) {
        return;
    }

    if (!m_HeaderSent) {
        this->sendHeaders();
    }

    if (m_Chunked) {
        APE_socket_write(m_Con->getSocket(),
            (char *)CONST_STR_LEN("0\r\n\r\n"), APE_DATA_STATIC);
    }

    const char *header_connection = m_Con->getHeader("connection");
    if (header_connection && strcasecmp(header_connection, "close") == 0) {
        /*
            TODO: temporise to (try to) avoid active close?
        */
        m_Con->close();
    }
}

void NativeHTTPResponse::setHeader(const char *key, const char *val)
{
    ape_array_add_camelkey_n(m_Headers, key, strlen(key), val, strlen(val));
}

void NativeHTTPResponse::removeHeader(const char *key)
{
    ape_array_delete(m_Headers, key, strlen(key));
}

void NativeHTTPResponse::setData(char *buf, size_t len)
{
    if (m_Content) {
        return;
    }
    m_Content = buffer_new(0);
    m_Content->data = (unsigned char *)buf;
    m_Content->size = m_Content->used = len;
}

const buffer &NativeHTTPResponse::getHeadersString()
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
    NativeUtils::HTTPTime(httpdate);

    this->setHeader("Date", httpdate);

    buffer_append_string_n(m_Headers_str, CONST_STR_LEN("\r\n"));

    if (!m_Chunked) {
        if (m_Content && m_Content->used) {
            sprintf(tmpbuf, "%ld", m_Content->used);
            this->setHeader("Content-Length", tmpbuf);
        } else {
            this->setHeader("Content-Length", "0");
        }
    } else {
        this->setHeader("Transfer-Encoding", "chunked");
        this->removeHeader("Content-Length");
    }
    buffer *k, *v;
    if (this->getHeaders()) {
        APE_A_FOREACH(this->getHeaders(), k, v) {
            buffer_append_string_n(m_Headers_str, (char *)k->data, k->used);
            buffer_append_string_n(m_Headers_str, ": ", 2);
            buffer_append_string_n(m_Headers_str, (char *)v->data, v->used);
            buffer_append_string_n(m_Headers_str, CONST_STR_LEN("\r\n"));
        }
    } else {
        buffer_append_string_n(m_Headers_str, CONST_STR_LEN("\r\n"));
    }
    buffer_append_string_n(m_Headers_str, CONST_STR_LEN("\r\n"));

    return *m_Headers_str;
}

const buffer *NativeHTTPResponse::getDataBuffer()
{
    return m_Content;
}

const char *NativeHTTPResponse::getStatusDesc() const
{
    for (int i = 0; NativeHTTP_Codes[i].desc != NULL; i++) {
        if (NativeHTTP_Codes[i].code == m_Statuscode) {
            return NativeHTTP_Codes[i].desc;
        }
    }

    return "Unknown";
}

void NativeHTTPResponse::dataOwnershipTransfered(bool onlyHeaders)
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
