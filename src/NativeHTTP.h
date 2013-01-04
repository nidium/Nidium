#ifndef nativehttp_h__
#define nativehttp_h__

#include <native_netlib.h>
#include "ape_http_parser.h"
#include "ape_array.h"


class NativeHTTPDelegate;

class NativeHTTP
{
  private:
    void *ptr;
  public:
    enum DataType {
        DATA_STRING = 1,
        DATA_BINARY,
        DATA_IMAGE,
        DATA_JSON,
        DATA_NULL,
        DATA_END
    } native_http_data_type;

    ape_global *net;
    ape_socket *currentSock;
    char *host;
    char *path;
    u_short port;
    int err;
    NativeHTTPDelegate *delegate;

    struct HTTPData {
        http_parser parser;
        struct {
            ape_array_t *list;
            buffer *tkey;
            buffer *tval;
        } headers;
        buffer *data;
        int ended;
    } http;

    static int ParseURI(char *url, size_t url_len, char *host,
    u_short *port, char *file);
    void requestEnded();
    void setPrivate(void *ptr);
    void *getPrivate();

    NativeHTTP(const char *url, ape_global *n);
    int request(NativeHTTPDelegate *delegate);
    ~NativeHTTP();
};

class NativeHTTPDelegate
{
  public:
    virtual void onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType)=0;
    NativeHTTP *httpref;
};


#endif
