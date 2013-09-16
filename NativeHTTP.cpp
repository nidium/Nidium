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

#include <NativeHTTP.h>
//#include "ape_http_parser.h"
#include <http_parser.h>
#include <native_netlib.h>

#include <stdio.h>
#include <string.h>
#include <limits.h>

#ifndef ULLONG_MAX
# define ULLONG_MAX ((uint64_t) -1) /* 2^64-1 */
#endif

#define HTTP_PREFIX "http://"
#define SOCKET_WRITE_STATIC(data) APE_socket_write(s, \
    (unsigned char *)CONST_STR_LEN(data), APE_DATA_STATIC)

#define SOCKET_WRITE_OWN(data) APE_socket_write(s, (unsigned char *)data, \
    strlen(data), APE_DATA_OWN)

static struct native_http_mime {
    const char *str;
    NativeHTTP::DataType data_type;
} native_mime[] = {
    {"text/plain",              NativeHTTP::DATA_STRING},
    {"application/x-javascript",NativeHTTP::DATA_STRING},
    {"application/octet-stream",NativeHTTP::DATA_STRING},
    {"image/jpeg",              NativeHTTP::DATA_IMAGE},
    {"image/png",               NativeHTTP::DATA_IMAGE},
    {"audio/mp3",               NativeHTTP::DATA_AUDIO},
    {"audio/mpeg",              NativeHTTP::DATA_AUDIO},
    {"audio/wave",              NativeHTTP::DATA_AUDIO},
    {"audio/ogg",               NativeHTTP::DATA_AUDIO},
    {"audio/x-wav",             NativeHTTP::DATA_AUDIO},
    {"video/ogg",               NativeHTTP::DATA_AUDIO},
    {"audio/webm",              NativeHTTP::DATA_AUDIO},
    {"application/json",        NativeHTTP::DATA_JSON},
    {"text/html",               NativeHTTP::DATA_STRING}, /* TODO: use dom.js */
    {"application/octet-stream",NativeHTTP::DATA_BINARY},
    {NULL,                      NativeHTTP::DATA_END}
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
    NativeHTTP *nhttp = (NativeHTTP *)p->data;

    nhttp->clearTimeout();

    return 0;
}

static int headers_complete_cb(http_parser *p)
{
    NativeHTTP *nhttp = (NativeHTTP *)p->data;

    if (nhttp->http.headers.tval != NULL) {
        buffer_append_char(nhttp->http.headers.tval, '\0');
    }

    if (p->content_length >= LLONG_MAX) {
        nhttp->http.contentlength = 0;
        nhttp->headerEnded();
        return 0;
    }

    if (p->content_length > HTTP_MAX_CL) {
        return -1;
    }

    /* /!\ TODO: what happend if there is no content-length? */
    if (p->content_length) nhttp->http.data = buffer_new(p->content_length);

    nhttp->http.contentlength = p->content_length;
    nhttp->headerEnded();

    return 0;
}

static int message_complete_cb(http_parser *p)
{
    NativeHTTP *nhttp = (NativeHTTP *)p->data;

    nhttp->requestEnded();

    return 0;
}

static int header_field_cb(http_parser *p, const char *buf, size_t len)
{
    NativeHTTP *nhttp = (NativeHTTP *)p->data;

    switch (nhttp->http.headers.prevstate) {
        case NativeHTTP::PSTATE_NOTHING:
            nhttp->http.headers.list = ape_array_new(16);
        case NativeHTTP::PSTATE_VALUE:
            nhttp->http.headers.tkey = buffer_new(16);
            if (nhttp->http.headers.tval != NULL) {
                buffer_append_char(nhttp->http.headers.tval, '\0');
            }
            break;
        default:
            break;
    }

    nhttp->http.headers.prevstate = NativeHTTP::PSTATE_FIELD;

    if (len != 0) {
        buffer_append_data(nhttp->http.headers.tkey,
            (const unsigned char *)buf, len);
    }

    return 0;
}

