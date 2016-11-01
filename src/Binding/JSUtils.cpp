/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSUtils.h"

#include <string.h>
#include <strings.h>

#include <jsfriendapi.h>
#include <js/CharacterEncoding.h>

#include "Binding/NidiumJS.h"

namespace Nidium {
namespace Binding {

// {{{ JSUtils
bool JSUtils::StrToJsval(JSContext *cx,
                         const char *buf,
                         size_t len,
                         JS::MutableHandleValue ret,
                         const char *encoding)
{
    ret.setNull();

    if (encoding) {

        JS::RootedString str(
            cx, JSUtils::NewStringWithEncoding(cx, buf, len, encoding));
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

char16_t *JSUtils::Utf8ToUtf16(JSContext *cx,
                               const char *str,
                               size_t len,
                               size_t *outputlen)
{
    return JS::LossyUTF8CharsToNewTwoByteCharsZ(cx, JS::UTF8Chars(str, len),
                                                outputlen)
        .get();
}

JSString *JSUtils::NewStringWithEncoding(JSContext *cx,
                                         const char *buf,
                                         size_t len,
                                         const char *encoding)
{

    if (encoding != NULL && strcasecmp(encoding, "utf8") == 0) {
        size_t jlen = 0;

        Core::PtrAutoDelete<char16_t *> content(
            JSUtils::Utf8ToUtf16(cx, buf, len, &jlen), free);

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

char *JSUtils::CurrentJSCaller(JSContext *cx)
{
    if (cx == NULL) {
        /* lookup in the TLS */
        NidiumJS *js = NidiumJS::GetObject();
        if (!js || (cx = js->getJSContext()) == NULL) {
            return NULL;
        }
    }

    unsigned lineno;

    JS::AutoFilename af;
    JS::DescribeScriptedCaller(cx, &af, &lineno);

    return strdup(af.get());
}
// }}}

// {{{ JSTransferable
bool JSTransferable::set(JSContext *cx, JS::HandleValue val)
{
    if (!JS_WriteStructuredClone(cx, val, &m_Data, &m_Bytes, nullptr, nullptr,
                                 JS::NullHandleValue)) {
        return false;
    }

    return true;
}

bool JSTransferable::transfert()
{
    bool ok = JS_ReadStructuredClone(m_DestCx, m_Data, m_Bytes,
                                     JS_STRUCTURED_CLONE_VERSION, &m_Val,
                                     nullptr, nullptr);

    JS_ClearStructuredClone(m_Data, m_Bytes, nullptr, nullptr);

    m_Data  = NULL;
    m_Bytes = 0;

    return ok;
}

JSTransferable::~JSTransferable()
{
    JSAutoRequest ar(m_DestCx);
    JSAutoCompartment ac(m_DestCx, m_DestGlobal);

    if (m_Data != NULL) {
        JS_ClearStructuredClone(m_Data, m_Bytes, nullptr, NULL);
    }

    m_Val.set(JS::UndefinedHandleValue);
}

JS::Value JSTransferable::get()
{
    if (m_Data) {
        if (!this->transfert()) {
            return JS::NullValue();
        }
    }

    return m_Val.get();
}

bool JSTransferableFunction::set(JSContext *cx, JS::HandleValue val)
{
    if (!val.isNull() && (!val.isObject()
                          || !JS::IsCallable(val.toObjectOrNull()))) {
        return false;
    }

    return JSTransferable::set(cx, val);
}

bool JSTransferableFunction::call(JS::HandleObject obj,
                                  JS::HandleValueArray params,
                                  JS::MutableHandleValue rval)
{
    JSAutoRequest ar(m_DestCx);
    JSAutoCompartment ac(m_DestCx, m_DestGlobal);

    this->get();

    if (m_Val.get().isNullOrUndefined()) {
        return false;
    }

    return JS_CallFunctionValue(m_DestCx, obj, m_Val, params, rval);
}
// }}}

} // namespace Binding
} // namespace Nidium
