#include "NativeJS.h"
#include "NativeSkia.h"
#include "NativeSkGradient.h"
#include "NativeSkImage.h"
#include "NativeSharedMessages.h"
#include "NativeJSSocket.h"
#include "NativeJSThread.h"
#include "NativeJSHttp.h"
#include "NativeJSImage.h"
#include "NativeJSAudio.h"
#include "NativeJSNative.h"
#include "NativeJSWindow.h"
#include "NativeFileIO.h"
#include "NativeJSWebGL.h"
#include "NativeJSCanvas.h"
#include "NativeJSFileIO.h"

#include "NativeCanvasHandler.h"
#include "NativeCanvas2DContext.h"

#include "SkImageDecoder.h"

#include <stdio.h>
#include <jsapi.h>
#include <jsprf.h>
#include <stdint.h>

#ifdef __linux__
   #define UINT32_MAX 4294967295u
#endif

#include <jsfriendapi.h>
#include <jsdbgapi.h>

#include <math.h>

struct _native_sm_timer
{
    JSContext *cx;
    JSObject *global;
    jsval func;

    unsigned argc;
    jsval *argv;
    int ms;
    int cleared;
    struct _ticks_callback *timer;
    ape_timer *timerng;
};

/* Assume that we can not use more than 5e5 bytes of C stack by default. */
#if (defined(DEBUG) && defined(__SUNPRO_CC))  || defined(JS_CPU_SPARC)
/* Sun compiler uses larger stack space for js_Interpret() with debug
   Use a bigger gMaxStackSize to make "make check" happy. */
#define DEFAULT_MAX_STACK_SIZE 5000000
#else
#define DEFAULT_MAX_STACK_SIZE 500000
#endif

size_t gMaxStackSize = DEFAULT_MAX_STACK_SIZE;


#define NJS ((class NativeJS *)JS_GetRuntimePrivate(JS_GetRuntime(cx)))

static void native_timer_wrapper(struct _native_sm_timer *params, int *last);
static int native_timerng_wrapper(void *arg);


static JSClass global_class = {
    "global", JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};