static int header_value_cb(http_parser *p, const char *buf, size_t len)
{
    NativeHTTP *nhttp = (NativeHTTP *)p->data;

    switch (nhttp->http.headers.prevstate) {
        case NativeHTTP::PSTATE_NOTHING:
            return -1;
        case NativeHTTP::PSTATE_FIELD:
            nhttp->http.headers.tval = buffer_new(64);
            buffer_append_char(nhttp->http.headers.tkey, '\0');
            ape_array_add_b(nhttp->http.headers.list,
                    nhttp->http.headers.tkey, nhttp->http.headers.tval);
            break;
        default:
            break;
    }

    nhttp->http.headers.prevstate = NativeHTTP::PSTATE_VALUE;

    if (len != 0) {
        buffer_append_data(nhttp->http.headers.tval,
            (const unsigned char *)buf, len);
    }
    return 0;
}

static int request_url_cb(http_parser *p, const char *buf, size_t len)
{
    printf("Request URL cb\n");
    return 0;
}

static int body_cb(http_parser *p, const char *buf, size_t len)
{
    NativeHTTP *nhttp = (NativeHTTP *)p->data;

    if (nhttp->http.data == NULL) {
        nhttp->http.data = buffer_new(2048);
    }

    if (nhttp->http.data->used+len > HTTP_MAX_CL) {
        return -1;
    }

    if (len != 0) {
        buffer_append_data(nhttp->http.data,
            (const unsigned char *)buf, len);
    }

    nhttp->onData(nhttp->http.data->used - len, len);

    return 0;
}


static void native_http_connected(ape_socket *s,
    ape_global *ape, void *socket_arg)
{
    NativeHTTP *nhttp = (NativeHTTP *)s->ctx;

    if (nhttp == NULL) return;

    nhttp->http.headers.list = NULL;
    nhttp->http.headers.tkey = NULL;
    nhttp->http.headers.tval = NULL;
    nhttp->http.data = NULL;
    nhttp->http.ended = 0;
    nhttp->http.headers.prevstate = NativeHTTP::PSTATE_NOTHING;

    http_parser_init(&nhttp->http.parser, HTTP_RESPONSE);
    nhttp->http.parser.data = nhttp;

    buffer *data = nhttp->getRequest()->getHeadersData();

    APE_socket_write(s, data->data, data->used, APE_DATA_COPY);

    if (nhttp->getRequest()->getData() != NULL &&
        nhttp->getRequest()->method == NativeHTTPRequest::NATIVE_HTTP_POST) {

        APE_socket_write(s, (unsigned char *)nhttp->getRequest()->getData(),
            nhttp->getRequest()->getDataLength(), APE_DATA_OWN);
    }
    buffer_destroy(data);
}

static void native_http_disconnect(ape_socket *s,
    ape_global *ape, void *socket_arg)
{
    NativeHTTP *nhttp = (NativeHTTP *)s->ctx;

    if (nhttp == NULL ||
        (nhttp->currentSock != NULL && s != nhttp->currentSock)) {

        return;
    }

    nhttp->clearTimeout();

    http_parser_execute(&nhttp->http.parser, &settings,
        NULL, 0);

    nhttp->currentSock = NULL;

    if (!nhttp->http.ended) {
        nhttp->delegate->onError(NativeHTTP::ERROR_DISCONNECTED);
    }
}

static void native_http_read(ape_socket *s, ape_global *ape,
    void *socket_arg)
{
    size_t nparsed;
    NativeHTTP *nhttp = (NativeHTTP *)s->ctx;

    if (nhttp == NULL) return;

    nparsed = http_parser_execute(&nhttp->http.parser, &settings,
        (const char *)s->data_in.data, (size_t)s->data_in.used);

    if (nparsed != s->data_in.used) {
        printf("[HTTP] (socket %p) Parser returned %ld with error %s\n", s, nparsed,
            http_errno_description(HTTP_PARSER_ERRNO(&nhttp->http.parser)));

        nhttp->delegate->onError(NativeHTTP::ERROR_RESPONSE);

        APE_socket_shutdown_now(s);
    }
}

