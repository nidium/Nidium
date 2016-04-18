/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "HTTP.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "Core/Path.h"

namespace Nidium {
namespace Net {

#ifndef ULLONG_MAX
# define ULLONG_MAX ((uint64_t) -1) /* 2^64-1 */
#endif

#define HTTP_PREFIX "http://"
#define SOCKET_WRITE_STATIC(data) APE_socket_write(s, \
    (unsigned char *)CONST_STR_LEN(data), APE_DATA_STATIC)

#define SOCKET_WRITE_OWN(data) APE_socket_write(s, (unsigned char *)data, \
    strlen(data), APE_DATA_OWN)

static struct nidium_http_mime {
    const char *str;
    HTTP::DataType data_type;
} nidium_mime[] = {
    {"text/plain",                  HTTP::DATA_STRING},
    {"application/x-javascript",    HTTP::DATA_STRING},
    {"application/javascript",      HTTP::DATA_STRING},
    {"application/octet-stream",    HTTP::DATA_STRING},
    {"image/jpeg",                  HTTP::DATA_IMAGE},
    {"image/png",                   HTTP::DATA_IMAGE},
    {"audio/mp3",                   HTTP::DATA_AUDIO},
    {"audio/mpeg",                  HTTP::DATA_AUDIO},
    {"audio/wave",                  HTTP::DATA_AUDIO},
    {"audio/ogg",                   HTTP::DATA_AUDIO},
    {"audio/x-wav",                 HTTP::DATA_AUDIO},
    {"video/ogg",                   HTTP::DATA_AUDIO},
    {"audio/webm",                  HTTP::DATA_AUDIO},
    {"application/json",            HTTP::DATA_JSON},
    {"text/html",                   HTTP::DATA_STRING}, /* TODO: use dom.js */
    {"application/octet-stream",    HTTP::DATA_BINARY},
    {NULL,                          HTTP::DATA_END}
};

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

static int message_begin_cb(http_parser *p)
{
    HTTP *nhttp = (HTTP *)p->data;

    nhttp->clearState();

    nhttp->clearTimeout();

    nhttp->http.data = buffer_new(0);

    return 0;
}

static int headers_complete_cb(http_parser *p)
{
    HTTP *nhttp = (HTTP *)p->data;

    if (nhttp->http.headers.tval != NULL) {
        buffer_append_char(nhttp->http.headers.tval, '\0');
    }

    if (p->content_length == ULLONG_MAX) {
        nhttp->http.contentlength = 0;
        nhttp->headerEnded();
        return 0;
    }

    if (p->content_length > HTTP_MAX_CL) {
        return -1;
    }

    nhttp->http.contentlength = p->content_length;
    nhttp->headerEnded();

    return 0;
}

static int message_complete_cb(http_parser *p)
{
    HTTP *nhttp = (HTTP *)p->data;

    nhttp->requestEnded();

    return 0;
}

static int header_field_cb(http_parser *p, const char *buf, size_t len)
{
    HTTP *nhttp = (HTTP *)p->data;

    switch (nhttp->http.headers.prevstate) {
        case HTTP::PSTATE_NOTHING:
            nhttp->http.headers.list = ape_array_new(16);
            /* fall through */
        case HTTP::PSTATE_VALUE:
            nhttp->http.headers.tkey = buffer_new(16);
            if (nhttp->http.headers.tval != NULL) {
                buffer_append_char(nhttp->http.headers.tval, '\0');
            }
            break;
        default:
            break;
    }

    nhttp->http.headers.prevstate = HTTP::PSTATE_FIELD;

    if (len != 0) {
        buffer_append_data_tolower(nhttp->http.headers.tkey,
            (const unsigned char *)buf, len);
    }

    return 0;
}

static int header_value_cb(http_parser *p, const char *buf, size_t len)
{
    HTTP *nhttp = (HTTP *)p->data;

    switch (nhttp->http.headers.prevstate) {
        case HTTP::PSTATE_NOTHING:
            return -1;
        case HTTP::PSTATE_FIELD:
            nhttp->http.headers.tval = buffer_new(64);
            buffer_append_char(nhttp->http.headers.tkey, '\0');
            ape_array_add_b(nhttp->http.headers.list,
                    nhttp->http.headers.tkey, nhttp->http.headers.tval);
            break;
        default:
            break;
    }

    nhttp->http.headers.prevstate = HTTP::PSTATE_VALUE;

    if (len != 0) {
        buffer_append_data(nhttp->http.headers.tval,
            (const unsigned char *)buf, len);
    }
    return 0;
}

