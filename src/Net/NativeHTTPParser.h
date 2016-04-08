/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativehttpparser_h__
#define nativehttpparser_h__

#include <http_parser.h>
#include <ape_array.h>

class NativeHTTPParser
{
public:
    virtual ~NativeHTTPParser();

    bool HTTPParse(const char *data, size_t len);
    void HTTPClearState();
    const char *HTTPGetHeader(const char *key);

    virtual void HTTPHeaderEnded()=0;
    virtual void HTTPRequestEnded()=0;
    virtual void HTTPOnData(size_t offset, size_t len)=0;

    enum PrevState {
        PSTATE_NOTHING,
        PSTATE_FIELD,
        PSTATE_VALUE
    };

    http_parser m_Parser;
    bool m_Parser_rdy;
    struct {
        ape_array_t *list;
        buffer *tkey;
        buffer *tval;
        PrevState prevstate;
    } m_Headers;

    buffer *m_Data;
    int m_Ended;
    uint64_t m_Contentlength;
protected:
    NativeHTTPParser();
};

#endif

