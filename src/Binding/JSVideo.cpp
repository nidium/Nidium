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

static bool
nidium_Video_constructor(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_video_play(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_video_pause(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_video_stop(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_video_close(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_video_open(JSContext *cx, unsigned argc, JS::Value *vp);
static bool
nidium_video_get_audionode(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_video_nextframe(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_video_prevframe(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_video_frameat(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_video_setsize(JSContext *cx, unsigned argc, JS::Value *vp);

static bool nidium_video_prop_getter(JSContext *cx,
                                     JS::HandleObject obj,
                                     uint8_t id,
                                     JS::MutableHandleValue vp);
static bool nidium_video_prop_setter(JSContext *cx,
                                     JS::HandleObject obj,
                                     uint8_t id,
                                     bool strict,
                                     JS::MutableHandleValue vp);

static void Video_Finalize(JSFreeOp *fop, JSObject *obj);

static JSFunctionSpec Video_funcs[]
    = { JS_FN("play", nidium_video_play, 0, NIDIUM_JS_FNPROPS),
        JS_FN("pause", nidium_video_pause, 0, NIDIUM_JS_FNPROPS),
        JS_FN("stop", nidium_video_stop, 0, NIDIUM_JS_FNPROPS),
        JS_FN("close", nidium_video_close, 0, NIDIUM_JS_FNPROPS),
        JS_FN("open", nidium_video_open, 1, NIDIUM_JS_FNPROPS),
        JS_FN("getAudioNode", nidium_video_get_audionode, 0, NIDIUM_JS_FNPROPS),
        JS_FN("nextFrame", nidium_video_nextframe, 0, NIDIUM_JS_FNPROPS),
        JS_FN("prevFrame", nidium_video_prevframe, 0, NIDIUM_JS_FNPROPS),
        JS_FN("frameAt", nidium_video_frameat, 1, NIDIUM_JS_FNPROPS),
        JS_FN("setSize", nidium_video_setsize, 2, NIDIUM_JS_FNPROPS),
        JS_FS_END };

static JSClass Video_class
    = { "Video",
        JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
        JS_PropertyStub,
        JS_DeletePropertyStub,
        JS_PropertyStub,
        JS_StrictPropertyStub,
        JS_EnumerateStub,
        JS_ResolveStub,
        JS_ConvertStub,
        Video_Finalize,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        JSCLASS_NO_INTERNAL_MEMBERS };

template <>
JSClass *JSExposer<JSVideo>::jsclass = &Video_class;

static JSPropertySpec Video_props[] = {

    NIDIUM_JS_PSG("width", VIDEO_PROP_WIDTH, nidium_video_prop_getter),
    NIDIUM_JS_PSG("height", VIDEO_PROP_HEIGHT, nidium_video_prop_getter),

    NIDIUM_JS_PSG("duration", SOURCE_PROP_DURATION, nidium_video_prop_getter),
    NIDIUM_JS_PSG("metadata", SOURCE_PROP_METADATA, nidium_video_prop_getter),
    NIDIUM_JS_PSG("bitrate", SOURCE_PROP_BITRATE, nidium_video_prop_getter),

    NIDIUM_JS_PSGS("position",
                   SOURCE_PROP_POSITION,
                   nidium_video_prop_getter,
                   nidium_video_prop_setter),


    JS_PS_END
};

static bool nidium_video_prop_getter(JSContext *cx,
                                     JS::HandleObject obj,
                                     uint8_t id,
                                     JS::MutableHandleValue vp)
{
    JSVideo *v = (JSVideo *)JS_GetPrivate(obj);
    if (v == NULL) {
        return false;
    }

    switch (id) {
        case VIDEO_PROP_WIDTH:
            vp.setInt32(v->m_Video->m_CodecCtx->width);
            break;
        case VIDEO_PROP_HEIGHT:
            vp.setInt32(v->m_Video->m_CodecCtx->height);
            break;
        default:
            return JSAVSource::PropGetter(v->m_Video, cx, id, vp);
            break;
    }

    return true;
}

static bool nidium_video_prop_setter(JSContext *cx,
                                     JS::HandleObject obj,
                                     uint8_t id,
                                     bool strict,
                                     JS::MutableHandleValue vp)
{
    JSVideo *v = (JSVideo *)JS_GetPrivate(obj);
    if (v == NULL) {
        return false;
    }

    return JSAVSource::PropSetter(v->m_Video, id, vp);
}

JSVideo::JSVideo(JS::HandleObject obj,
                 Canvas2DContext *canvasCtx,
                 JSContext *cx)
    : JSExposer<JSVideo>(obj, cx), m_Video(NULL), m_AudioNode(NULL),
      m_ArrayContent(NULL), m_Width(-1), m_Height(-1), m_Left(0), m_Top(0),
      m_IsDestructing(false), m_CanvasCtx(canvasCtx), m_Cx(cx)
{
    m_Video = new Video((ape_global *)JS_GetContextPrivate(cx));
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

static bool nidium_video_play(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSVideo, &Video_class);

    CppObj->m_Video->play();

    return true;
}

static bool nidium_video_pause(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSVideo, &Video_class);

    CppObj->m_Video->pause();

    return true;
}

static bool nidium_video_stop(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSVideo, &Video_class);

    CppObj->m_Video->stop();

    return true;
}

static bool nidium_video_close(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSVideo, &Video_class);

    CppObj->close();

    return true;
}

static bool nidium_video_open(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSVideo *v;
    NIDIUM_JS_PROLOGUE_CLASS(JSVideo, &Video_class);
    v = CppObj;

    JS::RootedValue src(cx, args[0]);
    int ret = -1;

    if (src.isString()) {
        JSAutoByteString csrc(cx, src.toString());
        ret = v->m_Video->open(csrc.ptr());
    } else if (src.isObject()) {
        JS::RootedObject arrayBuff(cx, src.toObjectOrNull());

        if (!JS_IsArrayBufferObject(arrayBuff)) {
            JS_ReportError(cx, "Data is not an ArrayBuffer\n");
            return false;
        }

        int length        = JS_GetArrayBufferByteLength(arrayBuff);
        v->m_ArrayContent = JS_StealArrayBufferContents(cx, arrayBuff);
        if (v->m_Video->open(v->m_ArrayContent, length) < 0) {
            args.rval().setBoolean(false);
            return true;
        }
    }

    args.rval().setBoolean(false);

    return true;
}

static bool
nidium_video_get_audionode(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSAudio *jaudio = JSAudio::GetContext();
    JSVideo *v;

    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(JSVideo, &Video_class);

    v = CppObj;

    if (!jaudio) {
        JS_ReportError(cx, "No Audio context");
        args.rval().setNull();
        return false;
    }

    if (v->m_AudioNode.get()) {
        JS::RootedObject retObj(cx, v->m_AudioNode);
        args.rval().setObjectOrNull(retObj);
        return true;
    }

    AudioSource *source = v->m_Video->getAudioNode(jaudio->m_Audio);

    if (source != NULL) {
        JS::RootedObject audioNode(
            cx, JS_NewObjectForConstructor(cx, &AudioNode_class, args));
        v->m_AudioNode = audioNode;

        JSAudioNode *node
            = new JSAudioNode(audioNode, cx, Audio::SOURCE,
                              static_cast<class AudioNode *>(source), jaudio);

        JS::RootedString name(cx, JS_NewStringCopyN(cx, "video-source", 12));
        JS::RootedObject an(cx, v->m_AudioNode);
        jaudio->initNode(node, an, name);

        JS_SetReservedSlot(node->getJSObject(), 0,
                           OBJECT_TO_JSVAL(v->getJSObject()));
        JS::RootedObject retObj(cx, v->m_AudioNode);

        args.rval().setObjectOrNull(retObj);
    } else {
        args.rval().setNull();
    }

    return true;
}

static bool nidium_video_nextframe(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSVideo, &Video_class);

    CppObj->m_Video->nextFrame();

    return true;
}

static bool nidium_video_prevframe(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSVideo, &Video_class);

    CppObj->m_Video->prevFrame();

    return true;
}

static bool nidium_video_frameat(JSContext *cx, unsigned argc, JS::Value *vp)
{
    double time;
    bool keyframe = false;

    NIDIUM_JS_PROLOGUE_CLASS(JSVideo, &Video_class);

    if (!JS_ConvertArguments(cx, args, "db", &time, keyframe)) {
        return true;
    }

    CppObj->m_Video->frameAt(time, keyframe);

    return true;
}

static bool nidium_video_setsize(JSContext *cx, unsigned argc, JS::Value *vp)
{
    uint32_t width, height;
    NIDIUM_JS_PROLOGUE_CLASS(JSVideo, &Video_class);

    NIDIUM_JS_CHECK_ARGS("setSize", 2)

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

    CppObj->setSize(width, height);

    return true;
}

static bool
nidium_Video_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JS::RootedObject canvas(cx);
    if (!JS_ConvertArguments(cx, args, "o", canvas.address())) {
        return true;
    }

    JSCanvas *jscanvas = JSCanvas::GetInstance(canvas);
    if (!jscanvas) {
        JS_ReportError(cx, "Video constructor argument must be Canvas");
        return false;
    }

    CanvasHandler *handler = jscanvas->getHandler();

    if (!handler) {
        JS_ReportError(cx, "Video constructor argument must be Canvas");
        return false;
    }

    CanvasContext *ncc = handler->getContext();
    if (ncc == NULL || ncc->m_Mode != CanvasContext::CONTEXT_2D) {
        JS_ReportError(
            cx,
            "Invalid canvas context. Did you called canvas.getContext('2d') ?");
        return false;
    }
    JSContext *m_Cx = cx;
    JS::RootedObject ret(cx,
                         JS_NewObjectForConstructor(cx, &Video_class, args));
    NJS->rootObjectUntilShutdown(ret);
    JS_DefineFunctions(cx, ret, Video_funcs);
    JS_DefineProperties(cx, ret, Video_props);
    JSVideo *v = new JSVideo(ret, (Canvas2DContext *)ncc, cx);
    JS_SetPrivate(ret, v);

    JS_DefineProperty(cx, ret, "canvas", args[0],
                      JSPROP_PERMANENT | JSPROP_READONLY);

    args.rval().setObjectOrNull(ret);

    return true;
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

static void Video_Finalize(JSFreeOp *fop, JSObject *obj)
{
    JSVideo *v = (JSVideo *)JS_GetPrivate(obj);

    if (v != NULL) {
        JS_SetPrivate(obj, nullptr);
        delete v;
    }
}


NIDIUM_JS_OBJECT_EXPOSE(Video);

} // namespace Binding
} // namespace Nidium