static int request_url_cb(http_parser *p, const char *buf, size_t len)
{
    return 0;
}

static int body_cb(http_parser *p, const char *buf, size_t len)
{
    HTTP *nhttp = (HTTP *)p->data;

    if (nhttp->http.data == NULL) {
        nhttp->http.data = buffer_new(2048);
    }

    if ((uint64_t)(nhttp->http.data->used + len) > HTTP_MAX_CL) {
        return -1;
    }

    if (len != 0) {
        buffer_append_data(nhttp->http.data,
            (const unsigned char *)buf, len);
    }

    nhttp->onData(nhttp->http.data->used - len, len);

    return 0;
}

static void nidium_http_connected(ape_socket *s,
    ape_global *ape, void *socket_arg)
{
    HTTP *nhttp = (HTTP *)s->ctx;

    if (nhttp == NULL) return;

    http_parser_init(&nhttp->http.parser, HTTP_RESPONSE);
    nhttp->http.parser.data = nhttp;
    nhttp->http.parser_rdy = true;

    HTTPRequest *request = nhttp->getRequest();
    buffer *headers = request->getHeadersData();

    if (request->getData() != NULL &&
        (request->method == HTTPRequest::HTTP_POST ||
            request->method == HTTPRequest::HTTP_PUT)) {

        PACK_TCP(s->s.fd);
        APE_socket_write(s, headers->data, headers->used, APE_DATA_COPY);

        /* APE_DATA_OWN? Warum? It's good as long as
        the lifetime of the data is tied to the socket lifetime */
        APE_socket_write(s, (unsigned char *)request->getData(),
            nhttp->getRequest()->getDataLength(), APE_DATA_OWN);
        FLUSH_TCP(s->s.fd);
    } else {
        APE_socket_write(s, headers->data, headers->used, APE_DATA_COPY);
    }

    buffer_destroy(headers);
}

static void nidium_http_disconnect(ape_socket *s,
    ape_global *ape, void *socket_arg)
{
    HTTP *nhttp = (HTTP *)s->ctx;

    if (nhttp == NULL ||
        (nhttp->m_CurrentSock != NULL && s != nhttp->m_CurrentSock)) {
        return;
    }

    nhttp->clearTimeout();

    if (!nhttp->isParsing() && nhttp->http.parser_rdy) {
        http_parser_execute(&nhttp->http.parser, &settings,
            NULL, 0);
    }

    nhttp->m_CurrentSock = NULL;

    if (!nhttp->http.ended) {
        nhttp->setPendingError(HTTP::ERROR_DISCONNECTED);
    }

    nhttp->clearState();

    nhttp->canDoRequest(true);

    s->ctx = NULL;

}

static void nidium_http_read(ape_socket *s,
    const uint8_t *data, size_t len, ape_global *ape, void *socket_arg)
{
    size_t nparsed;
    HTTP *nhttp = (HTTP *)s->ctx;

    if (nhttp == NULL || nhttp->http.ended) {
        return;
    }

    nhttp->parsing(true);
    nparsed = http_parser_execute(&nhttp->http.parser, &settings,
        (const char *)data, len);
    nhttp->parsing(false);

    if (nparsed != len && !nhttp->http.ended) {
        fprintf(stderr, "[HTTP] (socket %p) Parser returned %ld with error %s\n", s, (unsigned long) nparsed,
            http_errno_description(HTTP_PARSER_ERRNO(&nhttp->http.parser)));

        nhttp->setPendingError(HTTP::ERROR_RESPONSE);

        APE_socket_shutdown_now(s);
    }
}

HTTP::HTTP(ape_global *n) :
    ptr(NULL), net(n), m_CurrentSock(NULL),
    err(0), m_Timeout(HTTP_DEFAULT_TIMEOUT),
    m_TimeoutTimer(0), delegate(NULL),
    m_FileSize(0), m_isParsing(false), m_Request(NULL), m_CanDoRequest(true),
    m_PendingError(ERROR_NOERR), m_MaxRedirect(8), m_FollowLocation(true)
{
    memset(&http, 0, sizeof(http));
    memset(&m_Redirect, 0, sizeof(m_Redirect));

    http.headers.prevstate = HTTP::PSTATE_NOTHING;
    nidium_http_data_type = DATA_NULL;
}

