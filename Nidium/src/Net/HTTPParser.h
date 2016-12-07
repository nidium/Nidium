/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef net_httpparser_h__
#define net_httpparser_h__

#include <http_parser.h>
#include <ape_array.h>

namespace Nidium {
namespace Net {

class HTTPParser
{
public:
    virtual ~HTTPParser();

    bool HTTPParse(const char *data, size_t len);
    void HTTPClearState();
    const char *HTTPGetHeader(const char *key);

    virtual void HTTPOnUpgrade(){};
    virtual void HTTPHeaderEnded() = 0;
    virtual void HTTPRequestEnded() = 0;
    virtual void HTTPOnData(const char *data, size_t len) = 0;

    enum PrevState
    {
        kPrevState_Nothing,
        kPrevState_Field,
        kPrevState_Value
    };

    http_parser m_Parser;
    struct
    {
        ape_array_t *list;
        buffer *tkey;
        buffer *tval;
        PrevState prevstate;
    } m_Headers;

    buffer *m_Data;
    int m_Ended;
    uint64_t m_Contentlength;

protected:
    HTTPParser();
};

} // namespace Net
} // namespace Nidium

#endif
