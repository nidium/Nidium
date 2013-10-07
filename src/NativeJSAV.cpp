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

static JSBool native_audio_getcontext(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audio_run(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audio_load(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audio_createnode(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audio_connect(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audio_disconnect(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_audiothread_print(JSContext *cx, unsigned argc, jsval *vp);

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

static JSBool native_audio_prop_setter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp);
static JSBool native_audionode_custom_prop_setter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp);
static JSBool native_audionode_source_prop_setter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp);
static JSBool native_audionode_source_prop_getter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSMutableHandleValue vp);

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
    {"onbuffer", NODE_CUSTOM_PROP_BUFFER, 0, JSOP_NULLWRAPPER, JSOP_WRAPPER(native_audionode_custom_prop_setter)},
    {"oninit", NODE_CUSTOM_PROP_INIT, 0, JSOP_NULLWRAPPER, JSOP_WRAPPER(native_audionode_custom_prop_setter)},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSFunctionSpec Audio_funcs[] = {
    JS_FN("getContext", native_audio_getcontext, 3, 0),
    JS_FN("run", native_audio_run, 1, 0),
    JS_FN("load", native_audio_load, 1, 0),
    JS_FN("createNode", native_audio_createnode, 3, 0),
    JS_FN("connect", native_audio_connect, 2, 0),
    JS_FN("disconnect", native_audio_disconnect, 2, 0),
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

static JSFunctionSpec glob_funcs_threaded[] = {
    JS_FN("echo", native_audiothread_print, 1, 0),
    JS_FS_END
};


static JSBool native_Video_constructor(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_video_play(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_video_pause(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_video_stop(JSContext *cx, unsigned argc, jsval *vp);
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

NativeJSAudio *NativeJSAudio::getContext(JSContext *cx, JSObject *obj, int bufferSize, int channels, int sampleRate) 
{
    ape_global *net = static_cast<ape_global *>(JS_GetContextPrivate(cx));
    NativeJSAudio *jaudio;
    NativeAudio *audio;

    try {
        audio = new NativeAudio(net, bufferSize, channels, sampleRate);
    } catch (...) {
        return NULL;
    }

    jaudio = new NativeJSAudio();
    jaudio->audio = audio;
    jaudio->jsobj = obj;

    NativeJSAudio::instance = jaudio;
    jaudio->cx = cx;

    JS_SetPrivate(obj, jaudio);

    NJS->rootObjectUntilShutdown(obj);

    JS_DefineFunctions(cx, obj, Audio_funcs);
    JS_DefineProperties(cx, obj, Audio_props);

    pthread_cond_init(&jaudio->shutdownCond, NULL);
    pthread_mutex_init(&jaudio->shutdownLock, NULL);

    return jaudio;
}

NativeJSAudio *NativeJSAudio::getContext()
{
    return NativeJSAudio::instance;
}

NativeJSAudio::NativeJSAudio()
    : nodes(NULL), shutdowned(false), jsobj(NULL), gbl(NULL), rt(NULL), tcx(NULL)
{
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

    // Delete all ndoes
    pthread_mutex_lock(&this->audio->recurseLock);
    pthread_mutex_lock(&this->audio->tracksLock);

    NativeJSAudio::Nodes *nodes = this->nodes;
    NativeJSAudio::Nodes *next = NULL;
    while (nodes != NULL) {
        next = nodes->next;
        // Node destructor will remove the node 
        // from the nodes linked list
        delete nodes->curr;
        nodes = next;
    }

    pthread_mutex_unlock(&this->audio->recurseLock);
    pthread_mutex_unlock(&this->audio->tracksLock);

    // Shutdown the audio
    this->audio->shutdown();

    // And delete it
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
        if (nodes->curr->type == NativeAudio::CUSTOM) {
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

    msg = static_cast<struct NativeJSAudioNode::Message*>(custom);
    tcx = msg->jsNode->audio->tcx;

    JSAutoRequest ar(tcx);

    if (msg->jsNode->hashObj == NULL) {
        if (!msg->jsNode->createHashObj()) {
            JS_ReportError(tcx, "Failed to create hash object");
            return;
        }
    }

    if (!JS_ReadStructuredClone(tcx,
                msg->clone.datap,
                msg->clone.nbytes,
                JS_STRUCTURED_CLONE_VERSION, &val, NULL, NULL)) {

        JS_ReportError(tcx, "Failed to read structured clone");
        delete msg;
    }

    JS_SetProperty(tcx, msg->jsNode->hashObj, msg->name, &val);
    JS_free(msg->jsNode->cx, msg->name);

    JS_ClearStructuredClone(msg->clone.datap, msg->clone.nbytes);
    delete msg;
}

void NativeJSAudioNode::customCallback(const struct NodeEvent *ev)
{
    NativeJSAudioNode *thiz;
    JSContext *tcx;

    thiz = static_cast<NativeJSAudioNode *>(ev->custom);

    if (!thiz->audio->tcx || !thiz->cx || !thiz->jsobj || !thiz->node) {
        return;
    }

    tcx = thiz->audio->tcx;

    JSAutoRequest ar(tcx);

    if (!thiz->bufferFn && thiz->bufferStr) {
        const char *args[2] = {"ev", "scope"};

        thiz->bufferFn = JS_CompileFunction(tcx, JS_GetGlobalObject(tcx), "CustomAudioNode_onbuffer", 2, args, thiz->bufferStr, strlen(thiz->bufferStr), "FILENAME (TODO)", 0);
        if (!thiz->bufferFn) {
            JS_ReportError(tcx, "Failed to compile CustomAudioNode_onbuffer function\n%s\n", thiz->bufferStr);
            JS_free(tcx, (void *)thiz->bufferStr);
            thiz->bufferStr = NULL;
            return;
        }

        thiz->nodeObj = JS_NewObject(tcx, &AudioNode_threaded_class, NULL, NULL);
        JS_AddObjectRoot(tcx, &thiz->nodeObj);

        JS_free(tcx, (void *)thiz->bufferStr);

        thiz->bufferObj = JS_GetFunctionObject(thiz->bufferFn);
        JS_AddObjectRoot(tcx, &thiz->bufferObj);
        /*
        thiz->fnval = OBJECT_TO_JSVAL(funObj);
        JS_AddValueRoot(tcx, &thiz->fnval);
        */

        JS_DefineFunctions(tcx, thiz->nodeObj, AudioNodeCustom_threaded_funcs);
        JS_SetPrivate(thiz->nodeObj, static_cast<void *>(thiz));
    }

    if (thiz->bufferFn) {
        jsval rval, params[2], vFrames, vSize;
        JSObject *obj, *frames;
        unsigned long size;
        int count;
        JSContext *tcx;

        tcx = thiz->audio->tcx;
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

        JS_CallFunction(tcx, thiz->nodeObj, thiz->bufferFn, 2, params, &rval);

        for (int i = 0; i < count; i++) 
        {
            jsval val;

            JS_GetElement(tcx, frames, i, &val);

            memcpy(ev->data[i], JS_GetFloat32ArrayData(JSVAL_TO_OBJECT(val)), size);
        }
    }
}

void NativeJSAudioNode::customInitCallback(NativeAudioNode *node, void *custom)
{
    NativeJSAudioNode *thiz;
    JSContext *tcx;

    thiz = static_cast<NativeJSAudioNode *>(custom);
    tcx = thiz->audio->tcx;

    if (!tcx || !thiz->cx || !thiz->jsobj || !thiz->initStr) {
        return;
    }

    const char *args[1] = {"scope"};
    jsval params[1];
    jsval rval;

    JSObject *obj = JS_NewObject(tcx, &AudioNode_threaded_class, NULL, NULL);
    JSFunction *fn = JS_CompileFunction(tcx, JS_GetGlobalObject(tcx), "CustomAudioNode_oninit", 1, args, thiz->initStr, strlen(thiz->initStr), "FILENAME (TODO)", 0);

    JS_free(tcx, (void *)thiz->initStr);
    thiz->initStr = NULL;

    JS_DefineFunctions(tcx, obj, AudioNodeCustom_threaded_funcs);

    params[0] = OBJECT_TO_JSVAL(JS_GetGlobalObject(tcx));

    JS_CallFunction(tcx, obj, fn, 1, params, &rval);
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
    const char *prop;

    thiz = static_cast<NativeJSAudioNode *>(cev->custom);

    // FIXME : use cev->fromThread to avoid posting message 
    // if message is comming from main thread
    ev = new NativeJSAVMessageCallback(thiz->jsobj, cev->ev, cev->value1, cev->value2);
    
    thiz->njs->postMessage(ev, NATIVE_AV_THREAD_MESSAGE_CALLBACK);

    delete cev;
}

bool NativeJSAudioNode::createHashObj() 
{
    this->hashObj = JS_NewObject(this->audio->tcx, NULL, NULL, NULL);
    JS_AddObjectRoot(this->audio->tcx, &this->hashObj);
    return true;
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
    if (node->bufferObj != NULL) {
        JS_RemoveObjectRoot(node->audio->tcx, &node->bufferObj);
    }
    if (node->hashObj != NULL) {
        JS_RemoveObjectRoot(node->audio->tcx, &node->hashObj);
    }

    node->finalized = true;

    pthread_cond_signal(&node->shutdownCond);
}

NativeJSAudioNode::~NativeJSAudioNode()
{
    // Remove node from nodes linked list
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
                // Set audioNode to null, because we are going to destroy it bellow
                // and we don't want stopAudio() to try to destroy it too
                video->audioNode = NULL; 
                JS_SetReservedSlot(this->jsobj, 0, JSVAL_NULL);
                video->stopAudio();
            }
        }
    }

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
    // XXX : Might be better to check if current thred == audio thread, 
    // instead of always posting a message
    if (this->type == NativeAudio::CUSTOM && !this->finalized) {
        this->node->callback(
                NativeJSAudioNode::shutdownCallback, 
                this);

        this->audio->audio->wakeup();

        if (!this->finalized) {
            pthread_cond_wait(&this->shutdownCond, &this->shutdownLock);
        }
    } 

    if (this->arrayContent != NULL) {
        free(this->arrayContent);
    }

    this->node->unref();

    JS_SetPrivate(this->jsobj, NULL);
}

static JSBool native_audio_getcontext(JSContext *cx, unsigned argc, jsval *vp)
{
    unsigned int bufferSize, channels, sampleRate;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuu", &bufferSize, &channels, &sampleRate)) {
        return JS_FALSE;
    }

    switch (bufferSize) {
        case 128:
        case 256:
        case 512:
        case 1024:
        case 2048:
        case 4096:
            // Supported buffer size
            break;
        default :
            JS_ReportError(cx, "Unsuported buffer size %d. Supported values are : 128, 256, 512, 1024, 2048, 4096\n", bufferSize);
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

    if (!audio->tcx) {
        audio->audio->sharedMsg->postMessage(
                (void *)new NativeAudioNode::CallbackMessage(NativeJSAudio::ctxCallback, NULL, static_cast<void *>(audio)), 
                NATIVE_AUDIO_NODE_CALLBACK);
    }

    audio->audio->sharedMsg->postMessage(
            (void *)new NativeAudioNode::CallbackMessage(NativeJSAudio::runCallback, NULL, static_cast<void *>(funStr)), 
            NATIVE_AUDIO_CALLBACK);


    return JS_TRUE;
}

static JSBool native_audio_load(JSContext *cx, unsigned argc, jsval *vp)
{
    JS_ReportError(cx, "Not implemented");
    return JS_TRUE;
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
        return JS_TRUE;
    }

    JSAutoByteString cname(cx, name);
    ret = JS_NewObjectForConstructor(cx, &AudioNode_class, vp);
    JS_SetReservedSlot(ret, 0, JSVAL_NULL);

    try {
        if (strcmp("source", cname.ptr()) == 0) {
            node = new NativeJSAudioNode(NativeAudio::SOURCE, in, out, audio);
            NativeAudioTrack *source = static_cast<NativeAudioTrack *>(node->node);
            source->eventCallback(NativeJSAudioNode::eventCbk, node);
            JS_DefineFunctions(cx, ret, AudioNodeSource_funcs);
            JS_DefineProperties(cx, ret, AudioNodeSource_props);
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
            return JS_FALSE;
        }
    } catch (NativeAudioNodeException *e) {
        delete node;
        JS_ReportError(cx, "Error while creating node : %s\n", e->what());
        return JS_FALSE;
    }

    if (node == NULL || node->node == NULL) {
        delete node;
        JS_ReportError(cx, "Error while creating node : %s\n", cname.ptr());
        return JS_FALSE;
    }

    node->njs = NJS;
    node->jsobj = ret;
    node->cx = cx;

    NJS->rootObjectUntilShutdown(node->jsobj);
    JS_SetPrivate(ret, node);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
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
    int intVal;
    double doubleVal;
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
        return JS_FALSE;
    }

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &name)) {
        return JS_TRUE;
    }

    node = static_cast<NativeAudioNodeCustom *>(jnode->node);

    msg = new NativeJSAudioNode::Message();
    msg->name = JS_EncodeString(cx, name);
    msg->jsNode = jnode;

    if (!JS_WriteStructuredClone(cx, JS_ARGV(cx, vp)[1], 
            &msg->clone.datap, &msg->clone.nbytes,
            NULL, NULL, JSVAL_VOID)) {
        JS_ReportError(cx, "Failed to write structured clone");
        delete msg;
        return JS_FALSE;
    }

    if (!jnode->audio->tcx) {
        node->callback(NativeJSAudio::ctxCallback, jnode->audio);
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
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    if (argc != 2) {
        JS_ReportError(cx, "set() require two arguments\n");
    }

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &name)) {
        return JS_TRUE;
    }

    JSAutoByteString str(cx, name);

    JS_SetProperty(cx, jnode->hashObj, str.ptr(), &JS_ARGV(cx, vp)[1]);

    return JS_TRUE;
}