NativeHTTP::NativeHTTP(NativeHTTPRequest *req, ape_global *n) :
    ptr(NULL), net(n), currentSock(NULL),
    err(0), timeout(HTTP_DEFAULT_TIMEOUT),
    timeoutTimer(0), delegate(NULL), m_FileSize(0)
{
    this->req = req;

    http.data = NULL;
    http.headers.tval = NULL;
    http.headers.tkey = NULL;
    http.headers.list = NULL;
    http.ended = 0;
    http.contentlength = 0;

    native_http_data_type = DATA_NULL;

}

void NativeHTTP::setPrivate(void *ptr)
{
    this->ptr = ptr;
}

void *NativeHTTP::getPrivate()
{
    return this->ptr;
}

void NativeHTTP::onData(size_t offset, size_t len)
{
    this->delegate->onProgress(offset, len,
        &this->http, this->native_http_data_type);
}

void NativeHTTP::headerEnded()
{
#define REQUEST_HEADER(header) ape_array_lookup(http.headers.list, \
    CONST_STR_LEN(header "\0"))

    if (http.headers.list != NULL) {
        buffer *content_type, *content_range;

        if ((content_type = REQUEST_HEADER("Content-Type")) != NULL &&
            content_type->used > 3) {
            int i;

            for (i = 0; native_mime[i].str != NULL; i++) {
                if (strncasecmp(native_mime[i].str, (const char *)content_type->data,
                    strlen(native_mime[i].str)) == 0) {
                    native_http_data_type = native_mime[i].data_type;
                    break;
                }
            }
        }

        if ((content_range = REQUEST_HEADER("Content-Range")) != NULL) {
            char *ptr = (char *)memchr(content_range->data,
                '/', content_range->used);

            if (ptr != NULL) {
                m_FileSize = atoll(&ptr[1]);
                if (m_FileSize >= LLONG_MAX) {
                    m_FileSize = 0;
                }
            }
        } else {
            m_FileSize = http.contentlength;
        }
    }

    switch (http.parser.status_code) {
        case 200:
            this->delegate->onHeader();
            break;
        case 404:
        default:
            this->delegate->onError(ERROR_HTTPCODE);
            break;
    }
    
#undef REQUEST_HEADER
}

void NativeHTTP::stopRequest(bool timeout)
{
    this->clearTimeout();
    
    if (!http.ended) {
        if (http.headers.list) {
            ape_array_destroy(http.headers.list);
        }

        if (http.data) buffer_destroy(http.data);

        http.data = NULL;
        http.headers.tval = NULL;
        http.headers.tkey = NULL;
        http.headers.list = NULL;

        if (currentSock) {
            APE_socket_shutdown_now(currentSock);
        }

        if (timeout) {
            this->delegate->onError(ERROR_TIMEOUT);
        }
    }
}

void NativeHTTP::requestEnded()
{
    if (!http.ended) {
        http.ended = 1;

        delegate->onRequest(&http, native_http_data_type);

        if (http.headers.list) {
            ape_array_destroy(http.headers.list);
        }

        if (http.data) {
            buffer_destroy(http.data);
        }
        http.data = NULL;
        http.headers.tval = NULL;
        http.headers.tkey = NULL;
        http.headers.list = NULL;

        if (currentSock) {
            APE_socket_shutdown(currentSock);
        }
    } 
}

static int NativeHTTP_handle_timeout(void *arg)
{
    ((NativeHTTP *)arg)->stopRequest(true);

    return 0;
}

void NativeHTTP::clearTimeout()
{
    if (this->timeoutTimer) {
        clear_timer_by_id(&net->timersng, this->timeoutTimer, 1);
        this->timeoutTimer = 0;
    }
}

