#include "NativeJSAV.h"
#include "NativeSharedMessages.h"
#include "NativeJSThread.h"
#include "NativeJS.h"

#include "NativeCanvasHandler.h"
#include "NativeCanvas2DContext.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
}

// TODO : Need to handle nodes GC, similar to https://dvcs.w3.org/hg/audio/raw-file/tip/webaudio/specification.html#lifetime-AudioNode
// TODO : Need to handle video GC
// TODO : When stop/pause/kill fade out sound

NativeJSAudio *NativeJSAudio::instance = NULL;
extern JSClass Canvas_class;

#define NJS (NativeJS::getNativeClass(cx))
#define JS_PROPAGATE_ERROR(cx, ...)\
JS_ReportError(cx, __VA_ARGS__);\
if (!JS_ReportPendingException(cx)) {\
    JS_ClearPendingException(cx);\
}
#define NATIVE_AUDIO_GETTER(obj) ((class NativeJSAudio *)JS_GetPrivate(obj))
#define NATIVE_AUDIO_NODE_GETTER(obj) ((class NativeJSAudioNode *)JS_GetPrivate(obj))
#define NATIVE_VIDEO_GETTER(obj) ((class NativeJSVideo *)JS_GetPrivate(obj));
#define CHECK_INVALID_CTX(obj) if (!obj) {\
JS_ReportError(cx, "Invalid NativeAudio context");\
return JS_FALSE;\
}
void FIXMEReportError(JSContext *cx, const char *message, JSErrorReport *report)
{
    printf("%s\n", message);
}

extern void reportError(JSContext *cx, const char *message, JSErrorReport *report);

static void AudioNode_Finalize(JSFreeOp *fop, JSObject *obj);
static void Audio_Finalize(JSFreeOp *fop, JSObject *obj);

