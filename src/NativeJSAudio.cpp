#include "NativeJSAudio.h"
#include "NativeSharedMessages.h"
#include "NativeJSThread.h"
#include "NativeJS.h"

#define NATIVE_AUDIO_GETTER(obj) ((class NativeJSAudio *)JS_GetPrivate(obj))
#define NATIVE_AUDIO_NODE_GETTER(obj) ((class NativeJSAudioNode *)JS_GetPrivate(obj))
#define NJS ((class NativeJS *)JS_GetRuntimePrivate(JS_GetRuntime(cx)))

extern void reportError(JSContext *cx, const char *message, JSErrorReport *report);

static void AudioNode_Finalize(JSFreeOp *fop, JSObject *obj);
static void Audio_Finalize(JSFreeOp *fop, JSObject *obj);

static JSBool native_Audio_constructor(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audio_createnode(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audio_connect(JSContext *cx, unsigned argc, jsval *vp);

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

static JSBool native_audionode_custom_prop_set(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp);

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
    "AudioNode", JSCLASS_HAS_PRIVATE,
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

static JSPropertySpec AudioNodeEvent_props[] = {
    {"data", NODE_EV_PROP_DATA, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER},
    {"size", NODE_EV_PROP_SIZE, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSPropertySpec AudioNodeCustom_props[] = {
    {"onbuffer", NODE_CUSTOM_PROP_BUFFER, 0, JSOP_NULLWRAPPER, JSOP_WRAPPER(native_audionode_custom_prop_set)},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSFunctionSpec Audio_funcs[] = {
    JS_FN("createNode", native_audio_createnode, 3, 0),
    JS_FN("connect", native_audio_connect, 2, 0),
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

static JSFunctionSpec glob_funcs_threaded[] = {
    JS_FN("echo", native_audiothread_print, 1, 0),
    JS_FS_END
};

NativeJSAudio::NativeJSAudio(int size, int channels, int frequency) 
    : rt(NULL), tcx(NULL)
{
    this->audio = new NativeAudio(size, channels, frequency);

    pthread_create(&threadDecode, NULL, NativeAudio::decodeThread, audio);
    pthread_create(&threadQueue, NULL, NativeAudio::queueThread, audio);
}

bool NativeJSAudio::createContext() 
{

    if (this->rt == NULL) {
        JSObject *gbl;

        if ((this->rt = JS_NewRuntime(128L * 1024L * 1024L, JS_USE_HELPER_THREADS)) == NULL) {
            printf("Failed to init JS runtime\n");
            return false;
        }

        /*
        JS_SetGCParameter(this->rt, JSGC_MAX_BYTES, 0xffffffff);
        JS_SetGCParameter(this->rt, JSGC_MODE, JSGC_MODE_INCREMENTAL);
        JS_SetGCParameter(this->rt, JSGC_SLICE_TIME_BUDGET, 15);
        */

        if ((this->tcx = JS_NewContext(this->rt, 8192)) == NULL) {
            printf("Failed to init JS context\n");
            return false;     
        }

        JSAutoRequest ar(this->tcx);

        //JS_SetGCParameterForThread(this->tcx, JSGC_MAX_CODE_CACHE_BYTES, 16 * 1024 * 1024);

        gbl = JS_NewGlobalObject(this->tcx, &global_AudioThread_class, NULL);
        JS_AddObjectRoot(this->tcx, &gbl);
        if (!JS_InitStandardClasses(tcx, gbl)) {
            printf("Failed to init std class\n");
            return false;
        }

        JS_SetVersion(this->tcx, JSVERSION_LATEST);

        JS_SetOptions(this->tcx, JSOPTION_VAROBJFIX | JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE | JSOPTION_ION);

        JS_SetErrorReporter(this->tcx, reportError);
        JS_SetGlobalObject(this->tcx, gbl);
        JS_DefineFunctions(this->tcx, gbl, glob_funcs_threaded);

        //JS_SetContextPrivate(this->tcx, static_cast<void *>(this));
    }

    return true;
    
}

NativeJSAudio::~NativeJSAudio() {
    this->audio->shutdown();
    pthread_join(this->threadQueue, NULL);
    pthread_join(this->threadDecode, NULL);
    delete this->audio;
}

void NativeJSAudioNode::ctxCallback(NativeAudioNode *node, void *custom)
{
    NativeJSAudioNode *jsNode = static_cast<NativeJSAudioNode *>(custom);

    if (!jsNode->audio->createContext()) {
        printf("Failed to create audio thread context\n");
    }
}

void NativeJSAudioNode::setPropCallback(NativeAudioNode *node, void *custom)
{
    NativeJSAudioNode::Message *msg;
    jsval val;
    JSContext *tcx;

    msg = static_cast<struct NativeJSAudioNode::Message*>(custom);
    tcx = msg->jsNode->audio->tcx;

    JSAutoRequest ar(tcx);

    JS_ReadStructuredClone(tcx,
                msg->clone.datap,
                msg->clone.nbytes,
                JS_STRUCTURED_CLONE_VERSION, &val, NULL, NULL);

    if (msg->jsNode->hashObj == NULL) {
        if (!msg->jsNode->createHashObj()) {
            JS_ReportError(tcx, "Failed to create hash object");
            return;
        }
    }

    JS_SetProperty(tcx, msg->jsNode->hashObj, msg->name, &val);

    JS_free(msg->jsNode->cx, msg->name);
    delete msg;
}

void NativeJSAudioNode::cbk(const struct NodeEvent *ev)
{
    NativeJSAudioNode *thiz;
    JSContext *tcx;

    thiz = static_cast<NativeJSAudioNode *>(ev->custom);

    if (!thiz->audio->tcx || !thiz->cx || !thiz->jsobj || !thiz->node) {
        return;
    }

    tcx = thiz->audio->tcx;

    JSAutoRequest ar(tcx);

    if (!thiz->bufferFn) {
        const char *args[2] = {"ev", "scope"};

        thiz->nodeObj = JS_NewObject(tcx, &AudioNode_threaded_class, NULL, NULL);
        JS_AddObjectRoot(tcx, &thiz->nodeObj);
        thiz->bufferFn = JS_CompileFunction(tcx, JS_GetGlobalObject(tcx), "audioNodeBuffer", 2, args, thiz->bufferStr, strlen(thiz->bufferStr), "AudioNode.onbuffer", 0);

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
                JS_ReportError(tcx, "OOM");
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

bool NativeJSAudioNode::createHashObj() 
{
    this->hashObj = JS_NewObject(this->audio->tcx, NULL, NULL, NULL);
    return true;
}

NativeJSAudioNode::~NativeJSAudioNode()
{
    if (this->arrayBuff != NULL) {
        JS_RemoveValueRoot(this->cx, this->arrayBuff);
    }
    if (this->nodeObj != NULL) {
        JS_RemoveObjectRoot(this->audio->tcx, &this->nodeObj);
    }
    if (this->bufferObj != NULL) {
        JS_RemoveObjectRoot(this->audio->tcx, &this->bufferObj);
    }
    delete this->node;
}

static JSBool native_Audio_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    unsigned int size, channels, frequency;
    NativeJSAudio *naudio;
    JSObject *ret = JS_NewObjectForConstructor(cx, &Audio_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uuu", &size, &channels, &frequency)) {
        return JS_TRUE;
    }

    switch (size) {
        case 128:
        case 256:
        case 512:
        case 1024:
        case 2048:
        case 4096:
            // Supported buffer size
            break;
        default :
            JS_ReportError(cx, "Unsuported buffer size %d. Supported values are : 128, 256, 512, 1024, 2048, 4096\n", size);
            return JS_TRUE;
            break;
    }

    if (channels < 1 || channels > 32) {
        JS_ReportError(cx, "Unsuported channels number %d. Channels must be between 1 and 32\n", size);
        return JS_TRUE;
    }

    if (frequency < 22050 || frequency > 96000) {
        JS_ReportError(cx, "Unsuported sample rate %dKHz. Sample rate must be between 22050 and 96000\n", size);
        return JS_TRUE;
    }

    naudio = new NativeJSAudio(size, channels, frequency);
    naudio->cx = cx;
    naudio->jsobj = ret;


    if (naudio->audio == NULL) {
        delete naudio;
        // XXX : How should I free ret?
        JS_ReportError(cx, "Failed to initialize audio context\n");
        return JS_TRUE;
    }

    JS_SetPrivate(ret, naudio);

    JS_DefineFunctions(cx, ret, Audio_funcs);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
    return JS_TRUE;
}

static JSBool native_audio_createnode(JSContext *cx, unsigned argc, jsval *vp)
{
    int in, out;
    JSObject *ret;
    JSString *name;
    NativeJSAudio *audio = NATIVE_AUDIO_GETTER(JS_THIS_OBJECT(cx, vp));
    NativeAudio *naudio = audio->audio;
    NativeJSAudioNode *node;

    node = NULL;
    ret = NULL;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "Suu", &name, &in, &out)) {
        return JS_TRUE;
    }

    JSAutoByteString cname(cx, name);
    ret = JS_NewObjectForConstructor(cx, &AudioNode_class, vp);

    if (strcmp("source", cname.ptr()) == 0) {
        node = new NativeJSAudioNode(naudio->createNode(NativeAudio::SOURCE, in, out));
        JS_DefineFunctions(cx, ret, AudioNodeSource_funcs);
    } else if (strcmp("custom", cname.ptr()) == 0) {
        node = new NativeJSAudioNode(naudio->createNode(NativeAudio::CUSTOM, in, out), audio);
        JS_DefineProperties(cx, ret, AudioNodeCustom_props);
        JS_DefineFunctions(cx, ret, AudioNodeCustom_funcs);
    } else if (strcmp("gain", cname.ptr()) == 0) {
        node = new NativeJSAudioNode(naudio->createNode(NativeAudio::GAIN, in, out));
    } else if (strcmp("target", cname.ptr()) == 0) {                      
        node = new NativeJSAudioNode(naudio->createNode(NativeAudio::TARGET, in, out));
    } else {
        JS_ReportError(cx, "Unknown node name : %s\n", cname.ptr());
        return JS_TRUE;
    }

    if (node == NULL) {
        // TODO : Free ret
        JS_ReportError(cx, "Error while creating node : %s\n", cname.ptr());
        delete node;
        return JS_TRUE;
    }

    node->njs = NJS;
    node->jsobj = ret;
    node->cx = cx;

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
    NativeAudio *audio = NATIVE_AUDIO_GETTER(JS_THIS_OBJECT(cx, vp))->audio;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oo", &link1, &link2)) {
        return JS_TRUE;
    }

    nlink1 = (NodeLink *)JS_GetInstancePrivate(cx, link1, &AudioNodeLink_class, JS_ARGV(cx, vp));
    nlink2 = (NodeLink *)JS_GetInstancePrivate(cx, link2, &AudioNodeLink_class, JS_ARGV(cx, vp));

    if (nlink1 == NULL || nlink2 == NULL) {
        JS_ReportError(cx, "Bad AudioNodeLink\n");
        return JS_TRUE;
    }

    if ((nlink1->type == INPUT && nlink2->type == OUTPUT) ||
        (nlink1->type == OUTPUT && nlink2->type == INPUT)) {
        audio->connect(nlink1, nlink2);
    } else {
        JS_ReportError(cx, "You must give one input and one output\n");
        return JS_TRUE;
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

static JSBool native_audionode_input(JSContext *cx, unsigned argc, jsval *vp)
{
    int channel;
    NativeAudioNode *node = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp))->node;
    JSObject *ret = JS_NewObject(cx, &AudioNodeLink_class, NULL, NULL);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &channel)) {
        return JS_TRUE;
    }

    if (channel < 0 || channel >= node->inCount) {
        JS_ReportError(cx, "Wrong input index\n");
        return JS_TRUE;
    }

    JS_SetPrivate(ret, node->input[channel]);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;

}

static JSBool native_audionode_output(JSContext *cx, unsigned argc, jsval *vp)
{
    int channel;
    NativeAudioNode *node = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp))->node;
    JSObject *ret = JS_NewObject(cx, &AudioNodeLink_class, NULL, NULL);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "u", &channel)) {
        return JS_TRUE;
    }

    if (channel < 0 || channel > node->outCount) {
        JS_ReportError(cx, "Wrong ouput index\n");
        return JS_TRUE;
    }

    JS_SetPrivate(ret, node->output[channel]);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