static JSClass mouseEvent_class = {
    "MouseEvent", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass messageEvent_class = {
    "MessageEvent", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass textEvent_class = {
    "TextInputEvent", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass keyEvent_class = {
    "keyEvent", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

jsval gfunc  = JSVAL_VOID;

/******** Natives ********/
static JSBool Print(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_load(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_set_timeout(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_set_interval(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_clear_timeout(JSContext *cx, unsigned argc, jsval *vp);

/*************************/


static JSFunctionSpec glob_funcs[] = {
    
    JS_FN("echo", Print, 0, 0),
    JS_FN("load", native_load, 1, 0),
    JS_FN("setTimeout", native_set_timeout, 2, 0),
    JS_FN("setInterval", native_set_interval, 2, 0),
    JS_FN("clearTimeout", native_clear_timeout, 1, 0),
    JS_FN("clearInterval", native_clear_timeout, 1, 0),
    JS_FS_END
};


void
reportError(JSContext *cx, const char *message, JSErrorReport *report)
{
    int i, j, k, n;
    char *prefix, *tmp;
    const char *ctmp;

    if (!report) {
        fprintf(stdout, "%s\n", message);
        fflush(stdout);
        return;
    }

    /* Conditionally ignore reported warnings. */
    /*if (JSREPORT_IS_WARNING(report->flags) && !reportWarnings)
        return;*/

    prefix = NULL;
    if (report->filename)
        prefix = JS_smprintf("%s:", report->filename);
    if (report->lineno) {
        tmp = prefix;
        prefix = JS_smprintf("%s%u: ", tmp ? tmp : "", report->lineno);
        JS_free(cx, tmp);
    }
    if (JSREPORT_IS_WARNING(report->flags)) {
        tmp = prefix;
        prefix = JS_smprintf("%s%swarning: ",
                             tmp ? tmp : "",
                             JSREPORT_IS_STRICT(report->flags) ? "strict " : "");
        JS_free(cx, tmp);
    }

    /* embedded newlines -- argh! */
    while ((ctmp = strchr(message, '\n')) != 0) {
        ctmp++;
        if (prefix)
            fputs(prefix, stdout);
        fwrite(message, 1, ctmp - message, stdout);
        message = ctmp;
    }

    /* If there were no filename or lineno, the prefix might be empty */
    if (prefix)
        fputs(prefix, stdout);
    fputs(message, stdout);

    if (!report->linebuf) {
        fputc('\n', stdout);
        goto out;
    }

    /* report->linebuf usually ends with a newline. */
    n = strlen(report->linebuf);
    fprintf(stdout, ":\n%s%s%s%s",
            prefix,
            report->linebuf,
            (n > 0 && report->linebuf[n-1] == '\n') ? "" : "\n",
            prefix);
    n = report->tokenptr - report->linebuf;
    for (i = j = 0; i < n; i++) {
        if (report->linebuf[i] == '\t') {
            for (k = (j + 8) & ~7; j < k; j++) {
                fputc('.', stdout);
            }
            continue;
        }
        fputc('.', stdout);
        j++;
    }
    fputs("^\n", stdout);
 out:
    fflush(stdout);
    if (!JSREPORT_IS_WARNING(report->flags)) {
    /*
        if (report->errorNumber == JSMSG_OUT_OF_MEMORY) {
            gExitCode = EXITCODE_OUT_OF_MEMORY;
        } else {
            gExitCode = EXITCODE_RUNTIME_ERROR;
        }*/
    }
    JS_free(cx, prefix);
}

static JSBool
PrintInternal(JSContext *cx, unsigned argc, jsval *vp, FILE *file)
{
    jsval *argv;
    unsigned i;
    JSString *str;
    char *bytes;

    argv = JS_ARGV(cx, vp);
    for (i = 0; i < argc; i++) {
        str = JS_ValueToString(cx, argv[i]);
        if (!str)
            return false;
        bytes = JS_EncodeString(cx, str);
        if (!bytes)
            return false;
        fprintf(file, "%s%s", i ? " " : "", bytes);
        JS_free(cx, bytes);
    }

    fputc('\n', file);
    fflush(file);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return true;
}

static JSBool
Print(JSContext *cx, unsigned argc, jsval *vp)
{
    return PrintInternal(cx, argc, vp, stdout);
}

void NativeJS::postDraw()
{
    char fps[16];
    if (NativeJSNative::showFPS) {
        sprintf(fps, "%d fps", currentFPS);
        surface->system(fps, 5, 300);
        surface->flush();
    }
}

void NativeJS::callFrame()
{
    jsval rval;

    if (gfunc != JSVAL_VOID) {
        JSAutoRequest ar(cx);
        JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), gfunc, 0, NULL, &rval);
    }
}

void NativeJS::mouseWheel(int xrel, int yrel, int x, int y)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY)
    
    jsval rval, jevent, canvas, onwheel;
    JSObject *event;

    JSAutoRequest ar(cx);

    event = JS_NewObject(cx, &mouseEvent_class, NULL, NULL);
    JS_AddObjectRoot(cx, &event);

    EVENT_PROP("xrel", INT_TO_JSVAL(xrel));
    EVENT_PROP("yrel", INT_TO_JSVAL(yrel));
    EVENT_PROP("x", INT_TO_JSVAL(x));
    EVENT_PROP("y", INT_TO_JSVAL(y));

    jevent = OBJECT_TO_JSVAL(event);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "Native", &canvas);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(canvas), "onmousewheel", &onwheel) &&
        !JSVAL_IS_PRIMITIVE(onwheel) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onwheel))) {

        JS_CallFunctionValue(cx, event, onwheel, 1, &jevent, &rval);
    }


    JS_RemoveObjectRoot(cx, &event);    
}

