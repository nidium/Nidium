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

#include <jsapi.h>
#include <jsfriendapi.h>

#include "NativeJSUtils.h"
#include "NativeUtils.h"

bool NativeJSUtils::strToJsval(JSContext *cx, const char *buf, size_t len, JS::MutableHandleValue ret,
    const char *encoding)
{
    ret.setNull();

    if (encoding) {

        JS::RootedString str(cx, NativeJSUtils::newStringWithEncoding(cx, buf, len, encoding));
        if (!str) {
            ret.set(JS_GetEmptyStringValue(cx));
            return false;
        }

        ret.setString(str);

    } else {
        JS::RootedObject arrayBuffer(cx, JS_NewArrayBuffer(cx, len));

        if (arrayBuffer == NULL) {
            JS_ReportOutOfMemory(cx);
            return false;
        } else {
            uint8_t *adata = JS_GetArrayBufferData(arrayBuffer);
            memcpy(adata, buf, len);

            ret.setObject(*arrayBuffer);
        }        
    }

    return true;
}

JSString *NativeJSUtils::newStringWithEncoding(JSContext *cx, const char *buf,
        size_t len, const char *encoding)
{

    if (encoding != NULL && strcasecmp(encoding, "utf8") == 0) {
        size_t jlen = 0;

        NativePtrAutoDelete<char16_t *> content(NativeUtils::Utf8ToUtf16(buf, len, &jlen), free);

        if (content.ptr() == NULL) {
            JS_ReportError(cx, "Could not decode string to utf8");
            return NULL;
        }

        JS::RootedString str(cx, JS_NewUCString(cx, content.ptr(), jlen));
        if (str.get() == NULL) {
            return NULL;
        }

        content.disable(); /* JS_NewUCString took ownership */
        
        return str;

    }

    return JS_NewStringCopyN(cx, buf, len);
}
