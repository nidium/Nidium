/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsutils_h__
#define binding_jsutils_h__

#include <jspubtd.h>
#include <jsapi.h>

namespace Nidium {
namespace Binding {

class JSUtils
{
public:
    /*
        output a jsval corresponding to the given data.
        Encoding accepts : utf8, NULL, anything else.
        In case encoding is NULL, the jsval is an arraybuffer.
    */
    static bool strToJsval(JSContext *cx, const char *buf, size_t len,
        JS::MutableHandleValue jval, const char *encoding);

    static JSString *newStringWithEncoding(JSContext *cx, const char *buf,
        size_t len, const char *encoding);

    static char16_t *Utf8ToUtf16(JSContext *cx, const char *str, size_t len,
        size_t *outputlen);
};

} // namespace Binding
} // namespace Nidium

#endif

