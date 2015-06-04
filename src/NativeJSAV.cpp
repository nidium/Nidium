#include "NativeJSAV.h"
#include "NativeSharedMessages.h"
#include "NativeJSThread.h"
#include "NativeJS.h"
#include <NativeJSConsole.h>

#include "NativeJSCanvas.h"
#include "NativeCanvasHandler.h"
#include "NativeCanvas2DContext.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
}

// TODO : Need to handle nodes GC, similar to
//        https://dvcs.w3.org/hg/audio/raw-file/tip/webaudio/specification.html#lifetime-AudioNode
// TODO : Need to handle video GC
// TODO : When stop/pause/kill fade out sound

NativeJSAudio *NativeJSAudio::instance = NULL;
extern JSClass Canvas_class;

#define NJS (NativeJS::getNativeClass(m_Cx))
#define JS_PROPAGATE_ERROR(cx, ...)\
JS_ReportError(cx, __VA_ARGS__); \
if (!JS_ReportPendingException(cx)) {\
    JS_ClearPendingException(cx); \
}

#define JSNATIVE_AV_GET_NODE(type, var)\
    JSNATIVE_PROLOGUE_CLASS(NativeJSAudioNode, &AudioNode_class); \
    var = static_cast<type *>(CppObj->node);

#define CHECK_INVALID_CTX(obj) if (!obj) {\
JS_ReportError(cx, "Invalid NativeAudio context"); \
return false; \
}

extern void reportError(JSContext *cx, const char *message, JSErrorReport *report);

static void AudioNode_Finalize(JSFreeOp *fop, JSObject *obj);
static void AudioContext_Finalize(JSFreeOp *fop, JSObject *obj);