static JSBool native_Audio_constructor(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audio_getcontext(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audio_run(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audio_pFFT(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audio_load(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audio_createnode(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audio_connect(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audio_disconnect(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_audiothread_print(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_AudioNode_constructor(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_input(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_output(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_set(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_get(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_custom_set(JSContext *cx, unsigned argc, jsval *vp);
//static JSBool native_audionode_custom_get(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_custom_threaded_set(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_custom_threaded_get(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_custom_threaded_send(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_audionode_source_open(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_source_play(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_source_pause(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_source_stop(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_source_close(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_audionode_custom_source_play(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_custom_source_pause(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_custom_source_stop(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_audio_prop_setter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp);
static JSBool native_audionode_custom_prop_setter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp);
static JSBool native_audionode_source_prop_setter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp);
static JSBool native_audionode_source_prop_getter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSMutableHandleValue vp);
static JSBool native_audionode_custom_source_prop_setter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp);

static JSClass Audio_class = {
    "Audio", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Audio_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass global_AudioThread_class = {
    "_GLOBALAudioThread", JSCLASS_GLOBAL_FLAGS | JSCLASS_IS_GLOBAL,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass AudioNodeLink_class = {
    "AudioNodeLink", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass AudioNodeEvent_class = {
    "AudioNodeEvent", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass AudioNode_class = {
    "AudioNode", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, AudioNode_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass AudioNode_threaded_class = {
    "AudioNodeThreaded", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSPropertySpec Audio_props[] = {
    {"type", NODE_PROP_TYPE, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER},
    {"volume", AUDIO_PROP_VOLUME, 0, JSOP_NULLWRAPPER, JSOP_WRAPPER(native_audio_prop_setter)},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSPropertySpec AudioNode_props[] = {
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSPropertySpec AudioNodeEvent_props[] = {
    {"data", NODE_EV_PROP_DATA, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER},
    {"size", NODE_EV_PROP_SIZE, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSPropertySpec AudioNodeCustom_props[] = {
    {"process", NODE_CUSTOM_PROP_PROCESS, 0, JSOP_NULLWRAPPER, JSOP_WRAPPER(native_audionode_custom_prop_setter)},
    {"init", NODE_CUSTOM_PROP_INIT, 0, JSOP_NULLWRAPPER, JSOP_WRAPPER(native_audionode_custom_prop_setter)},
    {"setter", NODE_CUSTOM_PROP_SETTER, 0, JSOP_NULLWRAPPER, JSOP_WRAPPER(native_audionode_custom_prop_setter)},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSFunctionSpec Audio_funcs[] = {
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
    JS_FN("input", native_audionode_input, 1, 0),
    JS_FN("output", native_audionode_output, 1, 0),
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
    {"position", SOURCE_PROP_POSITION, 
        JSPROP_ENUMERATE|JSPROP_PERMANENT, 
        JSOP_WRAPPER(native_audionode_source_prop_getter), 
        JSOP_WRAPPER(native_audionode_source_prop_setter)},
    {"duration", SOURCE_PROP_DURATION, 
        JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY, 
        JSOP_WRAPPER(native_audionode_source_prop_getter), 
        JSOP_NULLWRAPPER},
    {"metadata", SOURCE_PROP_METADATA, 
        JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY, 
        JSOP_WRAPPER(native_audionode_source_prop_getter), 
        JSOP_NULLWRAPPER},
     {"bitrate", SOURCE_PROP_BITRATE, 
        JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY, 
        JSOP_WRAPPER(native_audionode_source_prop_getter), 
        JSOP_NULLWRAPPER},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSPropertySpec AudioNodeCustomSource_props[] = {
    {"seek", CUSTOM_SOURCE_PROP_SEEK, 
        JSPROP_ENUMERATE|JSPROP_PERMANENT, 
        JSOP_NULLWRAPPER, 
        JSOP_WRAPPER(native_audionode_custom_source_prop_setter)},
#if 0
    {"duration", SOURCE_PROP_DURATION, 
        JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY, 
        JSOP_WRAPPER(native_audionode_source_prop_getter), 
        JSOP_NULLWRAPPER},
    {"metadata", SOURCE_PROP_METADATA, 
        JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY, 
        JSOP_WRAPPER(native_audionode_source_prop_getter), 
        JSOP_NULLWRAPPER},
     {"bitrate", SOURCE_PROP_BITRATE, 
        JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY, 
        JSOP_WRAPPER(native_audionode_source_prop_getter), 
        JSOP_NULLWRAPPER},
#endif
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSFunctionSpec glob_funcs_threaded[] = {
    JS_FN("echo", native_audiothread_print, 1, 0),
    JS_FS_END
};


static JSBool native_Video_constructor(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_video_play(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_video_pause(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_video_stop(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_video_close(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_video_open(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_video_get_audionode(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_video_nextframe(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_video_prevframe(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_video_frameat(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_video_prop_getter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSMutableHandleValue vp);
static JSBool native_video_prop_setter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp);

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
    JS_FS_END
};

static JSClass Video_class = {
    "Video", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Video_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSPropertySpec Video_props[] = {
    {"width", VIDEO_PROP_WIDTH, 
        JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT, 
        JSOP_WRAPPER(native_video_prop_getter), 
        JSOP_NULLWRAPPER},
    {"height", VIDEO_PROP_HEIGHT, 
        JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT, 
        JSOP_WRAPPER(native_video_prop_getter), 
        JSOP_NULLWRAPPER},
    {"position", SOURCE_PROP_POSITION, 
        JSPROP_ENUMERATE|JSPROP_PERMANENT, 
        JSOP_WRAPPER(native_video_prop_getter), 
        JSOP_WRAPPER(native_video_prop_setter)},
    {"duration", SOURCE_PROP_DURATION, 
        JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY, 
        JSOP_WRAPPER(native_video_prop_getter), 
        JSOP_NULLWRAPPER},
    {"metadata", SOURCE_PROP_METADATA, 
        JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY, 
        JSOP_WRAPPER(native_video_prop_getter), 
        JSOP_NULLWRAPPER},
     {"bitrate", SOURCE_PROP_BITRATE, 
        JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY, 
        JSOP_WRAPPER(native_video_prop_getter), 
        JSOP_NULLWRAPPER},
    {"onframe", VIDEO_PROP_ONFRAME, 
        JSPROP_ENUMERATE|JSPROP_PERMANENT, 
        JSOP_NULLWRAPPER, 
        JSOP_NULLWRAPPER},
    {"canvas", VIDEO_PROP_CANVAS, 
        JSPROP_ENUMERATE|JSPROP_PERMANENT, 
        JSOP_NULLWRAPPER, 
        JSOP_NULLWRAPPER},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

int FFT(int dir,int nn,double *x,double *y)
{
   long m,i,i1,j,k,i2,l,l1,l2;
   double c1,c2,tx,ty,t1,t2,u1,u2,z;

   m = log2(nn);

   /* Do the bit reversal */
   i2 = nn >> 1;
   j = 0;
   for (i=0;i<nn-1;i++) {
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
   for (l=0;l<m;l++) {
      l1 = l2;
      l2 <<= 1;
      u1 = 1.0;
      u2 = 0.0;
      for (j=0;j<l1;j++) {
         for (i=j;i<nn;i+=l2) {
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
      for (i=0;i<nn;i++) {
         x[i] /= (double)nn;
         y[i] /= (double)nn;
      }
   }

   return true;
}

bool JSTransferableFunction::prepare(JSContext *cx, jsval val)
{
    if (!JS_WriteStructuredClone(cx, val, &m_Data, &m_Bytes, NULL, NULL, JSVAL_VOID)) {
        return false;
    }

    return true;
}

bool JSTransferableFunction::call(JSContext *cx, JSObject *obj, int argc, jsval *params, jsval *rval)
{
    if (m_Fn == NULL) {
        if (m_Data == NULL) return false;

        m_Fn = new JS::Value();
        JS_AddValueRoot(cx, m_Fn);
        m_Fn->setNull();

        if (!this->transfert(cx)) {
            JS_RemoveValueRoot(cx, m_Fn);
            delete m_Fn;
            m_Fn = NULL;
            return false;
        } else {
            // Function is transfered
        }
    }

    return JS_CallFunctionValue(cx, obj, *m_Fn, argc, params, rval);
}

bool JSTransferableFunction::transfert(JSContext *destCx)
{
    if (m_DestCx != NULL) return false;

    m_DestCx = destCx;

    bool ok = JS_ReadStructuredClone(destCx, m_Data, m_Bytes, 
                JS_STRUCTURED_CLONE_VERSION, m_Fn, NULL, NULL);

    JS_ClearStructuredClone(m_Data, m_Bytes);

    m_Data = NULL;
    m_Bytes = 0;

    return ok;
}

JSTransferableFunction::~JSTransferableFunction()
{
    if (m_Data != NULL) {
        JS_ClearStructuredClone(m_Data, m_Bytes);
    }

    if (m_Fn != NULL) {
        delete m_Fn;
        JSAutoCompartment(m_DestCx, JS_GetGlobalObject(m_DestCx));
        JS_RemoveValueRoot(m_DestCx, m_Fn);
    }
}

NativeJSAudio *NativeJSAudio::getContext(JSContext *cx, JSObject *obj, int bufferSize, int channels, int sampleRate) 
{
    ape_global *net = static_cast<ape_global *>(JS_GetContextPrivate(cx));
    NativeAudio *audio;

    try {
        audio = new NativeAudio(net, bufferSize, channels, sampleRate);
    } catch (...) {
        return NULL;
    }

    return new NativeJSAudio(audio, cx, obj);
}

NativeJSAudio *NativeJSAudio::getContext()
{
    return NativeJSAudio::instance;
}

NativeJSAudio::NativeJSAudio(NativeAudio *audio, JSContext *cx, JSObject *obj)
    : audio(audio), nodes(NULL), shutdowned(false), jsobj(obj), gbl(NULL), rt(NULL), tcx(NULL)
{
    this->cx = cx;

    NativeJSAudio::instance = this;

    JS_SetPrivate(obj, this);

    NJS->rootObjectUntilShutdown(obj);

    JS_DefineFunctions(cx, obj, Audio_funcs);
    JS_DefineProperties(cx, obj, Audio_props);

    pthread_cond_init(&this->shutdownCond, NULL);
    pthread_mutex_init(&this->shutdownLock, NULL);

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
        JS_SetGCParameter(this->rt, JSGC_MODE, JSGC_MODE_INCREMENTAL);
        JS_SetGCParameter(this->rt, JSGC_SLICE_TIME_BUDGET, 15);

        if ((this->tcx = JS_NewContext(this->rt, 8192)) == NULL) {
            printf("Failed to init JS context\n");
            return false;     
        }

        JS_SetStructuredCloneCallbacks(this->rt, NativeJS::jsscc);

        JSAutoRequest ar(this->tcx);

        //JS_SetGCParameterForThread(this->tcx, JSGC_MAX_CODE_CACHE_BYTES, 16 * 1024 * 1024);

        this->gbl = JS_NewGlobalObject(this->tcx, &global_AudioThread_class, NULL);
        JS_AddObjectRoot(this->tcx, &this->gbl);
        if (!JS_InitStandardClasses(tcx, this->gbl)) {
            printf("Failed to init std class\n");
            return false;
        }

        JS_SetVersion(this->tcx, JSVERSION_LATEST);

        JS_SetOptions(this->tcx, JSOPTION_VAROBJFIX | JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE | JSOPTION_ION);

        JS_SetErrorReporter(this->tcx, FIXMEReportError);
        JS_SetGlobalObject(this->tcx, this->gbl);
        JS_DefineFunctions(this->tcx, this->gbl, glob_funcs_threaded);

        //JS_SetContextPrivate(this->tcx, static_cast<void *>(this));
    }

    return true;
    
}

bool NativeJSAudio::run(char *str)
{
    jsval rval;
    JSFunction *fun;

    if (!this->tcx) {
        printf("No JS context for audio thread\n");
        return false;
    }

    fun = JS_CompileFunction(this->tcx, JS_GetGlobalObject(this->tcx), "Audio_run", 0, NULL, str, strlen(str), "FILENAME (TODO)", 0);

    if (!fun) {
        JS_ReportError(this->tcx, "Failed to execute script on audio thread\n");
        return false;
    }

    JS_CallFunction(this->tcx, JS_GetGlobalObject(this->tcx), fun, 0, NULL, &rval);

    return true;
}

NativeJSAudio::~NativeJSAudio() 
{
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

    // Unroot custom nodes objets and clear threaded js context
    this->audio->sharedMsg->postMessage(
            (void *)new NativeAudioNode::CallbackMessage(NativeJSAudio::shutdownCallback, NULL, this),
            NATIVE_AUDIO_CALLBACK);

    // If audio doesn't have any tracks playing, the queue thread might sleep
    // So we need to wake it up to deliver the message
    this->audio->wakeup();

    if (!this->shutdowned) {
        pthread_cond_wait(&this->shutdownCond, &this->shutdownLock);
    }

    // Shutdown the audio
    this->audio->shutdown();

    // And delete the audio
    delete this->audio;

    NativeJSAudio::instance = NULL;
}

void NativeJSAudio::unroot() 
{
    NativeJSAudio::Nodes *nodes = this->nodes;

    NativeJS *njs = NativeJS::getNativeClass(this->cx);

    if (this->jsobj != NULL) {
        JS_RemoveObjectRoot(njs->cx, &this->jsobj);
    }
    if (this->gbl != NULL) {
        JS_RemoveObjectRoot(njs->cx, &this->gbl);
    }

    while (nodes != NULL) {
        JS_RemoveObjectRoot(njs->cx, &nodes->curr->jsobj);
        nodes = nodes->next;
    }
}

void NativeJSAudio::shutdownCallback(NativeAudioNode *dummy, void *custom)
{
    NativeJSAudio *audio = static_cast<NativeJSAudio *>(custom);
    NativeJSAudio::Nodes *nodes = audio->nodes;

    // Let's shutdown and destroy all nodes
    while (nodes != NULL) {
        if (nodes->curr->type == NativeAudio::CUSTOM ||
            nodes->curr->type == NativeAudio::CUSTOM_SOURCE) {
            nodes->curr->shutdownCallback(nodes->curr->node, nodes->curr);
        }

        nodes = nodes->next;
    }

    if (audio->tcx != NULL) {
        JSRuntime *rt = JS_GetRuntime(audio->tcx);
        JS_DestroyContext(audio->tcx);
        JS_DestroyRuntime(rt);
        audio->tcx = NULL;
    }

    audio->shutdowned = true;

    pthread_cond_signal(&audio->shutdownCond);
}

void NativeJSAudioNode::add() 
{
    NativeJSAudio::Nodes *nodes = new NativeJSAudio::Nodes(this, NULL, this->audio->nodes);

    if (this->audio->nodes != NULL) {
        this->audio->nodes->prev = nodes;
    }

    this->audio->nodes = nodes;
}

void NativeJSAudioNode::setPropCallback(NativeAudioNode *node, void *custom)
{
    NativeJSAudioNode::Message *msg;
    jsval val;
    JSContext *tcx;
    JSTransferableFunction *setterFn;

    msg = static_cast<struct NativeJSAudioNode::Message*>(custom);
    tcx = msg->jsNode->audio->tcx;

    JSAutoRequest ar(tcx);

    if (!JS_ReadStructuredClone(tcx,
                msg->clone.datap,
                msg->clone.nbytes,
                JS_STRUCTURED_CLONE_VERSION, &val, NULL, NULL)) {
        JS_PROPAGATE_ERROR(tcx, "Failed to read structured clone");

        JS_free(msg->jsNode->cx, msg->name);
        JS_ClearStructuredClone(msg->clone.datap, msg->clone.nbytes);
        delete msg;

        return;
    }

    JS_SetProperty(tcx, msg->jsNode->hashObj, msg->name, &val);

    setterFn = msg->jsNode->m_TransferableFuncs[NativeJSAudioNode::SETTER_FN];
    if (setterFn) {
        jsval params[3];
        jsval rval;
        params[0] = STRING_TO_JSVAL(JS_NewStringCopyZ(tcx, msg->name));
        params[1] = val;
        params[2] = OBJECT_TO_JSVAL(JS_GetGlobalObject(tcx));

        setterFn->call(tcx, msg->jsNode->nodeObj, 3, params, &rval);
    }

    JS_free(msg->jsNode->cx, msg->name);
    JS_ClearStructuredClone(msg->clone.datap, msg->clone.nbytes);

    delete msg;
}

void NativeJSAudioNode::customCallback(const struct NodeEvent *ev)
{
    NativeJSAudioNode *thiz;
    JSContext *tcx;
    JSTransferableFunction *processFn, *initFn;

    thiz = static_cast<NativeJSAudioNode *>(ev->custom);

    if (!thiz->audio->tcx || !thiz->cx || !thiz->jsobj || !thiz->node) {
        return;
    }

    tcx = thiz->audio->tcx;

    JSAutoRequest ar(tcx);

    processFn = thiz->m_TransferableFuncs[NativeJSAudioNode::PROCESS_FN];
    initFn = thiz->m_TransferableFuncs[NativeJSAudioNode::INIT_FN];

    if (initFn) {
        jsval params[1];
        jsval rval;

        params[0] = OBJECT_TO_JSVAL(JS_GetGlobalObject(tcx));

        initFn->call(tcx, thiz->nodeObj, 1, params, &rval);

        thiz->m_TransferableFuncs[NativeJSAudioNode::INIT_FN] = NULL;

        delete initFn;
    }

    if (!processFn) return;

    jsval rval, params[2], vFrames, vSize;
    JSObject *obj, *frames;
    unsigned long size;
    int count;

    count = thiz->node->inCount > thiz->node->outCount ? thiz->node->inCount : thiz->node->outCount;
    size = thiz->node->audio->outputParameters->bufferSize/2;

    obj = JS_NewObject(tcx, &AudioNodeEvent_class, NULL, NULL);
    JS_DefineProperties(tcx, obj, AudioNodeEvent_props);

    frames = JS_NewArrayObject(tcx, 0, NULL);

    for (int i = 0; i < count; i++) 
    {
        JSObject *arrBuff, *arr;
        uint8_t *data;

        // TODO : Avoid memcpy (custom allocator for NativeAudioNode?)
        arrBuff = JS_NewArrayBuffer(tcx, size);
        data = JS_GetArrayBufferData(arrBuff);

        memcpy(data, ev->data[i], size);

        arr = JS_NewFloat32ArrayWithBuffer(tcx, arrBuff, 0, -1);
        if (arr != NULL) {
            JS_DefineElement(tcx, frames, i, OBJECT_TO_JSVAL(arr), 
                NULL, NULL, JSPROP_ENUMERATE | JSPROP_PERMANENT);
        } else {
            JS_ReportOutOfMemory(tcx);
            return;
        }
    }

    vFrames = OBJECT_TO_JSVAL(frames);
    vSize = DOUBLE_TO_JSVAL(ev->size);

    JS_SetProperty(tcx, obj, "data", &vFrames);
    JS_SetProperty(tcx, obj, "size", &vSize);

    params[0] = OBJECT_TO_JSVAL(obj);
    params[1] = OBJECT_TO_JSVAL(JS_GetGlobalObject(tcx));

    //JS_CallFunction(tcx, thiz->nodeObj, thiz->processFn, 2, params, &rval);
    processFn->call(tcx, thiz->nodeObj, 2, params, &rval);

    for (int i = 0; i < count; i++) 
    {
        jsval val;

        JS_GetElement(tcx, frames, i, &val);

        memcpy(ev->data[i], JS_GetFloat32ArrayData(JSVAL_TO_OBJECT(val)), size);
    }
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

void NativeJSAudioNode::eventCbk(const struct NativeAVSourceEvent *cev) 
{
    NativeJSAudioNode *thiz;
    NativeJSAVMessageCallback *ev;

    thiz = static_cast<NativeJSAudioNode *>(cev->custom);
    thiz->node->ref();

    // FIXME : use cev->fromThread to avoid posting message 
    // if message is comming from main thread
    ev = new NativeJSAVMessageCallback(thiz->jsobj, cev->ev, cev->value1, cev->value2);
    
    thiz->njs->postMessage(ev, NATIVE_AV_THREAD_MESSAGE_CALLBACK);

    delete cev;
}

void NativeJSAudio::runCallback(NativeAudioNode *node, void *custom)
{
    NativeJSAudio *audio = NativeJSAudio::getContext();

    if (!audio) return;// This should not happend

    char *str = static_cast<char *>(custom);
    audio->run(str);
    JS_free(audio->cx, custom);
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

    if (node->nodeObj != NULL) {
        JS_RemoveObjectRoot(node->audio->tcx, &node->nodeObj);
    }
    if (node->hashObj != NULL) {
        JS_RemoveObjectRoot(node->audio->tcx, &node->hashObj);
    }

    node->finalized = true;

    pthread_cond_signal(&node->shutdownCond);
}

void NativeJSAudioNode::initCustomObject(NativeAudioNode *node, void *custom)
{
    NativeJSAudioNode *jnode = static_cast<NativeJSAudioNode *>(custom);
    JSContext *tcx = jnode->audio->tcx;

    jnode->hashObj = JS_NewObject(tcx, NULL, NULL, NULL);

    if (!jnode->hashObj) {
        JS_PROPAGATE_ERROR(tcx, "Failed to create hash object for custom node");
        return;
    }
    JS_AddObjectRoot(tcx, &jnode->hashObj);

    jnode->nodeObj= JS_NewObject(tcx, &AudioNode_threaded_class, NULL, NULL);
    if (!jnode->nodeObj) {
        JS_PROPAGATE_ERROR(tcx, "Failed to create node object for custom node");
        return;
    }

    JS_AddObjectRoot(tcx, &jnode->nodeObj);

    JS_DefineFunctions(tcx, jnode->nodeObj, AudioNodeCustom_threaded_funcs);

    JS_SetPrivate(jnode->nodeObj, static_cast<void *>(jnode));
}


NativeJSAudioNode::~NativeJSAudioNode()
{
    NativeJSAudio::Nodes *nodes = this->audio->nodes;

    if (this->type == NativeAudio::SOURCE) {
        // Only source from NativeVideo has reserved slot
        JS::Value s = JS_GetReservedSlot(this->jsobj, 0);
        JSObject *obj = s.toObjectOrNull();
        if (obj != NULL) {
            // If it exist, we must inform the video 
            // that audio node no longer exist
            NativeJSVideo *video = NATIVE_VIDEO_GETTER(obj);
            if (video != NULL) {
                JS_SetReservedSlot(this->jsobj, 0, JSVAL_NULL);
                video->stopAudio();
            }
        }
    }

    // Remove node from nodes linked list
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

    // Custom node must be finalized (in his own thread)
    if ((this->type == NativeAudio::CUSTOM ||
         this->type == NativeAudio::CUSTOM_SOURCE) && !this->finalized) {
        this->node->callback(
                NativeJSAudioNode::shutdownCallback, this);

        this->audio->audio->wakeup();

        if (!this->finalized) {
            pthread_cond_wait(&this->shutdownCond, &this->shutdownLock);
        }
    }

    if (this->arrayContent != NULL) {
        free(this->arrayContent);
    }

    for (int i = 0; i < NativeJSAudioNode::END_FN; i++) {
        delete m_TransferableFuncs[i];
        m_TransferableFuncs[i] = NULL;
    }

    // Block NativeAudio threads execution.
    // While the node is destructed we don't want any thread 
    // to call some method on a node that is being destroyed
    this->audio->audio->lockThreads();

    this->node->unref();

    JS_SetPrivate(this->jsobj, NULL);

    this->audio->audio->unlockThreads();
}


static JSBool native_audio_pFFT(JSContext *cx, unsigned argc, jsval *vp)
{
    JS::CallArgs args = CallArgsFromVp(argc, vp);
    JSObject *x, *y;
    int dir, n;

//int FFT(int dir,int m,double *x,double *y)

    if (!JS_ConvertArguments(cx, args.length(), args.array(),
        "ooii", &x, &y, &n, &dir)) {
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

static JSBool native_Audio_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JS_ReportError(cx, "Illegal constructor");
    return false;
}

static JSBool native_audio_getcontext(JSContext *cx, unsigned argc, jsval *vp)
{
    unsigned int bufferSize, channels, sampleRate;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuu", &bufferSize, &channels, &sampleRate)) {
        return false;
    }

    switch (bufferSize) {
        case 16:
        case 32:
        case 64:
        case 128:
        case 256:
        case 512:
            // Supported buffer size
            // Multiply by 8 to get the bufferSize in bytes 
            // rather than in samples per buffer
            bufferSize *= 8;
            break;
        default :
            JS_ReportError(cx, "Unsuported buffer size %d. Supported values are : 16, 32, 64, 128, 256, 512 \n", bufferSize);
            return JS_FALSE;
            break;
    }

    if (channels < 1 || channels > 32) {
        JS_ReportError(cx, "Unsuported channels number %d. Channels must be between 1 and 32\n", channels);
        return JS_FALSE;
    }

    if (sampleRate < 22050 || sampleRate> 96000) {
        JS_ReportError(cx, "Unsuported sample rate %dKHz. Sample rate must be between 22050 and 96000\n", sampleRate);
        return JS_FALSE;
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
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jaudio->jsobj));
        return JS_TRUE;
    }

    if (paramsChanged) {
        JS_SetPrivate(jaudio->jsobj, NULL);
        NJS->unrootObject(jaudio->jsobj);
        delete jaudio;
    } 

    JSObject *ret = JS_NewObjectForConstructor(cx, &Audio_class, vp);
    NativeJSAudio *naudio = NativeJSAudio::getContext(cx, ret, bufferSize, channels, sampleRate);

    JS_DefineProperty(cx, ret, "bufferSize", INT_TO_JSVAL(bufferSize/8), NULL, NULL, 
            JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
    JS_DefineProperty(cx, ret, "channels", INT_TO_JSVAL(channels), NULL, NULL, 
            JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
    JS_DefineProperty(cx, ret, "sampleRate", INT_TO_JSVAL(sampleRate), NULL, NULL, 
            JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);

    if (naudio == NULL) {
        delete naudio;
        JS_ReportError(cx, "Failed to initialize audio context\n");
        return JS_FALSE;
    }

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

static JSBool native_audio_run(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *fn;
    JSFunction *nfn;
    jsval *argv = JS_ARGV(cx, vp);
    NativeJSAudio *audio = NativeJSAudio::getContext();

    CHECK_INVALID_CTX(audio);

    if ((nfn = JS_ValueToFunction(cx, argv[0])) == NULL ||
        (fn = JS_DecompileFunctionBody(cx, nfn, 0)) == NULL) {
        JS_ReportError(cx, "Failed to read callback function\n");
        return JS_FALSE;
    } 

    char *funStr = JS_EncodeString(cx, fn);

    audio->audio->sharedMsg->postMessage(
            (void *)new NativeAudioNode::CallbackMessage(NativeJSAudio::runCallback, NULL, static_cast<void *>(funStr)), 
            NATIVE_AUDIO_CALLBACK);


    return JS_TRUE;
}

static JSBool native_audio_load(JSContext *cx, unsigned argc, jsval *vp)
{
    JS_ReportError(cx, "Not implemented");
    return false;
}

static JSBool native_audio_createnode(JSContext *cx, unsigned argc, jsval *vp)
{
    int in, out;
    JSObject *ret;
    JSString *name;
    NativeJSAudio *audio = NATIVE_AUDIO_GETTER(JS_THIS_OBJECT(cx, vp));
    NativeJSAudioNode *node;

    CHECK_INVALID_CTX(audio);

    node = NULL;
    ret = NULL;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "Suu", &name, &in, &out)) {
        return false;
    }

    JSAutoByteString cname(cx, name);
    ret = JS_NewObjectForConstructor(cx, &AudioNode_class, vp);
    if (!ret) {
        JS_ReportOutOfMemory(cx);
        return false;
    }

    JS_SetReservedSlot(ret, 0, JSVAL_NULL);

    try {
        if (strcmp("source", cname.ptr()) == 0) {
            node = new NativeJSAudioNode(NativeAudio::SOURCE, in, out, audio);
            NativeAudioSource *source = static_cast<NativeAudioSource*>(node->node);
            source->eventCallback(NativeJSAudioNode::eventCbk, node);
            JS_DefineFunctions(cx, ret, AudioNodeSource_funcs);
            JS_DefineProperties(cx, ret, AudioNodeSource_props);
        } else if (strcmp("custom-source", cname.ptr()) == 0) {
            node = new NativeJSAudioNode(NativeAudio::CUSTOM_SOURCE, in, out, audio);
            JS_DefineProperties(cx, ret, AudioNodeCustom_props);
            JS_DefineFunctions(cx, ret, AudioNodeCustom_funcs);
            JS_DefineFunctions(cx, ret, AudioNodeCustomSource_funcs);
            JS_DefineProperties(cx, ret, AudioNodeCustomSource_props);
        } else if (strcmp("custom", cname.ptr()) == 0) {
            node = new NativeJSAudioNode(NativeAudio::CUSTOM, in, out, audio);
            JS_DefineProperties(cx, ret, AudioNodeCustom_props);
            JS_DefineFunctions(cx, ret, AudioNodeCustom_funcs);
        } else if (strcmp("reverb", cname.ptr()) == 0) {
            node = new NativeJSAudioNode(NativeAudio::REVERB, in, out, audio);
        } else if (strcmp("delay", cname.ptr()) == 0) {
            node = new NativeJSAudioNode(NativeAudio::DELAY, in, out, audio);
        } else if (strcmp("gain", cname.ptr()) == 0) {
            node = new NativeJSAudioNode(NativeAudio::GAIN, in, out, audio);
        } else if (strcmp("target", cname.ptr()) == 0) {                      
            node = new NativeJSAudioNode(NativeAudio::TARGET, in, out, audio);
        } else if (strcmp("stereo-enhancer", cname.ptr()) == 0) {
            node = new NativeJSAudioNode(NativeAudio::STEREO_ENHANCER, in, out, audio);
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

    jsval tmp = STRING_TO_JSVAL(name);
    JS_SetProperty(cx, ret, "type", &tmp);

    node->njs = NJS;
    node->jsobj = ret;
    node->cx = cx;

    NJS->rootObjectUntilShutdown(node->jsobj);
    JS_SetPrivate(ret, node);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return true;
}

static JSBool native_audio_connect(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *link1;
    JSObject *link2;
    NodeLink *nlink1;
    NodeLink *nlink2;
    NativeJSAudio *jaudio = NATIVE_AUDIO_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jaudio);

    NativeAudio *audio = jaudio->audio;;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oo", &link1, &link2)) {
        return JS_TRUE;
    }

    nlink1 = (NodeLink *)JS_GetInstancePrivate(cx, link1, &AudioNodeLink_class, JS_ARGV(cx, vp));
    nlink2 = (NodeLink *)JS_GetInstancePrivate(cx, link2, &AudioNodeLink_class, JS_ARGV(cx, vp));

    if (nlink1 == NULL || nlink2 == NULL) {
        JS_ReportError(cx, "Bad AudioNodeLink\n");
        return JS_FALSE;
    }

    if (nlink1->type == INPUT && nlink2->type == OUTPUT) {
        if (!audio->connect(nlink2, nlink1)) {
            JS_ReportError(cx, "connect() failed (max connection reached)\n");
            return JS_FALSE;
        }
    } else if (nlink1->type == OUTPUT && nlink2->type == INPUT) {
        if (!audio->connect(nlink1, nlink2)) {
            JS_ReportError(cx, "connect() failed (max connection reached)\n");
            return JS_FALSE;
        }
    } else {
        JS_ReportError(cx, "connect() take one input and one output\n");
        return JS_FALSE;
    }

    return JS_TRUE;
}

static JSBool native_audio_disconnect(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *link1;
    JSObject *link2;
    NodeLink *nlink1;
    NodeLink *nlink2;
    NativeJSAudio *jaudio = NATIVE_AUDIO_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jaudio);

    NativeAudio *audio = jaudio->audio;;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oo", &link1, &link2)) {
        return JS_TRUE;
    }

    nlink1 = (NodeLink *)JS_GetInstancePrivate(cx, link1, &AudioNodeLink_class, JS_ARGV(cx, vp));
    nlink2 = (NodeLink *)JS_GetInstancePrivate(cx, link2, &AudioNodeLink_class, JS_ARGV(cx, vp));

    if (nlink1 == NULL || nlink2 == NULL) {
        JS_ReportError(cx, "Bad AudioNodeLink\n");
        return JS_FALSE;
    }

    if (nlink1->type == INPUT && nlink2->type == OUTPUT) {
        audio->disconnect(nlink2, nlink1);
    } else if (nlink1->type == OUTPUT && nlink2->type == INPUT) {
        audio->disconnect(nlink1, nlink2);
    } else {
        JS_ReportError(cx, "disconnect() take one input and one output\n");
        return JS_FALSE;
    }

    return JS_TRUE;
}

static JSBool native_audiothread_print(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *str;
    jsval *argv;
    char *bytes;

    argv = JS_ARGV(cx, vp);

    str = JS_ValueToString(cx, argv[0]);
    if (!str)
        return JS_TRUE;

    bytes = JS_EncodeString(cx, str);
    if (!bytes)
        return JS_TRUE;

    printf("%s\n", bytes);

    JS_free(cx, bytes);

    return JS_TRUE;
}

static JSBool native_audio_prop_setter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
    NativeJSAudio *jaudio = NativeJSAudio::getContext();

    CHECK_INVALID_CTX(jaudio);

    if (vp.isNumber()) {
        jaudio->audio->setVolume((float)vp.toNumber());
    } 

    return JS_TRUE;
}

static JSBool native_AudioNode_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JS_ReportError(cx, "Illegal constructor");
    return false;
}

static JSBool native_audionode_input(JSContext *cx, unsigned argc, jsval *vp)
{
    int channel;
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    NativeAudioNode *node = jnode->node;
    JSObject *ret = JS_NewObject(cx, &AudioNodeLink_class, NULL, NULL);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &channel)) {
        return JS_TRUE;
    }

    if (channel < 0 || channel >= node->inCount) {
        JS_ReportError(cx, "Wrong input channel\n");
        return JS_FALSE;
    }

    JS_SetPrivate(ret, node->input[channel]);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;

}

static JSBool native_audionode_output(JSContext *cx, unsigned argc, jsval *vp)
{
    int channel;
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    NativeAudioNode *node = jnode->node;
    JSObject *ret = JS_NewObject(cx, &AudioNodeLink_class, NULL, NULL);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &channel)) {
        return JS_TRUE;
    }

    if (channel < 0 || channel > node->outCount) {
        JS_ReportError(cx, "Wrong output channel\n");
        return JS_FALSE;
    }

    JS_SetPrivate(ret, node->output[channel]);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

static JSBool native_audionode_set(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *name;
    ArgType type;
    void *value;
    int intVal = 0;
    double doubleVal = 0;
    unsigned long size;
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    NativeAudioNode *node = jnode->node;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &name)) {
        return JS_TRUE;
    }

    JSAutoByteString cname(cx, name);
    JS::Value val = JS_ARGV(cx, vp)[1];

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
        return JS_FALSE;
    }

    if (!node->set(cname.ptr(), type, value, size)) {
        JS_ReportError(cx, "Unknown argument name %s\n", cname.ptr());
        return JS_FALSE;
    }

    return JS_TRUE;
}

static JSBool native_audionode_get(JSContext *cx, unsigned argc, jsval *vp)
{
    return JS_TRUE;
}

static JSBool native_audionode_custom_set(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *name;
    NativeJSAudioNode::Message *msg;
    NativeAudioNodeCustom *node;
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    if (argc != 2) {
        JS_ReportError(cx, "set() require two arguments");
        return false;
    }

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &name)) {
        return false;
    }

    node = static_cast<NativeAudioNodeCustom *>(jnode->node);

    msg = new NativeJSAudioNode::Message();
    msg->name = JS_EncodeString(cx, name);
    msg->jsNode = jnode;

    if (!JS_WriteStructuredClone(cx, JS_ARGV(cx, vp)[1], 
            &msg->clone.datap, &msg->clone.nbytes,
            NULL, NULL, JSVAL_VOID)) {
        JS_ReportError(cx, "Failed to write structured clone");

        JS_free(cx, msg->name);
        delete msg;

        return false;
    }

    node->callback(NativeJSAudioNode::setPropCallback, msg);

    return JS_TRUE;
}

#if 0
static JSBool native_audionode_custom_get(JSContext *cx, unsigned argc, jsval *vp)
{
    printf("hello get\n");

    NativeJSAudioNode *jsNode;
    jsval val;
    JSString *name;
    printf("get node\n");

    jsNode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    printf("convert\n");
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &name)) {
        return JS_TRUE;
    }

    printf("str\n");
    JSAutoByteString str(cx, name);

    JS_SetRuntimeThread(JS_GetRuntime(cx));

    printf("get\n");
    JS_GetProperty(jsNode->audio->tcx, jsNode->hashObj, str.ptr(), &val);

    printf("return\n");
    JS_ClearRuntimeThread(JS_GetRuntime(cx));

    JS_SET_RVAL(cx, vp, val);

    return JS_TRUE;
}
#endif

static JSBool native_audionode_custom_threaded_set(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *name;
    JS::Value val;
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));
    JSTransferableFunction *fn;

    CHECK_INVALID_CTX(jnode);

    if (argc != 2) {
        JS_ReportError(cx, "set() require two arguments\n");
        return false;
    }

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &name)) {
        return false;
    }

    fn = jnode->m_TransferableFuncs[NativeJSAudioNode::SETTER_FN];
    val = JS_ARGV(cx, vp)[1];

    JSAutoByteString str(cx, name);

    JS_SetProperty(cx, jnode->hashObj, str.ptr(), &val);

    if (fn) {
        jsval params[4];
        jsval rval;

        params[0] = STRING_TO_JSVAL(name);
        params[1] = val;
        params[2] = OBJECT_TO_JSVAL(JS_GetGlobalObject(cx));

        fn->call(cx, jnode->nodeObj, 3, params, &rval);
    }

    return true;
}

static JSBool native_audionode_custom_threaded_get(JSContext *cx, unsigned argc, jsval *vp)
{
    jsval val;
    JSString *name;
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    if (jnode->hashObj == NULL) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return false;
    }

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &name)) {
        return false;
    }

    JSAutoByteString str(cx, name);

    JS_GetProperty(jnode->audio->tcx, jnode->hashObj, str.ptr(), &val);

    JS_SET_RVAL(cx, vp, val);

    return true;
}

static JSBool native_audionode_custom_threaded_send(JSContext *cx, unsigned argc, jsval *vp)
{
    uint64_t *datap;
    size_t nbytes;
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    struct native_thread_msg *msg;

    if (!JS_WriteStructuredClone(cx, JS_ARGV(cx, vp)[0], &datap, &nbytes,
        NULL, NULL, JSVAL_VOID)) {
        JS_ReportError(cx, "Failed to write structured clone");
        return JS_FALSE;
    }

    msg = new struct native_thread_msg;

    msg->data   = datap;
    msg->nbytes = nbytes;
    msg->callee = jnode->jsobj;

    jnode->njs->postMessage(msg, NATIVE_THREAD_MESSAGE);

    return JS_TRUE;
}

static JSBool native_audionode_source_open(JSContext *cx, unsigned argc, jsval *vp) 
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    NativeAudioSource *source = (NativeAudioSource *)jnode->node;

    JS::Value src = JS_ARGV(cx, vp)[0];

    int ret = -1;

    if (src.isString()) {
        JSAutoByteString csrc(cx, src.toString());
        ret = source->open(NativeJS::getNativeClass(cx)->getPath(), csrc.ptr());
    } else if (src.isObject()) {
        JSObject *arrayBuff = src.toObjectOrNull();

        if (!JS_IsArrayBufferObject(arrayBuff)) {
            JS_ReportError(cx, "Data is not an ArrayBuffer\n");
            return JS_FALSE;
        }

        int length;
        uint8_t *data;

        length = JS_GetArrayBufferByteLength(arrayBuff);
        JS_StealArrayBufferContents(cx, arrayBuff, &jnode->arrayContent, &data);

        ret = source->open(data, length);
    }

    
    if (ret < 0) {
        JS_ReportError(cx, "Failed to open stream %d\n", ret);
        return JS_FALSE;
    }

    /*
    int bufferSize = sizeof(uint8_t)*4*1024*1024;
    uint8_t *buffer;

    buffer = (uint8_t *)malloc(bufferSize);
    load("/tmp/foo.wav", buffer, bufferSize);
    int ret = source->open(buffer, bufferSize);
    */

    return JS_TRUE;
}
static JSBool native_audionode_source_play(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    NativeAudioSource *source = static_cast<NativeAudioSource *>(jnode->node);

    source->play();

    return JS_TRUE;
}

static JSBool native_audionode_source_pause(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    NativeAudioSource *source = static_cast<NativeAudioSource *>(jnode->node);

    source->pause();
    return JS_TRUE;
}

static JSBool native_audionode_source_stop(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    NativeAudioSource *source = static_cast<NativeAudioSource *>(jnode->node);

    source->stop();
    return JS_TRUE;
}

static JSBool native_audionode_source_close(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    NativeAudioSource *source = static_cast<NativeAudioSource *>(jnode->node);

    source->close();
    return JS_TRUE;
}

static JSBool native_audionode_source_prop_getter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSMutableHandleValue vp)
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(obj);

    CHECK_INVALID_CTX(jnode);

    NativeAudioSource *source = static_cast<NativeAudioSource *>(jnode->node);

    return NativeJSAVSource::propGetter(source, cx, JSID_TO_INT(id), vp);
}

static JSBool native_audionode_source_prop_setter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(obj);

    CHECK_INVALID_CTX(jnode);

    NativeAudioSource *source = static_cast<NativeAudioSource*>(jnode->node);

    return NativeJSAVSource::propSetter(source, JSID_TO_INT(id), vp);
}

static JSBool native_audionode_custom_source_play(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    NativeAudioCustomSource *source = static_cast<NativeAudioCustomSource *>(jnode->node);

    source->play();

    return JS_TRUE;
}

static JSBool native_audionode_custom_source_pause(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    NativeAudioCustomSource *source = static_cast<NativeAudioCustomSource *>(jnode->node);

    source->pause();

    return JS_TRUE;
}

static JSBool native_audionode_custom_source_stop(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    NativeAudioCustomSource *source = static_cast<NativeAudioCustomSource *>(jnode->node);

    source->stop();

    return JS_TRUE;
}

static JSBool native_audionode_custom_source_prop_setter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(obj);

    CHECK_INVALID_CTX(jnode);

    return NativeJSAudioNode::propSetter(jnode, cx, JSID_TO_INT(id), vp);
}

static JSBool native_audionode_custom_prop_setter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(obj);
    JSTransferableFunction *fun;
    NativeAudioNodeCustom *node;
    NativeJSAudioNode::TransferableFunction funID;

    CHECK_INVALID_CTX(jnode);

    node = static_cast<NativeAudioNodeCustom *>(jnode->node);

    switch(JSID_TO_INT(id)) {
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

    fun = new JSTransferableFunction();

    if (!fun->prepare(cx, vp.get())) {
        JS_ReportError(cx, "Failed to read custom node callback function\n");
        vp.set(JSVAL_VOID);
        delete fun;
        return false;
    }

    if (jnode->m_TransferableFuncs[funID] != NULL) {
        delete jnode->m_TransferableFuncs[funID];
    }

    jnode->m_TransferableFuncs[funID] = fun;


    if (JSID_TO_INT(id) == NODE_CUSTOM_PROP_PROCESS) {
        node->callback(NativeJSAudioNode::initCustomObject, static_cast<void *>(jnode));
        node->setCallback(NativeJSAudioNode::customCallback, static_cast<void *>(jnode));
    }

    return true;
}

static JSBool native_video_prop_getter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSMutableHandleValue vp)
{
    NativeJSVideo *v = NATIVE_VIDEO_GETTER(obj);

    switch(JSID_TO_INT(id)) {
        case VIDEO_PROP_WIDTH:
            vp.set(INT_TO_JSVAL(v->video->width));
        break;
        case VIDEO_PROP_HEIGHT:
            vp.set(INT_TO_JSVAL(v->video->height));
        break;
        default:
            return NativeJSAVSource::propGetter(v->video, cx, JSID_TO_INT(id), vp);
            break;
    }

    return true;
}

static JSBool native_video_prop_setter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
    NativeJSVideo *v = NATIVE_VIDEO_GETTER(obj);

    return NativeJSAVSource::propSetter(v->video, JSID_TO_INT(id), vp);
}

void Audio_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSAudio *audio= NATIVE_AUDIO_GETTER(obj);
    if (audio != NULL) {
        delete audio;
    }
}

void AudioNode_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSAudioNode *node = NATIVE_AUDIO_NODE_GETTER(obj);
    if (node != NULL) {
        delete node;
    } 
}

NativeJSVideo::NativeJSVideo(NativeSkia *nskia, JSContext *cx) 
    : video(NULL), audioNode(NULL), arrayContent(NULL), nskia(nskia), cx(cx)
{
    this->video = new NativeVideo((ape_global *)JS_GetContextPrivate(cx));
    this->video->frameCallback(NativeJSVideo::frameCallback, this);
    this->video->eventCallback(NativeJSVideo::eventCbk, this);
}

void NativeJSVideo::stopAudio() 
{
    this->video->stopAudio();

#if 0 // Code never used?

    if (this->audioNode) {
        NJS->unrootObject(this->audioNode);
        NativeJSAudioNode *jnode = static_cast<NativeJSAudioNode *>(JS_GetPrivate(this->audioNode));
        JS_SetReservedSlot(jnode->jsobj, 0, JSVAL_NULL);
        this->audioNode = NULL;
        //This will remove the source from 
        // NativeJSAudio nodes list and NativeAudio sources
        // (and free source resources)
        delete jnode; 
    } 
#endif
}

void NativeJSVideo::eventCbk(const struct NativeAVSourceEvent *cev) 
{
    NativeJSVideo *thiz;
    NativeJSAVMessageCallback *ev;
    thiz = static_cast<NativeJSVideo *>(cev->custom);

    NativeJS *njs = NativeJS::getNativeClass(thiz->cx);
    // TODO : use cev->fromThread to avoid posting message 
    // if message is comming from main thread

    ev = new NativeJSAVMessageCallback(thiz->jsobj, cev->ev, cev->value1, cev->value2);
    
    njs->postMessage(ev, NATIVE_AV_THREAD_MESSAGE_CALLBACK);

    delete cev;
}

void NativeJSVideo::frameCallback(uint8_t *data, void *custom)
{
    NativeJSVideo *v = (NativeJSVideo *)custom;

    v->nskia->drawPixels(data, v->video->width, v->video->height, 0, 0);

    jsval onframe;
    if (JS_GetProperty(v->cx, v->jsobj, "onframe", &onframe) &&
        !JSVAL_IS_PRIMITIVE(onframe) &&
        JS_ObjectIsCallable(v->cx, JSVAL_TO_OBJECT(onframe))) {
        jsval params, rval;

        params = OBJECT_TO_JSVAL(v->jsobj);

        JSAutoRequest ar(v->cx);
        JS_CallFunctionValue(v->cx, v->jsobj, onframe, 1, &params, &rval);
    }
}

static JSBool native_video_play(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSVideo *v = NATIVE_VIDEO_GETTER(JS_THIS_OBJECT(cx, vp));

    v->video->play();

    return JS_TRUE;
}

static JSBool native_video_pause(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSVideo *v = NATIVE_VIDEO_GETTER(JS_THIS_OBJECT(cx, vp));

    v->video->pause();

    return JS_TRUE;
}

static JSBool native_video_stop(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSVideo *v = NATIVE_VIDEO_GETTER(JS_THIS_OBJECT(cx, vp));

    v->video->stop();

    return JS_TRUE;
}

static JSBool native_video_close(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSVideo *v = NATIVE_VIDEO_GETTER(JS_THIS_OBJECT(cx, vp));

    v->video->close();

    return JS_TRUE;
}

static JSBool native_video_open(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSVideo *v = NATIVE_VIDEO_GETTER(JS_THIS_OBJECT(cx, vp));

    JS::Value src = JS_ARGV(cx, vp)[0];
    int ret = -1;

    if (src.isString()) {
        JSAutoByteString csrc(cx, src.toString());
        ret = v->video->open(NativeJS::getNativeClass(cx)->getPath(), csrc.ptr());
    } else if (src.isObject()) {
        JSObject *arrayBuff = src.toObjectOrNull();

        if (!JS_IsArrayBufferObject(arrayBuff)) {
            JS_ReportError(cx, "Data is not an ArrayBuffer\n");
            return JS_FALSE;
        }

        int length;
        uint8_t *data;

        length = JS_GetArrayBufferByteLength(arrayBuff);
        JS_StealArrayBufferContents(cx, arrayBuff, &v->arrayContent, &data);

        if (v->video->open(data, length) < 0) {
            JS_SET_RVAL(cx, vp, JSVAL_FALSE);
            return JS_TRUE;
        }
    }

    JS_SET_RVAL(cx, vp, JSVAL_TRUE);

    return JS_TRUE;
}

static JSBool native_video_get_audionode(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSVideo *v = NATIVE_VIDEO_GETTER(JS_THIS_OBJECT(cx, vp));
    NativeJSAudio *jaudio = NativeJSAudio::getContext();

    if (!jaudio) {
        JS_ReportError(cx, "No Audio context");
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_FALSE;
    }

    if (v->audioNode) {
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(v->audioNode));
        return true;
    }

    NativeAudioSource *source = v->video->getAudioNode(jaudio->audio);

    if (source != NULL) {
        NativeJSAudioNode *node = new NativeJSAudioNode(NativeAudio::SOURCE, static_cast<class NativeAudioNode *>(source), jaudio);

        v->audioNode = JS_NewObjectForConstructor(cx, &AudioNode_class, vp);

        node->njs = NJS;
        node->jsobj = v->audioNode;
        node->cx = cx;

        //JS_AddObjectRoot(cx, &node->jsobj);
        NJS->rootObjectUntilShutdown(v->audioNode);

        JS_SetReservedSlot(node->jsobj, 0, OBJECT_TO_JSVAL(v->jsobj));
        JS_SetPrivate(v->audioNode, node);

        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(v->audioNode));
    } else {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
    }

    return JS_TRUE;
}

static JSBool native_video_nextframe(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSVideo *v = NATIVE_VIDEO_GETTER(JS_THIS_OBJECT(cx, vp));
    v->video->nextFrame();
    return JS_TRUE;
}

static JSBool native_video_prevframe(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSVideo *v = NATIVE_VIDEO_GETTER(JS_THIS_OBJECT(cx, vp));
    v->video->prevFrame();
    return JS_TRUE;
}

static JSBool native_video_frameat(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSVideo *v = NATIVE_VIDEO_GETTER(JS_THIS_OBJECT(cx, vp));
    double time;
    JSBool keyframe;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "db", &time, &keyframe)) {
        return JS_TRUE;
    }

    v->video->frameAt(time, keyframe == JS_TRUE ? true : false);

    return JS_TRUE;
}

