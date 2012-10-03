#include "NativeHTTP.h"
#include "ape_http_parser.h"
#include "stdio.h"

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
    {"image/jpeg",              NativeHTTP::DATA_IMAGE},
    {"image/png",               NativeHTTP::DATA_IMAGE},
    {"application/json",        NativeHTTP::DATA_JSON},
    {"text/html",               NativeHTTP::DATA_STRING}, /* TODO: use dom.js */
    {"application/octet-stream",NativeHTTP::DATA_BINARY},
    {NULL,                      NativeHTTP::DATA_END}
};

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

static void native_http_connected(ape_socket *s, ape_global *ape)
{
    NativeHTTP *nhttp = (NativeHTTP *)s->ctx;

    if (nhttp == NULL) return;

    nhttp->http.headers.list = ape_array_new(16);
    nhttp->http.headers.tkey = buffer_new(16);
    nhttp->http.headers.tval = buffer_new(64);
    nhttp->http.data = NULL;
    nhttp->http.ended = 0;

    nhttp->http.parser.ctx[0] = nhttp;

    SOCKET_WRITE_STATIC("GET ");
    SOCKET_WRITE_OWN(nhttp->path);
    SOCKET_WRITE_STATIC(" HTTP/1.0\nHost: ");
    SOCKET_WRITE_OWN(nhttp->host);
    SOCKET_WRITE_STATIC("\nUser-Agent: Nativestudio/1.0\nConnection: close\n\n");
}

static void native_http_disconnect(ape_socket *s, ape_global *ape)
{
    NativeHTTP *nhttp = (NativeHTTP *)s->ctx;

    if (nhttp == NULL) return;

    nhttp->requestEnded();
}

static void native_http_read(ape_socket *s, ape_global *ape)
{
    unsigned int i;
    NativeHTTP *nhttp = (NativeHTTP *)s->ctx;

    if (nhttp == NULL) return;

    for (i = 0; i < s->data_in.used; i++) {

        if (!parse_http_char(&nhttp->http.parser,
            s->data_in.data[i])) {
            printf("Failed at %d %c\n", i, s->data_in.data[i]);
            printf("next %s\n", &s->data_in.data[i]);
            // TODO : graceful shutdown
            shutdown(s->s.fd, 2);
            break;
        }

    //printf("%c", socket_client->data_in.data[i]);
    }
}

int NativeHTTP::ParseURI(char *url, char *host,
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

    /* We might overrun */
    strncpy(host, url, 1023);

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

NativeHTTP::NativeHTTP(const char *url, ape_global *n) :
    ptr(NULL), net(n), host(NULL), path(NULL), port(0),
    err(0), delegate(NULL)
{
    size_t url_len = strlen(url);
    char *durl = strdup(url);

    host = (char *)malloc(sizeof(char) * url_len);
    path = (char *)malloc(sizeof(char) * url_len);

    if (ParseURI(durl, host, &port, path) == -1) {
        err = 1;
        memset(host, 0, url_len);
        memset(path, 0, url_len);
        port = 0;
    }

    HTTP_PARSER_RESET(&http.parser);

    http.headers.list    = NULL;
    http.parser.callback = native_http_callback;

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
    if (!http.ended) {
        buffer *content_type;
        DataType type = DATA_NULL;
        http.ended = 1;

        if ((content_type = REQUEST_HEADER("Content-Type")) != NULL &&
            content_type->used > 3) {
            int i;
            char *ctype = (char *)&content_type->data[1];

            for (i = 0; native_mime[i].str != NULL; i++) {
                if (strncasecmp(native_mime[i].str, ctype,
                    strlen(native_mime[i].str)) == 0) {
                    type = native_mime[i].data_type;
                    break;
                }
            }
        }

        delegate->onRequest(&http, type);

        buffer_destroy(http.headers.tkey);
        buffer_destroy(http.headers.tval);
        buffer_destroy(http.data);
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

    return 1;
}

NativeHTTP::~NativeHTTP()
{
    free(host);
    free(path);
}
