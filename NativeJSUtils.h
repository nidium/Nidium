/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejsutils_h__
#define nativejsutils_h__

#include <jspubtd.h>
#include <jsapi.h>

class NativeJSUtils
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
};

#endif