void HTTP::reportPendingError()
{
    if (this->delegate && m_PendingError != ERROR_NOERR) {
        this->delegate->onError(m_PendingError);
    }

    m_PendingError = ERROR_NOERR;
}

void HTTP::setPrivate(void *ptr)
{
    this->ptr = ptr;
}

void *HTTP::getPrivate()
{
    return this->ptr;
}

void HTTP::onData(size_t offset, size_t len)
{
    this->delegate->onProgress(offset, len,
        &this->http, this->nidium_http_data_type);
}

void HTTP::headerEnded()
{
#define REQUEST_HEADER(header) ape_array_lookup(http.headers.list, \
    CONST_STR_LEN(header "\0"))

    m_Redirect.enabled = false;

    if (http.headers.list != NULL) {
        buffer *content_type, *content_range, *location;

        if ((content_type = REQUEST_HEADER("Content-Type")) != NULL &&
            content_type->used > 3) {
            int i;

            for (i = 0; nidium_mime[i].str != NULL; i++) {
                if (strncasecmp(nidium_mime[i].str, (const char *)content_type->data,
                    strlen(nidium_mime[i].str)) == 0) {
                    nidium_http_data_type = nidium_mime[i].data_type;
                    break;
                }
            }
        }

        if (http.parser.status_code == 206 &&
            (content_range = REQUEST_HEADER("Content-Range")) != NULL) {
            char *ptr = (char *)memchr(content_range->data,
                '/', content_range->used);

            if (ptr != NULL) {
                m_FileSize = atoll(&ptr[1]);
                if (m_FileSize >= LLONG_MAX) {
                    m_FileSize = 0;
                }
            }
        } else if (m_FollowLocation && (http.parser.status_code == 301 ||
                    http.parser.status_code == 302) &&
                (location = REQUEST_HEADER("Location")) != NULL) {

            m_FileSize = 0;

            m_Redirect.enabled = true;
            m_Redirect.to = (const char *)location->data;
            m_Redirect.count++;

            if (m_Redirect.count > m_MaxRedirect) {
                setPendingError(ERROR_REDIRECTMAX);
            }

            return;
        } else {
            m_FileSize = http.contentlength;
        }
    }
/*
    switch (http.parser.status_code/100) {
        case 1:
        case 2:
        case 3:
            this->delegate->onHeader();
            break;
        case 4:
        case 5:
        default:
            this->delegate->onError(ERROR_HTTPCODE);
            break;
    }
*/

    this->delegate->onHeader();

#undef REQUEST_HEADER
}

/*
    stopRequest can be used to shutdown slow or maliscious connections
    since the shutdown is not queued
*/
void HTTP::stopRequest(bool timeout)
{
    this->clearTimeout();

    if (!http.ended) {
        http.ended = 1;

        /*
            Make sur the connection is closed right now
        */
        this->close(true);

        if (timeout) {
            this->setPendingError(ERROR_TIMEOUT);
        }

        this->clearState();

        if (!m_isParsing && http.parser_rdy) {
            http_parser_execute(&http.parser, &settings, NULL, 0);
        }
    }
}

void HTTP::requestEnded()
{
    m_CanDoRequest = true;

    if (m_Redirect.enabled && !hasPendingError()) {

        if (URLSCHEME_MATCH(m_Redirect.to, "http")) {
            m_Request->resetURL(m_Redirect.to);

        } else {
            m_Request->setPath(m_Redirect.to);

        }
        this->clearState();
        this->request(m_Request, delegate, true);
        return;
    }

    if (!http.ended) {
        http.ended = 1;
        bool doclose = !this->isKeepAlive();

        if (!hasPendingError()) {
            delegate->onRequest(&http, nidium_http_data_type);
        }

        this->clearState();

        if (doclose) {
            this->close();
            nidium_http_disconnect(m_CurrentSock, m_CurrentSock->ape, NULL);
        }
    }
}

void HTTP::clearState()
{
    this->reportPendingError();

    ape_array_destroy(http.headers.list);
    buffer_destroy(http.data);
    http.data = NULL;

    memset(&http.headers, 0, sizeof(http.headers));
    http.headers.prevstate = HTTP::PSTATE_NOTHING;

}

