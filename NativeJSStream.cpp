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

#include "NativeJSStream.h"
#include "NativeJS.h"
#include "NativeUtils.h"
#include "NativeStreamInterface.h"

#include <string>

enum {
    STREAM_PROP_FILESIZE
};

static bool native_stream_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp);

static void Stream_Finalize(JSFreeOp *fop, JSObject *obj);
static bool native_stream_seek(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_stream_start(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_stream_stop(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_stream_getNextPacket(JSContext *cx, unsigned argc, JS::Value *vp);

static JSClass Stream_class = {
    "Stream", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Stream_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *NativeJSExposer<NativeJSStream>::jsclass = &Stream_class;

static JSFunctionSpec Stream_funcs[] = {
    JS_FN("seek", native_stream_seek, 1, 0),
    JS_FN("start", native_stream_start, 0, 0),
    JS_FN("stop", native_stream_stop, 0, 0),
    JS_FN("getNextPacket", native_stream_getNextPacket, 0, 0),
    JS_FS_END
};

/* STREAM_PROP_FILESIZE */
static JSPropertySpec Stream_props[] = {
    {"fileSize", JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE | JSPROP_NATIVE_ACCESSORS,
        NATIVE_JS_GETTER(STREAM_PROP_FILESIZE, native_stream_prop_get),
        JSOP_NULLWRAPPER},
    JS_PS_END
};

static void Stream_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSStream *nstream = (NativeJSStream *)JS_GetPrivate(obj);

    if (nstream != NULL) {
        delete nstream;
    }
}

static bool native_stream_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp)
{

    NativeJSStream *stream = (NativeJSStream *)JS_GetPrivate(obj);

    switch(id) {
        case STREAM_PROP_FILESIZE:
            vp.setInt32(stream->getStream()->getFileSize());
            break;
        default:break;
    }
    return true;
}

static bool native_stream_stop(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    if (!JS_InstanceOf(cx, caller, &Stream_class, &args)) {
        return true;
    }

    ((NativeJSStream *)JS_GetPrivate(caller))->getStream()->stop();

    return true;
}

static bool native_stream_seek(JSContext *cx, unsigned argc, JS::Value *vp)
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

    ((NativeJSStream *)JS_GetPrivate(caller))->getStream()->seek(pos);

    return true;
}

static bool native_stream_start(JSContext *cx, unsigned argc, JS::Value *vp)
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

    ((NativeJSStream *)JS_GetPrivate(caller))->getStream()->start(packetlen);

    NativeJSObj(cx)->rootObjectUntilShutdown(caller);

    return true;
}

static bool native_stream_getNextPacket(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    if (!JS_InstanceOf(cx, caller, &Stream_class, &args)) {
        return true;
    }

    size_t                len;
    int                   err;
    const unsigned char   *ret;

    ret = ((NativeJSStream *)JS_GetPrivate(caller))->
        getStream()->getNextPacket(&len, &err);

    if (ret == NULL) {
        switch(err) {
            case NativeBaseStream::STREAM_END:
                JS_ReportError(cx, "Stream has ended");
                return false;
            case NativeBaseStream::STREAM_ERROR:
                JS_ReportError(cx, "Stream error (unknown)");
                return false;
            case NativeBaseStream::STREAM_EAGAIN:
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

static bool native_Stream_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
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

    NativeJSStream *jstream = new NativeJSStream(ret, cx,
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

NativeJSStream::NativeJSStream(JS::HandleObject obj, JSContext *cx,
    ape_global *net, const char *url) :
    NativeJSExposer<NativeJSStream>(obj, cx)
{
    std::string str = url;
    //str += NativeJS::getNativeClass(cx)->getPath();

    m_Stream = NativeBaseStream::create(NativePath(str.c_str()));

    if (!m_Stream) {
        return;
    }

    m_Stream->setListener(this);
}

NativeJSStream::~NativeJSStream()
{
    delete m_Stream;
}

#if 0
void NativeJSStream::onProgress(size_t buffered, size_t total)
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

void NativeJSStream::onMessage(const NativeSharedMessages::Message &msg)
{
    JS::RootedValue onavailable_callback(m_Cx);
    JS::RootedValue onerror_callback(m_Cx);
    JS::RootedValue rval(m_Cx);

    JS::RootedObject obj(m_Cx, m_JSObject);

    switch (msg.event()) {
        case NATIVESTREAM_AVAILABLE_DATA:
            if (JS_GetProperty(m_Cx, obj, "onavailabledata", &onavailable_callback) &&
                JS_TypeOfValue(m_Cx, onavailable_callback) == JSTYPE_FUNCTION) {

                JS_CallFunctionValue(m_Cx, obj, onavailable_callback, JS::HandleValueArray::empty(), &rval);
            }
            break;
        case NATIVESTREAM_ERROR:
        	{
            NativeBaseStream::StreamErrors err = 
                (NativeBaseStream::StreamErrors)msg.args[0].toInt();
            int code = msg.args[1].toInt();
            switch (err) {
                case NativeBaseStream::NATIVESTREAM_ERROR_OPEN:
                    break;
                case NativeBaseStream::NATIVESTREAM_ERROR_READ:
                    break;
                case NativeBaseStream::NATIVESTREAM_ERROR_SEEK:
                    
                    break;
                default:
                    break;
            }
            break;
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

NATIVE_OBJECT_EXPOSE(Stream)
