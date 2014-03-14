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

static JSBool native_stream_prop_get(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSMutableHandleValue vp);

static void Stream_Finalize(JSFreeOp *fop, JSObject *obj);
static JSBool native_stream_seek(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_stream_start(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_stream_stop(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_stream_getNextPacket(JSContext *cx, unsigned argc, jsval *vp);

static JSClass Stream_class = {
    "Stream", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Stream_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSFunctionSpec Stream_funcs[] = {
    JS_FN("seek", native_stream_seek, 1, 0),
    JS_FN("start", native_stream_start, 0, 0),
    JS_FN("stop", native_stream_stop, 0, 0),
    JS_FN("getNextPacket", native_stream_getNextPacket, 0, 0),
    JS_FS_END
};

static JSPropertySpec Stream_props[] = {
    {"fileSize", STREAM_PROP_FILESIZE, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE,
        JSOP_WRAPPER(native_stream_prop_get),
        JSOP_NULLWRAPPER},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static void Stream_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSStream *nstream = (NativeJSStream *)JS_GetPrivate(obj);

    if (nstream != NULL) {
        delete nstream;
    }
}

static JSBool native_stream_prop_get(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSMutableHandleValue vp)
{

    NativeJSStream *stream = (NativeJSStream *)JS_GetPrivate(obj.get());    

    switch(JSID_TO_INT(id)) {
        case STREAM_PROP_FILESIZE:
            vp.setInt32(stream->getStream()->getFileSize());
            break;
        default:break;
    }
    return true;
}

static JSBool native_stream_stop(JSContext *cx, unsigned argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    if (!JS_InstanceOf(cx, caller, &Stream_class, args.array())) {
        return true;
    }

    ((NativeJSStream *)JS_GetPrivate(caller))->getStream()->stop();

    return true;
}

static JSBool native_stream_seek(JSContext *cx, unsigned argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    uint32_t pos;

    if (!JS_InstanceOf(cx, caller, &Stream_class, args.array())) {
        return true;
    }

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "u", &pos)) {
        return false;
    }

    ((NativeJSStream *)JS_GetPrivate(caller))->getStream()->seek(pos);

    return true;
}

static JSBool native_stream_start(JSContext *cx, unsigned argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    size_t packetlen = 4096;

    if (!JS_InstanceOf(cx, caller, &Stream_class, args.array())) {
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

static JSBool native_stream_getNextPacket(JSContext *cx, unsigned argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    if (!JS_InstanceOf(cx, caller, &Stream_class, args.array())) {
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

static JSBool native_Stream_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *url;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!JS_IsConstructing(cx, vp)) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "S", &url)) {
        return false;
    }

    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &Stream_class, vp));

    JSAutoByteString curl(cx, url);

    NativeJSStream *jstream = new NativeJSStream(cx,
        (ape_global *)JS_GetContextPrivate(cx), curl.ptr());

    JS_SetPrivate(ret, jstream);
    jstream->cx = cx;
    jstream->jsobj = ret;

    JS_DefineFunctions(cx, ret, Stream_funcs);
    JS_DefineProperties(cx, ret, Stream_props);

    args.rval().setObject(*ret);

    return true;
}

NativeJSStream::NativeJSStream(JSContext *cx, ape_global *net, const char *url)
{
    std::string str = url;
    //str += NativeJS::getNativeClass(cx)->getPath();

    printf("Opening : %s\n", str.c_str());
    m_Stream = NativeBaseStream::create(str.c_str());

    m_Stream->setListener(this);

    printf("CReated stream\n");
}

NativeJSStream::~NativeJSStream()
{
    delete m_Stream;
}

#if 0
void NativeJSStream::onProgress(size_t buffered, size_t total)
{
    JS::RootedObject obj(cx, this->jsobj);
    jsval onprogress_callback;

    if (JS_GetProperty(this->cx, obj, "onProgress", &onprogress_callback) &&
        JS_TypeOfValue(this->cx, onprogress_callback) == JSTYPE_FUNCTION) {

        jsval args[2], rval;

        args[0] = INT_TO_JSVAL(buffered);
        args[1] = INT_TO_JSVAL(total);

        JS_CallFunctionValue(this->cx, obj, onprogress_callback,
            2, args, &rval);
    }
}
#endif

void NativeJSStream::onMessage(const NativeSharedMessages::Message &msg)
{
    jsval onavailable_callback, rval;
    JS::RootedObject obj(this->cx, this->jsobj);

    switch (msg.event()) {
        case NATIVESTREAM_AVAILABLE_DATA:
            if (JS_GetProperty(this->cx, obj, "onAvailableData", &onavailable_callback) &&
                JS_TypeOfValue(this->cx, onavailable_callback) == JSTYPE_FUNCTION) {

                JS_CallFunctionValue(this->cx, obj, onavailable_callback,
                    0, NULL, &rval);
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

            // TODO: onerror
        }
        default:
            break;
    }
}

NATIVE_OBJECT_EXPOSE(Stream)