void NativeJS::keyupdown(int keycode, int mod, int state, int repeat)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY)
    
    JSObject *event;
    jsval jevent, onkeyupdown, canvas, rval;

    JSAutoRequest ar(cx);

    event = JS_NewObject(cx, &keyEvent_class, NULL, NULL);
    JS_AddObjectRoot(cx, &event);

    EVENT_PROP("keyCode", INT_TO_JSVAL(keycode));
    EVENT_PROP("altKey", BOOLEAN_TO_JSVAL(!!(mod & NATIVE_KEY_ALT)));
    EVENT_PROP("ctrlKey", BOOLEAN_TO_JSVAL(!!(mod & NATIVE_KEY_CTRL)));
    EVENT_PROP("shiftKey", BOOLEAN_TO_JSVAL(!!(mod & NATIVE_KEY_SHIFT)));
    EVENT_PROP("repeat", BOOLEAN_TO_JSVAL(!!(repeat)));

    jevent = OBJECT_TO_JSVAL(event);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "Native", &canvas);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(canvas),
        (state ? "onkeydown" : "onkeyup"), &onkeyupdown) &&
        !JSVAL_IS_PRIMITIVE(onkeyupdown) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onkeyupdown))) {

        JS_CallFunctionValue(cx, event, onkeyupdown, 1, &jevent, &rval);
    }

    JS_RemoveObjectRoot(cx, &event);
}

void NativeJS::textInput(const char *data)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY)

    JSObject *event;
    jsval jevent, ontextinput, canvas, rval;

    JSAutoRequest ar(cx);

    event = JS_NewObject(cx, &textEvent_class, NULL, NULL);
    JS_AddObjectRoot(cx, &event);

    EVENT_PROP("val",
        STRING_TO_JSVAL(JS_NewStringCopyN(cx, data, strlen(data))));

    jevent = OBJECT_TO_JSVAL(event);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "Native", &canvas);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(canvas), "ontextinput", &ontextinput) &&
        !JSVAL_IS_PRIMITIVE(ontextinput) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(ontextinput))) {

        JS_CallFunctionValue(cx, event, ontextinput, 1, &jevent, &rval);
    }

    JS_RemoveObjectRoot(cx, &event);
}

void NativeJS::mouseClick(int x, int y, int state, int button)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY)

    jsval rval, jevent;
    JSObject *event;

    jsval canvas, onclick;

    JSAutoRequest ar(cx);

    event = JS_NewObject(cx, &mouseEvent_class, NULL, NULL);
    JS_AddObjectRoot(cx, &event);

    EVENT_PROP("x", INT_TO_JSVAL(x));
    EVENT_PROP("y", INT_TO_JSVAL(y));
    EVENT_PROP("clientX", INT_TO_JSVAL(x));
    EVENT_PROP("clientY", INT_TO_JSVAL(y));
    EVENT_PROP("which", INT_TO_JSVAL(button));

    jevent = OBJECT_TO_JSVAL(event);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "Native", &canvas);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(canvas),
        (state ? "onmousedown" : "onmouseup"), &onclick) &&
        !JSVAL_IS_PRIMITIVE(onclick) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onclick))) {

        JS_CallFunctionValue(cx, event, onclick, 1, &jevent, &rval);
    }

    JS_RemoveObjectRoot(cx, &event);

}

void NativeJS::mouseMove(int x, int y, int xrel, int yrel)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY)
    
    jsval rval, jevent, canvas, onmove;
    JSObject *event;

    JSAutoRequest ar(cx);

    event = JS_NewObject(cx, &mouseEvent_class, NULL, NULL);
    JS_AddObjectRoot(cx, &event);

    EVENT_PROP("x", INT_TO_JSVAL(x));
    EVENT_PROP("y", INT_TO_JSVAL(y));
    EVENT_PROP("xrel", INT_TO_JSVAL(xrel));
    EVENT_PROP("yrel", INT_TO_JSVAL(yrel));
    EVENT_PROP("clientX", INT_TO_JSVAL(x));
    EVENT_PROP("clientY", INT_TO_JSVAL(y));

    jevent = OBJECT_TO_JSVAL(event);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "Native", &canvas);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(canvas), "onmousemove", &onmove) &&
        !JSVAL_IS_PRIMITIVE(onmove) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onmove))) {

        JS_CallFunctionValue(cx, event, onmove, 1, &jevent, &rval);
    }


    JS_RemoveObjectRoot(cx, &event);
}

