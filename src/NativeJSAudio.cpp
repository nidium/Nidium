#include "NativeJSAudio.h"

#define NATIVE_AUDIO_GETTER(obj) ((class NativeJSAudio *)JS_GetPrivate(obj))
#define NATIVE_AUDIO_NODE_GETTER(obj) ((class NativeJSAudioNode *)JS_GetPrivate(obj))

static void AudioNode_Finalize(JSFreeOp *fop, JSObject *obj);
static void Audio_Finalize(JSFreeOp *fop, JSObject *obj);

static JSBool native_Audio_constructor(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audio_createnode(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audio_connect(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_audionode_input(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_output(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_set(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_get(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_audionode_source_open(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_source_play(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_audionode_source_stop(JSContext *cx, unsigned argc, jsval *vp);

static JSClass Audio_class = {
    "Audio", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Audio_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass AudioNodeLink_class = {
    "AudioNodeLink", JSCLASS_HAS_PRIVATE,
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

static JSFunctionSpec audio_funcs[] = {
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

static JSFunctionSpec audionode_source_funcs[] = {
    JS_FN("open", native_audionode_source_open, 1, 0),
    JS_FN("play", native_audionode_source_play, 0, 0),
    JS_FN("stop", native_audionode_source_stop, 0, 0),
    JS_FS_END
};

NativeJSAudio::NativeJSAudio(int size, int channels, int frequency) {
    audio = new NativeAudio(size, channels, frequency);

    // 1) Create thread for I/O
    //pthread_create(&threadIO, NULL, thread_io, audio);

    pthread_create(&threadDecode, NULL, NativeAudio::decodeThread, audio);
    pthread_create(&threadQueue, NULL, NativeAudio::queueThread, audio);
}

NativeJSAudio::~NativeJSAudio() {
    this->audio->shutdown();
    pthread_join(this->threadQueue, NULL);
    pthread_join(this->threadDecode, NULL);
    delete this->audio;
}

NativeJSAudioNode::~NativeJSAudioNode()
{
    if (this->arrayBuff != NULL) {
        JS_RemoveObjectRoot(this->cx, &this->arrayBuff);
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

    JS_DefineFunctions(cx, ret, audio_funcs);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
    return JS_TRUE;
}

static JSBool native_audio_createnode(JSContext *cx, unsigned argc, jsval *vp)
{
    int in, out;
    JSObject *ret;
    JSString *name;
    NativeAudio *audio = NATIVE_AUDIO_GETTER(JS_THIS_OBJECT(cx, vp))->audio;
    NativeJSAudioNode *node;

    node = NULL;
    ret = NULL;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "Suu", &name, &in, &out)) {
        return JS_TRUE;
    }

    JSAutoByteString cname(cx, name);
    ret = JS_NewObjectForConstructor(cx, &AudioNode_class, vp);

    if (strcmp("source", cname.ptr()) == 0) {
        node = new NativeJSAudioNode(audio->createNode(NativeAudio::SOURCE, in, out));
        JS_DefineFunctions(cx, ret, audionode_source_funcs);
    } else if (strcmp("gain", cname.ptr()) == 0) {
        node = new NativeJSAudioNode(audio->createNode(NativeAudio::GAIN, in, out));
    } else if (strcmp("target", cname.ptr()) == 0) {                      
        node = new NativeJSAudioNode(audio->createNode(NativeAudio::TARGET, in, out));
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
    NativeAudioNode::NodeLink *nlink1;
    NativeAudioNode::NodeLink *nlink2;
    NativeAudio *audio = NATIVE_AUDIO_GETTER(JS_THIS_OBJECT(cx, vp))->audio;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oo", &link1, &link2)) {
        return JS_TRUE;
    }

    nlink1 = (NativeAudioNode::NodeLink *)JS_GetInstancePrivate(cx, link1, &AudioNodeLink_class, JS_ARGV(cx, vp));
    nlink2 = (NativeAudioNode::NodeLink *)JS_GetInstancePrivate(cx, link2, &AudioNodeLink_class, JS_ARGV(cx, vp));

    if (nlink1 == NULL || nlink2 == NULL) {
        JS_ReportError(cx, "Bad AudioNodeLink\n");
        return JS_TRUE;
    }

    if ((nlink1->type == NativeAudioNode::INPUT && nlink2->type == NativeAudioNode::OUTPUT) ||
        (nlink1->type == NativeAudioNode::OUTPUT && nlink2->type == NativeAudioNode::INPUT)) {
        audio->connect(nlink1, nlink2);
    } else {
        JS_ReportError(cx, "You must give one input and one output\n");
        return JS_TRUE;
    }

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
    NativeAudioNode::ArgType type;
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
        type = NativeAudioNode::INT;
        size = sizeof(int);
        intVal = val.toInt32();
        value = &intVal;
    } else if (val.isDouble()) {
        type = NativeAudioNode::DOUBLE;
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

static JSBool native_audionode_source_open(JSContext *cx, unsigned argc, jsval *vp) 
{
    NativeJSAudioNode *jnode = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));
    NativeAudioTrack *source = (NativeAudioTrack *)jnode->node;
    JSObject *arrayBuff;

    JS_AddValueRoot(cx, &JS_ARGV(cx, vp)[0]);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &arrayBuff)) {
        return JS_TRUE;
    }

    // TODO : use JS_StealArrayBufferContents

    if (!JS_IsArrayBufferObject(arrayBuff, cx)) {
        JS_ReportError(cx, "Data is not an ArrayBuffer\n");
        return JS_TRUE;
    }

    NativeJSAudioNode *node = NATIVE_AUDIO_NODE_GETTER(JS_THIS_OBJECT(cx, vp));
    node->arrayBuff = arrayBuff;
    //JS_AddObjectRoot(cx, &node->arrayBuff);

    if (int ret = source->open(
                JS_GetArrayBufferData(arrayBuff, cx), 
                JS_GetArrayBufferByteLength(arrayBuff, cx)) < 0) {
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
static JSBool native_audionode_source_stop(JSContext *cx, unsigned argc, jsval *vp)
{
    // Nothing yet
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
