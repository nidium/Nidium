/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativehttp_h__
#define nativehttp_h__

#include <string>

#include <http_parser.h>

#include <native_netlib.h>
#include <ape_array.h>

#define HTTP_MAX_CL (1024ULL*1024ULL*1024ULL*2ULL)
#define HTTP_DEFAULT_TIMEOUT 15000

#include "Core/NativeMessages.h"
#include "IO/NativeIStreamer.h"

namespace Native {
namespace Core {

class HTTPRequest
{
    public:
        enum {
            NATIVE_HTTP_GET,
            NATIVE_HTTP_POST,
            NATIVE_HTTP_HEAD,
            NATIVE_HTTP_PUT,
            NATIVE_HTTP_DELETE
        } method;

        explicit HTTPRequest(const char *url);

        ~HTTPRequest() {
            ape_array_destroy(this->headers);
            if (data != NULL && datafree != NULL) {
                datafree(data);
            }
            free(host);
            free(path);
        };

        void recycle() {
            ape_array_destroy(this->headers);
            this->headers = ape_array_new(8);
            if (data != NULL && datafree != NULL) {
                datafree(data);
            }
            data = NULL;
            datalen = 0;
            method = NATIVE_HTTP_GET;

            this->setDefaultHeaders();
        }

        void setHeader(const char *key, const char *val)
        {
            ape_array_add_camelkey_n(this->headers,
                key, strlen(key), val, strlen(val));
        }

        const char *getHeader(const char *key)
        {
            buffer *ret = ape_array_lookup(headers, key, strlen(key));
            return ret ? (const char *)ret->data : NULL;
        }

        buffer *getHeadersData() const;

        const bool isValid() const {
            return (this->host[0] != 0);
        }

        const char *getHost() const {
            return this->host;
        }

        const char *getPath() const {
            return this->path;
        }

        void setPath(const char *lpath) {
            if (this->path && lpath != this->path) {
                free(this->path);
            }

            this->path = strdup(lpath);
        }

        u_short getPort() const {
            return this->port;
        }

        ape_array_t *getHeaders() const {
            return this->headers;
        }

        void setData(char *data, size_t len) {
            this->data = data;
            this->datalen = len;
        }

        const char *getData() const {
            return this->data;
        }

        size_t getDataLength() const {
            return this->datalen;
        }

        void setDataReleaser(void (*datafree)(void *))
        {
            this->datafree = datafree;
        }

        bool isSSL() const {
            return m_isSSL;
        }

        bool resetURL(const char *url);
    private:

        void setDefaultHeaders();

        char *host;
        char *path;
        char *data;
        size_t datalen;
        void (*datafree)(void *);
        u_short port;

        ape_array_t *headers;

        bool m_isSSL;
};

class HTTPDelegate;

class HTTP : public NativeIStreamer, public NativeMessages
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
        ERROR_NOERR,
        ERROR_TIMEOUT,
        ERROR_RESPONSE,
        ERROR_DISCONNECTED,
        ERROR_SOCKET,
        ERROR_HTTPCODE,
        ERROR_REDIRECTMAX
    };

    enum PrevState {
        PSTATE_NOTHING,
        PSTATE_FIELD,
        PSTATE_VALUE
    };

    ape_global *net;
    ape_socket *m_CurrentSock;

    int err;
    uint32_t m_Timeout;
    uint64_t m_TimeoutTimer;

    HTTPDelegate *delegate;

    struct HTTPData {
        http_parser parser;
        bool parser_rdy;
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
    u_short *port, char *file, const char *prefix = "http://", u_short default_port = 80);
    void requestEnded();
    void headerEnded();
    void stopRequest(bool timeout=false);
    void clearTimeout();
    void onData(size_t offset, size_t len);
    void setPrivate(void *ptr);
    void *getPrivate();

    void parsing(bool val) {
        m_isParsing = val;
    }

    bool isParsing() const {
        return m_isParsing;
    }

    void resetData() {
        if (http.data == NULL) {
            return;
        }
        http.data->used = 0;
    }

    uint64_t getFileSize() const {
        return m_FileSize;
    }

    uint16_t getStatusCode() const {
        return http.parser.status_code;
    }

    HTTPRequest *getRequest() const {
        return m_Request;
    }

    const char *getHeader(const char *key);

    void close(bool now = false) {
        if (m_CurrentSock) {
            if (now) {
                APE_socket_shutdown_now(m_CurrentSock);
            } else {
                APE_socket_shutdown(m_CurrentSock);
            }
        }
    }

    void clearState();
    bool request(HTTPRequest *req, HTTPDelegate *delegate,
        bool forceNewConnection = false);
    bool isKeepAlive();

    bool canDoRequest() const {
        return m_CanDoRequest && !hasPendingError();
    }

    bool canDoRequest(bool val) {
        m_CanDoRequest = val;

        return canDoRequest();
    }

    void setPendingError(HTTP::HTTPError err) {
        m_PendingError = err;
    }

    void clearPendingError() {
        m_PendingError = ERROR_NOERR;
    }

    bool hasPendingError() const {
        return (m_PendingError != ERROR_NOERR);
    }

    void setMaxRedirect(int max) {
        m_MaxRedirect = max;
    }

    void setFollowLocation(bool toggle) {
        m_FollowLocation = toggle;
    }

    const char *getPath() const {
        return m_Path.c_str();
    }

    HTTP(ape_global *n);
    ~HTTP();
private:
    void reportPendingError();

    bool createConnection();

    uint64_t m_FileSize;
    bool m_isParsing; // http_parser_execute is working
    HTTPRequest *m_Request;
    bool m_CanDoRequest;
    HTTPError m_PendingError;

    int m_MaxRedirect;
    bool m_FollowLocation;

    std::string m_Path;

    struct {
        const char *to;
        bool enabled;
        int count;
    } m_Redirect;
};

class HTTPDelegate
{
  public:
    virtual void onRequest(HTTP::HTTPData *h, HTTP::DataType)=0;
    virtual void onProgress(size_t offset, size_t len, HTTP::HTTPData *h,
        HTTP::DataType)=0;
    virtual void onError(HTTP::HTTPError err)=0;
    virtual void onHeader()=0;
    HTTP *httpref;
};

}  /* namespace Core */
}  /* namespace Native */

#endif

