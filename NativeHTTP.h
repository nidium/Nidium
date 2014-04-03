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

#ifndef nativehttp_h__
#define nativehttp_h__

#include <native_netlib.h>
//#include "ape_http_parser.h"
#include <ape_array.h>
#include <http_parser.h>

#define HTTP_MAX_CL 1024L*1024L*1024L*2L
#define HTTP_DEFAULT_TIMEOUT 15000

#include "NativeIStreamer.h"
#include "NativeMessages.h"

class NativeHTTPDelegate;

class NativeHTTPRequest
{
    public:
        enum {
            NATIVE_HTTP_GET,
            NATIVE_HTTP_POST,
            NATIVE_HTTP_HEAD
        } method;

        explicit NativeHTTPRequest(const char *url);

        ~NativeHTTPRequest(){
            ape_array_destroy(this->headers);
            if (data != NULL && datafree != NULL) {
                datafree(data);
            }
        };

        void setHeader(const char *key, const char *val)
        {
            ape_array_add(this->headers, key, val);
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

class NativeHTTP : public NativeIStreamer, public NativeMessages
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
        ERROR_SOCKET,
        ERROR_HTTPCODE
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
    int m_TimeoutTimer;

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

    NativeHTTPRequest *getRequest() const {
        return req;
    }

    NativeHTTP(NativeHTTPRequest *req, ape_global *n);
    int request(NativeHTTPDelegate *delegate);
    ~NativeHTTP();
    private:
        NativeHTTPRequest *req;
        uint64_t m_FileSize;
        bool m_isParsing; // http_parser_execute is working
};

class NativeHTTPDelegate
{
  public:
    virtual void onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType)=0;
    virtual void onProgress(size_t offset, size_t len, NativeHTTP::HTTPData *h,
        NativeHTTP::DataType)=0;
    virtual void onError(NativeHTTP::HTTPError err)=0;
    virtual void onHeader()=0;
    NativeHTTP *httpref;
};


#endif