static JSBool native_Video_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *ret = JS_NewObjectForConstructor(cx, &Video_class, vp);
    JSObject *canvas;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &canvas)) {
        return JS_TRUE;
    }

    NativeCanvasHandler *handler = static_cast<class NativeCanvasHandler *>(JS_GetInstancePrivate(cx, canvas, &Canvas_class, JS_ARGV(cx, vp)));

    if (!handler) {
        JS_ReportError(cx, "Video constructor argument must be Canvas");
        return JS_FALSE;
    }

    NativeCanvasContext *ncc = handler->getContext();
    if (ncc == NULL || ncc->m_Mode != NativeCanvasContext::CONTEXT_2D) {
        JS_ReportError(cx, "Invalid destination canvas (must be backed by a 2D context)");
        return false;
    }
    NJS->rootObjectUntilShutdown(ret);

    NativeSkia *nskia = ((NativeCanvas2DContext *)ncc)->getSurface();

    NativeJSVideo *v = new NativeJSVideo(nskia, cx);

    JS_DefineFunctions(cx, ret, Video_funcs);
    JS_DefineProperties(cx, ret, Video_props);

    JS_SetPrivate(ret, v);
    v->jsobj = ret;

    JS_SetProperty(cx, ret, "canvas", &(JS_ARGV(cx, vp)[0]));

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