static JSBool native_load(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *script;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &script)) {
        return JS_TRUE;
    }

    JSAutoByteString scriptstr(cx, script);

    if (!NJS->LoadScript(scriptstr.ptr())) {
        return JS_TRUE;
    }

    return JS_TRUE;
}

static void gccb(JSRuntime *rt, JSGCStatus status)
{
    //printf("Gc TH1 callback?\n");
}

NativeJS::NativeJS(int width, int height)
{
    JSRuntime *rt;
    JSObject *gbl;

    static int isUTF8 = 0;
    gfunc = JSVAL_VOID;
    /* TODO: BUG */
    if (!isUTF8) {
        //JS_SetCStringsAreUTF8();
        isUTF8 = 1;
    }
    //printf("New JS runtime\n");

    currentFPS = 0;

    if ((rt = JS_NewRuntime(128L * 1024L * 1024L,
        JS_USE_HELPER_THREADS)) == NULL) {
        
        printf("Failed to init JS runtime\n");
        return;
    }

    JS_SetGCParameter(rt, JSGC_MAX_BYTES, 0xffffffff);
    JS_SetGCParameter(rt, JSGC_MODE, JSGC_MODE_INCREMENTAL);
    JS_SetGCParameter(rt, JSGC_SLICE_TIME_BUDGET, 15);
    JS_SetGCParameterForThread(cx, JSGC_MAX_CODE_CACHE_BYTES, 16 * 1024 * 1024);

    JS_SetNativeStackQuota(rt, gMaxStackSize);

    if ((cx = JS_NewContext(rt, 8192)) == NULL) {
        printf("Failed to init JS context\n");
        return;     
    }
    //JS_BeginRequest(cx);
    //JSAutoRequest ar(cx);
    JS_SetVersion(cx, JSVERSION_LATEST);

    JS_SetOptions(cx, JSOPTION_VAROBJFIX  | JSOPTION_METHODJIT |
        JSOPTION_TYPE_INFERENCE | JSOPTION_ION);

    //ion::js_IonOptions.gvnIsOptimistic = true;

    JS_SetErrorReporter(cx, reportError);

    JSAutoRequest ar(cx);

    gbl = JS_NewGlobalObject(cx, &global_class, NULL);
    if (gbl == NULL) {
        printf("Cant create global object\n");
        return;
    }

    if (!JS_InitStandardClasses(cx, gbl))
        return;

    //JS_DefineProfilingFunctions(cx, gbl);

    JS_SetGCCallback(rt, gccb);

    /* TODO: HAS_CTYPE in clang */
    //JS_InitCTypesClass(cx, gbl);

    JS_SetGlobalObject(cx, gbl);
    JS_DefineFunctions(cx, gbl, glob_funcs);

    /* surface contains the window frame buffer */

    rootHandler = new NativeCanvasHandler(width, height);
    rootHandler->context = new NativeCanvas2DContext(width, height);

    surface = rootHandler->context->skia;

    //JS_SetContextPrivate(cx, nskia);
    JS_SetRuntimePrivate(rt, this);


    LoadGlobalObjects(surface, width, height);

    messages = new NativeSharedMessages();

    //animationframeCallbacks = ape_new_pool(sizeof(ape_pool_t), 8);
}

void NativeJS::forceLinking()
{
#ifdef __linux__
    CreateJPEGImageDecoder();
    CreatePNGImageDecoder();
    //CreateGIFImageDecoder();
    CreateBMPImageDecoder();
    CreateICOImageDecoder();
    CreateWBMPImageDecoder();
#endif
}

