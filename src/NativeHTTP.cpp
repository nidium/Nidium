#include "NativeHTTP.h"
//#include "ape_http_parser.h"
#include <http_parser.h>

#include <stdio.h>
#include <string.h>

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
    printf("Message begin\n");
    return 0;
}

static int headers_complete_cb(http_parser *p)
{
    NativeHTTP *nhttp = (NativeHTTP *)p->data;

    if (p->content_length > HTTP_MAX_CL) {
        return -1;
    }

    if (p->content_length) nhttp->http.data = buffer_new(p->content_length);

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
    return 0;
}

#if 0
static int native_http_callback(void **ctx, callback_type type,
        int value, uint32_t step)
{
    NativeHTTP *nhttp = (NativeHTTP *)ctx[0];
    
    switch(type) {
        case HTTP_HEADER_KEYC:
            buffer_append_char(nhttp->http.headers.tkey, (unsigned char)value);
            break;
        case HTTP_HEADER_VALC:
            buffer_append_char(nhttp->http.headers.tval, (unsigned char)value);
            break;
        case HTTP_CL_VAL:
            nhttp->http.data = buffer_new(value);
            break;
        case HTTP_HEADER_VAL:
            buffer_append_char(nhttp->http.headers.tkey, '\0');
            buffer_append_char(nhttp->http.headers.tval, '\0');
            ape_array_add_b(nhttp->http.headers.list,
                    nhttp->http.headers.tkey, nhttp->http.headers.tval);
            nhttp->http.headers.tkey = buffer_new(16);
            nhttp->http.headers.tval = buffer_new(64);
            
            break;
        case HTTP_BODY_CHAR:

            if (nhttp->http.data == NULL) {
                nhttp->http.data = buffer_new(2048);
            }

            buffer_append_char(nhttp->http.data, (unsigned char)value);
            break;
        case HTTP_READY:
            nhttp->requestEnded();
            break;
        default:break;
    }

    return 1;
}
#endif

static void native_http_connected(ape_socket *s, ape_global *ape)
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

    SOCKET_WRITE_STATIC("GET ");
    SOCKET_WRITE_OWN(nhttp->path);
    SOCKET_WRITE_STATIC(" HTTP/1.1\nHost: ");
    SOCKET_WRITE_OWN(nhttp->host);
    SOCKET_WRITE_STATIC("\nUser-Agent: Nativestudio/1.0\nConnection: close\n\n");
}

static void native_http_disconnect(ape_socket *s, ape_global *ape)
{
    NativeHTTP *nhttp = (NativeHTTP *)s->ctx;

    if (nhttp == NULL) return;

    http_parser_execute(&nhttp->http.parser, &settings,
        NULL, 0);

    //nhttp->requestEnded();
    nhttp->currentSock = NULL;
}

static void native_http_read(ape_socket *s, ape_global *ape)
{
    unsigned int i;
    size_t nparsed;
    NativeHTTP *nhttp = (NativeHTTP *)s->ctx;

    if (nhttp == NULL) return;

#if 0
    for (i = 0; i < s->data_in.used; i++) {
        //printf("parse...\n");
        if (http_parser_execute(&nhttp->http.parser, &settings,
            (const char *)&s->data_in.data[i], 1) != 1) {
            printf("Failed at %d %c\n", i, s->data_in.data[i]);
            printf("next %s\n", &s->data_in.data[i]);
            // TODO : graceful shutdown
            shutdown(s->s.fd, 2);
            break;
        }

    //printf("%c", socket_client->data_in.data[i]);
    }
#endif
    #if 1
    nparsed = http_parser_execute(&nhttp->http.parser, &settings,
        (const char *)s->data_in.data, (size_t)s->data_in.used);

    if (nparsed != s->data_in.used) {
        printf("Failed to parse http response\n");
        shutdown(s->s.fd, 2);
    }
#endif
}

NativeHTTP::NativeHTTP(const char *url, ape_global *n) :
    ptr(NULL), net(n), currentSock(NULL),
    host(NULL), path(NULL), port(0),
    err(0), delegate(NULL)
{
    size_t url_len = strlen(url);
    char *durl = (char *)malloc(sizeof(char) * (url_len+1));

    memcpy(durl, url, url_len+1);

    host = (char *)malloc(url_len+1);
    path = (char *)malloc(url_len+1);
    memset(host, 0, url_len+1);
    memset(path, 0, url_len+1);

    if (ParseURI(durl, url_len, host, &port, path) == -1) {
        err = 1;
        memset(host, 0, url_len+1);
        memset(path, 0, url_len+1);
        port = 0;
    }

    http.data = NULL;
    http.headers.tval = NULL;
    http.headers.tkey = NULL;
    http.headers.list = NULL;
    http.ended = 1;

    free(durl);
}

void NativeHTTP::setPrivate(void *ptr)
{
    this->ptr = ptr;
}

void *NativeHTTP::getPrivate()
{
    return this->ptr;
}

void NativeHTTP::requestEnded()
{
#define REQUEST_HEADER(header) ape_array_lookup(http.headers.list, \
    CONST_STR_LEN(header "\0"))
    if (!http.ended && http.headers.list != NULL) {
        buffer *content_type;
        DataType type = DATA_NULL;
        http.ended = 1;

        if ((content_type = REQUEST_HEADER("Content-Type")) != NULL &&
            content_type->used > 3) {
            int i;

            for (i = 0; native_mime[i].str != NULL; i++) {
                if (strncasecmp(native_mime[i].str, (const char *)content_type->data,
                    strlen(native_mime[i].str)) == 0) {
                    type = native_mime[i].data_type;
                    break;
                }
            }
        }

        delegate->onRequest(&http, type);

        ape_array_destroy(http.headers.list);

        http.data = NULL;
        http.headers.tval = NULL;
        http.headers.tkey = NULL;
        http.headers.list = NULL;

        APE_socket_shutdown(currentSock);
    } 
}

int NativeHTTP::request(NativeHTTPDelegate *delegate)
{
    ape_socket *socket;

    if ((socket = APE_socket_new(APE_SOCKET_PT_TCP, 0, net)) == NULL) {
        printf("[Socket] Cant load socket (new)\n");
        return 0;
    }

    if (APE_socket_connect(socket, port, host) == -1) {
        printf("[Socket] Cant connect (0)\n");
        return 0;
    }

    socket->callbacks.on_connected = native_http_connected;
    socket->callbacks.on_read = native_http_read;
    socket->callbacks.on_disconnect = native_http_disconnect;

    socket->ctx = this;
    this->delegate = delegate;
    this->currentSock = socket;
    delegate->httpref = this;

    return 1;
}

NativeHTTP::~NativeHTTP()
{
    free(host);
    free(path);

    if (currentSock != NULL) {
        currentSock->ctx = NULL;

        APE_socket_shutdown_now(currentSock);
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
