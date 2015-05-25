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