static bool native_Audio_constructor(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audio_getcontext(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audio_run(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audio_pFFT(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audio_load(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audio_createnode(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audio_connect(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audio_disconnect(JSContext *cx, unsigned argc, JS::Value *vp);

static bool native_audiothread_print(JSContext *cx, unsigned argc, JS::Value *vp);

static bool native_AudioNode_constructor(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audionode_set(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audionode_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audionode_custom_set(JSContext *cx, unsigned argc, JS::Value *vp);
//static bool native_audionode_custom_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audionode_custom_threaded_set(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audionode_custom_threaded_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audionode_custom_threaded_send(JSContext *cx, unsigned argc, JS::Value *vp);

static bool native_audionode_source_open(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audionode_source_play(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audionode_source_pause(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audionode_source_stop(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audionode_source_close(JSContext *cx, unsigned argc, JS::Value *vp);

static bool native_audionode_custom_source_play(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audionode_custom_source_pause(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_audionode_custom_source_stop(JSContext *cx, unsigned argc, JS::Value *vp);

static bool native_audio_prop_setter(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp);
static bool native_audio_prop_getter(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp);
static bool native_audionode_custom_prop_setter(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp);
static bool native_audionode_source_prop_setter(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp);
static bool native_audionode_source_prop_getter(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp);
static bool native_audionode_custom_source_prop_setter(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp);

static JSClass messageEvent_class = {
    "ThreadMessageEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass Audio_class = {
    "Audio", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass AudioContext_class = {
    "AudioContext", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, AudioContext_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *NativeJSExposer<NativeJSAudio>::jsclass = &AudioContext_class;

static JSClass global_AudioThread_class = {
    "_GLOBALAudioThread", JSCLASS_GLOBAL_FLAGS | JSCLASS_IS_GLOBAL,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass AudioNodeLink_class = {
    "AudioNodeLink", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass AudioNodeEvent_class = {
    "AudioNodeEvent", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass AudioNode_class = {
    "AudioNode", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, AudioNode_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *NativeJSExposer<NativeJSAudioNode>::jsclass = &AudioNode_class;

static JSClass AudioNode_threaded_class = {
    "AudioNodeThreaded", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSPropertySpec AudioContext_props[] = {
    {"volume", JSPROP_ENUMERATE|JSPROP_PERMANENT,
        NATIVE_JS_GETTER(AUDIO_PROP_VOLUME, native_audio_prop_getter),
        NATIVE_JS_SETTER(AUDIO_PROP_VOLUME, native_audio_prop_setter)},
    {"bufferSize", JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY,
        NATIVE_JS_GETTER(AUDIO_PROP_BUFFERSIZE, native_audio_prop_getter),
        JSOP_NULLWRAPPER},
    {"channels", JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY,
        NATIVE_JS_GETTER(AUDIO_PROP_CHANNELS, native_audio_prop_getter),
        JSOP_NULLWRAPPER},
    {"sampleRate", JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY,
        NATIVE_JS_GETTER(AUDIO_PROP_SAMPLERATE, native_audio_prop_getter),
        JSOP_NULLWRAPPER},
    JS_PS_END
};

static JSPropertySpec AudioNode_props[] = {
    /* type, input, ouput readonly props are created in createnode function */
    JS_PS_END
};

static JSPropertySpec AudioNodeEvent_props[] = {
    {"data", 0,
         NATIVE_JS_STUBGETTER(),
         JSOP_NULLWRAPPER},
    {"size", 0,
        NATIVE_JS_STUBGETTER(),
        JSOP_NULLWRAPPER},
    JS_PS_END
};

static JSPropertySpec AudioNodeCustom_props[] = {
    {"process", 0,
        NATIVE_JS_STUBGETTER(),
        NATIVE_JS_SETTER(NODE_CUSTOM_PROP_PROCESS, native_audionode_custom_prop_setter)},
    {"init", 0,
        NATIVE_JS_STUBGETTER(),
        NATIVE_JS_SETTER(NODE_CUSTOM_PROP_INIT, native_audionode_custom_prop_setter)},
    {"setter", 0,
        NATIVE_JS_STUBGETTER(),
        NATIVE_JS_SETTER(NODE_CUSTOM_PROP_SETTER, native_audionode_custom_prop_setter)},
    JS_PS_END
};

static JSFunctionSpec AudioContext_funcs[] = {
    JS_FN("run", native_audio_run, 1, 0),
    JS_FN("load", native_audio_load, 1, 0),
    JS_FN("createNode", native_audio_createnode, 3, 0),
    JS_FN("connect", native_audio_connect, 2, 0),
    JS_FN("disconnect", native_audio_disconnect, 2, 0),
    JS_FN("pFFT", native_audio_pFFT, 2, 0),
    JS_FS_END
};

static JSFunctionSpec Audio_static_funcs[] = {
    JS_FN("getContext", native_audio_getcontext, 3, 0),
    JS_FS_END
};

static JSFunctionSpec AudioNode_funcs[] = {
    JS_FN("set", native_audionode_set, 2, 0),
    JS_FN("get", native_audionode_get, 1, 0),
    JS_FS_END
};

static JSFunctionSpec AudioNodeCustom_funcs[] = {
    JS_FN("set", native_audionode_custom_set, 2, 0),
    //JS_FN("get", native_audionode_custom_get, 1, 0),
    JS_FS_END
};

static JSFunctionSpec AudioNodeCustom_threaded_funcs[] = {
    JS_FN("set", native_audionode_custom_threaded_set, 2, 0),
    JS_FN("get", native_audionode_custom_threaded_get, 1, 0),
    JS_FN("send", native_audionode_custom_threaded_send, 2, 0),
    JS_FS_END
};

static JSFunctionSpec AudioNodeSource_funcs[] = {
    JS_FN("open", native_audionode_source_open, 1, 0),
    JS_FN("play", native_audionode_source_play, 0, 0),
    JS_FN("pause", native_audionode_source_pause, 0, 0),
    JS_FN("stop", native_audionode_source_stop, 0, 0),
    JS_FN("close", native_audionode_source_close, 0, 0),
    JS_FS_END
};

static JSFunctionSpec AudioNodeCustomSource_funcs[] = {
    JS_FN("play", native_audionode_custom_source_play, 0, 0),
    JS_FN("pause", native_audionode_custom_source_pause, 0, 0),
    JS_FN("stop", native_audionode_custom_source_stop, 0, 0),
    JS_FS_END
};

static JSPropertySpec AudioNodeSource_props[] = {
    {"position", JSPROP_ENUMERATE|JSPROP_PERMANENT,
        NATIVE_JS_GETTER(SOURCE_PROP_POSITION, native_audionode_source_prop_getter),
        NATIVE_JS_SETTER(SOURCE_PROP_POSITION, native_audionode_source_prop_setter)},
    {"duration", JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY,
        NATIVE_JS_GETTER(SOURCE_PROP_DURATION, native_audionode_source_prop_getter),
        JSOP_NULLWRAPPER},
    {"metadata", JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY,
        NATIVE_JS_GETTER(SOURCE_PROP_METADATA, native_audionode_source_prop_getter),
        JSOP_NULLWRAPPER},
     {"bitrate", JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY,
        NATIVE_JS_GETTER(SOURCE_PROP_BITRATE, native_audionode_source_prop_getter),
        JSOP_NULLWRAPPER},
    JS_PS_END
};

static JSPropertySpec AudioNodeCustomSource_props[] = {
    {"seek", JSPROP_ENUMERATE|JSPROP_PERMANENT,
        NATIVE_JS_STUBGETTER(),
        NATIVE_JS_SETTER(CUSTOM_SOURCE_PROP_SEEK, native_audionode_custom_source_prop_setter)},
#if 0
    {"duration",
        JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY,
        NATIVE_JS_GETTER(SOURCE_PROP_DURATION, native_audionode_source_prop_getter),
        JSOP_NULLWRAPPER},
    {"metadata",
        JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY,
        NATIVE_JS_GETTER(SOURCE_PROP_METADATA, native_audionode_source_prop_getter),
        JSOP_NULLWRAPPER},
     {"bitrate",
        JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY,
        NATIVE_JS_GETTER(SOURCE_PROP_BITRATE, native_audionode_source_prop_getter),
        JSOP_NULLWRAPPER},
#endif
    JS_PS_END
};

static JSFunctionSpec glob_funcs_threaded[] = {
    JS_FN("echo", native_audiothread_print, 1, 0),
    JS_FS_END
};


static bool native_Video_constructor(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_video_play(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_video_pause(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_video_stop(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_video_close(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_video_open(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_video_get_audionode(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_video_nextframe(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_video_prevframe(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_video_frameat(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_video_setsize(JSContext *cx, unsigned argc, JS::Value *vp);

static bool native_video_prop_getter(JSContext *cx, JS::HandleObject obj, uint8_t id, JS::MutableHandleValue vp);
static bool native_video_prop_setter(JSContext *cx, JS::HandleObject obj, uint8_t id, bool strict, JS::MutableHandleValue vp);

static void Video_Finalize(JSFreeOp *fop, JSObject *obj);

static JSFunctionSpec Video_funcs[] = {
    JS_FN("play", native_video_play, 0, 0),
    JS_FN("pause", native_video_pause, 0, 0),
    JS_FN("stop", native_video_stop, 0, 0),
    JS_FN("close", native_video_close, 0, 0),
    JS_FN("open", native_video_open, 1, 0),
    JS_FN("getAudioNode", native_video_get_audionode, 0, 0),
    JS_FN("nextFrame", native_video_nextframe, 0, 0),
    JS_FN("prevFrame", native_video_prevframe, 0, 0),
    JS_FN("frameAt", native_video_frameat, 1, 0),
    JS_FN("setSize", native_video_setsize, 2, 0),
    JS_FS_END
};

static JSClass Video_class = {
    "Video", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Video_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *NativeJSExposer<NativeJSVideo>::jsclass = &Video_class;

static JSPropertySpec Video_props[] = {
    {"width", JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,
        NATIVE_JS_GETTER(VIDEO_PROP_WIDTH, native_video_prop_getter),
        JSOP_NULLWRAPPER},
    {"height", JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT,
        NATIVE_JS_GETTER(VIDEO_PROP_HEIGHT, native_video_prop_getter),
        JSOP_NULLWRAPPER},
    {"position",  JSPROP_ENUMERATE|JSPROP_PERMANENT,
        NATIVE_JS_GETTER(SOURCE_PROP_POSITION, native_video_prop_getter),
        NATIVE_JS_SETTER(SOURCE_PROP_POSITION, native_video_prop_setter)},
    {"duration", JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY,
        NATIVE_JS_GETTER(SOURCE_PROP_DURATION, native_video_prop_getter),
        JSOP_NULLWRAPPER},
    {"metadata", JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY,
        NATIVE_JS_GETTER(SOURCE_PROP_METADATA, native_video_prop_getter),
        JSOP_NULLWRAPPER},
     {"bitrate", JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY,
        NATIVE_JS_GETTER(SOURCE_PROP_BITRATE, native_video_prop_getter),
        JSOP_NULLWRAPPER},
    {"onframe", JSPROP_ENUMERATE|JSPROP_PERMANENT,
        NATIVE_JS_STUBGETTER(),
        JSOP_NULLWRAPPER},
    {"canvas", JSPROP_ENUMERATE|JSPROP_PERMANENT,
        NATIVE_JS_STUBGETTER(),
        JSOP_NULLWRAPPER},
    JS_PS_END
};

static int FFT(int dir, int nn, double *x, double *y)
{
   long m, i, i1, j, k, i2, l, l1, l2;
   double c1, c2, tx, ty, t1, t2, u1, u2, z;

   m = log2(nn);

   /* Do the bit reversal */
   i2 = nn >> 1;
   j = 0;
   for (i = 0; i < nn - 1; i++) {
      if (i < j) {
         tx = x[i];
         ty = y[i];
         x[i] = x[j];
         y[i] = y[j];
         x[j] = tx;
         y[j] = ty;
      }
      k = i2;
      while (k <= j) {
         j -= k;
         k >>= 1;
      }
      j += k;
   }

   /* Compute the FFT */
   c1 = -1.0;
   c2 = 0.0;
   l2 = 1;
   for (l = 0; l < m; l++) {
      l1 = l2;
      l2 <<= 1;
      u1 = 1.0;
      u2 = 0.0;
      for (j = 0; j < l1; j++) {
         for (i = j; i < nn; i += l2) {
            i1 = i + l1;
            t1 = u1 * x[i1] - u2 * y[i1];
            t2 = u1 * y[i1] + u2 * x[i1];
            x[i1] = x[i] - t1;
            y[i1] = y[i] - t2;
            x[i] += t1;
            y[i] += t2;
         }
         z =  u1 * c1 - u2 * c2;
         u2 = u1 * c2 + u2 * c1;
         u1 = z;
      }
      c2 = sqrt((1.0 - c1) / 2.0);
      if (dir == 1)
         c2 = -c2;
      c1 = sqrt((1.0 + c1) / 2.0);
   }

   /* Scaling for forward transform */
   if (dir == 1) {
      for (i = 0; i < nn; i++) {
         x[i] /= (double)nn;
         y[i] /= (double)nn;
      }
   }

   return true;
}

const char *NativeJSAVEventRead(int ev)
{
    switch (ev) {
        case SOURCE_EVENT_PAUSE:
            return "onpause";
        break;
        case SOURCE_EVENT_PLAY:
            return "onplay";
        break;
        case SOURCE_EVENT_STOP:
            return "onstop";
        break;
        case SOURCE_EVENT_EOF:
            return "onend";
        break;
        case SOURCE_EVENT_ERROR:
            return "onerror";
        break;
        case SOURCE_EVENT_BUFFERING:
            return "onbuffering";
        break;
        case SOURCE_EVENT_READY:
            return "onready";
        break;
        default:
            return NULL;
        break;
    }
}

static void native_av_thread_message(JSContext *cx, JS::HandleObject obj, const NativeSharedMessages::Message &msg)
{
    JS::RootedValue jscbk(cx);
    JS::RootedValue rval(cx);
    if (msg.event() == CUSTOM_SOURCE_SEND) {
        native_thread_msg *ptr = static_cast<struct native_thread_msg *>(msg.dataPtr());

        if (JS_GetProperty(cx, obj, "onmessage", &jscbk) &&
            !jscbk.isPrimitive() && JS_ObjectIsCallable(cx, &jscbk.toObject())) {

            JS::RootedValue inval(cx, JSVAL_NULL);

            if (!JS_ReadStructuredClone(cx, ptr->data, ptr->nbytes,
                JS_STRUCTURED_CLONE_VERSION, &inval, nullptr, NULL)) {
                JS_PROPAGATE_ERROR(cx, "Failed to transfer custom node message to audio thread");
                return;
            }
            JS::RootedObject event(cx, JS_NewObject(cx, &messageEvent_class, JS::NullPtr(), JS::NullPtr()));
            JS_DefineProperty(cx, event, "data", inval, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
            JS::RootedValue jsvalEvent(cx, OBJECT_TO_JSVAL(event));
            JS_CallFunctionValue(cx, event, jscbk, jsvalEvent, &rval);
        }
        delete ptr;
    } else {
        NativeAVSourceEvent *cmsg = static_cast<struct NativeAVSourceEvent*>(msg.dataPtr());

        const char *prop = NativeJSAVEventRead(msg.event());
        if (!prop) {
            // delete cmsg; // XXX : Unknown message. Don't delete it.
            return;
        }

        if (JS_GetProperty(cx, obj, prop, &jscbk) &&
            !jscbk.isPrimitive() &&
            JS_ObjectIsCallable(cx, jscbk.toObjectOrNull())) {

            if (cmsg->ev == SOURCE_EVENT_ERROR) {
                JS::AutoValueArray<2> event(cx);
                int errorCode = cmsg->args[0].toInt();
                const char *errorStr = NativeAVErrorsStr[errorCode];

                event[0].set(INT_TO_JSVAL(errorCode));
                event[1].set(STRING_TO_JSVAL(JS_NewStringCopyN(cx, errorStr, strlen(errorStr))));

                JS_CallFunctionValue(cx, obj, jscbk, event, &rval);
            } else if (cmsg->ev == SOURCE_EVENT_BUFFERING) {
                JS::AutoValueArray<2> event(cx);
                event[0].set(INT_TO_JSVAL(cmsg->args[0].toInt()));
                event[1].set(INT_TO_JSVAL(cmsg->args[1].toInt()));
                event[2].set(INT_TO_JSVAL(cmsg->args[2].toInt()));
                JS_CallFunctionValue(cx, obj, jscbk, event, &rval);
            } else {
                JS_CallFunctionValue(cx, obj, jscbk, JS::HandleValueArray::empty(), &rval);
            }
        }

        delete cmsg;
    }
}

bool JSTransferableFunction::prepare(JSContext *cx, JS::HandleValue val)
{
    if (!JS_WriteStructuredClone(cx, val, &m_Data, &m_Bytes, nullptr, nullptr, JS::NullHandleValue)) {
        return false;
    }

    return true;
}

bool JSTransferableFunction::call(JSContext *cx, JS::HandleObject obj, JS::HandleValueArray params, JS::MutableHandleValue rval)
{
    if (m_Fn == JSVAL_VOID) {
        if (m_Data == NULL) {
        	return false;
        }
        m_Fn = JSVAL_VOID;
        if (!this->transfert(cx)) {
            m_Fn = JSVAL_NULL;
            return false;
        } else {
            // Function is transfered
        }
    }
    JS::RootedValue fun(cx, m_Fn.get());
    return JS_CallFunctionValue(cx, obj, fun, params, rval);
}

bool JSTransferableFunction::transfert(JSContext *destCx)
{
    if (m_DestCx != NULL) return false;

    m_DestCx = destCx;
    JS::RootedValue fun(m_DestCx, m_Fn.get());
    bool ok = JS_ReadStructuredClone(m_DestCx, m_Data, m_Bytes,
                JS_STRUCTURED_CLONE_VERSION, &fun, nullptr, NULL);

    JS_ClearStructuredClone(m_Data, m_Bytes, nullptr, NULL);

    m_Data = NULL;
    m_Bytes = 0;

    return ok;
}

JSTransferableFunction::~JSTransferableFunction()
{
    if (m_Data != NULL) {
        JS_ClearStructuredClone(m_Data, m_Bytes, nullptr, NULL);
    }

    if (m_Fn != JSVAL_VOID) {
    	m_Fn = JSVAL_VOID;
    }
}

NativeJSAudio *NativeJSAudio::getContext(JSContext *cx, JS::HandleObject obj, int bufferSize, int channels, int sampleRate)
{
    ape_global *net = static_cast<ape_global *>(JS_GetContextPrivate(cx));
    NativeAudio *audio;

    try {
        audio = new NativeAudio(net, bufferSize, channels, sampleRate);
    } catch (...) {
        return NULL;
    }

    audio->setMainCtx(cx);

    return new NativeJSAudio(audio, cx, obj);
}

void NativeJSAudio::initNode(NativeJSAudioNode *node, JS::HandleObject jnode, JS::HandleString name)
{
    int in = node->node->inCount;
    int out = node->node->outCount;
    JS::RootedValue inputLinks(m_Cx);
    JS::RootedValue outputLinks(m_Cx);
    JS::RootedValue nameVal(m_Cx, STRING_TO_JSVAL(name));

    JS_DefineProperty(m_Cx, jnode, "type", nameVal,
        JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
    JS::RootedObject arrayIn(m_Cx, JS_NewArrayObject(m_Cx, in));
    JS::RootedObject arrayOut(m_Cx, JS_NewArrayObject(m_Cx, out));
    inputLinks.setObjectOrNull(arrayIn);
    outputLinks.setObjectOrNull(arrayOut);

    for (int i = 0; i < in; i++) {
        JS::RootedObject link(m_Cx, JS_NewObject(m_Cx, &AudioNodeLink_class, JS::NullPtr(), JS::NullPtr()));
        JS_SetPrivate(link, node->node->input[i]);

        JS_DefineElement(m_Cx, inputLinks.toObjectOrNull(), i, OBJECT_TO_JSVAL(link), nullptr, nullptr, 0);
    }

    for (int i = 0; i < out; i++) {
        JS::RootedObject link(m_Cx, JS_NewObject(m_Cx, &AudioNodeLink_class, JS::NullPtr(), JS::NullPtr()));
        JS_SetPrivate(link, node->node->output[i]);

        JS_DefineElement(m_Cx, outputLinks.toObjectOrNull(), i, OBJECT_TO_JSVAL(link), nullptr, nullptr, 0);
    }

    if (in > 0) {
        JS_DefineProperty(m_Cx, jnode, "input", inputLinks,
            JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
    }

    if (out > 0) {
        JS_DefineProperty(m_Cx, jnode, "output", outputLinks,
            JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
    }

    node->njs = NJS;

    node->setJSObject(jnode);
    node->setJSContext(m_Cx);

    NJS->rootObjectUntilShutdown(node->getJSObject());
    JS_SetPrivate(jnode, node);
}

NativeJSAudio *NativeJSAudio::getContext()
{
    return NativeJSAudio::instance;
}

NativeJSAudio::NativeJSAudio(NativeAudio *audio, JSContext *cx, JS::HandleObject obj)
    :
      NativeJSExposer<NativeJSAudio>(obj, cx),
      audio(audio), nodes(NULL), gbl(cx, nullptr), rt(NULL), tcx(NULL),
      target(NULL)
{
    NativeJSAudio::instance = this;

    JS_SetPrivate(obj, this);

    NJS->rootObjectUntilShutdown(obj);
    JS::RootedObject ob(cx, obj);
    JS_DefineFunctions(cx, ob, AudioContext_funcs);
    JS_DefineProperties(cx, ob, AudioContext_props);

    NATIVE_PTHREAD_VAR_INIT(&this->m_ShutdownWait)

    this->audio->sharedMsg->postMessage(
            (void *)new NativeAudioNode::CallbackMessage(NativeJSAudio::ctxCallback, NULL, static_cast<void *>(this)),
            NATIVE_AUDIO_NODE_CALLBACK);
}

bool NativeJSAudio::createContext()
{
    if (this->rt == NULL) {
        if ((this->rt = JS_NewRuntime(128L * 1024L * 1024L, JS_USE_HELPER_THREADS)) == NULL) {
            printf("Failed to init JS runtime\n");
            return false;
        }
        //JS_SetRuntimePrivate(rt, this->cx);

        JS_SetGCParameter(this->rt, JSGC_MAX_BYTES, 0xffffffff);
        JS_SetGCParameter(this->rt, JSGC_SLICE_TIME_BUDGET, 15);

        if ((this->tcx = JS_NewContext(this->rt, 8192)) == NULL) {
            printf("Failed to init JS context\n");
            return false;
        }

        JS_SetStructuredCloneCallbacks(this->rt, NativeJS::jsscc);

        JSAutoRequest ar(this->tcx);

        //JS_SetGCParameterForThread(this->tcx, JSGC_MAX_CODE_CACHE_BYTES, 16 * 1024 * 1024);
        JS::CompartmentOptions options;
        options.setVersion(JSVERSION_LATEST);
        JS::RootedObject global(this->tcx, this->gbl = JS_NewGlobalObject(this->tcx,
            &global_AudioThread_class, nullptr, JS::DontFireOnNewGlobalHook, options));
        JSAutoCompartment ac(this->tcx, global);

        js::SetDefaultObjectForContext(this->tcx, global);
        if (!JS_InitStandardClasses(tcx, global)) {
            printf("Failed to init std class\n");
            return false;
        }
        JS_SetErrorReporter(this->tcx, reportError);
        JS_FireOnNewGlobalObject(this->tcx, global);
        JS_DefineFunctions(this->tcx, global, glob_funcs_threaded);
        NativeJSconsole::registerObject(this->tcx);

        JS_SetRuntimePrivate(this->rt, NativeJS::getNativeClass(this->audio->getMainCtx()));

        //JS_SetContextPrivate(this->tcx, static_cast<void *>(this));
    }

    return true;

}

bool NativeJSAudio::run(char *str)
{
    if (!this->tcx) {
        printf("No JS context for audio thread\n");
        return false;
    }
    JSAutoRequest ar(this->tcx);

    JS::RootedValue rval(this->tcx);
    JS::RootedFunction fun(this->tcx);
    JS::RootedObject globalObj(this->tcx, JS::CurrentGlobalOrNull(this->tcx));
    JS::CompileOptions options(this->tcx);
    options.setIntroductionType("audio Thread").setUTF8(true);
    fun = JS_CompileFunction(this->tcx, globalObj, "Audio_run", 0, nullptr, str, strlen(str), options);

    if (!fun) {
        JS_ReportError(this->tcx, "Failed to execute script on audio thread\n");
        return false;
    }
    JS_CallFunction(this->tcx, globalObj, fun, JS::HandleValueArray::empty(), &rval);

    return true;
}

void NativeJSAudio::unroot()
{
    NativeJSAudio::Nodes *nodes = this->nodes;

    NativeJS *njs = NativeJS::getNativeClass(m_Cx);

    if (m_JSObject != NULL) {
        JS::RemoveObjectRoot(njs->cx, &m_JSObject);
    }
    if (this->gbl.get()) {
    	this->gbl = nullptr;
    }

    while (nodes != NULL) {
        nodes->curr->setJSObject(nullptr);
        nodes = nodes->next;
    }
}

void NativeJSAudio::shutdownCallback(NativeAudioNode *dummy, void *custom)
{
    NativeJSAudio *audio = static_cast<NativeJSAudio *>(custom);
    NativeJSAudio::Nodes *nodes = audio->nodes;

    // Let's shutdown all custom nodes
    while (nodes != NULL) {
        if (nodes->curr->type == NativeAudio::CUSTOM ||
            nodes->curr->type == NativeAudio::CUSTOM_SOURCE) {
            nodes->curr->shutdownCallback(nodes->curr->node, nodes->curr);
        }

        nodes = nodes->next;
    }

    if (audio->tcx != NULL) {
        JS_BeginRequest(audio->tcx);

        JSRuntime *rt = JS_GetRuntime(audio->tcx);
        audio->gbl = nullptr;

        JS_EndRequest(audio->tcx);

        JS_DestroyContext(audio->tcx);
        JS_DestroyRuntime(rt);
        audio->tcx = NULL;
    }

    NATIVE_PTHREAD_SIGNAL(&audio->m_ShutdownWait)
}

NativeJSAudio::~NativeJSAudio()
{
    this->audio->lockSources();
    this->audio->lockQueue();

    // Unroot all js audio nodes
    this->unroot();

    // Delete all nodes
    NativeJSAudio::Nodes *nodes = this->nodes;
    NativeJSAudio::Nodes *next = NULL;
    while (nodes != NULL) {
        next = nodes->next;
        // Node destructor will remove the node
        // from the nodes linked list
        delete nodes->curr;
        nodes = next;
    }

    // Unroot custom nodes objects and clear threaded js context
    this->audio->sharedMsg->postMessage(
            (void *)new NativeAudioNode::CallbackMessage(NativeJSAudio::shutdownCallback, NULL, this),
            NATIVE_AUDIO_CALLBACK);

    this->audio->wakeup();

    NATIVE_PTHREAD_WAIT(&this->m_ShutdownWait)

    // Unlock the sources, so the decode thread can exit
    // when we call NativeAudio::shutdown()
    this->audio->unlockSources();

    // Shutdown the audio
    this->audio->shutdown();

    this->audio->unlockQueue();

    // And delete the audio
    delete this->audio;

    NativeJSAudio::instance = NULL;
}

void NativeJSAudioNode::add()
{
    NativeJSAudio::Nodes *nodes = new NativeJSAudio::Nodes(this, NULL, this->audio->nodes);

    if (this->audio->nodes != NULL) {
        this->audio->nodes->prev = nodes;
    }

    this->audio->nodes = nodes;
}

void native_audio_node_custom_set_internal(JSContext *cx, NativeJSAudioNode *node,
    JS::HandleObject obj, const char *name, JS::HandleValue val)
{
    JS_SetProperty(cx, obj, name, val);

    JSTransferableFunction *setterFn;
    setterFn = node->m_TransferableFuncs[NativeJSAudioNode::SETTER_FN];
    if (setterFn) {
        JS::RootedValue rval(cx);
        JS::AutoValueArray<3> params(cx);
        JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
        JS::RootedObject nodeObj(cx, node->nodeObj);
        params[0].set(STRING_TO_JSVAL(JS_NewStringCopyZ(cx, name)));
        params[1].set(val);
        params[2].set(OBJECT_TO_JSVAL(global));

        setterFn->call(cx, nodeObj, params, &rval);
    }
}

void NativeJSAudioNode::setPropCallback(NativeAudioNode *node, void *custom)
{
    JSContext *tcx;
    NativeJSAudioNode::Message *msg;

    msg = static_cast<struct NativeJSAudioNode::Message*>(custom);
    tcx = msg->jsNode->audio->tcx;
    JS::RootedValue data(tcx);

    JSAutoRequest ar(tcx);

    if (!JS_ReadStructuredClone(tcx,
                msg->clone.datap,
                msg->clone.nbytes,
                JS_STRUCTURED_CLONE_VERSION, &data, nullptr, NULL)) {
        JS_PROPAGATE_ERROR(tcx, "Failed to read structured clone");

        JS_free(msg->jsNode->getJSContext(), msg->name);
        JS_ClearStructuredClone(msg->clone.datap, msg->clone.nbytes, nullptr, NULL);
        delete msg;

        return;
    }
    JS::RootedObject hashObj(tcx, msg->jsNode->hashObj);
    if (msg->name == NULL) {
        JS::RootedObject props(tcx, &data.toObject());
        JS::AutoIdArray ida(tcx, JS_Enumerate(tcx, props));
        for (size_t i = 0; i < ida.length(); i++) {
            JS::RootedId id(tcx, ida[i]);
            JS::RootedValue val(tcx);
            JSAutoByteString name(tcx, JSID_TO_STRING(id));

            if (!JS_GetPropertyById(tcx, props, id, &val)) {
                break;
            }
            native_audio_node_custom_set_internal(tcx, msg->jsNode, hashObj, name.ptr(), val);
        }
    } else {
        native_audio_node_custom_set_internal(tcx, msg->jsNode, hashObj, msg->name, data);
    }

    if (msg->name != NULL) {
        JS_free(msg->jsNode->getJSContext(), msg->name);
    }

    JS_ClearStructuredClone(msg->clone.datap, msg->clone.nbytes, nullptr, NULL);

    delete msg;
}

void NativeJSAudioNode::customCallback(const struct NodeEvent *ev)
{
    NativeJSAudioNode *thiz;
    JSContext *tcx;
    JSTransferableFunction *processFn;

    thiz = static_cast<NativeJSAudioNode *>(ev->custom);

    if (!thiz->audio->tcx || !thiz->m_Cx || !thiz->m_JSObject || !thiz->node) {
        return;
    }

    tcx = thiz->audio->tcx;

    JSAutoRequest ar(tcx);

    processFn = thiz->m_TransferableFuncs[NativeJSAudioNode::PROCESS_FN];

    if (!processFn) return;

    JS::AutoValueArray<2> params(tcx);
    JS::RootedValue rval(tcx);
    JS::RootedValue vFrames(tcx);
    JS::RootedValue vSize(tcx);
    JS::RootedObject obj(tcx);
    JS::RootedObject frames(tcx);
    unsigned long size;
    int count;

    count = thiz->node->inCount > thiz->node->outCount ? thiz->node->inCount : thiz->node->outCount;
    size = thiz->node->audio->outputParameters->bufferSize/2;

    obj = JS_NewObject(tcx, &AudioNodeEvent_class, JS::NullPtr(), JS::NullPtr());
    JS_DefineProperties(tcx, obj, AudioNodeEvent_props);

    frames = JS_NewArrayObject(tcx, 0);

    for (int i = 0; i < count; i++) {
        JS::RootedObject arrBuff(tcx);
        JS::RootedObject arr(tcx);
        uint8_t *data;

        // TODO : Avoid memcpy (custom allocator for NativeAudioNode?)
        arrBuff = JS_NewArrayBuffer(tcx, size);
        data = JS_GetArrayBufferData(arrBuff);

        memcpy(data, ev->data[i], size);

        arr = JS_NewFloat32ArrayWithBuffer(tcx, arrBuff, 0, -1);
        if (arr != NULL) {
            JS::RootedValue arrVal(tcx, OBJECT_TO_JSVAL(arr));
            JS_DefineElement(tcx, frames, i, arrVal, nullptr, nullptr,
                 JSPROP_ENUMERATE | JSPROP_PERMANENT);
        } else {
            JS_ReportOutOfMemory(tcx);
            return;
        }
    }

    vFrames = OBJECT_TO_JSVAL(frames);
    vSize = DOUBLE_TO_JSVAL(ev->size);

    JS_SetProperty(tcx, obj, "data", vFrames);
    JS_SetProperty(tcx, obj, "size", vSize);
    JS::RootedObject global(tcx, JS::CurrentGlobalOrNull(tcx));
    JS::RootedObject nodeObj(tcx, thiz->nodeObj);
    params[0].set(OBJECT_TO_JSVAL(obj));
    params[1].set(OBJECT_TO_JSVAL(global));

    //JS_CallFunction(tcx, thiz->nodeObj, thiz->processFn, params, &rval);
    processFn->call(tcx, nodeObj, params, &rval);

    for (int i = 0; i < count; i++) {
        JS::RootedValue val(tcx);
        JS::RootedObject arr(tcx);

        JS_GetElement(tcx, frames, i, &val);
        if (!val.isObject()) {
            continue;
        }

        arr = val.toObjectOrNull();
        if (arr == NULL || !JS_IsFloat32Array(arr) || JS_GetTypedArrayLength(arr) != ev->size) {
            continue;
        }

        memcpy(ev->data[i], JS_GetFloat32ArrayData(arr), size);
    }
}

void NativeJSAudioNode::onMessage(const NativeSharedMessages::Message &msg)
{
    if (m_IsDestructing) return;
    JS::RootedObject obj(m_Cx, m_JSObject);
    native_av_thread_message(m_Cx, obj, msg);
}

void NativeJSAudioNode::onEvent(const struct NativeAVSourceEvent *cev)
{
    NativeJSAudioNode *jnode = static_cast<NativeJSAudioNode *>(cev->custom);
    jnode->postMessage((void *)cev, cev->ev);
}

void NativeJSAudio::runCallback(NativeAudioNode *node, void *custom)
{
    NativeJSAudio *audio = NativeJSAudio::getContext();

    if (!audio) return; // This should not happend

    char *str = static_cast<char *>(custom);
    audio->run(str);
    JS_free(audio->getJSContext(), custom);
}


void NativeJSAudio::ctxCallback(NativeAudioNode *dummy, void *custom)
{
    NativeJSAudio *audio = static_cast<NativeJSAudio*>(custom);

    if (!audio->createContext()) {
        printf("Failed to create audio thread context\n");
        //JS_ReportError(jsNode->audio->cx, "Failed to create audio thread context\n");
        // XXX : Can't report error from another thread?
    }
}



void NativeJSAudioNode::shutdownCallback(NativeAudioNode *nnode, void *custom)
{
    NativeJSAudioNode *node = static_cast<NativeJSAudioNode *>(custom);
    JSAutoRequest ar(node->audio->tcx);

    if (!node->nodeObj.get()) {
    	node->nodeObj = nullptr;
    }
    if (!node->hashObj.get()) {
    	node->hashObj = nullptr;
    }

    // JSTransferableFunction need to be destroyed on the JS thread
    // since they belong to it
    for (int i = 0; i < NativeJSAudioNode::END_FN; i++) {
        delete node->m_TransferableFuncs[i];
        node->m_TransferableFuncs[i] = NULL;
    }

    NATIVE_PTHREAD_SIGNAL(&node->m_ShutdownWait)
}

void NativeJSAudioNode::initCustomObject(NativeAudioNode *node, void *custom)
{
    NativeJSAudioNode *jnode = static_cast<NativeJSAudioNode *>(custom);
    JSContext *tcx = jnode->audio->tcx;

    if (jnode->hashObj.get() != nullptr || jnode->nodeObj.get() != nullptr) {
        return;
    }

    JSAutoRequest ar(tcx);

    jnode->hashObj = JS_NewObject(tcx, nullptr, JS::NullPtr(), JS::NullPtr());
    if (!jnode->hashObj.get()) {
        JS_PROPAGATE_ERROR(tcx, "Failed to create hash object for custom node");
        return;
    }
    jnode->nodeObj = JS_NewObject(tcx, &AudioNode_threaded_class, JS::NullPtr(), JS::NullPtr());
    if (!jnode->nodeObj) {
        JS_PROPAGATE_ERROR(tcx, "Failed to create node object for custom node");
        return;
    }
    JS::RootedObject nodeObj(tcx, jnode->nodeObj);
    JS_DefineFunctions(tcx, nodeObj, AudioNodeCustom_threaded_funcs);
    JS_SetPrivate(jnode->nodeObj, static_cast<void *>(jnode));

    JSTransferableFunction *initFn =
        jnode->m_TransferableFuncs[NativeJSAudioNode::INIT_FN];

    if (initFn) {
        JS::AutoValueArray<1> params(tcx);
        JS::RootedValue rval(tcx);
        JS::RootedObject global(tcx, JS::CurrentGlobalOrNull(tcx));
        JS::RootedValue glVal(tcx, OBJECT_TO_JSVAL(global));
        params[0].set(glVal);

        initFn->call(tcx, nodeObj, params, &rval);

        jnode->m_TransferableFuncs[NativeJSAudioNode::INIT_FN] = NULL;

        delete initFn;
    }
}

NativeJSAudioNode::~NativeJSAudioNode()
{
    NativeJSAudio::Nodes *nodes = this->audio->nodes;

    m_IsDestructing = true;

    // Block NativeAudio threads execution.
    // While the node is destructed we don't want any thread
    // to call some method on a node that is being destroyed
    this->audio->audio->lockQueue();
    this->audio->audio->lockSources();

    // Wakeup audio thread. This will flush all pending messages.
    // That way, we are sure nothing will need to be processed
    // later for this node.
    this->audio->audio->wakeup();

    if (this->type == NativeAudio::SOURCE) {
        // Only source from NativeVideo has reserved slot
        JS::RootedValue s(m_Cx, JS_GetReservedSlot(m_JSObject, 0));
        JS::RootedObject obj(m_Cx, s.toObjectOrNull());
        if (obj != NULL) {
            // If it exist, we must inform the video
            // that audio node no longer exist
            NativeJSVideo *video = (NativeJSVideo *)JS_GetPrivate(obj);
            if (video != NULL) {
                JS_SetReservedSlot(m_JSObject, 0, JSVAL_NULL);
                video->stopAudio();
            }
        }
    }

    // Remove JS node from nodes linked list
    while (nodes != NULL) {
        if (nodes->curr == this) {
            if (nodes->prev != NULL) {
                nodes->prev->next = nodes->next;
            } else {
                this->audio->nodes = nodes->next;
            }

            if (nodes->next != NULL) {
                nodes->next->prev = nodes->prev;
            }

            delete nodes;


            break;
        }
        nodes = nodes->next;
    }


    // Custom nodes and sources must release all JS object on the JS thread
    if (this->node != NULL && this->audio->tcx != NULL &&
            (this->type == NativeAudio::CUSTOM ||
             this->type == NativeAudio::CUSTOM_SOURCE)) {

        this->node->callback(NativeJSAudioNode::shutdownCallback, this);

        this->audio->audio->wakeup();

        NATIVE_PTHREAD_WAIT(&this->m_ShutdownWait);
    }

    if (this->arrayContent != NULL) {
        free(this->arrayContent);
    }

    if (m_JSObject != NULL) {
        JS_SetPrivate(m_JSObject, nullptr);
    }

    delete node;

    this->audio->audio->unlockQueue();
    this->audio->audio->unlockSources();
}


static bool native_audio_pFFT(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject x(cx);
    JS::RootedObject y(cx);
    int dir, n;

    if (!JS_ConvertArguments(cx, args, "ooii", x.address(), y.address(), &n, &dir)) {
        return false;
    }

    if (!JS_IsTypedArrayObject(x) || !JS_IsTypedArrayObject(y)) {
        JS_ReportError(cx, "Bad argument");
        return false;
    }

    double *dx, *dy;
    uint32_t dlenx, dleny;

    if (JS_GetObjectAsFloat64Array(x, &dlenx, &dx) == NULL) {
        JS_ReportError(cx, "Can't convert typed array (expected Float64Array)");
        return false;
    }
    if (JS_GetObjectAsFloat64Array(y, &dleny, &dy) == NULL) {
        JS_ReportError(cx, "Can't convert typed array (expected Float64Array)");
        return false;
    }

    if (dlenx != dleny) {
        JS_ReportError(cx, "Buffers size must match");
        return false;
    }

    if ((n & (n -1)) != 0 || n < 32 || n > 4096) {
        JS_ReportError(cx, "Invalid frame size");
        return false;
    }

    if (n > dlenx) {
        JS_ReportError(cx, "Buffer is too small");
        return false;
    }

    FFT(dir, n, dx, dy);

    return true;
}

static bool native_Audio_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS_ReportError(cx, "Illegal constructor");
    return false;
}

static bool native_audio_getcontext(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    unsigned int bufferSize, channels, sampleRate;

    if (argc > 0) {
        bufferSize = args.array()[0].toInt32();
    } else {
        bufferSize = 2048;
    }

    if (argc > 1) {
        channels = args.array()[1].toInt32();
    } else {
        channels = 2;
    }

    if (argc >= 2) {
        sampleRate = args.array()[2].toInt32();
    } else {
        sampleRate = 44100;
    }

    switch (bufferSize) {
        case 16:
        case 32:
        case 64:
        case 128:
        case 256:
        case 512:
        case 1024:
        case 2048:
        case 4096:
            // Supported buffer size
            // Multiply by 8 to get the bufferSize in bytes
            // rather than in samples per buffer
            bufferSize *= 8;
            break;
        default :
            JS_ReportError(cx, "Unsuported buffer size %d. "
                "Supported values are : 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 \n", bufferSize);
            return false;
            break;
    }

    if (channels < 1 || channels > 32) {
        JS_ReportError(cx, "Unsuported channels number %d. Channels must be between 1 and 32\n", channels);
        return false;
    }

    if (sampleRate < 22050 || sampleRate> 96000) {
        JS_ReportError(cx, "Unsuported sample rate %dKHz. Sample rate must be between 22050 and 96000\n", sampleRate);
        return false;
    }

    bool paramsChanged = false;
    NativeJSAudio *jaudio = NativeJSAudio::getContext();

    if (jaudio) {
        NativeAudioParameters *params = jaudio->audio->outputParameters;
        if (params->bufferSize != bufferSize ||
            params->channels != channels ||
            params->sampleRate != sampleRate) {
            paramsChanged = true;
        }
    }

    if (!paramsChanged && jaudio) {
        args.rval().set(OBJECT_TO_JSVAL(jaudio->getJSObject()));
        return true;
    }

    if (paramsChanged) {
        JSContext *m_Cx = cx;
        JS_SetPrivate(jaudio->getJSObject(), NULL);
        NJS->unrootObject(jaudio->getJSObject());
        delete jaudio;
    }

    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &AudioContext_class, args));
    NativeJSAudio *naudio = NativeJSAudio::getContext(cx, ret, bufferSize, channels, sampleRate);

    if (naudio == NULL) {
        delete naudio;
        JS_ReportError(cx, "Failed to initialize audio context\n");
        return false;
    }

    args.rval().set(OBJECT_TO_JSVAL(ret));

    return true;
}

static bool native_audio_run(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedString fn(cx);
    JS::RootedFunction nfn(cx);
    JS::RootedValue argv0(cx, args.array()[0]);
    NativeJSAudio *audio = NativeJSAudio::getContext();

    CHECK_INVALID_CTX(audio);

    if ((nfn = JS_ValueToFunction(cx, argv0)) == NULL ||
        (fn = JS_DecompileFunctionBody(cx, nfn, 0)) == NULL) {
        JS_ReportError(cx, "Failed to read callback function\n");
        return false;
    }

    char *funStr = JS_EncodeString(cx, fn);

    audio->audio->sharedMsg->postMessage(
            (void *)new NativeAudioNode::CallbackMessage(NativeJSAudio::runCallback, NULL, static_cast<void *>(funStr)),
            NATIVE_AUDIO_CALLBACK);

    return true;
}

static bool native_audio_load(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS_ReportError(cx, "Not implemented");
    return false;
}

static bool native_audio_createnode(JSContext *cx, unsigned argc, JS::Value *vp)
{
    int in, out;
    JS::RootedObject ret(cx);
    JS::RootedString name(cx);
    NativeJSAudio *audio;
    NativeJSAudioNode *node;

    JSNATIVE_PROLOGUE_CLASS(NativeJSAudio, &AudioContext_class);
    audio = CppObj;

    node = NULL;
    ret = NULL;

    if (!JS_ConvertArguments(cx, args, "Suu", name.address(), &in, &out)) {
        return false;
    }

    if (in == 0 && out == 0) {
        JS_ReportError(cx, "Node must have at least one input or output");
        return false;
    } else if (in < 0 || out < 0) {
        JS_ReportError(cx, "Wrong channel count (Must be greater or equal to 0)");
        return false;
    } else if (in > 32 || out > 32) {
        JS_ReportError(cx, "Wrong channel count (Must be lower or equal to 32)");
        return false;
    }

    JSAutoByteString cname(cx, name);
    ret = JS_NewObjectForConstructor(cx, &AudioNode_class, args);
    if (!ret) {
        JS_ReportOutOfMemory(cx);
        return false;
    }

    JS_SetReservedSlot(ret, 0, JSVAL_NULL);

    try {
        if (strcmp("source", cname.ptr()) == 0) {
            node = new NativeJSAudioNode(ret, cx, NativeAudio::SOURCE, in, out, audio);
            NativeAudioSource *source = static_cast<NativeAudioSource*>(node->node);
            source->eventCallback(NativeJSAudioNode::onEvent, node);
            JS_DefineFunctions(cx, ret, AudioNodeSource_funcs);
            JS_DefineProperties(cx, ret, AudioNodeSource_props);
        } else if (strcmp("custom-source", cname.ptr()) == 0) {
            node = new NativeJSAudioNode(ret, cx, NativeAudio::CUSTOM_SOURCE, in, out, audio);
            JS_DefineProperties(cx, ret, AudioNodeCustom_props);
            JS_DefineFunctions(cx, ret, AudioNodeCustom_funcs);
            JS_DefineFunctions(cx, ret, AudioNodeCustomSource_funcs);
            JS_DefineProperties(cx, ret, AudioNodeCustomSource_props);
        } else if (strcmp("custom", cname.ptr()) == 0) {
            node = new NativeJSAudioNode(ret, cx, NativeAudio::CUSTOM, in, out, audio);
            JS_DefineProperties(cx, ret, AudioNodeCustom_props);
            JS_DefineFunctions(cx, ret, AudioNodeCustom_funcs);
        } else if (strcmp("reverb", cname.ptr()) == 0) {
            node = new NativeJSAudioNode(ret, cx, NativeAudio::REVERB, in, out, audio);
        } else if (strcmp("delay", cname.ptr()) == 0) {
            node = new NativeJSAudioNode(ret, cx, NativeAudio::DELAY, in, out, audio);
        } else if (strcmp("gain", cname.ptr()) == 0) {
            node = new NativeJSAudioNode(ret, cx, NativeAudio::GAIN, in, out, audio);
        } else if (strcmp("target", cname.ptr()) == 0) {
            if (audio->target != NULL) {
                args.rval().set(OBJECT_TO_JSVAL(audio->target->getJSObject()));
                return true;
            } else {
                node = new NativeJSAudioNode(ret, cx, NativeAudio::TARGET, in, out, audio);
                audio->target = node;
            }
        } else if (strcmp("stereo-enhancer", cname.ptr()) == 0) {
            node = new NativeJSAudioNode(ret, cx, NativeAudio::STEREO_ENHANCER, in, out, audio);
        } else {
            JS_ReportError(cx, "Unknown node name : %s\n", cname.ptr());
            return false;
        }
    } catch (NativeAudioNodeException *e) {
        delete node;
        JS_ReportError(cx, "Error while creating node : %s\n", e->what());
        return false;
    }

    if (node == NULL || node->node == NULL) {
        delete node;
        JS_ReportError(cx, "Error while creating node : %s\n", cname.ptr());
        return false;
    }

    audio->initNode(node, ret, name);

    args.rval().set(OBJECT_TO_JSVAL(ret));

    return true;
}

static bool native_audio_connect(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedObject link1(cx);
    JS::RootedObject link2(cx);
    NodeLink *nlink1;
    NodeLink *nlink2;
    NativeJSAudio *jaudio;

    JSNATIVE_PROLOGUE_CLASS(NativeJSAudio, &AudioContext_class);
    jaudio = CppObj;

    NativeAudio *audio = jaudio->audio;

    if (!JS_ConvertArguments(cx, args, "oo", link1.address(), link2.address())) {
        return true;
    }

    nlink1 = (NodeLink *)JS_GetInstancePrivate(cx, link1, &AudioNodeLink_class, &args);
    nlink2 = (NodeLink *)JS_GetInstancePrivate(cx, link2, &AudioNodeLink_class, &args);

    if (nlink1 == NULL || nlink2 == NULL) {
        JS_ReportError(cx, "Bad AudioNodeLink\n");
        return false;
    }

    if (nlink1->type == INPUT && nlink2->type == OUTPUT) {
        if (!audio->connect(nlink2, nlink1)) {
            JS_ReportError(cx, "connect() failed (max connection reached)\n");
            return false;
        }
    } else if (nlink1->type == OUTPUT && nlink2->type == INPUT) {
        if (!audio->connect(nlink1, nlink2)) {
            JS_ReportError(cx, "connect() failed (max connection reached)\n");
            return false;
        }
    } else {
        JS_ReportError(cx, "connect() take one input and one output\n");
        return false;
    }

    return true;
}

static bool native_audio_disconnect(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedObject link1(cx);
    JS::RootedObject link2(cx);
    NodeLink *nlink1;
    NodeLink *nlink2;
    NativeJSAudio *jaudio;

    JSNATIVE_PROLOGUE_CLASS(NativeJSAudio, &AudioContext_class);
    jaudio = CppObj;

    NativeAudio *audio = jaudio->audio;

    if (!JS_ConvertArguments(cx, args, "oo", link1.address(), link2.address())) {
        return true;
    }

    nlink1 = (NodeLink *)JS_GetInstancePrivate(cx, link1, &AudioNodeLink_class, &args);
    nlink2 = (NodeLink *)JS_GetInstancePrivate(cx, link2, &AudioNodeLink_class, &args);

    if (nlink1 == NULL || nlink2 == NULL) {
        JS_ReportError(cx, "Bad AudioNodeLink\n");
        return false;
    }

    if (nlink1->type == INPUT && nlink2->type == OUTPUT) {
        audio->disconnect(nlink2, nlink1);
    } else if (nlink1->type == OUTPUT && nlink2->type == INPUT) {
        audio->disconnect(nlink1, nlink2);
    } else {
        JS_ReportError(cx, "disconnect() take one input and one output\n");
        return false;
    }

    return true;
}

static bool native_audiothread_print(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedValue argv0(cx, args.array()[0]);
    char *bytes;

    JS::RootedString str(cx, argv0.toString());
    if (!str.get())
        return true;

    bytes = JS_EncodeString(cx, str);
    if (!bytes)
        return true;

    printf("%s\n", bytes);

    JS_free(cx, bytes);

    return true;
}

static bool native_audio_prop_setter(JSContext *cx, JS::HandleObject obj, uint8_t id, bool strict, JS::MutableHandleValue vp)
{
    NativeJSAudio *jaudio = NativeJSAudio::getContext();

    CHECK_INVALID_CTX(jaudio);

    if (vp.isNumber()) {
        jaudio->audio->setVolume((float)vp.toNumber());
    }

    return true;
}

static bool native_audio_prop_getter(JSContext *cx, JS::HandleObject obj, uint8_t id, JS::MutableHandleValue vp)
{
    NativeJSAudio *jaudio = NativeJSAudio::getContext();

    CHECK_INVALID_CTX(jaudio);

    NativeAudioParameters *params = jaudio->audio->outputParameters;

    switch(id) {
        case AUDIO_PROP_BUFFERSIZE:
            vp.set(INT_TO_JSVAL(params->bufferSize/8));
        break;
        case AUDIO_PROP_CHANNELS:
            vp.set(INT_TO_JSVAL(params->channels));
        break;
        case AUDIO_PROP_SAMPLERATE:
            vp.set(INT_TO_JSVAL(params->sampleRate));
        break;
        case AUDIO_PROP_VOLUME:
            vp.set(DOUBLE_TO_JSVAL(jaudio->audio->getVolume()));
        break;
        default:
            return false;
        break;
    }

    return true;
}

static bool native_AudioNode_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS_ReportError(cx, "Illegal constructor");
    return false;
}

static void native_audionode_set_internal(JSContext *cx, NativeAudioNode *node, const char *prop, JS::HandleValue val)
{
    ArgType type;
    void *value;
    int intVal = 0;
    double doubleVal = 0;
    unsigned long size;

    if (val.isInt32()) {
        type = INT;
        size = sizeof(int);
        intVal = val.toInt32();
        value = &intVal;
    } else if (val.isDouble()) {
        type = DOUBLE;
        size = sizeof(double);
        doubleVal = val.toDouble();
        value = &doubleVal;
    } else {
        JS_ReportError(cx, "Unsuported value\n");
        return;
    }

    if (!node->set(prop, type, value, size)) {
        JS_ReportError(cx, "Unknown argument name %s\n", prop);
    }
}
static bool native_audionode_set(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString name(cx);
    NativeJSAudioNode *jnode;

    JSNATIVE_PROLOGUE_CLASS(NativeJSAudioNode, &AudioNode_class);
    jnode = CppObj;

    NativeAudioNode *node = jnode->node;
    if (!args.array()[0].isPrimitive()) {
    	JS::RootedValue arg0(cx, args.array()[0]);
        JS::RootedObject props(cx, &arg0.toObject());
        JS::AutoIdArray ida(cx, JS_Enumerate(cx, props));
        for (size_t i = 0; i < ida.length(); i++) {
            JS::RootedId id(cx, ida[i]);
            JS::RootedValue val(cx);
            JSAutoByteString name(cx, JSID_TO_STRING(id));

            if (!JS_GetPropertyById(cx, props, id, &val)) {
                break;
            }
            native_audionode_set_internal(cx, node, name.ptr(), val);
        }
    } else {
        if (!JS_ConvertArguments(cx, args, "S", name.address())) {
            return false;
        }

        JSAutoByteString cname(cx, name);
        JS::RootedValue val(cx, args.array()[1]);

        native_audionode_set_internal(cx, node, cname.ptr(), val);
    }

    return true;
}

static bool native_audionode_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
    return true;
}

static bool native_audionode_custom_set(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString name(cx);
    JS::RootedValue val(cx);
    NativeJSAudioNode::Message *msg;
    NativeAudioNodeCustom *node;
    NativeJSAudioNode *jnode;

    JSNATIVE_PROLOGUE_CLASS(NativeJSAudioNode, &AudioNode_class);
    jnode = CppObj;

    if (argc == 1 && !args.array()[0].isPrimitive()) {
        val = args.array()[0];
        name = NULL;
    } else if (argc == 1) {
        JS_ReportError(cx, "Invalid arguments");
        return false;
    } else {
        if (!JS_ConvertArguments(cx, args, "S", name.address())) {
            return false;
        }
        val = args.array()[1];
    }

    node = static_cast<NativeAudioNodeCustom *>(jnode->node);

    msg = new NativeJSAudioNode::Message();
    if (name != NULL) {
        msg->name = JS_EncodeString(cx, name);
    }
    msg->jsNode = jnode;
    JS::RootedValue args1(cx, args.array()[1]);

    if (!JS_WriteStructuredClone(cx, args1, &msg->clone.datap, &msg->clone.nbytes, nullptr, nullptr, JS::NullHandleValue)) {
        JS_ReportError(cx, "Failed to write structured clone");

        JS_free(cx, msg->name);
        delete msg;

        return false;
    }

    if (!jnode->nodeObj.get()) {
        node->callback(NativeJSAudioNode::initCustomObject, static_cast<void *>(jnode));
    }
    node->callback(NativeJSAudioNode::setPropCallback, msg);

    return true;
}

#if 0
static bool native_audionode_custom_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
    printf("hello get\n");

    NativeJSAudioNode *jsNode;
    JS::RootedValuel val(cx);
    JS::RootedString name(cx);
    printf("get node\n");

    jsNode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    printf("convert\n");
    if (!JS_ConvertArguments(cx, args, "S", name, address())) {
        return true;
    }

    printf("str\n");
    JSAutoByteString str(cx, name);

    JS_SetRuntimeThread(JS_GetRuntime(cx));

    printf("get\n");
    JS::RootedObject hashObj(cx, jnode->hashObj);
    JS_GetProperty(jsNode->audio->tcx, hashObj, str.ptr(), &val);

    printf("return\n");
    JS_ClearRuntimeThread(JS_GetRuntime(cx));

    args.rval().set(val);

    return true;
}
#endif

static bool native_audionode_custom_threaded_set(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString name(cx);
    JS::RootedValue val(cx);
    NativeJSAudioNode *jnode;
    JSTransferableFunction *fn;

    JSNATIVE_PROLOGUE_CLASS(NativeJSAudioNode, &AudioNode_class);
    jnode = CppObj;

    if (argc != 2) {
        JS_ReportError(cx, "set() require two arguments\n");
        return false;
    }

    if (!JS_ConvertArguments(cx, args, "S", name.address())) {
        return false;
    }

    fn = jnode->m_TransferableFuncs[NativeJSAudioNode::SETTER_FN];
    val = args.array()[1];

    JSAutoByteString str(cx, name);
    JS::RootedObject hash(cx, jnode->hashObj);

    JS_SetProperty(cx, hash, str.ptr(), val);

    if (fn) {
        JS::AutoValueArray<3> params(cx);
        JS::RootedValue rval(cx);
        JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
        JS::RootedObject nodeObj(cx, jnode->nodeObj);

        params[0].set(STRING_TO_JSVAL(name));
        params[1].set(val);
        params[2].set(OBJECT_TO_JSVAL(global));

        fn->call(cx, nodeObj, params, &rval);
    }

    return true;
}

static bool native_audionode_custom_threaded_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedValue val(cx);
    JS::RootedString name(cx);
    NativeJSAudioNode *jnode;

    JSNATIVE_PROLOGUE_CLASS(NativeJSAudioNode, &AudioNode_threaded_class);
    jnode = CppObj;

    if (!jnode->hashObj.get()) {
        args.rval().set(JSVAL_NULL);
        return false;
    }

    if (!JS_ConvertArguments(cx, args, "S", name.address())) {
        return false;
    }

    JSAutoByteString str(cx, name);
    JS::RootedObject hashObj(cx, jnode->hashObj);
    JS_GetProperty(jnode->audio->tcx, hashObj, str.ptr(), &val);

    args.rval().set(val);

    return true;
}

static bool native_audionode_custom_threaded_send(JSContext *cx, unsigned argc, JS::Value *vp)
{
    uint64_t *datap;
    size_t nbytes;
    NativeJSAudioNode *jnode;

    JSNATIVE_PROLOGUE_CLASS(NativeJSAudioNode, &AudioNode_threaded_class);
    jnode = CppObj;

    struct native_thread_msg *msg;
    JS::RootedValue args0(cx, args.array()[0]);
    if (!JS_WriteStructuredClone(cx, args0, &datap, &nbytes, nullptr, nullptr, JS::NullHandleValue)) {
        JS_ReportError(cx, "Failed to write structured clone");
        return false;
    }

    msg = new struct native_thread_msg;

    msg->data   = datap;
    msg->nbytes = nbytes;
    msg->callee = jnode->getJSObject();

    jnode->postMessage(msg, CUSTOM_SOURCE_SEND);

    return true;
}

static bool native_audionode_source_open(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativeJSAudioNode *jnode;

    JSNATIVE_PROLOGUE_CLASS(NativeJSAudioNode, &AudioNode_class);
    jnode = CppObj;

    NativeAudioSource *source = (NativeAudioSource *)jnode->node;

    JS::RootedValue src(cx, args.array()[0]);

    int ret = -1;

    if (src.isString()) {
        JSAutoByteString csrc(cx, src.toString());
        ret = source->open(csrc.ptr());
    } else if (src.isObject()) {
        JS::RootedObject arrayBuff(cx, src.toObjectOrNull());
        if (!JS_IsArrayBufferObject(arrayBuff)) {
            JS_ReportError(cx, "Data is not an ArrayBuffer\n");
            return false;
        }
        int length = JS_GetArrayBufferByteLength(arrayBuff);
        jnode->arrayContent = JS_StealArrayBufferContents(cx, arrayBuff);
        ret = source->open(jnode->arrayContent, length);
    }

    if (ret < 0) {
        JS_ReportError(cx, "Failed to open stream %d\n", ret);
        return false;
    }

    return true;
}
static bool native_audionode_source_play(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativeAudioSource *source;

    JSNATIVE_AV_GET_NODE(NativeAudioSource, source);

    source->play();

    return true;
}

static bool native_audionode_source_pause(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativeAudioSource *source;

    JSNATIVE_AV_GET_NODE(NativeAudioSource, source);

    source->pause();

    return true;
}

static bool native_audionode_source_stop(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativeAudioSource *source;

    JSNATIVE_AV_GET_NODE(NativeAudioSource, source);

    source->stop();

    return true;
}

static bool native_audionode_source_close(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativeAudioSource *source;

    JSNATIVE_AV_GET_NODE(NativeAudioSource, source);

    source->close();

    return true;
}

static bool native_audionode_source_prop_getter(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp)
{
    NativeJSAudioNode *jnode = (NativeJSAudioNode *)JS_GetPrivate(obj);

    CHECK_INVALID_CTX(jnode);

    NativeAudioSource *source = static_cast<NativeAudioSource *>(jnode->node);

    return NativeJSAVSource::propGetter(source, cx, id, vp);
}

static bool native_audionode_source_prop_setter(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp)
{
    NativeJSAudioNode *jnode = (NativeJSAudioNode *)JS_GetPrivate(obj);

    CHECK_INVALID_CTX(jnode);

    NativeAudioSource *source = static_cast<NativeAudioSource*>(jnode->node);

    return NativeJSAVSource::propSetter(source, id, vp);
}

static bool native_audionode_custom_source_play(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativeAudioCustomSource *source;

    JSNATIVE_AV_GET_NODE(NativeAudioCustomSource, source);

    source->play();

    return true;
}

static bool native_audionode_custom_source_pause(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativeAudioCustomSource *source;

    JSNATIVE_AV_GET_NODE(NativeAudioCustomSource, source);

    source->pause();

    return true;
}

static bool native_audionode_custom_source_stop(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativeAudioCustomSource *source;

    JSNATIVE_AV_GET_NODE(NativeAudioCustomSource, source);

    source->stop();

    return true;
}

static bool native_audionode_custom_source_prop_setter(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp)
{
    NativeJSAudioNode *jnode = (NativeJSAudioNode *)JS_GetPrivate(obj);

    CHECK_INVALID_CTX(jnode);

    return NativeJSAudioNode::propSetter(jnode, cx, id, vp);
}

static bool native_audionode_custom_prop_setter(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp)
{
    NativeJSAudioNode *jnode = (NativeJSAudioNode *)JS_GetPrivate(obj);
    JSTransferableFunction *fun;
    NativeAudioNodeCustom *node;
    NativeJSAudioNode::TransferableFunction funID;

    CHECK_INVALID_CTX(jnode);

    node = static_cast<NativeAudioNodeCustom *>(jnode->node);

    switch (id) {
        case NODE_CUSTOM_PROP_PROCESS:
            funID = NativeJSAudioNode::PROCESS_FN;
        break;
        case NODE_CUSTOM_PROP_SETTER:
            funID = NativeJSAudioNode::SETTER_FN;
        break;
        case NODE_CUSTOM_PROP_INIT :
            funID = NativeJSAudioNode::INIT_FN;
        break;
        default:
            return true;
            break;
    }

    fun = new JSTransferableFunction(cx);

    if (!fun->prepare(cx, vp)) {
        JS_ReportError(cx, "Failed to read custom node callback function\n");
        vp.set(JS::NullHandleValue);
        delete fun;
        return false;
    }

    if (jnode->m_TransferableFuncs[funID] != NULL) {
        delete jnode->m_TransferableFuncs[funID];
    }

    jnode->m_TransferableFuncs[funID] = fun;

    if (id == NODE_CUSTOM_PROP_PROCESS) {
        if (!jnode->nodeObj.get()) {
            node->callback(NativeJSAudioNode::initCustomObject, static_cast<void *>(jnode));
        }
        node->setCallback(NativeJSAudioNode::customCallback, static_cast<void *>(jnode));
    }

    return true;
}

static bool native_video_prop_getter(JSContext *cx, JS::HandleObject obj, uint8_t id, JS::MutableHandleValue vp)
{
    NativeJSVideo *v = (NativeJSVideo *)JS_GetPrivate(obj);
    if (v == NULL) {
        return false;
    }

    switch (id) {
        case VIDEO_PROP_WIDTH:
            vp.set(INT_TO_JSVAL(v->video->codecCtx->width));
        break;
        case VIDEO_PROP_HEIGHT:
            vp.set(INT_TO_JSVAL(v->video->codecCtx->height));
        break;
        default:
            return NativeJSAVSource::propGetter(v->video, cx, id, vp);
            break;
    }

    return true;
}

static bool native_video_prop_setter(JSContext *cx, JS::HandleObject obj, uint8_t id, bool strict, JS::MutableHandleValue vp)
{
    NativeJSVideo *v = (NativeJSVideo *)JS_GetPrivate(obj);
    if (v == NULL) {
        return false;
    }

    return NativeJSAVSource::propSetter(v->video, id, vp);
}

void AudioContext_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSAudio *audio = (NativeJSAudio *)JS_GetPrivate(obj);
    if (audio != NULL) {
        delete audio;
    }
}

void AudioNode_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSAudioNode *node = (NativeJSAudioNode *)JS_GetPrivate(obj);
    if (node != NULL) {
        delete node;
    }
}

NativeJSVideo::NativeJSVideo(JS::HandleObject obj,
    NativeCanvas2DContext *canvasCtx, JSContext *cx) :
    NativeJSExposer<NativeJSVideo>(obj, cx),
    video(NULL), audioNode(cx, nullptr), arrayContent(NULL),
    m_Width(-1), m_Height(-1), m_Left(0), m_Top(0), m_IsDestructing(false),
    m_CanvasCtx(canvasCtx), cx(cx)
{
    this->video = new NativeVideo((ape_global *)JS_GetContextPrivate(cx));
    this->video->frameCallback(NativeJSVideo::frameCallback, this);
    this->video->eventCallback(NativeJSVideo::onEvent, this);
    m_CanvasCtx->getHandler()->addListener(this);
}

void NativeJSVideo::stopAudio()
{
    this->video->stopAudio();

    if (this->audioNode.get()) {
        NJS->unrootObject(this->audioNode);
        NativeJSAudioNode *jnode = static_cast<NativeJSAudioNode *>(JS_GetPrivate(this->audioNode));
        JS_SetReservedSlot(jnode->getJSObject(), 0, JSVAL_NULL);
        this->audioNode = nullptr;
    }
}

void NativeJSVideo::onMessage(const NativeSharedMessages::Message &msg)
{
    if (m_IsDestructing) return;

    if (msg.event() == NATIVE_EVENT(NativeCanvasHandler, RESIZE_EVENT) && (m_Width == -1 || m_Height == -1)) {
        this->setSize(m_Width, m_Height);
    } else {
        if (msg.event() == SOURCE_EVENT_PLAY) {
            this->setSize(m_Width, m_Height);
        }
        JS::RootedObject obj(cx, this->getJSObject());
        native_av_thread_message(cx, obj, msg);
    }
}

void NativeJSVideo::onEvent(const struct NativeAVSourceEvent *cev)
{
    NativeJSVideo *thiz = static_cast<NativeJSVideo *>(cev->custom);
    thiz->postMessage((void *)cev, cev->ev);
}

void NativeJSVideo::frameCallback(uint8_t *data, void *custom)
{
    NativeJSVideo *v = (NativeJSVideo *)custom;
    NativeCanvasHandler *handler = v->m_CanvasCtx->getHandler();
    NativeSkia *surface = v->m_CanvasCtx->getSurface();

    surface->setFillColor(0xFF000000);
    surface->drawRect(0, 0, handler->getWidth(), handler->getHeight(), 0);
    surface->drawPixels(data, v->video->m_Width, v->video->m_Height, v->m_Left, v->m_Top);

    JS::RootedValue onframe(v->cx);
    JS::RootedObject vobj(v->cx, v->getJSObject());
    if (JS_GetProperty(v->cx, vobj, "onframe", &onframe) &&
        !onframe.isPrimitive() &&
        JS_ObjectIsCallable(v->cx, &onframe.toObject())) {
        JS::AutoValueArray<1> params(v->cx);
        JS::RootedValue rval(v->cx);

        params[0].set(OBJECT_TO_JSVAL(v->getJSObject()));

        JSAutoRequest ar(v->cx);
        JS_CallFunctionValue(v->cx, vobj, onframe, params, &rval);
    }
}

void NativeJSVideo::setSize(int width, int height)
{
    m_Width = width;
    m_Height = height;

    if (!video->codecCtx) {
        // setSize will be called again when video is ready
        return;
    }

    int canvasWidth = m_CanvasCtx->getHandler()->getWidth();
    int canvasHeight = m_CanvasCtx->getHandler()->getHeight();

    // Invalid dimension, force size to canvas
    if (width == 0) width = -1;
    if (height == 0) height = -1;

    // Size the video
    if (m_Width == -1 || m_Height == -1) {
        int videoWidth = video->codecCtx->width;
        int videoHeight = video->codecCtx->height;

        int maxWidth = native_min(m_Width == -1 ? canvasWidth : m_Width, canvasWidth);
        int maxHeight = native_min(m_Height == -1 ? canvasHeight : m_Height, canvasHeight);
        double ratio = native_max(videoHeight / (double)maxHeight, videoWidth / (double)maxWidth);

        width = videoWidth / ratio;
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

    this->video->setSize(width, height);
}

static bool native_video_play(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeJSVideo, &Video_class);

    CppObj->video->play();

    return true;
}

static bool native_video_pause(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeJSVideo, &Video_class);

    CppObj->video->pause();

    return true;
}

static bool native_video_stop(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeJSVideo, &Video_class);

    CppObj->video->stop();

    return true;
}

static bool native_video_close(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeJSVideo, &Video_class);

    CppObj->video->close();

    return true;
}

static bool native_video_open(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativeJSVideo *v;
    JSNATIVE_PROLOGUE_CLASS(NativeJSVideo, &Video_class);
    v = CppObj;

    JS::RootedValue src(cx, args.array()[0]);
    int ret = -1;

    if (src.isString()) {
        JSAutoByteString csrc(cx, src.toString());
        ret = v->video->open(csrc.ptr());
    } else if (src.isObject()) {
        JS::RootedObject arrayBuff(cx, src.toObjectOrNull());

        if (!JS_IsArrayBufferObject(arrayBuff)) {
            JS_ReportError(cx, "Data is not an ArrayBuffer\n");
            return false;
        }

        int length = JS_GetArrayBufferByteLength(arrayBuff);
        v->arrayContent = JS_StealArrayBufferContents(cx, arrayBuff);
        if (v->video->open(v->arrayContent, length) < 0) {
            args.rval().set(JSVAL_FALSE);
            return true;
        }
    }

    args.rval().set(JSVAL_TRUE);

    return true;
}

static bool native_video_get_audionode(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativeJSAudio *jaudio = NativeJSAudio::getContext();
    NativeJSVideo *v;

    JSNATIVE_PROLOGUE_CLASS(NativeJSVideo, &Video_class);

    v = CppObj;

    if (!jaudio) {
        JS_ReportError(cx, "No Audio context");
        args.rval().set(JSVAL_NULL);
        return false;
    }

    if (v->audioNode.get()) {
        args.rval().set(OBJECT_TO_JSVAL(v->audioNode));
        return true;
    }

    NativeAudioSource *source = v->video->getAudioNode(jaudio->audio);

    if (source != NULL) {
        v->audioNode = JS_NewObjectForConstructor(cx, &AudioNode_class, args);

        NativeJSAudioNode *node = new NativeJSAudioNode(v->audioNode, cx,
          NativeAudio::SOURCE, static_cast<class NativeAudioNode *>(source), jaudio);

        JS::RootedString name(cx, JS_NewStringCopyN(cx, "video-source", 12));
        JS::RootedObject an(cx, v->audioNode);
        jaudio->initNode(node, an, name);

        JS_SetReservedSlot(node->getJSObject(), 0, OBJECT_TO_JSVAL(v->getJSObject()));

        args.rval().set(OBJECT_TO_JSVAL(v->audioNode));
    } else {
        args.rval().set(JSVAL_NULL);
    }

    return true;
}

static bool native_video_nextframe(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeJSVideo, &Video_class);

    CppObj->video->nextFrame();

    return true;
}

static bool native_video_prevframe(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSNATIVE_PROLOGUE_CLASS(NativeJSVideo, &Video_class);

    CppObj->video->prevFrame();

    return true;
}

static bool native_video_frameat(JSContext *cx, unsigned argc, JS::Value *vp)
{
    double time;
    bool keyframe;

    JSNATIVE_PROLOGUE_CLASS(NativeJSVideo, &Video_class);

    if (!JS_ConvertArguments(cx, args, "db", &time, keyframe)) {
        return true;
    }

    CppObj->video->frameAt(time, keyframe);

    return true;
}

static bool native_video_setsize(JSContext *cx, unsigned argc, JS::Value *vp)
{
    double width;
    double height;
    JSNATIVE_PROLOGUE_CLASS(NativeJSVideo, &Video_class);

    if (argc < 2) {
        JS_ReportError(cx, "Wrong arguments count");
        return false;
    }

    JS::RootedValue jwidth(cx, args.array()[0]);
    JS::RootedValue jheight(cx, args.array()[1]);

    if (jwidth.isString()) {
        width = -1;
    } else if (jwidth.isNumber()) {
        width = jwidth.toDouble();
    } else {
        JS_ReportError(cx, "Wrong argument type for width");
        return false;
    }

    if (jheight.isString()) {
        height = -1;
    } else if (jheight.isNumber()) {
        height = jheight.toDouble();
    } else {
        JS_ReportError(cx, "Wrong argument type for height");
        return false;
    }

    CppObj->setSize(width, height);

    return true;
}

static bool native_Video_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &Video_class, args));
    JS::RootedObject canvas(cx);

    if (!JS_ConvertArguments(cx, args, "o", canvas.address())) {
        return true;
    }

    NativeCanvasHandler *handler = static_cast<class NativeJSCanvas*>(
        JS_GetInstancePrivate(cx, canvas, &Canvas_class, &args))->getHandler();

    if (!handler) {
        JS_ReportError(cx, "Video constructor argument must be Canvas");
        return false;
    }

    NativeCanvasContext *ncc = handler->getContext();
    if (ncc == NULL || ncc->m_Mode != NativeCanvasContext::CONTEXT_2D) {
        JS_ReportError(cx, "Invalid destination canvas (must be backed by a 2D context)");
        return false;
    }
    JSContext *m_Cx = cx;
    NJS->rootObjectUntilShutdown(ret);

    NativeJSVideo *v = new NativeJSVideo(ret, (NativeCanvas2DContext*)ncc, cx);

    JS_DefineFunctions(cx, ret, Video_funcs);
    JS_DefineProperties(cx, ret, Video_props);

    JS_SetPrivate(ret, v);
    JS::RootedValue canv(cx, args.array()[0]);

    JS_SetProperty(cx, ret, "canvas", canv);

    args.rval().set(OBJECT_TO_JSVAL(ret));

    return true;
}

NativeJSVideo::~NativeJSVideo()
{
    JSAutoRequest ar(cx);
    m_IsDestructing = true;
    if (this->audioNode.get()) {
        NativeJSAudioNode *node = static_cast<NativeJSAudioNode *>(JS_GetPrivate(this->audioNode));
        NJS->unrootObject(this->audioNode);
        if (node) {
            // This will remove the source from
            // NativeJSAudio nodes list and NativeAudio sources
            delete node;
        }
    }

    if (this->arrayContent != NULL) {
        free(this->arrayContent);
    }

    delete this->video;
}

static void Video_Finalize(JSFreeOp *fop, JSObject *obj) {
    NativeJSVideo *v = (NativeJSVideo *)JS_GetPrivate(obj);

    if (v != NULL) {
        delete v;
    }
}

bool NativeJSAVSource::propSetter(NativeAVSource *source, uint8_t id, JS::MutableHandleValue vp)
{
    switch(id) {
        case SOURCE_PROP_POSITION:
            if (vp.isNumber()) {
                source->seek(vp.toDouble());
            }
            break;
        default:
            break;
    }

    return true;
}

bool NativeJSAVSource::propGetter(NativeAVSource *source, JSContext *cx, uint8_t id, JS::MutableHandleValue vp)
{
    switch(id) {
        case SOURCE_PROP_POSITION:
            vp.setDouble(source->getClock());
        break;
        case SOURCE_PROP_DURATION:
            vp.setDouble(source->getDuration());
        break;
        case SOURCE_PROP_BITRATE:
            vp.setInt32(source->getBitrate());
        break;
        case SOURCE_PROP_METADATA:
        {
            AVDictionaryEntry *tag = NULL;
            AVDictionary *cmetadata = source->getMetadata();

            if (cmetadata != NULL) {
                JS::RootedObject metadata(cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));

                while ((tag = av_dict_get(cmetadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
                    JS::RootedString val(cx, JS_NewStringCopyN(cx, tag->value, strlen(tag->value)));
                    JS::RootedValue value(cx, STRING_TO_JSVAL(val));
                    JS_DefineProperty(cx, metadata, tag->key, value, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
                }

                vp.setObject(*metadata);
            } else {
                vp.setUndefined();
            }
        }
        break;
        default:
        break;
    }

    return true;
}

bool NativeJSAudioNode::propSetter(NativeJSAudioNode *jnode, JSContext *cx,
        uint8_t id, JS::MutableHandleValue vp)
{
    NativeAudioCustomSource *source = static_cast<NativeAudioCustomSource*>(jnode->node);

    switch(id) {
        case SOURCE_PROP_POSITION:
            if (vp.isNumber()) {
                source->seek(vp.toDouble());
            }
        break;
        case CUSTOM_SOURCE_PROP_SEEK: {
            JSTransferableFunction *fun = new JSTransferableFunction(cx);
            int funID = NativeJSAudioNode::SEEK_FN;
            if (!fun->prepare(cx, vp)) {
                JS_ReportError(cx, "Failed to read custom node callback function\n");
                vp.set(JS::NullHandleValue);
                delete fun;
                return false;
            }

            if (jnode->m_TransferableFuncs[funID] != NULL) {
                delete jnode->m_TransferableFuncs[funID];
            }

            jnode->m_TransferableFuncs[funID] = fun;

            source->setSeek(NativeJSAudioNode::seekCallback, jnode);
        }
        break;
        default:
        break;
    }

    return true;
}

void NativeJSAudioNode::seekCallback(NativeAudioCustomSource *node, double seekTime, void *custom)
{
    NativeJSAudioNode *jnode = static_cast<NativeJSAudioNode*>(custom);

    JSTransferableFunction *fn = jnode->m_TransferableFuncs[NativeJSAudioNode::SEEK_FN];
    if (!fn) return;

    JS::AutoValueArray<2> params(jnode->audio->tcx);
    JS::RootedValue rval(jnode->audio->tcx);
    JS::RootedObject global(jnode->audio->tcx, JS::CurrentGlobalOrNull(jnode->audio->tcx));
    JS::RootedObject nodeObj(jnode->audio->tcx, jnode->nodeObj);
    params[0].set(DOUBLE_TO_JSVAL(seekTime));
    params[1].set(OBJECT_TO_JSVAL(global));

    fn->call(jnode->audio->tcx, nodeObj, params, &rval);
}

void NativeJSAudioNode::registerObject(JSContext *cx)
{
    JS::RootedObject obj(cx);
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    obj = JS_InitClass(cx, global, JS::NullPtr(),
            &AudioNode_class, native_AudioNode_constructor, 0,
            AudioNode_props, AudioNode_funcs, nullptr, nullptr);
}

void NativeJSAudio::registerObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS_InitClass(cx, global, JS::NullPtr(),
        &Audio_class, native_Audio_constructor, 0,
        nullptr, nullptr, nullptr, Audio_static_funcs);

    JS_InitClass(cx, global, JS::NullPtr(),
        &AudioContext_class, native_Audio_constructor, 0,
        AudioContext_props, AudioContext_funcs, nullptr, nullptr);
}

NATIVE_OBJECT_EXPOSE(Video);

