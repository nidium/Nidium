/*
    NativeJS Core Library
    Copyright (C) 2015 Anthony Catel <paraboul@gmail.com>

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
#include "NativeHTTPParser.h"

#include <string.h>

#ifndef ULLONG_MAX
# define ULLONG_MAX ((uint64_t) -1) /* 2^64-1 */
#endif

#define HTTP_MAX_CL (1024ULL*1024ULL*1024ULL*2ULL)

static int message_begin_cb(http_parser *p);
static int headers_complete_cb(http_parser *p);
static int message_complete_cb(http_parser *p);
static int header_field_cb(http_parser *p, const char *buf, size_t len);
static int header_value_cb(http_parser *p, const char *buf, size_t len);
static int request_url_cb(http_parser *p, const char *buf, size_t len);
static int body_cb(http_parser *p, const char *buf, size_t len);

static http_parser_settings settings =
{
    .on_message_begin    = message_begin_cb,
    .on_header_field     = header_field_cb,
    .on_header_value     = header_value_cb,
    .on_url              = request_url_cb,
    .on_body             = body_cb,
    .on_headers_complete = headers_complete_cb,
    .on_message_complete = message_complete_cb
};

static int message_begin_cb(http_parser *p)
{
    NativeHTTPParser *nhttp = (NativeHTTPParser *)p->data;

    nhttp->HTTPClearState();

    nhttp->m_Data = buffer_new(0);

    return 0;
}

static int message_complete_cb(http_parser *p)
{
    NativeHTTPParser *nhttp = (NativeHTTPParser *)p->data;

    nhttp->HTTPRequestEnded();

    return 0;
}

static int body_cb(http_parser *p, const char *buf, size_t len)
{
    NativeHTTPParser *nhttp = (NativeHTTPParser *)p->data;

    if (nhttp->m_Data == NULL) {
        nhttp->m_Data = buffer_new(2048);
    }

    if ((uint64_t)(nhttp->m_Data->used + len) > HTTP_MAX_CL) {
        return -1;
    }

    if (len != 0) {
        buffer_append_data(nhttp->m_Data,
            (const unsigned char *)buf, len);
    }

    nhttp->HTTPOnData(nhttp->m_Data->used - len, len);

    return 0;
}

static int header_field_cb(http_parser *p, const char *buf, size_t len)
{
    NativeHTTPParser *nhttp = (NativeHTTPParser *)p->data;

    switch (nhttp->m_Headers.prevstate) {
        case NativeHTTPParser::PSTATE_NOTHING:
            nhttp->m_Headers.list = ape_array_new(16);
            /* fall through */
        case NativeHTTPParser::PSTATE_VALUE:
            nhttp->m_Headers.tkey = buffer_new(16);
            if (nhttp->m_Headers.tval != NULL) {
                buffer_append_char(nhttp->m_Headers.tval, '\0');
            }
            break;
        default:
            break;
    }

    nhttp->m_Headers.prevstate = NativeHTTPParser::PSTATE_FIELD;

    if (len != 0) {
        buffer_append_data_tolower(nhttp->m_Headers.tkey,
            (const unsigned char *)buf, len);
    }

    return 0;
}

static int header_value_cb(http_parser *p, const char *buf, size_t len)
{
    NativeHTTPParser *nhttp = (NativeHTTPParser *)p->data;

    switch (nhttp->m_Headers.prevstate) {
        case NativeHTTPParser::PSTATE_NOTHING:
            return -1;
        case NativeHTTPParser::PSTATE_FIELD:
            nhttp->m_Headers.tval = buffer_new(64);
            buffer_append_char(nhttp->m_Headers.tkey, '\0');
            ape_array_add_b(nhttp->m_Headers.list,
                    nhttp->m_Headers.tkey, nhttp->m_Headers.tval);
            break;
        default:
            break;
    }

    nhttp->m_Headers.prevstate = NativeHTTPParser::PSTATE_VALUE;

    if (len != 0) {
        buffer_append_data(nhttp->m_Headers.tval,
            (const unsigned char *)buf, len);
    }
    return 0;
}

static int request_url_cb(http_parser *p, const char *buf, size_t len)
{
    return 0;
}

static int headers_complete_cb(http_parser *p)
{
    NativeHTTPParser *nhttp = (NativeHTTPParser *)p->data;

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

bool NativeHTTPParser::HTTPParse(const char *data, size_t len)
{
    size_t nparsed;

    nparsed = http_parser_execute(&m_Parser, &settings,
        (const char *)data, len);

    return true;
}

void NativeHTTPParser::HTTPClearState()
{
    ape_array_destroy(m_Headers.list);
    buffer_destroy(m_Data);
    m_Data = NULL;

    memset(&m_Headers, 0, sizeof(m_Headers));
    m_Headers.prevstate = PSTATE_NOTHING;
}

NativeHTTPParser::NativeHTTPParser() :
    m_Data(NULL), m_Ended(0), m_Contentlength(0)
{
    http_parser_init(&m_Parser, HTTP_RESPONSE);
    m_Parser.data = this;

    m_Headers.prevstate = PSTATE_NOTHING;
    m_Headers.list = NULL;
    m_Headers.tkey = NULL;
    m_Headers.tval = NULL;
}

NativeHTTPParser::~NativeHTTPParser()
{

}

const char *NativeHTTPParser::HTTPGetHeader(const char *key)
{
    buffer *ret = ape_array_lookup_cstr(m_Headers.list, key, strlen(key));
    return ret ? (const char *)ret->data : NULL;
}

