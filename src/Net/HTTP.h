/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef net_http_h__
#define net_http_h__

#include <string>

#include <http_parser.h>

#include <ape_netlib.h>
#include <ape_array.h>

#define HTTP_MAX_CL (1024ULL * 1024ULL * 1024ULL * 2ULL)
#define HTTP_DEFAULT_TIMEOUT 15000

#include "Core/Messages.h"

namespace Nidium {
namespace Net {

// {{{ HTTPRequest

class HTTPRequest
{
public:
    enum HTTPMethod
    {
        kHTTPMethod_Get,
        kHTTPMethod_Post,
        kHTTPMethod_Head,
        kHTTPMethod_Put,
        kHTTPMethod_Delete
    } m_Method;

    explicit HTTPRequest(const char *url);

    ~HTTPRequest()
    {
        ape_array_destroy(m_Headers);
        if (m_Data != NULL && m_Datafree != NULL) {
            m_Datafree(m_Data);
        }
        free(m_Host);
        free(m_Path);
    };

    void recycle()
    {
        ape_array_destroy(m_Headers);
        m_Headers = ape_array_new(8);
        if (m_Data != NULL && m_Datafree != NULL) {
            m_Datafree(m_Data);
        }
        m_Data    = NULL;
        m_DataLen = 0;
        m_Method  = kHTTPMethod_Get;

        setDefaultHeaders();
    }

    void setHeader(const char *key, const char *val)
    {
        ape_array_add_camelkey_n(m_Headers, key, strlen(key), val, strlen(val));
    }

    const char *getHeader(const char *key)
    {
        buffer *ret = ape_array_lookup(m_Headers, key, strlen(key));
        return ret ? reinterpret_cast<const char *>(ret->data) : NULL;
    }

    buffer *getHeadersData() const;

    const bool isValid() const
    {
        return (m_Host[0] != 0);
    }

    const char *getHost() const
    {
        return m_Host;
    }

    const char *getPath() const
    {
        return m_Path;
    }

    void setPath(const char *lpath)
    {
        if (m_Path && lpath != m_Path) {
            free(m_Path);
        }

        m_Path = strdup(lpath);
    }

    u_short getPort() const
    {
        return m_Port;
    }

    ape_array_t *getHeaders() const
    {
        return m_Headers;
    }

    void setData(char *data, size_t len)
    {
        m_Data    = data;
        m_DataLen = len;
    }

    const char *getData() const
    {
        return m_Data;
    }

    size_t getDataLength() const
    {
        return m_DataLen;
    }

    void setDataReleaser(void (*datafree)(void *))
    {
        m_Datafree = datafree;
    }

    bool isSSL() const
    {
        return m_isSSL;
    }

    bool resetURL(const char *url);

private:
    void setDefaultHeaders();

    char *m_Host;
    char *m_Path;
    char *m_Data;
    size_t m_DataLen;
    void (*m_Datafree)(void *);
    u_short m_Port;

    ape_array_t *m_Headers;

    bool m_isSSL;
};
// }}}

// {{{ HTTP
class HTTPDelegate;
class HTTP : public Nidium::Core::Messages
{
private:
    void *m_Ptr;

public:
    enum DataType
    {
        DATA_STRING = 1,
        DATA_BINARY,
        DATA_IMAGE,
        DATA_AUDIO,
        DATA_JSON,
        DATA_NULL,
        DATA_END
    } nidium_http_data_type;

    enum HTTPError
    {
        kHTTPError_NoError,
        kHTTPError_Timeout,
        KHTTPError_Response,
        kHTTPError_Disconnected,
        KHTTPError_Socket,
        kHTTPError_HTTPCode,
        kHTTPError_RedirectMax,
        _kHTTPError_End_
    };

    const static char *HTTPErrorDescription[];

    enum PrevState
    {
        PSTATE_NOTHING,
        PSTATE_FIELD,
        PSTATE_VALUE
    };

    ape_global *m_Net;
    ape_socket *m_CurrentSock;

    int m_Err;
    uint32_t m_Timeout;
    uint64_t m_TimeoutTimer;

    HTTPDelegate *m_Delegate;

    struct HTTPData
    {
        http_parser parser;
        bool parser_rdy;
        struct
        {
            ape_array_t *list;
            buffer *tkey;
            buffer *tval;
            PrevState prevstate;
        } m_Headers;
        buffer *m_Data;
        int m_Ended;
        uint64_t m_ContentLength;
    } m_HTTP;

    static int ParseURI(char *url,
                        size_t url_len,
                        char *host,
                        u_short *port,
                        char *file,
                        const char *prefix   = "http://",
                        u_short default_port = 80);
    void requestEnded();
    void headerEnded();
    void stopRequest(bool timeout = false);
    void clearTimeout();
    void onData(size_t offset, size_t len);
    void setPrivate(void *ptr);
    void *getPrivate();

    void parsing(bool val)
    {
        m_isParsing = val;
    }

    bool isParsing() const
    {
        return m_isParsing;
    }

    void resetData()
    {
        if (m_HTTP.m_Data == NULL) {
            return;
        }
        m_HTTP.m_Data->used = 0;
    }

    uint64_t getFileSize() const
    {
        return m_FileSize;
    }

    uint16_t getStatusCode() const
    {
        return m_HTTP.parser.status_code;
    }

    HTTPRequest *getRequest() const
    {
        return m_Request;
    }

    const char *getHeader(const char *key);

    void close(bool now = false)
    {
        if (m_CurrentSock) {
            if (now) {
                APE_socket_shutdown_now(m_CurrentSock);
            } else {
                APE_socket_shutdown(m_CurrentSock);
            }
        }
    }

    void clearState();
    bool request(HTTPRequest *req,
                 HTTPDelegate *delegate,
                 bool forceNewConnection = false);
    bool isKeepAlive();

    bool canDoRequest() const
    {
        return m_CanDoRequest && !hasPendingError();
    }

    bool canDoRequest(bool val)
    {
        m_CanDoRequest = val;

        return canDoRequest();
    }

    void setPendingError(HTTP::HTTPError err)
    {
        m_PendingError = err;
    }

    void clearPendingError()
    {
        m_PendingError = ERROR_NOERR;
    }

    bool hasPendingError() const
    {
        return (m_PendingError != ERROR_NOERR);
    }

    void setMaxRedirect(int max)
    {
        m_MaxRedirect = max;
    }

    void setFollowLocation(bool toggle)
    {
        m_FollowLocation = toggle;
    }

    const char *getPath() const
    {
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

    struct
    {
        const char *to;
        bool enabled;
        int count;
    } m_Redirect;
};
// }}}

// {{{ HTTPDelegate
class HTTPDelegate
{
public:
    virtual void onRequest(HTTP::HTTPData *h, HTTP::DataType) = 0;
    virtual void
    onProgress(size_t offset, size_t len, HTTP::HTTPData *h, HTTP::DataType)
        = 0;
    virtual void onError(HTTP::HTTPError err) = 0;
    virtual void onHeader() = 0;
    HTTP *m_HTTPRef;
};
// }}}

} /* namespace Core */
} /* namespace Nidium */

#endif