int NativeHTTP::request(NativeHTTPDelegate *delegate)
{
    ape_socket *socket;

    if ((socket = APE_socket_new(APE_SOCKET_PT_TCP, 0, net)) == NULL) {
        printf("[Socket] Cant load socket (new)\n");
        this->delegate->onError(ERROR_SOCKET);
        return 0;
    }

    if (APE_socket_connect(socket, this->req->getPort(), this->req->getHost()) == -1) {
        printf("[Socket] Cant connect (0)\n");
        this->delegate->onError(ERROR_SOCKET);
        return 0;
    }

    socket->callbacks.on_connected = native_http_connected;
    socket->callbacks.on_read = native_http_read;
    socket->callbacks.on_disconnect = native_http_disconnect;

    http.ended = 0;
    socket->ctx = this;
    this->delegate = delegate;
    this->currentSock = socket;
    delegate->httpref = this;

    if (timeout) {
        ape_timer *ctimer;
        ctimer = add_timer(&net->timersng, timeout,
            NativeHTTP_handle_timeout, this);

        ctimer->flags &= ~APE_TIMER_IS_PROTECTED;
        timeoutTimer = ctimer->identifier;
    }

    return 1;
}

NativeHTTP::~NativeHTTP()
{
    if (req) {
        delete req;
    }

    if (currentSock != NULL) {
        currentSock->ctx = NULL;

        APE_socket_shutdown_now(currentSock);
    }

    if (timeoutTimer) {
        this->clearTimeout();
    }

    if (!http.ended) {
        if (http.headers.list) ape_array_destroy(http.headers.list);
        if (http.data) buffer_destroy(http.data);
    }
}


int NativeHTTP::ParseURI(char *url, size_t url_len, char *host,
    u_short *port, char *file)
{
    char *p;
    const char *p2;
    int len;

    len = strlen(HTTP_PREFIX);
    if (strncasecmp(url, HTTP_PREFIX, len)) {
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
        *port = 80;

    return 0;
}

NativeHTTPRequest::NativeHTTPRequest(const char *url) :
    method(NATIVE_HTTP_GET), data(NULL), datalen(0),
    datafree(free), headers(ape_array_new(8))
{
    size_t url_len = strlen(url);
    char *durl = (char *)malloc(sizeof(char) * (url_len+1));

    memcpy(durl, url, url_len+1);

    this->host = (char *)malloc(url_len+1);
    this->path = (char *)malloc(url_len+1);
    memset(this->host, 0, url_len+1);
    memset(this->path, 0, url_len+1);

    if (NativeHTTP::ParseURI(durl, url_len, this->host,
        &this->port, this->path) == -1) {

        memset(host, 0, url_len+1);
        memset(path, 0, url_len+1);
        port = 0;
    }

    free(durl);

}

buffer *NativeHTTPRequest::getHeadersData() const
{
    buffer *ret = buffer_new(1024);

    switch (this->method) {
        case NATIVE_HTTP_GET:
            buffer_append_string_n(ret, CONST_STR_LEN("GET "));
            break;
        case NATIVE_HTTP_HEAD:
            buffer_append_string_n(ret, CONST_STR_LEN("HEAD "));
            break;
        case NATIVE_HTTP_POST:
            buffer_append_string_n(ret, CONST_STR_LEN("POST "));
            break;
    }

    buffer_append_string(ret, this->path);
    buffer_append_string_n(ret, CONST_STR_LEN(" HTTP/1.1\n"));
    buffer_append_string_n(ret, CONST_STR_LEN("Host: "));
    buffer_append_string(ret, this->host);
    buffer_append_string_n(ret, CONST_STR_LEN("\n"));

    buffer *k, *v;
    APE_A_FOREACH(this->getHeaders(), k, v) {
        buffer_append_string_n(ret, (char *)k->data, k->used);
        buffer_append_string_n(ret, ": ", 2);
        buffer_append_string_n(ret, (char *)v->data, v->used);
        buffer_append_string_n(ret, "\n", 1);
    }

    buffer_append_string_n(ret, "\n", 1);

    return ret;
}