static JSBool native_audionode_set(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *name;
    NativeAudioNode *node = (NativeAudioNode *)NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp))->node;
    ArgType type;
    void *value;
    int intVal;
    double doubleVal;
    unsigned long size;

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
        return JS_TRUE;
    }

    if (!node->set("gain", type, value, size)) {
        JS_ReportError(cx, "Unknown argument name %s\n", cname.ptr());
        return JS_TRUE;
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
    NativeJSAudioNode *jsNode;
    NativeAudioNodeCustom *node;

    if (argc != 2) {
        JS_ReportError(cx, "set() require two arguments");
        return JS_TRUE;
    }

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &name)) {
        return JS_TRUE;
    }

    jsNode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));
    node = static_cast<NativeAudioNodeCustom *>(jsNode->node);

    msg = new NativeJSAudioNode::Message();
    msg->name = JS_EncodeString(cx, name);
    msg->jsNode = jsNode;

    if (!JS_WriteStructuredClone(cx, JS_ARGV(cx, vp)[1], 
            &msg->clone.datap, &msg->clone.nbytes,
            NULL, NULL, JSVAL_VOID)) {
        JS_ReportError(cx, "Failed to write structured clone");
        delete msg;
        return JS_TRUE;
    }

    if (!jsNode->audio->tcx) {
        node->callback(NativeJSAudioNode::ctxCallback, jsNode);
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
    NativeJSAudioNode *jsNode;
    JSString *name;

    jsNode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    if (argc != 2) {
        JS_ReportError(cx, "set() require two arguments\n");
        return JS_TRUE;
    }

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &name)) {
        return JS_TRUE;
    }

    JSAutoByteString str(cx, name);

    JS_SetProperty(cx, jsNode->hashObj, str.ptr(), &JS_ARGV(cx, vp)[1]);

    return JS_TRUE;
}

