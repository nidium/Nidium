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

bool NativeJSUtils::strToJsval(JSContext *cx, const char *buf, size_t len, JS::Value *ret,
    const char *encoding)
{
    *ret = JSVAL_NULL;

    if (encoding) {
        if (strcasecmp(encoding, "utf8") == 0) {
            size_t jlen = 0;

            NativePtrAutoDelete<jschar *> content(NativeUtils::Utf8ToUtf16(buf, len, &jlen));

            if (content.ptr() == NULL) {
                JS_ReportError(cx, "Could not decode string to utf8");
                return false;
            }

            JSString *str = JS_NewUCString(cx, content.ptr(), jlen);
            if (!str) {
                return false;
            }

            content.disable(); /* JS_NewUCString took ownership */
            *ret = STRING_TO_JSVAL(str);

        } else {
            *ret = STRING_TO_JSVAL(JS_NewStringCopyN(cx, buf, len));
        }
    } else {
        JSObject *arrayBuffer = JS_NewArrayBuffer(cx, len);

        if (arrayBuffer == NULL) {
            JS_ReportOutOfMemory(cx);
            return false;
        } else {
            uint8_t *adata = JS_GetArrayBufferData(arrayBuffer);
            memcpy(adata, buf, len);

            *ret = OBJECT_TO_JSVAL(arrayBuffer);
        }        
    }

    return true;
}