NativeJSVideo::~NativeJSVideo() 
{
    if (this->audioNode) {
        NJS->unrootObject(this->audioNode);
        NativeJSAudioNode *node = static_cast<NativeJSAudioNode *>(JS_GetPrivate(this->audioNode));
        // This will remove the source from 
        // NativeJSAudio nodes list and NativeAudio sources 
        delete node; 
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

bool NativeJSAVSource::propSetter(NativeAVSource *source, int id, JSMutableHandleValue vp) 
{
    switch(id) {
        case SOURCE_PROP_POSITION:
            if (vp.isNumber()) {
                source->seek(vp.toNumber());
            } 
            break;
        default:
            break;
    }

    return true;
}

bool NativeJSAVSource::propGetter(NativeAVSource *source, JSContext *cx, int id, JSMutableHandleValue vp)
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
                JSObject *metadata = JS_NewObject(cx, NULL, NULL, NULL);

                while ((tag = av_dict_get(cmetadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
                    JSString *val = JS_NewStringCopyN(cx, tag->value, strlen(tag->value));

                    JS_DefineProperty(cx, metadata, tag->key, STRING_TO_JSVAL(val), NULL, NULL, 
                            JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
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
        int id, JSMutableHandleValue vp) 
{
    NativeAudioCustomSource *source = static_cast<NativeAudioCustomSource*>(jnode->node);

    switch(id) {
        case SOURCE_PROP_POSITION:
            if (vp.isNumber()) {
                source->seek(vp.toNumber());
            } 
        break;
        case CUSTOM_SOURCE_PROP_SEEK: {
            JSTransferableFunction *fun = new JSTransferableFunction();
            int funID = NativeJSAudioNode::SEEK_FN;
            if (!fun->prepare(cx, vp.get())) {
                JS_ReportError(cx, "Failed to read custom node callback function\n");
                vp.set(JSVAL_VOID);
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

    jsval params[2];
    jsval rval;

    params[0] = DOUBLE_TO_JSVAL(seekTime);;
    params[1] = OBJECT_TO_JSVAL(JS_GetGlobalObject(jnode->audio->tcx));

    fn->call(jnode->audio->tcx, jnode->nodeObj, 2, params, &rval);
}

void native_av_thread_message(JSContext *cx, NativeSharedMessages::Message *msg)
{
    jsval jscbk, rval;

    NativeJSAVMessageCallback *cmsg = static_cast<struct NativeJSAVMessageCallback *>(msg->dataPtr());

    NativeJSAudioNode *jnode = static_cast<NativeJSAudioNode*>(JS_GetInstancePrivate(cx, cmsg->callee, &AudioNode_class, NULL));

    const char *prop = NativeJSAVEventRead(cmsg->ev);
    if (!prop) {
        if (jnode) {
            jnode->node->unref();
        }

        delete cmsg;

        return;
    }

    if (JS_GetProperty(cx, cmsg->callee, prop, &jscbk) &&
        !JSVAL_IS_PRIMITIVE(jscbk) &&
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(jscbk))) {

        if (cmsg->ev == SOURCE_EVENT_ERROR) {
            jsval event[2];
            const char *errorStr = NativeAVErrorsStr[cmsg->arg1];

            event[0] = INT_TO_JSVAL(cmsg->arg1);
            event[1] = STRING_TO_JSVAL(JS_NewStringCopyN(cx, errorStr, strlen(errorStr)));

            JS_CallFunctionValue(cx, cmsg->callee, jscbk, 2, event, &rval);
        } else if (cmsg->ev == SOURCE_EVENT_BUFFERING) {
            jsval event[2];

            event[0] = INT_TO_JSVAL(cmsg->arg1);
            event[1] = INT_TO_JSVAL(cmsg->arg2);

            JS_CallFunctionValue(cx, cmsg->callee, jscbk, 2, event, &rval);
        } else {
            JS_CallFunctionValue(cx, cmsg->callee, jscbk, 0, NULL, &rval);
        }
        
    }

    if (jnode) {
        jnode->node->unref();
    }

    delete cmsg;
}

void NativeJSAudioNode::registerObject(JSContext *cx)
{
    JSObject *obj;

    obj = JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, 
            &AudioNode_class, native_AudioNode_constructor, 0, 
            AudioNode_props, AudioNode_funcs, NULL, NULL);
}

void NativeJSAudio::registerObject(JSContext *cx)
{
    JSObject *obj;

    obj = JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, 
            &Audio_class, native_Audio_constructor, 0, 
            Audio_props, Audio_funcs, NULL, Audio_static_funcs);

    NATIVE_AV_THREAD_MESSAGE_CALLBACK = NativeJSObj(cx)->registerMessage(native_av_thread_message);
}

NATIVE_OBJECT_EXPOSE(Video);