NativeJS::~NativeJS()
{
    JSRuntime *rt;
    rt = JS_GetRuntime(cx);

    ape_global *net = (ape_global *)JS_GetContextPrivate(cx);

    JS_BeginRequest(cx);

    JS_RemoveValueRoot(cx, &gfunc);
    /* clear all non protected timers */
    del_timers_unprotected(&net->timersng);
    NativeJSAudio::shutdown();

    rootHandler->unrootHierarchy();
    //JS_SetAllNonReservedSlotsToUndefined(cx, JS_GetGlobalObject(cx));
    
    delete rootHandler;

    JS_EndRequest(cx);

    NativeSkia::glcontext = NULL;
    NativeSkia::glsurface = NULL;

    JS_DestroyContext(cx);

    JS_DestroyRuntime(rt);

    JS_ShutDown();


    delete messages;
    //delete nskia; /* TODO: why is that commented out? */
    // is it covered by Canvas_Finalize()?
}

void NativeJS::bufferSound(int16_t *data, int len)
{
    jsval canvas, onwheel;

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(canvas), "onmousewheel", &onwheel) &&
        !JSVAL_IS_PRIMITIVE(onwheel) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onwheel))) {

       // JS_CallFunctionValue(cx, event, onwheel, 0, NULL, &rval);
    }
}

static int Native_handle_messages(void *arg)
{
#define MAX_MSG_IN_ROW 20

#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY)

    NativeJS *njs = (NativeJS *)arg;
    JSContext *cx = njs->cx;
    struct native_thread_msg *ptr;
    jsval onmessage, jevent, rval;
    int nread = 0;

    JSObject *event;

    NativeSharedMessages::Message msg;
    JSAutoRequest ar(cx);

    while (++nread < MAX_MSG_IN_ROW && njs->messages->readMessage(&msg)) {
        switch (msg.event()) {
            case NATIVE_THREAD_MESSAGE:
            ptr = static_cast<struct native_thread_msg *>(msg.dataPtr());

            if (JS_GetProperty(cx, ptr->callee, "onmessage", &onmessage) &&
                !JSVAL_IS_PRIMITIVE(onmessage) && 
                JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onmessage))) {

                jsval inval;

                if (!JS_ReadStructuredClone(cx, ptr->data, ptr->nbytes,
                    JS_STRUCTURED_CLONE_VERSION, &inval, NULL, NULL)) {

                    printf("Failed to read input data (readMessage)\n");

                    continue;
                }

                event = JS_NewObject(cx, &messageEvent_class, NULL, NULL);
                JS_AddObjectRoot(cx, &event);

                EVENT_PROP("message", inval);

                jevent = OBJECT_TO_JSVAL(event);
                JS_CallFunctionValue(cx, event, onmessage, 1, &jevent, &rval);
                JS_RemoveObjectRoot(cx, &event);            

            }
            delete ptr;
            break;
            case NATIVE_AUDIO_THREAD_MESSAGE_CALLBACK: {
            NativeJSAudioNode::MessageCallback *cmsg = static_cast<struct NativeJSAudioNode::MessageCallback *>(msg.dataPtr());
            if (JS_GetProperty(cx, cmsg->callee, cmsg->prop, &onmessage) &&
                !JSVAL_IS_PRIMITIVE(onmessage) && 
                JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onmessage))) {

                if (cmsg->value != NULL) {
                    jevent = INT_TO_JSVAL(*cmsg->value);
                } else {
                    jevent = JSVAL_NULL;
                }
                JS_CallFunctionValue(cx, cmsg->callee, onmessage, 1, &jevent, &rval);
            }
            delete cmsg; 
            break;
            }
            default:break;
        }
    }

    return 1;
}

#if 0
void NativeJS::onNFIOOpen(NativeFileIO *nfio)
{
    printf("Open success\n");
    nfio->getContents();
}

void NativeJS::onNFIOError(NativeFileIO *nfio, int errno)
{
    printf("Error while opening file\n");
}

void NativeJS::onNFIORead(NativeFileIO *nfio, unsigned char *data, size_t len)
{
    printf("Data from file : %s\n", data);
}
#endif

