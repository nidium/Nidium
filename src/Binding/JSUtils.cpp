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

#include <js/RootingAPI.h>
#include <jsprf.h>


bool JS_ConvertArgumentsVA(JSContext *cx,
    const JS::CallArgs &args, const char *format, va_list ap);

bool JS_ConvertArguments(JSContext *cx, const JS::CallArgs &args, const char *format, ...)
{
    va_list ap;
    bool ok;

    va_start(ap, format);
    ok = JS_ConvertArgumentsVA(cx, args, format, ap);
    va_end(ap);
    return ok;
}

bool
JS_ConvertArgumentsVA(JSContext *cx, const JS::CallArgs &args, const char *format, va_list ap)
{
    using namespace Nidium::Binding;

    unsigned index = 0;
    bool required;
    char c;
    double d;
    JSString *str;
    JS::RootedObject obj(cx);
    JS::RootedValue val(cx);

    required = true;
    while ((c = *format++) != '\0') {
        if (isspace(c))
            continue;
        if (c == '/') {
            required = false;
            continue;
        }
        if (index == args.length()) {
            if (required) {
                if (JSFunction *fun = JSUtils::ReportIfNotFunction(cx, args.calleev())) {
                    char numBuf[12];
                    JS_snprintf(numBuf, sizeof numBuf, "%u", args.length());
                    JSAutoByteString funNameBytes;
                    if (const char *name = JSUtils::GetFunctionNameBytes(cx, fun, &funNameBytes)) {
                        JS_ReportErrorNumber(cx, js::GetErrorMessage, nullptr,
                                             JSMSG_MORE_ARGS_NEEDED,
                                             name, numBuf, (args.length() == 1) ? "" : "s");
                    }
                }
                return false;
            }
            break;
        }
        JS::MutableHandleValue arg = args[index++];
        switch (c) {
          case 'b':
            *va_arg(ap, bool *) = ToBoolean(arg);
            break;
          case 'c':
            if (!JS::ToUint16(cx, arg, va_arg(ap, uint16_t *)))
                return false;
            break;
          case 'i':
          case 'j': // "j" was broken, you should not use it.
            if (!JS::ToInt32(cx, arg, va_arg(ap, int32_t *)))
                return false;
            break;
          case 'u':
            if (!JS::ToUint32(cx, arg, va_arg(ap, uint32_t *)))
                return false;
            break;
          case 'd':
            if (!JS::ToNumber(cx, arg, va_arg(ap, double *)))
                return false;
            break;
          case 'I':
            if (!JS::ToNumber(cx, arg, &d))
                return false;
            *va_arg(ap, double *) = JS::ToInteger(d);
            break;
          case 'S':
          case 'W':
            str = JS::ToString(cx, arg);
            if (!str)
                return false;
            arg.setString(str);
            if (c == 'W') {

            } else {
                *va_arg(ap, JSString **) = str;
            }
            break;
          case 'o':
            if (arg.isNullOrUndefined()) {
                obj = nullptr;
            } else {
                obj = ToObject(cx, arg);
                if (!obj)
                    return false;
            }
            arg.setObjectOrNull(obj);
            *va_arg(ap, JSObject **) = obj;
            break;
          case 'v':
            *va_arg(ap, JS::Value *) = arg;
            break;
          case '*':
            break;
          default:
            JS_ReportErrorNumber(cx, js::GetErrorMessage, nullptr, JSMSG_BAD_CHAR, format);
            return false;
        }
    }
    return true;
}


namespace Nidium {
namespace Binding {


JSObject *JSUtils::NewObjectForConstructor(JSContext *cx,
    JSClass *jsclass, JS::CallArgs &args)
{
    JS::RootedObject newTarget(cx, args.newTarget().toObjectOrNull());

    JS::RootedValue protoVal(cx);
    
    if (!JS_GetProperty(cx, newTarget, "prototype", &protoVal)) {
        return nullptr;
    }

    JS::RootedObject protoObj(cx); 
    
    if (protoVal.isObject()) {
        protoObj = &protoVal.toObject();
    } else {
        protoObj = JS_GetObjectPrototype(cx, newTarget); 
    }

    JS::RootedObject ret(
        cx, JS_NewObjectWithGivenProto(cx, jsclass, protoObj));

    return ret;
}

const char* JSUtils::GetFunctionNameBytes(JSContext* cx,
        JSFunction* fun, JSAutoByteString* bytes)
{
    JSString *val = JS_GetFunctionId(fun);
    if (!val) {
        return "(anonymous)";
    }

    return bytes->encodeLatin1(cx, val);
}

JSFunction *JSUtils::ReportIfNotFunction(JSContext *cx, JS::HandleValue val)
{
    JS::RootedFunction ret(cx);

    if (val.isObject() && (ret = JS_GetObjectFunction(&val.toObject()))) {
        return ret;
    }

    JSAutoByteString fn(cx, JS::ToString(cx, val));

    JS_ReportErrorNumber(cx, js::GetErrorMessage, nullptr,
        JSMSG_NOT_FUNCTION, fn.ptr());

    return nullptr;
}

JSObject *JSUtils::NewArrayBufferWithCopiedContents(JSContext *cx,
        size_t len, const void *data)
{
    JS::RootedObject arr(cx, JS_NewArrayBuffer(cx, len));

    if (!arr) {
        JS_ReportOutOfMemory(cx);
        return nullptr;
    }

    JS::AutoCheckCannotGC nogc;
    bool shared;

    uint8_t *adata = JS_GetArrayBufferData(arr, &shared, nogc);

    memcpy(adata, data, len);

    return arr;
}

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
        JS::RootedObject arrayBuffer(cx,
            NewArrayBufferWithCopiedContents(cx, len, buf));

        if (arrayBuffer == nullptr) {
            return false;
        }

        ret.setObject(*arrayBuffer);
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


bool JSUtils::ValuePercent(JSContext *cx, JS::HandleValue val, double *out)
{
    if (!val.isString()) {
        if (!JS::ToNumber(cx, val, out)) {
            *out = 0;
        }
        return false;
    }

    JS::RootedString vpStr(cx, val.toString());
    JSAutoByteString type(cx, vpStr);
    int len = type.length();

    if (len >= 2 && type.ptr()[len-1] == '%') {
        *out = atof(type.ptr());
        return true;
    } else {
        *out = atof(type.ptr());
    }

    return false;
}

// }}}

// {{{ JSTransferable
bool JSTransferable::set(JSContext *cx, JS::HandleValue val)
{
    if (!JS_WriteStructuredClone(cx, val, &m_Data, &m_Bytes, NidiumJS::m_JsScc, nullptr,
                                 JS::NullHandleValue)) {
        return false;
    }

    return true;
}

bool JSTransferable::transfert()
{
    bool ok = JS_ReadStructuredClone(m_DestCx, m_Data, m_Bytes,
                                     JS_STRUCTURED_CLONE_VERSION, &m_Val,
                                     NidiumJS::m_JsScc, nullptr);

    JS_ClearStructuredClone(m_Data, m_Bytes, NidiumJS::m_JsScc, nullptr);

    m_Data  = NULL;
    m_Bytes = 0;

    return ok;
}

JSTransferable::~JSTransferable()
{
    JSAutoRequest ar(m_DestCx);
    JSAutoCompartment ac(m_DestCx, m_DestGlobal);

    if (m_Data != NULL) {
        JS_ClearStructuredClone(m_Data, m_Bytes, NidiumJS::m_JsScc, NULL);
    }

    m_Val = JS::UndefinedHandleValue;
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
