/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "JSStream.h"

#include <string>

#include "IO/Stream.h"

namespace Nidium {
namespace Binding {

enum {
    STREAM_PROP_FILESIZE
};

static bool nidium_stream_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp);

static void Stream_Finalize(JSFreeOp *fop, JSObject *obj);
static bool nidium_stream_seek(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_stream_start(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_stream_stop(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_stream_getNextPacket(JSContext *cx, unsigned argc, JS::Value *vp);

static JSClass Stream_class = {
    "Stream", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Stream_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *Nidium::Binding::JSExposer<JSStream>::jsclass = &Stream_class;

static JSFunctionSpec Stream_funcs[] = {
    JS_FN("seek", nidium_stream_seek, 1, NATIVE_JS_FNPROPS),
    JS_FN("start", nidium_stream_start, 0, NATIVE_JS_FNPROPS),
    JS_FN("stop", nidium_stream_stop, 0, NATIVE_JS_FNPROPS),
    JS_FN("getNextPacket", nidium_stream_getNextPacket, 0, NATIVE_JS_FNPROPS),
    JS_FS_END
};

static JSPropertySpec Stream_props[] = {
    NIDIUM_JS_PSG("filesize", STREAM_PROP_FILESIZE, nidium_stream_prop_get),
    JS_PS_END
};

static void Stream_Finalize(JSFreeOp *fop, JSObject *obj)
{
    JSStream *nstream = (JSStream *)JS_GetPrivate(obj);

    if (nstream != NULL) {
        delete nstream;
    }
}

static bool nidium_stream_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp)
{

    JSStream *stream = (JSStream *)JS_GetPrivate(obj);

    switch(id) {
        case STREAM_PROP_FILESIZE:
            vp.setInt32(stream->getStream()->getFileSize());
            break;
        default:break;
    }
    return true;
}

static bool nidium_stream_stop(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    if (!JS_InstanceOf(cx, caller, &Stream_class, &args)) {
        return true;
    }

    ((JSStream *)JS_GetPrivate(caller))->getStream()->stop();

    return true;
}

static bool nidium_stream_seek(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    uint32_t pos;

    if (!JS_InstanceOf(cx, caller, &Stream_class, &args)) {
        return true;
    }

    if (!JS_ConvertArguments(cx, args, "u", &pos)) {
        return false;
    }

    ((JSStream *)JS_GetPrivate(caller))->getStream()->seek(pos);

    return true;
}

static bool nidium_stream_start(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    size_t packetlen = 4096;

    if (!JS_InstanceOf(cx, caller, &Stream_class, &args)) {
        return true;
    }
    if (args.length() > 0 && args[0].isInt32()) {
        packetlen = args[0].toInt32();
        if (packetlen < 1) {
            JS_ReportError(cx, "Invalid packet size (must be greater than zero)");
            return false;
        }
    }

    ((JSStream *)JS_GetPrivate(caller))->getStream()->start(packetlen);

    NativeJSObj(cx)->rootObjectUntilShutdown(caller);

    return true;
}

static bool nidium_stream_getNextPacket(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    if (!JS_InstanceOf(cx, caller, &Stream_class, &args)) {
        return true;
    }

    size_t                len;
    int                   err;
    const unsigned char   *ret;

    ret = ((JSStream *)JS_GetPrivate(caller))->
        getStream()->getNextPacket(&len, &err);

    if (ret == NULL) {
        switch(err) {
            case Nidium::IO::Stream::STREAM_END:
                JS_ReportError(cx, "Stream has ended");
                return false;
            case Nidium::IO::Stream::STREAM_ERROR:
                JS_ReportError(cx, "Stream error (unknown)");
                return false;
            case Nidium::IO::Stream::STREAM_EAGAIN:
                args.rval().setNull();
                return true;
        }
    }

    if (len == 0 || ret == NULL) {
        args.rval().setNull();
        return true;
    }

    JS::RootedObject arrayBuffer(cx, JS_NewArrayBuffer(cx, len));
    uint8_t *data = JS_GetArrayBufferData(arrayBuffer);
    memcpy(data, ret, len);

    args.rval().setObject(*arrayBuffer);

    return true;
}

static bool nidium_Stream_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString url(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!args.isConstructing()) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    if (!JS_ConvertArguments(cx, args, "S", url.address())) {
        return false;
    }

    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &Stream_class, args));

    JSAutoByteString curl(cx, url);

    JSStream *jstream = new JSStream(ret, cx,
        (ape_global *)JS_GetContextPrivate(cx), curl.ptr());

    if (jstream->getStream() == NULL) {
        JS_ReportError(cx, "Failed to create stream");
        return false;
    }

    JS_SetPrivate(ret, jstream);

    JS_DefineFunctions(cx, ret, Stream_funcs);
    JS_DefineProperties(cx, ret, Stream_props);

    args.rval().setObject(*ret);

    return true;
}

JSStream::JSStream(JS::HandleObject obj, JSContext *cx,
    ape_global *net, const char *url) :
    Nidium::Binding::JSExposer<JSStream>(obj, cx)
{
    std::string str = url;
    //str += NativeJS::getNativeClass(cx)->getPath();

    m_Stream = Nidium::IO::Stream::create(NativePath(str.c_str()));

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
        args[0] = INT_TO_JSVAL(buffered);
        args[1] = INT_TO_JSVAL(total);

        JS_CallFunctionValue(this->cx, obj, onprogress_callback, args, &rval);
    }
}
#endif

void JSStream::onMessage(const Nidium::Core::SharedMessages::Message &msg)
{
    JS::RootedValue onavailable_callback(m_Cx);
    JS::RootedValue onerror_callback(m_Cx);
    JS::RootedValue rval(m_Cx);

    JS::RootedObject obj(m_Cx, m_JSObject);

    switch (msg.event()) {
        case Nidium::IO::STREAM_AVAILABLE_DATA:
            if (JS_GetProperty(m_Cx, obj, "onavailabledata", &onavailable_callback) &&
                JS_TypeOfValue(m_Cx, onavailable_callback) == JSTYPE_FUNCTION) {

                JS_CallFunctionValue(m_Cx, obj, onavailable_callback, JS::HandleValueArray::empty(), &rval);
            }
            break;
        case Nidium::IO::STREAM_ERROR:
            {
            Nidium::IO::Stream::StreamErrors err =
                (Nidium::IO::Stream::StreamErrors)msg.args[0].toInt();
            int code = msg.args[1].toInt();
            switch (err) {
                case Nidium::IO::Stream::STREAM_ERROR_OPEN:
                    break;
                case Nidium::IO::Stream::STREAM_ERROR_READ:
                    break;
                case Nidium::IO::Stream::STREAM_ERROR_SEEK:

                    break;
                default:
                    break;
            }
            if (JS_GetProperty(m_Cx, obj, "onerror", &onerror_callback) &&
                JS_TypeOfValue(m_Cx, onerror_callback) == JSTYPE_FUNCTION) {
                JS::AutoValueArray<1> args(m_Cx);

                args[0].setInt32(code);
                JS_CallFunctionValue(m_Cx, obj, onerror_callback, args, &rval);
                }
            }
            break;
        default:
            break;
    }
}


NIDIUM_JS_OBJECT_EXPOSE(Stream)

} // namespace Binding
} // namespace Nidium
