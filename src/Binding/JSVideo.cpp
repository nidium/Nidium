/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSAudio.h"
#include "Binding/JSVideo.h"
#include "Binding/JSCanvas2DContext.h"
#include "Binding/JSCanvas.h"

using namespace Nidium::AV;
using Nidium::Graphics::CanvasHandler;
using Nidium::Graphics::SkiaContext;
using Nidium::Graphics::CanvasContext;
using Nidium::Graphics::CanvasHandler;

namespace Nidium {
namespace Binding {

JSVideo::JSVideo(Canvas2DContext *canvasCtx, JSContext *cx)
    : m_Video(NULL), m_AudioNode(NULL),
      m_ArrayContent(NULL), m_Width(-1), m_Height(-1), m_Left(0), m_Top(0),
      m_IsDestructing(false), m_CanvasCtx(canvasCtx)
{
    m_Video = new Video((ape_global *)JS_GetContextPrivate(m_Cx));
    m_Video->frameCallback(JSVideo::FrameCallback, this);
    m_Video->eventCallback(JSVideo::onEvent, this);
    m_CanvasCtx->getHandler()->addListener(this);
}

void JSVideo::stopAudio()
{
    m_Video->stopAudio();

    this->releaseAudioNode();
}

void JSVideo::onMessage(const SharedMessages::Message &msg)
{
    if (m_IsDestructing) return;

    if (msg.event() == NIDIUM_EVENT(CanvasHandler, RESIZE_EVENT)
        && (m_Width == -1 || m_Height == -1)) {
        this->setSize(m_Width, m_Height);
    } else {
        if (msg.event() == SOURCE_EVENT_PLAY) {
            this->setSize(m_Width, m_Height);
        }

        const char *evName = JSAVEventRead(msg.event());
        if (!evName) {
            return;
        }

        JS::RootedValue ev(m_Cx, consumeSourceMessage(m_Cx, msg));
        if (!ev.isNull()) {
            //this->fireJSEvent(evName, &ev);
        }
    }
}

void JSVideo::onEvent(const struct AVSourceEvent *cev)
{
    JSVideo *thiz = static_cast<JSVideo *>(cev->m_Custom);
    thiz->postMessage((void *)cev, cev->m_Ev);
}

void JSVideo::FrameCallback(uint8_t *data, void *custom)
{
    JSVideo *v             = (JSVideo *)custom;
    CanvasHandler *handler = v->m_CanvasCtx->getHandler();
    SkiaContext *surface   = v->m_CanvasCtx->getSurface();
    JSContext *cx          = v->m_Cx;

    surface->setFillColor(0xFF000000);
    surface->drawRect(0, 0, handler->getWidth(), handler->getHeight(), 0);
    surface->drawPixels(data, v->m_Video->m_Width, v->m_Video->m_Height,
                        v->m_Left, v->m_Top);

    JS::RootedValue onframe(v->m_Cx);
    JS::RootedObject vobj(v->m_Cx, v->getJSObject());
    JS::RootedObject evObj(cx);

    evObj = JSEvents::CreateEventObject(cx);
    JSObjectBuilder ev(cx, evObj);
    ev.set("video", v->getJSObject());
    JS::RootedValue evjsval(cx, ev.jsval());

    //v->fireJSEvent("frame", &evjsval);
}

void JSVideo::setSize(int width, int height)
{
    m_Width  = width;
    m_Height = height;

    if (!m_Video->m_CodecCtx) {
        // setSize will be called again when video is ready
        return;
    }

    int canvasWidth  = m_CanvasCtx->getHandler()->getWidth();
    int canvasHeight = m_CanvasCtx->getHandler()->getHeight();

    // Invalid dimension, force size to canvas
    if (width == 0) width = -1;
    if (height == 0) height = -1;

    // Size the video
    if (m_Width == -1 || m_Height == -1) {
        int videoWidth  = m_Video->m_CodecCtx->width;
        int videoHeight = m_Video->m_CodecCtx->height;

        int maxWidth
            = nidium_min(m_Width == -1 ? canvasWidth : m_Width, canvasWidth);
        int maxHeight = nidium_min(m_Height == -1 ? canvasHeight : m_Height,
                                   canvasHeight);
        double ratio = nidium_max(videoHeight / (double)maxHeight,
                                  videoWidth / (double)maxWidth);

        width  = videoWidth / ratio;
        height = videoHeight / ratio;
    }

    if (height < canvasHeight) {
        m_Top = (canvasHeight / 2) - (height / 2);
    } else {
        m_Top = 0;
    }

    if (width < canvasWidth) {
        m_Left = (canvasWidth / 2) - (width / 2);
    } else {
        m_Left = 0;
    }

    m_Video->setSize(width, height);
}

bool JSVideo::JS_play(JSContext *cx, JS::CallArgs &args)
{
    this->m_Video->play();

    return true;
}

bool JSVideo::JS_pause(JSContext *cx, JS::CallArgs &args)
{
    this->m_Video->pause();

    return true;
}

bool JSVideo::JS_stop(JSContext *cx, JS::CallArgs &args)
{
    this->m_Video->stop();

    return true;
}

bool JSVideo::JS_close(JSContext *cx, JS::CallArgs &args)
{
    this->close();

    return true;
}

bool JSVideo::JS_open(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedValue src(cx, args[0]);
    int ret = -1;

    if (src.isString()) {
        JSAutoByteString csrc(cx, src.toString());
        ret = this->m_Video->open(csrc.ptr());
    } else if (src.isObject()) {
        JS::RootedObject arrayBuff(cx, src.toObjectOrNull());

        if (!JS_IsArrayBufferObject(arrayBuff)) {
            JS_ReportError(cx, "Data is not an ArrayBuffer\n");
            return false;
        }

        int length        = JS_GetArrayBufferByteLength(arrayBuff);
        this->m_ArrayContent = JS_StealArrayBufferContents(cx, arrayBuff);
        if (this->m_Video->open(this->m_ArrayContent, length) < 0) {
            args.rval().setBoolean(false);
            return true;
        }
    }

    args.rval().setBoolean(false);

    return true;
}

bool JSVideo::JS_getAudioNode(JSContext *cx, JS::CallArgs &args)
{
    JSAudio *jaudio = JSAudio::GetContext();

    if (!jaudio) {
        JS_ReportError(cx, "No Audio context");
        args.rval().setNull();
        return false;
    }

    if (this->m_AudioNode.get()) {
        JS::RootedObject retObj(cx, this->m_AudioNode);
        args.rval().setObjectOrNull(retObj);
        return true;
    }

    AudioSource *source = this->m_Video->getAudioNode(jaudio->m_Audio);

    if (source != NULL) {
        JS::RootedObject audioNode(
            cx, JS_NewObjectForConstructor(cx, &AudioNode_class, args));
        this->m_AudioNode = audioNode;

        JSAudioNode *node
            = new JSAudioNode(audioNode, cx, Audio::SOURCE,
                              static_cast<class AudioNode *>(source), jaudio);

        JS::RootedString name(cx, JS_NewStringCopyN(cx, "video-source", 12));
        JS::RootedObject an(cx, this->m_AudioNode);
        jaudio->initNode(node, an, name);

        JS_SetReservedSlot(node->getJSObject(), 0,
                           OBJECT_TO_JSVAL(this->getJSObject()));
        JS::RootedObject retObj(cx, this->m_AudioNode);

        args.rval().setObjectOrNull(retObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

bool JSVideo::JS_nextFrame(JSContext *cx, JS::CallArgs &args)
{
    this->m_Video->nextFrame();

    return true;
}

bool JSVideo::JS_prevFrame(JSContext *cx, JS::CallArgs &args)
{
    this->m_Video->prevFrame();

    return true;
}

bool JSVideo::JS_frameAt(JSContext *cx, JS::CallArgs &args)
{
    double time;
    bool keyframe = false;

    if (!JS_ConvertArguments(cx, args, "db", &time, keyframe)) {
        return true;
    }

    this->m_Video->frameAt(time, keyframe);

    return true;
}

bool JSVideo::JS_setSize(JSContext *cx, JS::CallArgs &args)
{
    uint32_t width, height;

    JS::RootedValue jwidth(cx, args[0]);
    JS::RootedValue jheight(cx, args[1]);

    if (jwidth.isString()) {
        width = -1;
    } else if (jwidth.isNumber()) {
        JS::ToUint32(cx, jwidth, &width);
    } else {
        JS_ReportError(cx, "Wrong argument type for width");
        return false;
    }

    if (jheight.isString()) {
        height = -1;
    } else if (jheight.isNumber()) {
        JS::ToUint32(cx, jheight, &height);
    } else {
        JS_ReportError(cx, "Wrong argument type for height");
        return false;
    }

    this->setSize(width, height);

    return true;
}

bool JSVideo::JSGetter_canvas(JSContext *cx, JS::MutableHandleValue vp)
{
    JS::RootedObject canvasObj(cx, this->m_CanvasCtx->getJSObject());
    vp.setObjectOrNull(canvasObj);

    return true;
}

bool JSVideo::JSGetter_width(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(this->m_Video->m_CodecCtx->width);

    return true;
}

bool JSVideo::JSGetter_height(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(this->m_Video->m_CodecCtx->height);

    return true;
}

bool JSVideo::JSGetter_duration(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setDouble(this->m_Video->getDuration());

    return true;
}

bool JSVideo::JSGetter_bitrate(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setDouble(this->m_Video->getBitrate());

    return true;
}


bool JSVideo::JSGetter_metadata(JSContext *cx, JS::MutableHandleValue vp)
{
    JSAVSource::GetMetadata(cx, this->m_Video, vp );

    return true;
}

bool JSVideo::JSGetter_position(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setDouble(this->m_Video->getClock());

    return true;
}

bool JSVideo::JSSetter_position(JSContext *cx, JS::MutableHandleValue vp)
{
    if (vp.isNumber()) {
        this->m_Video->seek(vp.toNumber());
    }

    return true;
}

JSVideo *Constructor(JSContext *cx, JS::CallArgs &args,
    JS::HandleObject obj)
{
    JS::RootedObject canvas(cx);
    if (!JS_ConvertArguments(cx, args, "o", canvas.address())) {
        return nullptr;
    }

    JSCanvas *jscanvas = JSCanvas::GetInstance(canvas);
    if (!jscanvas) {
        JS_ReportError(cx, "Video constructor argument must be Canvas");
        return nullptr;
    }

    CanvasHandler *handler = jscanvas->getHandler();

    if (!handler) {
        JS_ReportError(cx, "Video constructor argument must be Canvas");
        return nullptr;
    }

    CanvasContext *ncc = handler->getContext();
    if (ncc == NULL || ncc->m_Mode != CanvasContext::CONTEXT_2D) {
        JS_ReportError( cx,
            "Invalid canvas context. Did you called canvas.getContext('2d') ?");
        return nullptr;
    }
    JSVideo *vid = new JSVideo(static_cast<Canvas2DContext *>(ncc), cx);
    vid->root();

    return vid;
}


void JSVideo::releaseAudioNode()
{
    if (m_AudioNode) {
        JSAudioNode *node = JSAudioNode::GetObject(m_AudioNode, m_Cx);

        NJS->unrootObject(m_AudioNode);

        if (node) {
            JS_SetReservedSlot(node->getJSObject(), 0, JSVAL_NULL);
            // will remove the source from JSAudio and Audio
            delete node;
        }

        m_AudioNode = nullptr;
    }
}
void JSVideo::close()
{
    // Stop the audio first.
    // This will also release the JS audio node
    this->stopAudio();

    m_Video->close();
}
JSPropertySpec *JSVideo::ListProperties()
{
    static JSPropertySpec props[] = {
        CLASSMAPPER_PROP_G(JSVideo, canvas),
        CLASSMAPPER_PROP_G(JSVideo, width),
        CLASSMAPPER_PROP_G(JSVideo, height),
        CLASSMAPPER_PROP_G(JSVideo, duration),
        CLASSMAPPER_PROP_G(JSVideo, bitrate),
        CLASSMAPPER_PROP_G(JSVideo, metadata),

        CLASSMAPPER_PROP_GS(JSVideo, position),

        JS_PS_END
    };

    return props;
}

JSFunctionSpec *JSVideo::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSVideo, play, 0),
        CLASSMAPPER_FN(JSVideo, pause, 0),
        CLASSMAPPER_FN(JSVideo, stop, 0),
        CLASSMAPPER_FN(JSVideo, close, 0),
        CLASSMAPPER_FN(JSVideo, open, 1),
        CLASSMAPPER_FN(JSVideo, getAudioNode, 0),
        CLASSMAPPER_FN(JSVideo, nextFrame, 0),
        CLASSMAPPER_FN(JSVideo, prevFrame, 0),
        CLASSMAPPER_FN(JSVideo, frameAt, 1),
        CLASSMAPPER_FN(JSVideo, setSize, 2),
        JS_FS_END
    };

    return funcs;
}

JSVideo::~JSVideo()
{
    JSAutoRequest ar(m_Cx);
    m_IsDestructing = true;

    // Release JS AudioNode
    this->stopAudio();

    NJS->unrootObject(this->getJSObject());

    if (m_ArrayContent != NULL) {
        free(m_ArrayContent);
    }

    delete m_Video;
}

void JSVideo::RegisterObject(JSContext *cx)
{
    JSVideo::ExposeClass<1>(cx, "Video");

}

} // namespace Binding
} // namespace Nidium