static JSBool native_audionode_custom_threaded_get(JSContext *cx, unsigned argc, jsval *vp)
{
    jsval val;
    JSString *name;
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    if (jnode->hashObj == NULL) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &name)) {
        return JS_TRUE;
    }

    JSAutoByteString str(cx, name);

    JS_GetProperty(jnode->audio->tcx, jnode->hashObj, str.ptr(), &val);

    JS_SET_RVAL(cx, vp, val);

    return JS_TRUE;
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

    NativeAudioTrack *source = (NativeAudioTrack *)jnode->node;

    JS::Value src = JS_ARGV(cx, vp)[0];

    int ret = -1;

    if (src.isString()) {
        JSAutoByteString csrc(cx, src.toString());
        ret = source->open(csrc.ptr());
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

    NativeAudioTrack *source = static_cast<NativeAudioTrack *>(jnode->node);

    source->play();

    return JS_TRUE;
}

static JSBool native_audionode_source_pause(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    NativeAudioTrack *source = static_cast<NativeAudioTrack *>(jnode->node);

    source->pause();
    return JS_TRUE;
}
static JSBool native_audionode_source_stop(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    CHECK_INVALID_CTX(jnode);

    NativeAudioTrack *source = static_cast<NativeAudioTrack *>(jnode->node);

    source->stop();
    return JS_TRUE;
}

