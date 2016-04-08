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

