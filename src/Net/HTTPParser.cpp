/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Net/HTTPParser.h"

#include <string.h>

namespace Nidium {
namespace Net {

// {{{ Preambule
#ifndef ULLONG_MAX
#define ULLONG_MAX ((uint64_t)-1) /* 2^64-1 */
#endif

#define HTTP_MAX_CL (1024ULL * 1024ULL * 1024ULL * 2ULL)
// }}}

// {{{ Callbacks
static int message_begin_cb(http_parser *p);
static int headers_complete_cb(http_parser *p);
static int message_complete_cb(http_parser *p);
static int header_field_cb(http_parser *p, const char *buf, size_t len);
static int header_value_cb(http_parser *p, const char *buf, size_t len);
static int request_url_cb(http_parser *p, const char *buf, size_t len);
static int body_cb(http_parser *p, const char *buf, size_t len);

static http_parser_settings settings
    = {/*.on_message_begin    = */ message_begin_cb,
	   /*.on_url              = */ request_url_cb,
	   /*.on_status           = */ NULL,
       /*.on_header_field     = */ header_field_cb,
       /*.on_header_value     = */ header_value_cb,
	   /*.on_headers_complete = */ headers_complete_cb,
       /*.on_body             = */ body_cb,
       /*.on_message_complete = */ message_complete_cb 
};

static int message_begin_cb(http_parser *p)
{
    HTTPParser *nhttp = static_cast<HTTPParser *>(p->data);

    nhttp->HTTPClearState();

    nhttp->m_Data = buffer_new(0);

    return 0;
}

static int message_complete_cb(http_parser *p)
{
    HTTPParser *nhttp = static_cast<HTTPParser *>(p->data);

    nhttp->HTTPRequestEnded();

    return 0;
}

static int body_cb(http_parser *p, const char *buf, size_t len)
{
    HTTPParser *nhttp = static_cast<HTTPParser *>(p->data);

    if (nhttp->m_Data == NULL) {
        nhttp->m_Data = buffer_new(2048);
    }

    if (static_cast<uint64_t>(nhttp->m_Data->used + len) > HTTP_MAX_CL) {
        return -1;
    }

    if (len != 0) {
        buffer_append_data(nhttp->m_Data,
                           reinterpret_cast<const unsigned char *>(buf), len);
    }

    nhttp->HTTPOnData(buf, len);

    return 0;
}

static int header_field_cb(http_parser *p, const char *buf, size_t len)
{
    HTTPParser *nhttp = static_cast<HTTPParser *>(p->data);

    switch (nhttp->m_Headers.prevstate) {
        case HTTPParser::kPrevState_Nothing:
            nhttp->m_Headers.list = ape_array_new(16);
        /* fall through */
        case HTTPParser::kPrevState_Value:
            nhttp->m_Headers.tkey = buffer_new(16);
            if (nhttp->m_Headers.tval != NULL) {
                buffer_append_char(nhttp->m_Headers.tval, '\0');
            }
            break;
        default:
            break;
    }

    nhttp->m_Headers.prevstate = HTTPParser::kPrevState_Field;

    if (len != 0) {
        buffer_append_data_tolower(nhttp->m_Headers.tkey,
                                   reinterpret_cast<const unsigned char *>(buf),
                                   len);
    }

    return 0;
}

static int header_value_cb(http_parser *p, const char *buf, size_t len)
{
    HTTPParser *nhttp = static_cast<HTTPParser *>(p->data);

    switch (nhttp->m_Headers.prevstate) {
        case HTTPParser::kPrevState_Nothing:
            return -1;
        case HTTPParser::kPrevState_Field:
            nhttp->m_Headers.tval = buffer_new(64);
            buffer_append_char(nhttp->m_Headers.tkey, '\0');
            ape_array_add_b(nhttp->m_Headers.list, nhttp->m_Headers.tkey,
                            nhttp->m_Headers.tval);
            break;
        default:
            break;
    }

    nhttp->m_Headers.prevstate = HTTPParser::kPrevState_Value;

    if (len != 0) {
        buffer_append_data(nhttp->m_Headers.tval,
                           reinterpret_cast<const unsigned char *>(buf), len);
    }
    return 0;
}

static int request_url_cb(http_parser *p, const char *buf, size_t len)
{
    return 0;
}

static int headers_complete_cb(http_parser *p)
{
    HTTPParser *nhttp = static_cast<HTTPParser *>(p->data);

    if (nhttp->m_Headers.tval != NULL) {
        buffer_append_char(nhttp->m_Headers.tval, '\0');
    }

    if (p->content_length == ULLONG_MAX) {
        nhttp->m_Contentlength = 0;
        nhttp->HTTPHeaderEnded();
        return 0;
    }

    if (p->content_length > HTTP_MAX_CL) {
        return -1;
    }

    nhttp->m_Contentlength = p->content_length;
    nhttp->HTTPHeaderEnded();

    return 0;
}
// }}}

// {{{ Implementation
bool HTTPParser::HTTPParse(const char *data, size_t len)
{
    size_t nparsed;

    nparsed = http_parser_execute(&m_Parser, &settings,
                                  static_cast<const char *>(data), len);

    if (m_Parser.upgrade) {
        this->HTTPOnUpgrade();

        if (nparsed < len) {
            this->HTTPOnData(data + nparsed, len - nparsed);
        }
    }

    return nparsed != 0;
}

void HTTPParser::HTTPClearState()
{
    ape_array_destroy(m_Headers.list);
    buffer_destroy(m_Data);
    m_Data = NULL;

    memset(&m_Headers, 0, sizeof(m_Headers));
    m_Headers.prevstate = kPrevState_Nothing;
}

HTTPParser::HTTPParser() : m_Data(NULL), m_Ended(0), m_Contentlength(0)
{
    http_parser_init(&m_Parser, HTTP_RESPONSE);
    m_Parser.data = this;

    m_Headers.prevstate = kPrevState_Nothing;
    m_Headers.list      = NULL;
    m_Headers.tkey      = NULL;
    m_Headers.tval      = NULL;
}

HTTPParser::~HTTPParser()
{
}

const char *HTTPParser::HTTPGetHeader(const char *key)
{
    buffer *ret = ape_array_lookup_cstr(m_Headers.list, key, strlen(key));
    return ret ? reinterpret_cast<const char *>(ret->data) : NULL;
}
// }}}

} // namespace Net
} // namespace Nidium