static JSBool native_audionode_custom_threaded_get(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSAudioNode *jsNode;
    jsval val;
    JSString *name;

    jsNode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    if (jsNode->hashObj == NULL) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &name)) {
        return JS_TRUE;
    }

    JSAutoByteString str(cx, name);

    JS_GetProperty(jsNode->audio->tcx, jsNode->hashObj, str.ptr(), &val);

    JS_SET_RVAL(cx, vp, val);

    return JS_TRUE;
}

static JSBool native_audionode_custom_threaded_send(JSContext *cx, unsigned argc, jsval *vp)
{
    uint64_t *datap;
    size_t nbytes;
    NativeJSAudioNode *jsNode;

    jsNode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));

    if (jsNode == NULL) {
        JS_ReportError(cx, "NativeAudioNode.send() Could not retrieve node\n");
        return JS_TRUE;
    }

    struct native_thread_msg *msg;

    if (!JS_WriteStructuredClone(cx, JS_ARGV(cx, vp)[0], &datap, &nbytes,
        NULL, NULL, JSVAL_VOID)) {
        JS_ReportError(cx, "Failed to write structured clone");
        return JS_TRUE;
    }

    msg = new struct native_thread_msg;

    msg->data   = datap;
    msg->nbytes = nbytes;
    msg->callee = jsNode->jsobj;

    jsNode->njs->messages->postMessage(msg, NATIVE_THREAD_MESSAGE);

    return JS_TRUE;
}


