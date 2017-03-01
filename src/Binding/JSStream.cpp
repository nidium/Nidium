/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSStream.h"

#include <string.h>
#include <string>

using Nidium::IO::Stream;

namespace Nidium {
namespace Binding {


// {{{ JSStream
JSStream::JSStream(ape_global *net,
                   const char *url)
{
    std::string str = url;
    // str += NidiumJS::getNidiumClass(cx)->getPath();

    m_Stream = Stream::Create(Core::Path(str.c_str()));

    if (!m_Stream) {
        return;
    }

    m_Stream->setListener(this);
}

JSStream::~JSStream()
{
    delete m_Stream;
}

#if 0
void JSStream::onProgress(size_t buffered, size_t total)
{
    JS::RootedObject obj(cx, this->jsobj);
    JS::Value onprogress_callback;

    if (JS_GetProperty(this->cx, obj, "onProgress", &onprogress_callback) &&
        JS_TypeOfValue(this->cx, onprogress_callback) == JSTYPE_FUNCTION) {

        JS::RootedValue rval(this->cx);
        JS::AutoValueArray<2>args(this->cx);
        args[0] = JS::Int32Value(buffered);
        args[1] = JS::Int32Value(total);

        JS_CallFunctionValue(this->cx, obj, onprogress_callback, args, &rval);
    }
}
#endif

void JSStream::onMessage(const Core::SharedMessages::Message &msg)
{
    JS::RootedValue onavailable_callback(m_Cx);
    JS::RootedValue onerror_callback(m_Cx);
    JS::RootedValue rval(m_Cx);
    JS::RootedObject obj(m_Cx, m_Instance);

    switch (msg.event()) {
        case Stream::kEvents_AvailableData:
            if (JS_GetProperty(m_Cx, obj, "onavailabledata",
                               &onavailable_callback)
                && JS_TypeOfValue(m_Cx, onavailable_callback)
                       == JSTYPE_FUNCTION) {

                JS_CallFunctionValue(m_Cx, obj, onavailable_callback,
                                     JS::HandleValueArray::empty(), &rval);
            }
            break;
        case Stream::kEvents_Error: {
            Stream::Errors err
                = static_cast<Stream::Errors>(msg.m_Args[0].toInt());
            int code = msg.m_Args[1].toInt();
            switch (err) {
                case Stream::kErrors_Open:
                    break;
                case Stream::kErrors_Read:
                    break;
                case Stream::kErrors_Seek:

                    break;
                default:
                    break;
            }
            if (JS_GetProperty(m_Cx, obj, "onerror", &onerror_callback)
                && JS_TypeOfValue(m_Cx, onerror_callback) == JSTYPE_FUNCTION) {
                JS::AutoValueArray<1> args(m_Cx);

                args[0].setInt32(code);
                JS_CallFunctionValue(m_Cx, obj, onerror_callback, args, &rval);
            }
        } break;
        default:
            break;
    }
}
// }}}

// {{{ Implementation
bool JSStream::JSGetter_filesize(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(this->getStream()->getFileSize());

    return true;
}

bool JSStream::JS_stop(JSContext *cx, JS::CallArgs &args)
{
    this->getStream()->stop();

    return true;
}

bool JSStream::JS_seek(JSContext *cx, JS::CallArgs &args)
{
    uint32_t pos;
    if (!JS_ConvertArguments(cx, args, "u", &pos)) {
        return false;
    }

    this->getStream()->seek(pos);

    return true;
}

bool JSStream::JS_start(JSContext *cx, JS::CallArgs &args)
{
    size_t packetlen = 4096;

    if (args.length() > 0 && args[0].isInt32()) {
        packetlen = args[0].toInt32();
        if (packetlen < 1) {
            JS_ReportErrorUTF8(cx,
                           "Invalid packet size (must be greater than zero)");
            return false;
        }
    }

    this->getStream()->start(packetlen);
    this->root();

    return true;
}

bool JSStream::JS_getNextPacket(JSContext *cx, JS::CallArgs &args)
{
    size_t len;
    int err;
    const unsigned char *ret;

    ret = this->getStream()->getNextPacket(&len, &err);

    if (ret == NULL) {
        switch (err) {
            case Stream::Stream::kDataStatus_End:
                JS_ReportErrorUTF8(cx, "Stream has ended");
                return false;
            case Stream::Stream::kDataStatus_Error:
                JS_ReportErrorUTF8(cx, "Stream error (unknown)");
                return false;
            case Stream::Stream::kDataStatus_Again:
                args.rval().setNull();
                return true;
        }
    }

    if (len == 0 || ret == NULL) {
        args.rval().setNull();
        return true;
    }

    JS::RootedObject arrayBuffer(cx,
        JSUtils::NewArrayBufferWithCopiedContents(cx, len, ret));

    args.rval().setObject(*arrayBuffer);

    return true;
}

JSStream * JSStream::Constructor(JSContext *cx,  JS::CallArgs &args,
    JS::HandleObject obj)
{
    JS::RootedString url(cx);
    if (!JS_ConvertArguments(cx, args, "S", url.address())) {
        return nullptr;
    }

    JSAutoByteString curl(cx, url);

    JSStream *jstream = new JSStream(
        static_cast<ape_global *>(JS_GetContextPrivate(cx)), curl.ptr());

    if (jstream->getStream() == NULL) {
        JS_ReportErrorUTF8(cx, "Failed to create stream");
        return nullptr;
    }

    return jstream;
}
JSPropertySpec *JSStream::ListProperties()
{
    static JSPropertySpec props[] = {
        CLASSMAPPER_PROP_G(JSStream, filesize),

        JS_PS_END
    };

    return props;
}
JSFunctionSpec *JSStream::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSStream, seek, 1),
        CLASSMAPPER_FN(JSStream, start, 0),
        CLASSMAPPER_FN(JSStream, stop, 0),
        CLASSMAPPER_FN(JSStream, getNextPacket, 0),
        JS_FS_END
    };

    return funcs;
}
void JSStream::RegisterObject(JSContext *cx)
{
    JSStream::ExposeClass<1>(cx, "Stream");
}

// }}}

} // namespace Binding
} // namespace Nidium