bool HTTP::isKeepAlive()
{
    /*
        First check the server "connection" header
    */
    const char *header_connection = this->getHeader("connection");
    if (header_connection && strcasecmp(header_connection, "close") == 0) {
        return false;
    }

    /*
        Then check ours
    */
    if (m_Request) {
        header_connection = m_Request->getHeader("connection");
        if (header_connection && strcasecmp(header_connection, "close") == 0) {
            return false;
        }
    }
    return true;
}

static int Nidium_HTTP_handle_timeout(void *arg)
{
    ((HTTP *)arg)->stopRequest(true);

    return 0;
}

void HTTP::clearTimeout()
{
    if (this->m_TimeoutTimer) {
        APE_timer_clearbyid(net, this->m_TimeoutTimer, 1);
        this->m_TimeoutTimer = 0;
    }
}

bool HTTP::createConnection()
{
    if (!m_Request) {
        return false;
    }

    ape_socket *socket;

    if ((socket = APE_socket_new(m_Request->isSSL() ?
        APE_SOCKET_PT_SSL : APE_SOCKET_PT_TCP, 0, net)) == NULL) {

        printf("[Socket] Cant load socket (new)\n");
        if (this->delegate) {
            this->setPendingError(ERROR_SOCKET);
        }
        return false;
    }

    if (APE_socket_connect(socket, m_Request->getPort(), m_Request->getHost(), 0) == -1) {
        printf("[Socket] Cant connect (0)\n");
        if (this->delegate) {
            this->setPendingError(ERROR_SOCKET);
        }
        return false;
    }

    socket->callbacks.on_connected  = nidium_http_connected;
    socket->callbacks.on_read       = nidium_http_read;
    socket->callbacks.on_disconnect = nidium_http_disconnect;

    socket->ctx = this;

    this->m_CurrentSock = socket;

    return true;
}

bool HTTP::request(HTTPRequest *req,
    HTTPDelegate *delegate, bool forceNewConnection)
{
    if (!canDoRequest()) {
        this->clearState();
        return false;
    }

    /* A fresh request is given */
    if (m_Request && req != m_Request) {
        delete m_Request;
    }

    m_Request = req;
    bool reusesock = (m_CurrentSock != NULL);

    if (reusesock && forceNewConnection) {
        reusesock = false;

        m_CurrentSock->ctx = NULL;
        APE_socket_shutdown_now(m_CurrentSock);
    }

    /*
        If we have an available socket, reuse it (keep alive)
    */
    if (!reusesock && !createConnection()) {
        this->clearState();
        return false;
    }

    m_Path = req->isSSL() ? std::string("https://") : std::string("http://") + std::string(req->getHost());

    if (req->getPort() != 80 && req->getPort() != 443) {
        m_Path += std::string(":") + std::to_string(req->getPort());
    }

    m_Path += req->getPath();

    this->delegate = delegate;
    http.ended = 0;

    delegate->httpref = this;

    if (m_Timeout) {
        ape_timer_t *ctimer;
        ctimer = APE_timer_create(net, m_Timeout,
            Nidium_HTTP_handle_timeout, this);

        APE_timer_unprotect(ctimer);
        m_TimeoutTimer = APE_timer_getid(ctimer);
    }

    m_CanDoRequest = false;

    if (reusesock) {
        nidium_http_connected(m_CurrentSock, net, NULL);
    }

    return true;
}

HTTP::~HTTP()
{
    if (m_CurrentSock != NULL) {
        m_CurrentSock->ctx = NULL;
        this->close(true);
    }

    if (m_TimeoutTimer) {
        this->clearTimeout();
    }

    if (m_Request) {
        delete m_Request;
    }

    this->delegate = NULL;
    m_PendingError = ERROR_NOERR;

    this->clearState();

}

const char *HTTP::getHeader(const char *key)
{
    buffer *ret = ape_array_lookup_cstr(http.headers.list, key, strlen(key));
    return ret ? (const char *)ret->data : NULL;
}

