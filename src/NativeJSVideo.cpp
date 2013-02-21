
NativeJSVideo::NativeJSVideo(NativeAudio *audio, NativeSkia *nskia, JSContext *cx) 
    : cx(cx), nskia(nskia) 
{
    this->video = new NativeVideo(audio, (ape_global *)JS_GetContextPrivate(cx));
    this->video->setCallback(NativeJSVideo::frameCallback, this);
}

void NativeJSVideo::frameCallback(int width, int height, uint8_t *data, void *custom)
{
    NativeJSVideo *v = (NativeJSVideo *)custom;

    v->nskia->drawPixels(data, width, height, 0, 0);

    /*
    jsval rval, params[3];

    JSObject *arrBuff, *arr;
    arrBuff = JS_NewArrayBuffer(video->cx, width*height*4);
    uint8_t *arrData = JS_GetArrayBufferData(arrBuff);

    memcpy(arrData, data, width*height*4);

    arr = JS_NewUint8ArrayWithBuffer(video->cx, arrBuff, 0, -1);


    params[0] = OBJECT_TO_JSVAL(arr);
    params[1] = INT_TO_JSVAL(width);
    params[2] = INT_TO_JSVAL(height);


    JSAutoRequest ar(video->cx);
    JS_CallFunctionName(video->cx, video->jsobj, "onframe", 3, params, &rval);
    */
}


static JSBool native_video_play(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSVideo *v = NATIVE_VIDEO_GETTER(JS_THIS_OBJECT(cx, vp));

    v->video->play();

    return JS_TRUE;
}

static JSBool native_video_open(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSVideo *v = NATIVE_VIDEO_GETTER(JS_THIS_OBJECT(cx, vp));
    JSObject *arrayBuff;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &arrayBuff)) {
        return JS_TRUE;
    }

    if (!JS_IsArrayBufferObject(arrayBuff)) {
        JS_ReportError(cx, "Data is not an ArrayBuffer\n");
        return JS_TRUE;
    }

    int length;
    uint8_t *data;

    length = JS_GetArrayBufferByteLength(arrayBuff);
    JS_StealArrayBufferContents(cx, arrayBuff, &v->arrayContent, &data);

    if (int ret = v->video->open(
                data, 
                length) < 0) {
        JS_ReportError(cx, "Failed to open stream %d\n", ret);
        return JS_TRUE;
    }

    return JS_TRUE;
}

static JSBool native_video_get_audio(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSVideo *v = NATIVE_VIDEO_GETTER(JS_THIS_OBJECT(cx, vp));

    NativeAudioTrack *track = v->video->getAudio();

    return JS_TRUE;
}

static JSBool native_Video_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *ret = JS_NewObjectForConstructor(cx, &Video_class, vp);
    JSObject *canvas;
    JSObject *audio;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oo", &audio, &canvas)) {
        return JS_TRUE;
    }

    //NJS->rootUntilShutdown(ret);

    NativeSkia *nskia = ((class NativeCanvasHandler *)JS_GetPrivate(canvas))->context->skia;
    NativeAudio *naudio = (class NativeAudio *)JS_GetPrivate(audio);

    NativeJSVideo *v = new NativeJSVideo(naudio, nskia, cx);

    JS_DefineFunctions(cx, ret, Video_funcs);

    JS_SetPrivate(ret, v);
    v->jsobj = ret;

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

NativeJSVideo::~NativeJSVideo() {
    delete this->video;
}

static void Video_Finalize(JSFreeOp *fop, JSObject *obj) {
    NativeJSVideo *v = (NativeJSVideo *)JS_GetPrivate(obj);
    if (v != NULL) {
        delete v;
    }
}

NATIVE_OBJECT_EXPOSE(Video);
