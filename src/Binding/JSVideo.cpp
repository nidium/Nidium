/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSVideo.h"

#include "Binding/JSAudioContext.h"
#include "Binding/JSAudioNode.h"
#include "Binding/JSCanvas2DContext.h"
#include "Binding/JSCanvas.h"
#include "Binding/JSUtils.h"
#include "Graphics/SkiaContext.h"
#include "Graphics/CanvasHandler.h"

using namespace Nidium::AV;
using Nidium::Graphics::CanvasHandler;
using Nidium::Graphics::SkiaContext;
using Nidium::Graphics::CanvasContext;
using Nidium::Graphics::CanvasHandler;

namespace Nidium {
namespace Binding {

JSVideo::JSVideo(Canvas2DContext *canvasCtx, JSContext *cx)
    : m_CanvasCtx(canvasCtx)
{
    m_Video = new Video((ape_global *)JS_GetContextPrivate(cx));
    m_Video->frameCallback(JSVideo::FrameCallback, this);
    m_CanvasCtx->getHandler()->addListener(this);

    this->listenSourceEvents(m_Video);
}

void JSVideo::onSourceMessage(const SharedMessages::Message &msg)
{
    if (msg.event() == NIDIUM_EVENT(CanvasHandler, RESIZE_EVENT)
        && (m_Width == -1 || m_Height == -1)) {
        this->setSize(m_Width, m_Height);
    } else {
        if (msg.event() == SOURCE_EVENT_PLAY) {
            this->setSize(m_Width, m_Height);
        }
    }
}

void JSVideo::FrameCallback(uint8_t *data, void *custom)
{
    JSVideo *v             = (JSVideo *)custom;
    CanvasHandler *handler = v->m_CanvasCtx->getHandler();
    SkiaContext *surface   = v->m_CanvasCtx->getSkiaContext();
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

    v->fireJSEvent("frame", &evjsval);
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

bool JSVideo::JS_getAudioNode(JSContext *cx, JS::CallArgs &args)
{
    JSAudioContext *jaudio = JSAudioContext::GetContext();
    if (!jaudio) {
        jaudio = JSAudioContext::GetContext(cx, 0, 0, 0);
    }

    if (this->m_AudioNode != nullptr) {
        JS::RootedObject retObj(cx, this->m_AudioNode->getJSObject());
        args.rval().setObjectOrNull(retObj);
        return true;
    }

    VideoAudioSource *source = m_Video->getAudioNode(jaudio->m_Audio);

    if (source != nullptr) {
        this->m_AudioNode = new JSAudioNodeSourceVideo(cx, this, source);
        args.rval().setObjectOrNull(this->m_AudioNode->getJSObject());
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
    if (m_Video->m_CodecCtx) {
        vp.setInt32(m_Video->m_CodecCtx->width);
    } else {
        vp.setInt32(-1);
    }

    return true;
}

bool JSVideo::JSGetter_height(JSContext *cx, JS::MutableHandleValue vp)
{
    if (m_Video->m_CodecCtx) {
        vp.setInt32(this->m_Video->m_CodecCtx->height);
    } else {
        vp.setInt32(-1);
    }

    return true;
}

JSVideo *JSVideo::Constructor(JSContext *cx, JS::CallArgs &args,
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
        JS_ReportError(
            cx,
            "Invalid canvas context. Did you called canvas.getContext('2d') ?");
        return nullptr;
    }

    JSVideo *vid    = new JSVideo(static_cast<Canvas2DContext *>(ncc), cx);
    vid->m_Instance = obj;
    vid->root();

    return vid;
}

bool JSVideo::JS_close(JSContext *cx, JS::CallArgs &args)
{
    // The audio node needs to be released too
    delete m_AudioNode;
    m_AudioNode = nullptr;

    return JSAVSourceBase::JS__close(cx, args);
}

JSPropertySpec *JSVideo::ListProperties()
{
    static JSPropertySpec props[] = { CLASSMAPPER_PROP_G(JSVideo, canvas),
                                      CLASSMAPPER_PROP_G(JSVideo, width),
                                      CLASSMAPPER_PROP_G(JSVideo, height),

                                      CLASSMAPPER_PROP_GS(JSVideo, position),
                                      CLASSMAPPER_PROP_G(JSVideo, duration),
                                      CLASSMAPPER_PROP_G(JSVideo, metadata),
                                      CLASSMAPPER_PROP_G(JSVideo, bitrate),
                                      JS_PS_END };

    return props;
}

JSFunctionSpec *JSVideo::ListMethods()
{
    static JSFunctionSpec funcs[] = { CLASSMAPPER_FN(JSVideo, open, 1),
                                      CLASSMAPPER_FN(JSVideo, play, 0),
                                      CLASSMAPPER_FN(JSVideo, pause, 0),
                                      CLASSMAPPER_FN(JSVideo, stop, 0),

                                      CLASSMAPPER_FN(JSVideo, close, 0),
                                      CLASSMAPPER_FN(JSVideo, getAudioNode, 0),
                                      CLASSMAPPER_FN(JSVideo, nextFrame, 0),
                                      CLASSMAPPER_FN(JSVideo, prevFrame, 0),
                                      CLASSMAPPER_FN(JSVideo, frameAt, 1),
                                      CLASSMAPPER_FN(JSVideo, setSize, 2),
                                      JS_FS_END };

    return funcs;
}

JSVideo::~JSVideo()
{
    JSAutoRequest ar(m_Cx);
    m_IsReleased = true;

    delete m_AudioNode;

    delete m_Video;
}

void JSVideo::RegisterObject(JSContext *cx)
{
    JSVideo::ExposeClass<1>(cx, "Video");
}

void JSVideo::RegisterAllObjects(JSContext *cx)
{
    JSVideo::RegisterObject(cx);
    JSAudioNodeSourceVideo::RegisterObject(cx);
}

} // namespace Binding
} // namespace Nidium