static JSBool native_audionode_source_prop_getter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSMutableHandleValue vp)
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(obj);

    CHECK_INVALID_CTX(jnode);

    NativeAudioTrack *source = static_cast<NativeAudioTrack *>(jnode->node);

    NativeJSAVSource::propGetter(source, cx, JSID_TO_INT(id), vp);

    return JS_TRUE;
}

static JSBool native_audionode_source_prop_setter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(obj);

    CHECK_INVALID_CTX(jnode);

    NativeAudioTrack *source = static_cast<NativeAudioTrack *>(jnode->node);

    NativeJSAVSource::propSetter(source, JSID_TO_INT(id), vp);

    return JS_TRUE;
}

static JSBool native_audionode_custom_prop_setter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(obj);

    CHECK_INVALID_CTX(jnode);

    switch(JSID_TO_INT(id)) {
        case NODE_CUSTOM_PROP_BUFFER :
        {
            JSString *fn;
            JSFunction *nfn;
            NativeAudioNodeCustom *node;
            
            if ((nfn = JS_ValueToFunction(cx, vp)) == NULL ||
                (fn = JS_DecompileFunction(cx, nfn, 0)) == NULL) {
                JS_ReportError(cx, "Failed to read custom node onbuffer callback function\n");
                vp.set(JSVAL_VOID);
                return JS_FALSE;
            } 

            jnode->bufferStr = JS_EncodeString(cx, fn);
            node = static_cast<NativeAudioNodeCustom *>(jnode->node);

            if (!jnode->audio->tcx) {
                node->callback(NativeJSAudio::ctxCallback, jnode->audio);
            }

            node->setCallback(NativeJSAudioNode::customCallback, static_cast<void *>(jnode));
        }
        break;
        case NODE_CUSTOM_PROP_INIT :
        {
            JSString *fn;
            JSFunction *nfn;
            NativeAudioNodeCustom *node;
            
            if ((nfn = JS_ValueToFunction(cx, vp)) == NULL ||
                (fn = JS_DecompileFunctionBody(cx, nfn, 0)) == NULL) {
                JS_ReportError(cx, "Failed to read custom node oninit callback function\n");
                vp.set(JSVAL_VOID);
                return JS_FALSE;
            } 

            jnode->initStr = JS_EncodeString(cx, fn);
            node = static_cast<NativeAudioNodeCustom *>(jnode->node);

            if (!jnode->audio->tcx) {
                node->callback(NativeJSAudio::ctxCallback, jnode->audio);
            }

            node->callback(NativeJSAudioNode::customInitCallback, static_cast<void *>(jnode));
        }
        default:
            break;
    }
    return JS_TRUE;
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
            NativeJSAVSource::propGetter(v->video, cx, JSID_TO_INT(id), vp);
            break;
    }

    return JS_TRUE;
}