int HTTP::ParseURI(char *url, size_t url_len, char *host,
    u_short *port, char *file, const char *prefix, u_short default_port)
{
    char *p;
    const char *p2;
    int len;

    len = strlen(prefix);
    if (strncasecmp(url, prefix, len)) {
        return -1;
    }

    url += len;

    memcpy(host, url, (url_len-len));

    p = strchr(host, '/');
    if (p != NULL) {
        *p = '\0';
        p2 = p + 1;
    } else {
        p2 = NULL;
    }
    if (file != NULL) {
        /* Generate request file */
        if (p2 == NULL)
            p2 = "";
        sprintf(file, "/%s", p2);
    }

    p = strchr(host, ':');

    if (p != NULL) {
        *p = '\0';
        *port = atoi(p + 1);

        if (*port == 0)
            return -1;
    } else
        *port = default_port;

    return 0;
}

HTTPRequest::HTTPRequest(const char *url) :
    method(HTTP_GET), host(NULL), path(NULL), data(NULL), datalen(0),
    datafree(free), headers(ape_array_new(8)), m_isSSL(false) 
{
    this->resetURL(url);
    this->setDefaultHeaders();
}

bool HTTPRequest::resetURL(const char *url)
{
    m_isSSL = false;
    if (this->host) free(this->host);
    if (this->path) free(this->path);

    size_t url_len = strlen(url);
    char *durl = (char *)malloc(sizeof(char) * (url_len+1));

    memcpy(durl, url, url_len+1);

    this->host = (char *)malloc(url_len+1);
    this->path = (char *)malloc(url_len+1);

    memset(this->host, 0, url_len+1);
    memset(this->path, 0, url_len+1);

    u_short default_port = 80;

    const char *prefix = NULL;
    if (strncasecmp(url, CONST_STR_LEN("https://")) == 0) {
        prefix = "https://";
        m_isSSL = true;
        default_port = 443;
    } else if (strncasecmp(url, CONST_STR_LEN("http://")) == 0) {
        prefix = "http://";
    } else {
        /* No prefix provided. Assuming 'default url' => no SSL, port 80 */
        prefix = "";
    }

    if (HTTP::ParseURI(durl, url_len, this->host,
        &this->port, this->path, prefix, default_port) == -1) {
        memset(host, 0, url_len+1);
        memset(path, 0, url_len+1);
        port = 0;

        free(durl);
        return false;
    }

    free(durl);

    return true;
}

void HTTPRequest::setDefaultHeaders()
{
    this->setHeader("User-Agent", "Mozilla/5.0 (Unknown arch) nidium/" NATIVE_VERSION_STR " (nidium, like Gecko) nidium/" NATIVE_VERSION_STR);
    this->setHeader("Accept-Charset", "UTF-8");
    this->setHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
}

buffer *HTTPRequest::getHeadersData() const
{
    buffer *ret = buffer_new(1024);

    switch (this->method) {
        case HTTP_GET:
            buffer_append_string_n(ret, CONST_STR_LEN("GET "));
            break;
        case HTTP_HEAD:
            buffer_append_string_n(ret, CONST_STR_LEN("HEAD "));
            break;
        case HTTP_POST:
            buffer_append_string_n(ret, CONST_STR_LEN("POST "));
            break;
        case HTTP_PUT:
            buffer_append_string_n(ret, CONST_STR_LEN("PUT "));
            break;
        case HTTP_DELETE:
            buffer_append_string_n(ret, CONST_STR_LEN("DELETE "));
            break;
    }

    buffer_append_string(ret, this->path);
    buffer_append_string_n(ret, CONST_STR_LEN(" HTTP/1.1\r\n"));
    buffer_append_string_n(ret, CONST_STR_LEN("Host: "));
    buffer_append_string(ret, this->host);

    if (this->getPort() != 80 && this->getPort() != 443) {
        char portstr[8];
        sprintf(portstr, ":%hu", this->getPort());
        buffer_append_string(ret, portstr);
    }
    buffer_append_string_n(ret, CONST_STR_LEN("\r\n"));

    buffer *k, *v;
    APE_A_FOREACH(this->getHeaders(), k, v) {
        buffer_append_string_n(ret, (char *)k->data, k->used);
        buffer_append_string_n(ret, ": ", 2);
        buffer_append_string_n(ret, (char *)v->data, v->used);
        buffer_append_string_n(ret, CONST_STR_LEN("\r\n"));
    }

    buffer_append_string_n(ret, CONST_STR_LEN("\r\n"));

    return ret;
}

}
}