void NativeJS::bindNetObject(ape_global *net)
{
    JS_SetContextPrivate(cx, net);

    ape_timer *timer = add_timer(&net->timersng, 1,
        Native_handle_messages, this);

    timer->flags &= ~APE_TIMER_IS_PROTECTED;

    //NativeFileIO *io = new NativeFileIO("/tmp/foobar", this, net);
    //io->open();
}

int NativeJS::LoadScript(const char *filename)
{
    uint32_t oldopts;

    JSAutoRequest ar(cx);

    JSObject *gbl = JS_GetGlobalObject(cx);
    oldopts = JS_GetOptions(cx);

    JS_SetOptions(cx, oldopts | JSOPTION_COMPILE_N_GO | JSOPTION_NO_SCRIPT_RVAL);
    JS::CompileOptions options(cx);
    options.setUTF8(true)
           .setFileAndLine(filename, 1);
    js::RootedObject rgbl(cx, gbl);
    JSScript *script = JS::Compile(cx, rgbl, options, filename);

#if 0
    uint32_t encoded;
    void *data;
    data = JS_EncodeScript(cx, script, &encoded);

    printf("script encoded with %d size\n", encoded);

    FILE *jsc = fopen("./compiled.jsc", "w+");

    fwrite(data, 1, encoded, jsc);
    fclose(jsc);
#endif
    JS_SetOptions(cx, oldopts);

    if (script == NULL || !JS_ExecuteScript(cx, gbl, script, NULL)) {
        if (JS_IsExceptionPending(cx)) {
            if (!JS_ReportPendingException(cx)) {
                JS_ClearPendingException(cx);
            }
        }
        return 0;
    }
    
    return 1;
}

void NativeJS::LoadGlobalObjects(NativeSkia *currentSkia, int width, int height)
{
    /* File() object */
    NativeJSFileIO::registerObject(cx);
    /* CanvasRenderingContext2D object */
    NativeCanvas2DContext::registerObject(cx);
    /* Canvas() object */
    NativeJSCanvas::registerObject(cx);
    /* Socket() object */
    NativeJSSocket::registerObject(cx);
    /* Thread() object */
    NativeJSThread::registerObject(cx);
    /* Http() object */
    NativeJSHttp::registerObject(cx);
    /* Image() object */
    NativeJSImage::registerObject(cx);
    /* Audio() object */
    NativeJSAudio::registerObject(cx);
    NativeJSAudioNode::registerObject(cx);
    /* WebGL*() object */
    #if WEBGL_ENABLED
    NativeJSNativeGL::registerObject(cx);
    NativeJSWebGLRenderingContext::registerObject(cx);
    NativeJSWebGLObject::registerObject(cx);
    NativeJSWebGLBuffer::registerObject(cx);
    NativeJSWebGLFrameBuffer::registerObject(cx);
    NativeJSWebGLProgram::registerObject(cx);
    NativeJSWebGLRenderbuffer::registerObject(cx);
    NativeJSWebGLShader::registerObject(cx);
    NativeJSWebGLTexture::registerObject(cx);
    NativeJSWebGLUniformLocation::registerObject(cx);
    #endif
    /* Native() object */
    NativeJSNative::registerObject(cx, width, height);
    /* window() object */
    NativeJSwindow::registerObject(cx);

    //NativeJSDebug::registerObject(cx);

}

void NativeJS::gc()
{
    JS_GC(JS_GetRuntime(cx));
}


static int native_timer_deleted(void *arg)
{
    struct _native_sm_timer *params = (struct _native_sm_timer *)arg;

    JSAutoRequest ar(params->cx);

    if (params == NULL) {
        return 0;
    }

    JS_RemoveValueRoot(params->cx, &params->func);

    if (params->argv != NULL) {
        free(params->argv);
    }

    free(params);

    return 1;
}