static JSBool native_video_prop_setter(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
    NativeJSVideo *v = NATIVE_VIDEO_GETTER(obj);

    NativeJSAVSource::propSetter(v->video, JSID_TO_INT(id), vp);
    
    return JS_TRUE;
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
    NativeJSAudio *jaudio = NativeJSAudio::getContext();

    this->video->stopAudio();

    if (this->audioNode) {
        NJS->unrootObject(this->audioNode);
        NativeJSAudioNode *jnode = static_cast<NativeJSAudioNode *>(JS_GetPrivate(this->audioNode));
        JS_SetReservedSlot(jnode->jsobj, 0, JSVAL_NULL);
        this->audioNode = NULL;
        //This will remove the track from 
        // NativeJSAudio nodes list and NativeAudio tracks
        // (and free track resources)
        delete jnode; 
    } 
}

void NativeJSVideo::eventCbk(const struct NativeAVSourceEvent *cev) 
{
    NativeJSVideo *thiz;
    NativeJSAVMessageCallback *ev;
    const char *prop;
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

static JSBool native_video_open(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSVideo *v = NATIVE_VIDEO_GETTER(JS_THIS_OBJECT(cx, vp));

    JS::Value src = JS_ARGV(cx, vp)[0];
    int ret = -1;

    if (src.isString()) {
        JSAutoByteString csrc(cx, src.toString());
        ret = v->video->open(csrc.ptr());
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
        JS_ReportError(cx, "No NativeAudio context");
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_FALSE;
    }

    NativeAudioTrack *track = v->video->getAudioNode(jaudio->audio);

    if (track != NULL) {
        if (v->audioNode == NULL) {
            NativeJSAudioNode *node = new NativeJSAudioNode(NativeAudio::SOURCE, static_cast<class NativeAudioNode *>(track), jaudio);

            v->audioNode = JS_NewObjectForConstructor(cx, &AudioNode_class, vp);

            node->njs = NJS;
            node->jsobj = v->audioNode;
            node->cx = cx;

            //JS_AddObjectRoot(cx, &node->jsobj);
            NJS->rootObjectUntilShutdown(v->audioNode);
            JS_SetReservedSlot(node->jsobj, 0, OBJECT_TO_JSVAL(v->jsobj));
            JS_SetPrivate(v->audioNode, node);
        }
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
        //This will remove the track from 
        // NativeJSAudio nodes list and NativeAudio tracks
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

void NativeJSAVSource::propSetter(NativeAVSource *source, int id, JSMutableHandleValue vp) 
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
}

void NativeJSAVSource::propGetter(NativeAVSource *source, JSContext *cx, int id, JSMutableHandleValue vp)
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
}

void native_av_thread_message(JSContext *cx, NativeSharedMessages::Message *msg)
{
    jsval jscbk, rval;

    NativeJSAVMessageCallback *cmsg = static_cast<struct NativeJSAVMessageCallback *>(msg->dataPtr());

    const char *prop = NativeJSAVEventRead(cmsg->ev);
    if (!prop) {
        delete cmsg;
        return;
    }

    if (JS_GetProperty(cx, cmsg->callee, prop, &jscbk) &&
        !JSVAL_IS_PRIMITIVE(jscbk) &&
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(jscbk))) {

        if (cmsg->ev == SOURCE_EVENT_ERROR) {
            const char *errorStr = NativeAVErrorsStr[cmsg->arg1];
            jsval event[2];

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
    delete cmsg;
}

void NativeJSAudioNode::registerObject(JSContext *cx)
{
    JSObject *AudioNodeObj;
    AudioNodeObj = JS_DefineObject(cx, JS_GetGlobalObject(cx), "AudioNode",
        &AudioNode_class , NULL, 0);
    JS_DefineFunctions(cx, AudioNodeObj, AudioNode_funcs);
    JS_DefineProperties(cx, AudioNodeObj, AudioNode_props);

    NATIVE_AV_THREAD_MESSAGE_CALLBACK = NativeJSObj(cx)->registerMessage(native_av_thread_message);
}

NATIVE_OBJECT_EXPOSE(Video);
NATIVE_OBJECT_EXPOSE_NOT_INST(Audio)
//NATIVE_OBJECT_EXPOSE_NOT_INST(AudioNode)
