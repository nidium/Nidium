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
    NativeHTTPListener *http = (NativeHTTPListener *)p->data;

    return 0;
}

static int header_field_cb(http_parser *p, const char *buf, size_t len)
{
    NativeHTTPClientConnection *con = (NativeHTTPClientConnection *)p->data;
    NativeHTTPClientConnection::HTTPData *http_data = con->getHTTPState();

    switch (http_data->headers.prevstate) {
        case NativeHTTPClientConnection::PSTATE_NOTHING:
            http_data->headers.list = ape_array_new(16);
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
        buffer_append_data(http_data->headers.tkey,
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

    if (client->getHTTPListener()->onEnd(client)) {
        client->close();
    } else {
        /* Further reading possible */
        client->resetData();
    }

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

    printf("Request URL cb %.*s\n", (int)len, buf);
    return 0;
}

static void native_socket_onaccept(ape_socket *socket_server,
    ape_socket *socket_client, ape_global *ape, void *socket_arg)
{
    NativeHTTPListener *http = GET_HTTP_OR_FAIL(socket_server);

    http->onClientConnect(socket_client, ape);

    printf("New connexion\n");
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

    printf("Connexion closed\n");
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

bool NativeHTTPListener::start()
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

    return (APE_socket_listen(m_Socket, m_Port, m_IP) != -1);
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
NativeHTTPClientConnection::NativeHTTPClientConnection(NativeHTTPListener *httpserver,
    ape_socket *socket) :
    m_SocketClient(socket), m_HTTPListener(httpserver)
{
    m_HttpState.headers.prevstate = PSTATE_NOTHING;

    m_HttpState.headers.list = NULL;
    m_HttpState.headers.tkey = NULL;
    m_HttpState.headers.tval = NULL;
    m_HttpState.ended = 0;
    m_HttpState.data  = NULL;

    http_parser_init(&m_HttpState.parser, HTTP_REQUEST);
    m_HttpState.parser.data = this;
}

void NativeHTTPClientConnection::onRead(buffer *buf, ape_global *ape)
{
#define REQUEST_HEADER(header) ape_array_lookup(m_HttpState.headers.list, \
    CONST_STR_LEN(header "\0"))

    int nparsed = http_parser_execute(&m_HttpState.parser, &settings,
        (const char *)buf->data, (size_t)buf->used);

    if (m_HttpState.parser.upgrade) {
        this->onUpgrade((const char *)REQUEST_HEADER("upgrade")->data);
        if (nparsed < buf->used) {
            /* Non-http data in the current packet (after the header) */
            this->onContent((const char *)&buf->data[nparsed], buf->used - nparsed);
        }

        /* Change the callback for the next on_read calls */
        m_SocketClient->callbacks.on_read = native_socket_client_read_after_upgrade;
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
    if (m_HttpState.data) {
        buffer_destroy(m_HttpState.data);
    }
};

void NativeHTTPClientConnection::write(char *buf, size_t len)
{
    APE_socket_write(m_SocketClient, buf, len, APE_DATA_COPY);
}

void NativeHTTPClientConnection::sendResponse(NativeHTTPResponse *resp)
{
    const buffer &headers = resp->getHeadersString();
    const buffer *data = resp->getDataBuffer();

    printf("Response sent %s\n", headers.data);
    APE_socket_write(m_SocketClient, headers.data, headers.used, APE_DATA_AUTORELEASE);

    if (data && data->used) {
        APE_socket_write(m_SocketClient, data->data, data->used, APE_DATA_AUTORELEASE);
    }

    resp->dataOwnershipTransfered();
}

//////

NativeHTTPResponse::NativeHTTPResponse(uint16_t code) :
    m_Headers(ape_array_new(8)), m_Statuscode(code), m_ContentLength(0),
    m_Content(NULL), m_Headers_str(NULL)
{

}

NativeHTTPResponse::~NativeHTTPResponse()
{
    if (m_Headers) {
        ape_array_destroy(m_Headers);
    }

    if (m_Content) {
        buffer_destroy(m_Content);
    }
}

const buffer &NativeHTTPResponse::getHeadersString()
{
    if (m_Headers_str == NULL) {
        m_Headers_str = buffer_new(512);
    }

    char tmpbuf[32];
    sprintf(tmpbuf, "HTTP/1.1 %u ", m_Statuscode);

    buffer_append_string(m_Headers_str, tmpbuf);
    buffer_append_string(m_Headers_str, this->getStatusDesc());
    buffer_append_string_n(m_Headers_str, CONST_STR_LEN("\n"));

    if (m_Content) {
        sprintf(tmpbuf, "%ld", m_Content->used);
        this->setHeader("Content-Length", tmpbuf);
    } else {
        this->setHeader("Content-Length", "0");
    }

    buffer *k, *v;
    APE_A_FOREACH(this->getHeaders(), k, v) {
        buffer_append_string_n(m_Headers_str, (char *)k->data, k->used);
        buffer_append_string_n(m_Headers_str, ": ", 2);
        buffer_append_string_n(m_Headers_str, (char *)v->data, v->used);
        buffer_append_string_n(m_Headers_str, "\n", 1);
    }

    buffer_append_string_n(m_Headers_str, "\n", 1);

    return *m_Headers_str;
}

const buffer *NativeHTTPResponse::getDataBuffer(){
    return m_Content;
}

const char *NativeHTTPResponse::getStatusDesc() const
{
    for (int i = 0; NativeHTTP_Codes[i].desc != NULL; i++) {
        if (NativeHTTP_Codes[i].code == m_Statuscode) {
            return NativeHTTP_Codes[i].desc;
        }
    }

    return NULL;
}

void NativeHTTPResponse::dataOwnershipTransfered()
{
    /*
        Free the buffer object. We're only giving the data away.
    */
    free(m_Headers_str);
    free(m_Content);

    m_Headers_str = NULL;
    m_Content = NULL;
}
