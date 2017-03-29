/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Net/HTTP.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _MSC_VER
#include "Port/MSWindows.h"
#else
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#endif

#include "Core/Path.h"

namespace Nidium {
namespace Net {

// {{{ Preamble
#ifndef ULLONG_MAX
#define ULLONG_MAX ((uint64_t)-1) /* 2^64-1 */
#endif

#define HTTP_PREFIX "http://"
#define SOCKET_WRITE_STATIC(data) \
    APE_socket_write(s, (unsigned char *)CONST_STR_LEN(data), APE_DATA_STATIC)

#define SOCKET_WRITE_OWN(data) \
    APE_socket_write(s, (unsigned char *)data, strlen(data), APE_DATA_OWN)

const char *HTTP::HTTPErrorDescription[_kHTTPError_End_]
    = { "Unknown error",        "Timeout",      "Invalid response",
        "Disconnected",         "Socket error", "Failed to parse response",
        "Max redirect exceeded" };

static struct nidium_http_mime
{
    const char *m_Str;
    HTTP::DataType m_DataType;
} nidium_mime[] = { { "text/plain", HTTP::DATA_STRING },
                    { "application/x-javascript", HTTP::DATA_STRING },
                    { "application/javascript", HTTP::DATA_STRING },
                    { "application/octet-stream", HTTP::DATA_BINARY },
                    { "image/jpeg", HTTP::DATA_IMAGE },
                    { "image/png", HTTP::DATA_IMAGE },
                    { "audio/mp3", HTTP::DATA_AUDIO },
                    { "audio/mpeg", HTTP::DATA_AUDIO },
                    { "audio/wave", HTTP::DATA_AUDIO },
                    { "audio/ogg", HTTP::DATA_AUDIO },
                    { "audio/x-wav", HTTP::DATA_AUDIO },
                    { "video/ogg", HTTP::DATA_AUDIO },
                    { "audio/webm", HTTP::DATA_AUDIO },
                    { "application/json", HTTP::DATA_JSON },
                    { "text/html", HTTP::DATA_STRING }, /* TODO: use dom.js */
                    { NULL, HTTP::DATA_END } };
// }}}

// {{{ HTTP Parser callbacks
static int message_begin_cb(http_parser *p);
static int headers_complete_cb(http_parser *p);
static int message_complete_cb(http_parser *p);
static int header_field_cb(http_parser *p, const char *buf, size_t len);
static int header_value_cb(http_parser *p, const char *buf, size_t len);
static int request_url_cb(http_parser *p, const char *buf, size_t len);
static int body_cb(http_parser *p, const char *buf, size_t len);

static http_parser_settings settings
    = {/* .on_message_begin    = */ message_begin_cb,
       /* .on_url              = */ request_url_cb,
       /* .on_status           = */ NULL,
       /* .on_header_field     = */ header_field_cb,
       /* .on_header_value     = */ header_value_cb,
       /* .on_headers_complete = */ headers_complete_cb,
       /* .on_body             = */ body_cb,
       /* .on_message_complete = */ message_complete_cb
};


static int message_begin_cb(http_parser *p)
{
    HTTP *nhttp = static_cast<HTTP *>(p->data);

    nhttp->clearState();

    nhttp->clearTimeout();

    nhttp->m_HTTP.m_Data = buffer_new(0);

    return 0;
}

static int headers_complete_cb(http_parser *p)
{
    HTTP *nhttp = static_cast<HTTP *>(p->data);

    if (nhttp->m_HTTP.m_Headers.tval != NULL) {
        buffer_append_char(nhttp->m_HTTP.m_Headers.tval, '\0');
    }

    if (p->content_length == ULLONG_MAX) {
        nhttp->m_HTTP.m_ContentLength = 0;
        nhttp->headerEnded();
        return 0;
    }

    if (p->content_length > HTTP_MAX_CL) {
        return -1;
    }

    nhttp->m_HTTP.m_ContentLength = p->content_length;
    nhttp->headerEnded();

    return 0;
}

static int message_complete_cb(http_parser *p)
{
    HTTP *nhttp = static_cast<HTTP *>(p->data);

    nhttp->requestEnded();

    return 0;
}

static int header_field_cb(http_parser *p, const char *buf, size_t len)
{
    HTTP *nhttp = static_cast<HTTP *>(p->data);

    switch (nhttp->m_HTTP.m_Headers.prevstate) {
        case HTTP::PSTATE_NOTHING:
            nhttp->m_HTTP.m_Headers.list = ape_array_new(16);
        /* fall through */
        case HTTP::PSTATE_VALUE:
            nhttp->m_HTTP.m_Headers.tkey = buffer_new(16);
            if (nhttp->m_HTTP.m_Headers.tval != NULL) {
                buffer_append_char(nhttp->m_HTTP.m_Headers.tval, '\0');
            }
            break;
        default:
            break;
    }

    nhttp->m_HTTP.m_Headers.prevstate = HTTP::PSTATE_FIELD;

    if (len != 0) {
        buffer_append_data_tolower(nhttp->m_HTTP.m_Headers.tkey,
                                   reinterpret_cast<const unsigned char *>(buf),
                                   len);
    }

    return 0;
}

static int header_value_cb(http_parser *p, const char *buf, size_t len)
{
    HTTP *nhttp = static_cast<HTTP *>(p->data);

    switch (nhttp->m_HTTP.m_Headers.prevstate) {
        case HTTP::PSTATE_NOTHING:
            return -1;
        case HTTP::PSTATE_FIELD:
            nhttp->m_HTTP.m_Headers.tval = buffer_new(64);
            buffer_append_char(nhttp->m_HTTP.m_Headers.tkey, '\0');
            ape_array_add_b(nhttp->m_HTTP.m_Headers.list,
                            nhttp->m_HTTP.m_Headers.tkey,
                            nhttp->m_HTTP.m_Headers.tval);
            break;
        default:
            break;
    }

    nhttp->m_HTTP.m_Headers.prevstate = HTTP::PSTATE_VALUE;

    if (len != 0) {
        buffer_append_data(nhttp->m_HTTP.m_Headers.tval,
                           reinterpret_cast<const unsigned char *>(buf), len);
    }
    return 0;
}

static int request_url_cb(http_parser *p, const char *buf, size_t len)
{
    return 0;
}

static int body_cb(http_parser *p, const char *buf, size_t len)
{
    HTTP *nhttp = static_cast<HTTP *>(p->data);

    if (nhttp->m_HTTP.m_Data == NULL) {
        nhttp->m_HTTP.m_Data = buffer_new(2048);
    }

    if (static_cast<uint64_t>(nhttp->m_HTTP.m_Data->used + len) > HTTP_MAX_CL) {
        return -1;
    }

    if (len != 0) {
        buffer_append_data(nhttp->m_HTTP.m_Data,
                           reinterpret_cast<const unsigned char *>(buf), len);
    }

    nhttp->onData(nhttp->m_HTTP.m_Data->used - len, len);

    return 0;
}
// }}}

// {{{ HTTP callbacks (connect/disconnect/read)
static void
nidium_http_connected(ape_socket *s, ape_global *ape, void *socket_arg)
{
    HTTP *nhttp = static_cast<HTTP *>(s->ctx);

    if (nhttp == NULL) return;

    http_parser_init(&nhttp->m_HTTP.parser, HTTP_RESPONSE);
    nhttp->m_HTTP.parser.data = nhttp;
    nhttp->m_HTTP.parser_rdy  = true;

    HTTPRequest *request = nhttp->getRequest();
    buffer *headers      = request->getHeadersData();

    if (request->getData() != NULL
        && (request->m_Method == HTTPRequest::kHTTPMethod_Post
            || request->m_Method == HTTPRequest::kHTTPMethod_Put)) {

        PACK_TCP(s->s.fd);
        APE_socket_write(s, headers->data, headers->used, APE_DATA_COPY);

        /* APE_DATA_OWN? Warum? It's good as long as
        the lifetime of the data is tied to the socket lifetime */
        // TODO: new style cast
        APE_socket_write(s, (unsigned char *)(request->getData()),
                         nhttp->getRequest()->getDataLength(), APE_DATA_OWN);
        FLUSH_TCP(s->s.fd);
    } else {
        APE_socket_write(s, headers->data, headers->used, APE_DATA_COPY);
    }

    buffer_destroy(headers);
}

static void
nidium_http_disconnect(ape_socket *s, ape_global *ape, void *socket_arg)
{
    HTTP *nhttp = static_cast<HTTP *>(s->ctx);

    if (nhttp == NULL
        || (nhttp->m_CurrentSock != NULL && s != nhttp->m_CurrentSock)) {
        return;
    }

    nhttp->clearTimeout();

    if (!nhttp->isParsing() && nhttp->m_HTTP.parser_rdy) {
        http_parser_execute(&nhttp->m_HTTP.parser, &settings, NULL, 0);
    }

    nhttp->m_CurrentSock = NULL;

    if (!nhttp->m_HTTP.m_Ended) {
        nhttp->setPendingError(HTTP::kHTTPError_Disconnected);
    }

    nhttp->clearState();

    nhttp->canDoRequest(true);

    s->ctx = NULL;
}

static void nidium_http_read(ape_socket *s,
                             const uint8_t *data,
                             size_t len,
                             ape_global *ape,
                             void *socket_arg)
{
    size_t nparsed;
    HTTP *nhttp = static_cast<HTTP *>(s->ctx);

    if (nhttp == NULL || nhttp->m_HTTP.m_Ended) {
        return;
    }

    nhttp->parsing(true);
    nparsed = http_parser_execute(&nhttp->m_HTTP.parser, &settings,
                                  reinterpret_cast<const char *>(data), len);
    nhttp->parsing(false);

    if (nparsed != len && !nhttp->m_HTTP.m_Ended) {
        ndm_logf(NDM_LOG_ERROR, "HTTP",
                 "(socket %p) Parser returned %ld with error %s", s,
                 static_cast<unsigned long>(nparsed),
                 http_errno_description(HTTP_PARSER_ERRNO(&nhttp->m_HTTP.parser)));

        nhttp->setPendingError(HTTP::KHTTPError_Response);

        APE_socket_shutdown_now(s);
    }
}
// }}}

// {{{ HTTP Implementation
HTTP::HTTP(ape_global *n)
    : m_Ptr(NULL), m_Net(n), m_CurrentSock(NULL), m_Err(0),
      m_Timeout(HTTP_DEFAULT_TIMEOUT), m_TimeoutTimer(0), m_Delegate(NULL),
      m_FileSize(0), m_isParsing(false), m_Request(NULL), m_CanDoRequest(true),
      m_PendingError(kHTTPError_NoError), m_MaxRedirect(8), m_FollowLocation(true)
{
    memset(&m_HTTP, 0, sizeof(m_HTTP));
    memset(&m_Redirect, 0, sizeof(m_Redirect));

    m_HTTP.m_Headers.prevstate = HTTP::PSTATE_NOTHING;
    nidium_http_data_type      = DATA_NULL;
}

void HTTP::reportPendingError()
{
    if (m_Delegate && m_PendingError != kHTTPError_NoError) {
        m_Delegate->onError(m_PendingError);
    }

    m_PendingError = kHTTPError_NoError;
}

void HTTP::setPrivate(void *ptr)
{
    m_Ptr = ptr;
}

void *HTTP::getPrivate()
{
    return m_Ptr;
}

void HTTP::onData(size_t offset, size_t len)
{
    m_Delegate->onProgress(offset, len, &m_HTTP, this->nidium_http_data_type);
}

void HTTP::headerEnded()
{
#define REQUEST_HEADER(header) \
    ape_array_lookup(m_HTTP.m_Headers.list, CONST_STR_LEN(header "\0"))

    m_Redirect.enabled = false;

    if (m_HTTP.m_Headers.list != NULL) {
        buffer *content_type, *content_range, *location;

        if ((content_type = REQUEST_HEADER("Content-Type")) != NULL
            && content_type->used > 3) {
            int i;

            for (i = 0; nidium_mime[i].m_Str != NULL; i++) {
                if (strncasecmp(
                        nidium_mime[i].m_Str,
                        reinterpret_cast<const char *>(content_type->data),
                        strlen(nidium_mime[i].m_Str))
                    == 0) {
                    nidium_http_data_type = nidium_mime[i].m_DataType;
                    break;
                }
            }
        }

        if (m_HTTP.parser.status_code == 206
            && (content_range = REQUEST_HEADER("Content-Range")) != NULL) {
            char *ptr = static_cast<char *>(
                memchr(content_range->data, '/', content_range->used));

            if (ptr != NULL) {
                m_FileSize = atoll(&ptr[1]);
                if (m_FileSize >= LLONG_MAX) {
                    m_FileSize = 0;
                }
            }
        } else if (m_FollowLocation && (m_HTTP.parser.status_code == 301
                                        || m_HTTP.parser.status_code == 302)
                   && (location = REQUEST_HEADER("Location")) != NULL) {

            m_FileSize = 0;

            m_Redirect.enabled = true;
            m_Redirect.to      = reinterpret_cast<const char *>(location->data);
            m_Redirect.count++;

            if (m_Redirect.count > m_MaxRedirect) {
                setPendingError(kHTTPError_RedirectMax);
            }

            return;
        } else {
            m_FileSize = m_HTTP.m_ContentLength;
        }
    }
    /*
    switch (m_HTTP.parser.status_code/100) {
        case 1:
        case 2:
        case 3:
            m_Delegate->onHeader();
            break;
        case 4:
        case 5:
        default:
            m_Delegate->onError(kHTTPError_HTTPCode);
            break;
    }
*/

    m_Delegate->onHeader();

#undef REQUEST_HEADER
}

/*
    stopRequest can be used to shutdown slow or maliscious connections
    since the shutdown is not queued
*/
void HTTP::stopRequest(bool timeout)
{
    this->clearTimeout();

    if (!m_HTTP.m_Ended) {
        m_HTTP.m_Ended = 1;

        /*
            Make sur the connection is closed right now
        */
        this->close(true);

        if (timeout) {
            this->setPendingError(kHTTPError_Timeout);
        }

        this->clearState();

        if (!m_isParsing && m_HTTP.parser_rdy) {
            http_parser_execute(&m_HTTP.parser, &settings, NULL, 0);
        }

        m_CanDoRequest = true;
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
        this->request(m_Request, m_Delegate, true);
        return;
    }

    if (!m_HTTP.m_Ended) {
        m_HTTP.m_Ended = 1;
        bool doclose   = !this->isKeepAlive();

        if (!hasPendingError()) {
            m_Delegate->onRequest(&m_HTTP, nidium_http_data_type);
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

    ape_array_destroy(m_HTTP.m_Headers.list);
    buffer_destroy(m_HTTP.m_Data);
    m_HTTP.m_Data = NULL;

    memset(&m_HTTP.m_Headers, 0, sizeof(m_HTTP.m_Headers));
    m_HTTP.m_Headers.prevstate = HTTP::PSTATE_NOTHING;
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
    static_cast<HTTP *>(arg)->stopRequest(true);

    return 0;
}

void HTTP::clearTimeout()
{
    if (this->m_TimeoutTimer) {
        APE_timer_clearbyid(m_Net, this->m_TimeoutTimer, 1);
        this->m_TimeoutTimer = 0;
    }
}

bool HTTP::createConnection()
{
    if (!m_Request) {
        return false;
    }

    ape_socket *socket;

    if ((socket = APE_socket_new(m_Request->isSSL() ? APE_SOCKET_PT_SSL
                                                    : APE_SOCKET_PT_TCP,
                                 0, m_Net))
        == NULL) {

        ndm_log(NDM_LOG_ERROR, "HTTP", "Can't load socket (new)");
        if (m_Delegate) {
            this->setPendingError(KHTTPError_Socket);
        }
        return false;
    }

    if (APE_socket_connect(socket, m_Request->getPort(), m_Request->getHost(),
                           0)
        == -1) {
        ndm_log(NDM_LOG_ERROR, "HTTP", "Can't connect (0)");
        if (m_Delegate) {
            this->setPendingError(KHTTPError_Socket);
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
                   HTTPDelegate *delegate,
                   bool forceNewConnection)
{
    if (!canDoRequest()) {
        this->clearState();
        return false;
    }

    /* A fresh request is given */
    if (m_Request && req != m_Request) {
        delete m_Request;
    }

    m_Request      = req;
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

    m_Path = req->isSSL()
                 ? std::string("https://")
                 : std::string("http://") + std::string(req->getHost());

    if (req->getPort() != 80 && req->getPort() != 443) {
        m_Path += std::string(":") + std::to_string(req->getPort());
    }

    m_Path += req->getPath();

    m_Delegate     = delegate;
    m_HTTP.m_Ended = 0;

    delegate->m_HTTPRef = this;

    if (m_Timeout) {
        ape_timer_t *ctimer;
        ctimer = APE_timer_create(m_Net, m_Timeout, Nidium_HTTP_handle_timeout,
                                  this);

        APE_timer_unprotect(ctimer);
        m_TimeoutTimer = APE_timer_getid(ctimer);
    }

    m_CanDoRequest = false;

    if (reusesock) {
        nidium_http_connected(m_CurrentSock, m_Net, NULL);
    }

    return true;
}

const char *HTTP::getHeader(const char *key)
{
    buffer *ret
        = ape_array_lookup_cstr(m_HTTP.m_Headers.list, key, strlen(key));
    return ret ? reinterpret_cast<const char *>(ret->data) : NULL;
}

int HTTP::ParseURI(char *url,
                   size_t url_len,
                   char *host,
                   u_short *port,
                   char *file,
                   const char *prefix,
                   u_short default_port)
{
    char *p;
    const char *p2;
    int len;

    len = strlen(prefix);
    if (strncasecmp(url, prefix, len)) {
        return -1;
    }

    url += len;

    memcpy(host, url, (url_len - len));

    p = strchr(host, '/');
    if (p != NULL) {
        *p = '\0';
        p2 = p + 1;
    } else {
        p2 = NULL;
    }
    if (file != NULL) {
        /* Generate request file */
        if (p2 == NULL) p2 = "";
        sprintf(file, "/%s", p2);
    }

    p = strchr(host, ':');

    if (p != NULL) {
        *p    = '\0';
        *port = atoi(p + 1);

        if (*port == 0) return -1;
    } else
        *port = default_port;

    return 0;
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

    m_Delegate     = NULL;
    m_PendingError = kHTTPError_NoError;

    this->clearState();
}
// }}}

// {{{ HTTPRequest Implementation
HTTPRequest::HTTPRequest(const char *url)
    : m_Method(kHTTPMethod_Get), m_Host(NULL), m_Path(NULL), m_Data(NULL),
      m_DataLen(0), m_Datafree(free), m_Headers(ape_array_new(8)),
      m_isSSL(false)
{
    this->resetURL(url);
    this->setDefaultHeaders();
}

bool HTTPRequest::resetURL(const char *url)
{
    m_isSSL = false;
    if (m_Host) free(m_Host);
    if (m_Path) free(m_Path);

    size_t url_len = strlen(url);
    char *durl     = static_cast<char *>(malloc(sizeof(char) * (url_len + 1)));

    memcpy(durl, url, url_len + 1);

    m_Host = static_cast<char *>(malloc(url_len + 1));
    m_Path = static_cast<char *>(malloc(url_len + 1));

    memset(m_Host, 0, url_len + 1);
    memset(m_Path, 0, url_len + 1);

    u_short default_port = 80;

    const char *prefix = NULL;
    if (strncasecmp(url, CONST_STR_LEN("https://")) == 0) {
        prefix       = "https://";
        m_isSSL      = true;
        default_port = 443;
    } else if (strncasecmp(url, CONST_STR_LEN("http://")) == 0) {
        prefix = "http://";
    } else {
        /* No prefix provided. Assuming 'default url' => no SSL, port 80 */
        prefix = "";
    }

    if (HTTP::ParseURI(durl, url_len, m_Host, &m_Port, m_Path, prefix,
                       default_port)
        == -1) {
        memset(m_Host, 0, url_len + 1);
        memset(m_Path, 0, url_len + 1);
        m_Port = 0;

        free(durl);
        return false;
    }

    free(durl);

    return true;
}

void HTTPRequest::setDefaultHeaders()
{
    this->setHeader("User-Agent",
                    "Mozilla/5.0 (Unknown arch) nidium/" NIDIUM_VERSION_STR
                    " (nidium, like Gecko) nidium/" NIDIUM_VERSION_STR);
    this->setHeader("Accept-Charset", "UTF-8");
    this->setHeader("Accept",
                    "text/html,application/xhtml+xml,application/"
                    "xml;q=0.9,image/webp,*/*;q=0.8");
}

buffer *HTTPRequest::getHeadersData() const
{
    buffer *ret = buffer_new(1024);

    switch (m_Method) {
        case kHTTPMethod_Get:
            buffer_append_string_n(ret, CONST_STR_LEN("GET "));
            break;
        case kHTTPMethod_Head:
            buffer_append_string_n(ret, CONST_STR_LEN("HEAD "));
            break;
        case kHTTPMethod_Post:
            buffer_append_string_n(ret, CONST_STR_LEN("POST "));
            break;
        case kHTTPMethod_Put:
            buffer_append_string_n(ret, CONST_STR_LEN("PUT "));
            break;
        case kHTTPMethod_Delete:
            buffer_append_string_n(ret, CONST_STR_LEN("DELETE "));
            break;
    }

    buffer_append_string(ret, m_Path);
    buffer_append_string_n(ret, CONST_STR_LEN(" HTTP/1.1\r\n"));
    buffer_append_string_n(ret, CONST_STR_LEN("Host: "));
    buffer_append_string(ret, m_Host);

    if (this->getPort() != 80 && this->getPort() != 443) {
        char portstr[8];
        sprintf(portstr, ":%hu", this->getPort());
        buffer_append_string(ret, portstr);
    }
    buffer_append_string_n(ret, CONST_STR_LEN("\r\n"));

    buffer *k, *v;
    APE_A_FOREACH(this->getHeaders(), k, v)
    {
        buffer_append_string_n(ret, reinterpret_cast<char *>(k->data), k->used);
        buffer_append_string_n(ret, ": ", 2);
        buffer_append_string_n(ret, reinterpret_cast<char *>(v->data), v->used);
        buffer_append_string_n(ret, CONST_STR_LEN("\r\n"));
    }

    buffer_append_string_n(ret, CONST_STR_LEN("\r\n"));

    return ret;
}
// }}}

} // namespace Net
} // namespace Nidium
