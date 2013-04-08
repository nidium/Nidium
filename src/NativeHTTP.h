#ifndef nativehttp_h__
#define nativehttp_h__

#include <native_netlib.h>
//#include "ape_http_parser.h"
#include "ape_array.h"
#include <http_parser.h>


#define HTTP_MAX_CL 1024L*1024L*20L
#define HTTP_DEFAULT_TIMEOUT 20000

#include "NativeIStreamer.h"

class NativeHTTPDelegate;

class NativeHTTPRequest
{
    public:
        enum {
            NATIVE_HTTP_GET,
            NATIVE_HTTP_POST
        } method;

        NativeHTTPRequest(const char *url);

        ~NativeHTTPRequest(){
            ape_array_destroy(this->headers);
        };

        void setHeader(const char *key, const char *val)
        {
            ape_array_add(this->headers, key, val);
        }

        buffer *getHeadersData() const;

        const char *getHost() const {
            return this->host;
        }

        const char *getPath() const {
            return this->path;
        }

        u_short getPort() const {
            return this->port;
        }

        ape_array_t *getHeaders() const {
            return this->headers;
        }

    private:
        char *host;
        char *path;
        char *data;
        u_short port;

        ape_array_t *headers;
};

class NativeHTTP : public NativeIStreamer
{
  private:
    void *ptr;
  public:
    enum DataType {
        DATA_STRING = 1,
        DATA_BINARY,
        DATA_IMAGE,
        DATA_AUDIO,
        DATA_JSON,
        DATA_NULL,
        DATA_END
    } native_http_data_type;

    enum HTTPError {
        ERROR_TIMEOUT,
        ERROR_RESPONSE,
        ERROR_DISCONNECTED,
        ERROR_SOCKET
    };

    enum PrevState {
        PSTATE_NOTHING,
        PSTATE_FIELD,
        PSTATE_VALUE
    };

    ape_global *net;
    ape_socket *currentSock;

    int err;
    uint32_t timeout;
    int timeoutTimer;

    NativeHTTPDelegate *delegate;

    struct HTTPData {
        http_parser parser;
        struct {
            ape_array_t *list;
            buffer *tkey;
            buffer *tval;
            PrevState prevstate;
        } headers;
        buffer *data;
        int ended;
        uint64_t contentlength;
    } http;

    static int ParseURI(char *url, size_t url_len, char *host,
    u_short *port, char *file);
    void requestEnded();
    void headerEnded();
    void stopRequest();
    void clearTimeout();
    void onData(size_t offset, size_t len);
    void setPrivate(void *ptr);
    void *getPrivate();

    NativeHTTPRequest *getRequest() const {
        return req;
    }

    NativeHTTP(NativeHTTPRequest *req, ape_global *n);
    int request(NativeHTTPDelegate *delegate);
    ~NativeHTTP();
    private:
        NativeHTTPRequest *req;
};

class NativeHTTPDelegate
{
  public:
    virtual void onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType)=0;
    virtual void onProgress(size_t offset, size_t len, NativeHTTP::HTTPData *h,
        NativeHTTP::DataType)=0;
    virtual void onError(NativeHTTP::HTTPError err)=0;
    NativeHTTP *httpref;
};


#endif