static JSBool native_audionode_source_open(JSContext *cx, unsigned argc, jsval *vp) 
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));
    NativeAudioTrack *source = (NativeAudioTrack *)jnode->node;
    JSObject *arrayBuff;


    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &arrayBuff)) {
        return JS_TRUE;
    }

    // TODO : use JS_StealArrayBufferContents

    if (!JS_IsArrayBufferObject(arrayBuff)) {
        JS_ReportError(cx, "Data is not an ArrayBuffer\n");
        return JS_TRUE;
    }

    NativeJSAudioNode *node = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));
    node->arrayBuff = &JS_ARGV(cx, vp)[0];
    JS_AddValueRoot(cx, node->arrayBuff);

    if (int ret = source->open(
                JS_GetArrayBufferData(arrayBuff), 
                JS_GetArrayBufferByteLength(arrayBuff)) < 0) {
        JS_ReportError(cx, "Failed to open stream %d\n", ret);
        return JS_TRUE;
    }

    return JS_TRUE;
}
static JSBool native_audionode_source_play(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeAudioTrack *source = (NativeAudioTrack *) NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp))->node;

    source->play();

    return JS_TRUE;
}

static JSBool native_audionode_source_pause(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeAudioTrack *source = (NativeAudioTrack *) NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp))->node;

    source->pause();
    return JS_TRUE;
}
static JSBool native_audionode_source_stop(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeAudioTrack *source = (NativeAudioTrack *) NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp))->node;

    source->stop();
    return JS_TRUE;
}

static JSBool native_audionode_custom_prop_set(JSContext *cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
    NativeJSAudioNode *jsNode = NATIVE_AUDIO_NODE_GETTER(obj);

    switch(JSID_TO_INT(id)) {
        case NODE_CUSTOM_PROP_BUFFER :
        {
            JSString *fn;
            JSFunction *nfn;
            NativeAudioNodeCustom *node;
            
            if ((nfn = JS_ValueToFunction(cx, vp)) == NULL ||
                (fn = JS_DecompileFunctionBody(cx, nfn, 0)) == NULL) {
                JS_ReportError(cx, "Failed to read AudioNode callback function\n");
                vp.set(JSVAL_VOID);
                return JS_TRUE;
            } 

            jsNode->bufferStr = JS_EncodeString(cx, fn);

            node = static_cast<NativeAudioNodeCustom *>(jsNode->node);

            if (!jsNode->audio->tcx) {
                node->callback(NativeJSAudioNode::ctxCallback, jsNode);
            }

            node->setCallback(jsNode->cbk, static_cast<void *>(jsNode));
        }
        break;
        default:
            break;
    }
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
    NativeJSAudioNode *source = NATIVE_AUDIO_NODE_GETTER(obj);
    if (source != NULL) {
        delete source;
    } 
}

NATIVE_OBJECT_EXPOSE(Audio)
NATIVE_OBJECT_EXPOSE_NOT_INST(AudioNode)
