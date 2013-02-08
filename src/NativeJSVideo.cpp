#include "NativeJSVideo.h"
#include "NativeJS.h"
#include "NativeCanvasHandler.h"
#include "NativeCanvas2DContext.h"

#define NJS ((class NativeJS *)JS_GetRuntimePrivate(JS_GetRuntime(cx)))

static JSBool native_Video_constructor(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_video_play(JSContext *cx, unsigned argc, jsval *vp);

static void Video_Finalize(JSFreeOp *fop, JSObject *obj);

static JSFunctionSpec Video_funcs[] = {
    JS_FN("play", native_video_play, 0, 0),
    JS_FS_END
};

static JSClass Video_class = {
    "Video", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Video_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

void frameCallback(int width, int height, uint8_t *data, void *custom);

NativeJSVideo::NativeJSVideo(char *file, NativeSkia *nskia, JSContext *cx) : cx(cx), nskia(nskia) {
    this->video = new NativeVideo((ape_global *)JS_GetContextPrivate(cx), file);
    this->video->setCallback(frameCallback, this);
}

void frameCallback(int width, int height, uint8_t *data, void *custom)
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
    NativeJSVideo *v = (NativeJSVideo *)JS_GetPrivate(JS_THIS_OBJECT(cx, vp));

    v->video->play();

    return JS_TRUE;
}

static JSBool native_Video_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *ret = JS_NewObjectForConstructor(cx, &Video_class, vp);
    JSObject *canvas;
    JSString *file;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "So", &file, &canvas)) {
        return JS_TRUE;
    }

    JSAutoByteString cfile(cx, file);

    //NJS->rootUntilShutdown(ret);

    NativeSkia *nskia= ((class NativeCanvasHandler *)JS_GetPrivate(canvas))->context->skia;

    NativeJSVideo *v = new NativeJSVideo(cfile.ptr(), nskia, cx);

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