static JSBool native_set_timeout(JSContext *cx, unsigned argc, jsval *vp)
{
    struct _native_sm_timer *params;

    int ms, i;
    JSObject *obj = JS_THIS_OBJECT(cx, vp);

    params = (struct _native_sm_timer *)malloc(sizeof(*params));

    if (params == NULL || argc < 2) {
        if (params) free(params);
        return JS_TRUE;
    }

    params->cx = cx;
    params->global = obj;
    params->argc = argc-2;
    params->cleared = 0;
    params->timer = NULL;
    params->timerng = NULL;
    params->ms = 0;

    params->argv = (argc-2 ? (jsval *)malloc(sizeof(*params->argv) * argc-2) : NULL);

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[0], JSTYPE_FUNCTION, &params->func)) {
        free(params->argv);
        free(params);
        return JS_TRUE;
    }

    if (!JS_ConvertArguments(cx, 1, &JS_ARGV(cx, vp)[1], "i", &ms)) {
        free(params->argv);
        free(params);
        return JS_TRUE;
    }

    JS_AddValueRoot(cx, &params->func);

    for (i = 0; i < (int)argc-2; i++) {
        params->argv[i] = JS_ARGV(cx, vp)[i+2];
    }

    params->timerng = add_timer(&((ape_global *)JS_GetContextPrivate(cx))->timersng,
        ms, native_timerng_wrapper,
        (void *)params);

    params->timerng->flags &= ~APE_TIMER_IS_PROTECTED;
    params->timerng->clearfunc = native_timer_deleted;

    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(params->timerng->identifier));

    return JS_TRUE;
}

static JSBool native_set_interval(JSContext *cx, unsigned argc, jsval *vp)
{
    struct _native_sm_timer *params;
    int ms, i;
    JSObject *obj = JS_THIS_OBJECT(cx, vp);

    params = (struct _native_sm_timer *)malloc(sizeof(*params));

    if (params == NULL || argc < 2) {
        if (params) free(params);
        return JS_TRUE;
    }

    params->cx = cx;
    params->global = obj;
    params->argc = argc-2;
    params->cleared = 0;
    params->timer = NULL;

    params->argv = (argc-2 ? (jsval *)malloc(sizeof(*params->argv) * argc-2) : NULL);

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[0], JSTYPE_FUNCTION, &params->func)) {
        free(params->argv);
        free(params);
        return JS_TRUE;
    }

    if (!JS_ConvertArguments(cx, 1, &JS_ARGV(cx, vp)[1], "i", &ms)) {
        free(params->argv);
        free(params);
        return JS_TRUE;
    }

    params->ms = ms;

    JS_AddValueRoot(cx, &params->func);

    for (i = 0; i < (int)argc-2; i++) {
        params->argv[i] = JS_ARGV(cx, vp)[i+2];
    }

    params->timerng = add_timer(&((ape_global *)JS_GetContextPrivate(cx))->timersng,
        ms, native_timerng_wrapper,
        (void *)params);

    params->timerng->flags &= ~APE_TIMER_IS_PROTECTED;
    params->timerng->clearfunc = native_timer_deleted;

    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(params->timerng->identifier));

    return JS_TRUE; 
}

static JSBool native_clear_timeout(JSContext *cx, unsigned argc, jsval *vp)
{
    unsigned int identifier;

    if (!JS_ConvertArguments(cx, 1, JS_ARGV(cx, vp), "i", &identifier)) {
        return JS_TRUE;
    }

    clear_timer_by_id(&((ape_global *)JS_GetContextPrivate(cx))->timersng,
        identifier, 0);

    /* TODO: remove root / clear params */

    return JS_TRUE;    
}

static int native_timerng_wrapper(void *arg)
{
    jsval rval;
    struct _native_sm_timer *params = (struct _native_sm_timer *)arg;

    JSAutoRequest ar(params->cx);

    JS_CallFunctionValue(params->cx, params->global, params->func,
        params->argc, params->argv, &rval);


    //timers_stats_print(&((ape_global *)JS_GetContextPrivate(params->cx))->timersng);

    return params->ms;
}
