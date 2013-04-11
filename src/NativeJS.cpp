#include "NativeJS.h"
#include "NativeSkia.h"
#include "NativeSkGradient.h"
#include "NativeSkImage.h"
#include "NativeSharedMessages.h"
#include "NativeJSSocket.h"
#include "NativeJSThread.h"
#include "NativeJSHttp.h"
#include "NativeJSImage.h"
#include "NativeJSAV.h"
#include "NativeJSNative.h"
#include "NativeJSWindow.h"
#include "NativeFileIO.h"
#include "NativeJSWebGL.h"
#include "NativeJSCanvas.h"
#include "NativeJSFileIO.h"
#include "NativeJSConsole.h"

#include "NativeCanvasHandler.h"
#include "NativeCanvas2DContext.h"
#include "NativeUIInterface.h"

#include "NativeApp.h"

#include "SkImageDecoder.h"

#include <ape_hash.h>

#include <stdio.h>
#include <jsapi.h>
#include <jsprf.h>
#include <stdint.h>
#include <sys/stat.h>

#ifdef __linux__
   #define UINT32_MAX 4294967295u
#endif

#include <jsfriendapi.h>
#include <jsdbgapi.h>

#include <math.h>

#include "NativeJS_preload.h"
#include "NativeUtils.h"
#include "NativeStreamTest.h"

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

static int NativeJS_NativeJSLoadScriptReturn(JSContext *cx,
    const char *filename, jsval *ret);


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

static JSClass NMLEvent_class = {
    "NMLEvent", 0,
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

NativeStreamTest *streamTest;

/******** Natives ********/
static JSBool Print(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_load(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_set_timeout(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_set_interval(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_clear_timeout(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_readData(JSContext *cx, unsigned argc, jsval *vp);
/*************************/


static JSFunctionSpec glob_funcs[] = {
    
    JS_FN("echo", Print, 0, 0),
    JS_FN("load", native_load, 2, 0),
    JS_FN("require", native_load, 2, 0),
    JS_FN("setTimeout", native_set_timeout, 2, 0),
    JS_FN("setInterval", native_set_interval, 2, 0),
    JS_FN("clearTimeout", native_clear_timeout, 1, 0),
    JS_FN("clearInterval", native_clear_timeout, 1, 0),
    JS_FN("readData", native_readData, 0, 0),
    JS_FS_END
};

static JSBool native_readData(JSContext *cx, unsigned argc, jsval *vp)
{
    streamTest->read();
    return JS_TRUE;
}

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
        if (i) {
           NJS->UI->getConsole()->log(" "); 
        }
        NJS->UI->getConsole()->log(bytes);

        JS_free(cx, bytes);
    }
    NJS->UI->getConsole()->log("\n"); 

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return true;
}

static JSBool
Print(JSContext *cx, unsigned argc, jsval *vp)
{
    return PrintInternal(cx, argc, vp, stdout);
}

void NativeJS::createDebugCanvas()
{
    static const int DEBUG_HEIGHT = 60;
    debugHandler = new NativeCanvasHandler(surface->getWidth(), DEBUG_HEIGHT);
    debugHandler->context = new NativeCanvas2DContext(debugHandler, surface->getWidth(), DEBUG_HEIGHT, false);

    rootHandler->addChild(debugHandler);
    debugHandler->setRight(0);
    debugHandler->setOpacity(0.6);
}

void NativeJS::postDraw()
{
    if (NativeJSNative::showFPS && debugHandler) {

        NativeSkia *s = debugHandler->context->skia;
        debugHandler->bringToFront();

        s->setFillColor(0xFF000000u);
        s->drawRect(0, 0, debugHandler->getWidth(), debugHandler->getHeight(), 0);
        s->setFillColor(0xFFEEEEEEu);

        s->setFontType("monospace");
        s->drawTextf(5, 12, "NATiVE build %s %s", __DATE__, __TIME__);
        s->drawTextf(5, 25, "Frame: %lld (%lldms)\n", this->stats.nframe, stats.lastdifftime/1000000LL);
        s->drawTextf(5, 38, "Time : %lldns\n", stats.lastmeasuredtime-stats.starttime);
        s->drawTextf(5, 51, "FPS  : %.2f (%.2f)", stats.fps, stats.sampleminfps);

        s->setLineWidth(0.0);
        for (int i = 0; i < sizeof(stats.samples)/sizeof(float); i++) {
            //s->drawLine(300+i*3, 55, 300+i*3, (40/60)*stats.samples[i]);
            s->setStrokeColor(0xFF004400u);
            s->drawLine(debugHandler->getWidth()-20-i*3, 55, debugHandler->getWidth()-20-i*3, 20.f);
            s->setStrokeColor(0xFF00BB00u);   
            s->drawLine(debugHandler->getWidth()-20-i*3, 55, debugHandler->getWidth()-20-i*3, native_min(60-((40.f/62.f)*(float)stats.samples[i]), 55));
        }
        //s->setLineWidth(1.0);
        
        //s->translate(10, 10);
        //sprintf(fps, "%d fps", currentFPS);
        //s->system(fps, 5, 10);
        s->flush();
    }
}


void NativeJS::callFrame()
{
    jsval rval;
    uint64_t tmptime = NativeUtils::getTick();
    stats.nframe++;

    stats.lastdifftime = tmptime - stats.lastmeasuredtime;
    stats.lastmeasuredtime = tmptime;

    /* convert to ms */
    stats.cumultimems += (float)stats.lastdifftime / 1000000.f;
    stats.cumulframe++;

    stats.minfps = native_min(stats.minfps, 1000.f/(stats.lastdifftime/1000000.f));
    //printf("FPS : %f\n", 1000.f/(stats.lastdifftime/1000000.f));

    //printf("Last diff : %f\n", (float)(stats.lastdifftime/1000000.f));

    /* Sample every 1000ms */
    if (stats.cumultimems >= 1000.f) {
        stats.fps = 1000.f/(float)(stats.cumultimems/(float)stats.cumulframe);
        stats.cumulframe = 0;
        stats.cumultimems = 0.f;
        stats.sampleminfps = stats.minfps;
        stats.minfps = UINT32_MAX;

        memmove(&stats.samples[1], stats.samples, sizeof(stats.samples)-sizeof(float));

        stats.samples[0] = stats.fps;
    }

    if (gfunc != JSVAL_VOID) {
        JSAutoRequest ar(cx);
        JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), gfunc, 0, NULL, &rval);
    }
}

void NativeJS::mouseWheel(int xrel, int yrel, int x, int y)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)
    
    jsval rval, jevent, window, onwheel;
    JSObject *event;

    JSAutoRequest ar(cx);

    event = JS_NewObject(cx, &mouseEvent_class, NULL, NULL);

    EVENT_PROP("xrel", INT_TO_JSVAL(xrel));
    EVENT_PROP("yrel", INT_TO_JSVAL(yrel));
    EVENT_PROP("x", INT_TO_JSVAL(x));
    EVENT_PROP("y", INT_TO_JSVAL(y));

    jevent = OBJECT_TO_JSVAL(event);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "window", &window);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(window), "_onmousewheel", &onwheel) &&
        !JSVAL_IS_PRIMITIVE(onwheel) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onwheel))) {

        JS_CallFunctionValue(cx, event, onwheel, 1, &jevent, &rval);
    }
   
}

void NativeJS::keyupdown(int keycode, int mod, int state, int repeat)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)
    
    JSObject *event;
    jsval jevent, onkeyupdown, window, rval;

    JSAutoRequest ar(cx);

    event = JS_NewObject(cx, &keyEvent_class, NULL, NULL);

    EVENT_PROP("keyCode", INT_TO_JSVAL(keycode));
    EVENT_PROP("altKey", BOOLEAN_TO_JSVAL(!!(mod & NATIVE_KEY_ALT)));
    EVENT_PROP("ctrlKey", BOOLEAN_TO_JSVAL(!!(mod & NATIVE_KEY_CTRL)));
    EVENT_PROP("shiftKey", BOOLEAN_TO_JSVAL(!!(mod & NATIVE_KEY_SHIFT)));
    EVENT_PROP("repeat", BOOLEAN_TO_JSVAL(!!(repeat)));

    jevent = OBJECT_TO_JSVAL(event);
    
    JS_GetProperty(cx, JS_GetGlobalObject(cx), "window", &window);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(window),
        (state ? "_onkeydown" : "_onkeyup"), &onkeyupdown) &&
        !JSVAL_IS_PRIMITIVE(onkeyupdown) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onkeyupdown))) {

        JS_CallFunctionValue(cx, event, onkeyupdown, 1, &jevent, &rval);
    }
}

void NativeJS::textInput(const char *data)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    JSObject *event;
    jsval jevent, ontextinput, window, rval;

    JSAutoRequest ar(cx);

    event = JS_NewObject(cx, &textEvent_class, NULL, NULL);

    EVENT_PROP("val",
        STRING_TO_JSVAL(JS_NewStringCopyN(cx, data, strlen(data))));

    jevent = OBJECT_TO_JSVAL(event);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "window", &window);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(window), "_ontextinput", &ontextinput) &&
        !JSVAL_IS_PRIMITIVE(ontextinput) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(ontextinput))) {

        JS_CallFunctionValue(cx, event, ontextinput, 1, &jevent, &rval);
    }
}

void NativeJS::mouseClick(int x, int y, int state, int button)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    jsval rval, jevent;
    JSObject *event;

    jsval window, onclick;

    JSAutoRequest ar(cx);

    event = JS_NewObject(cx, &mouseEvent_class, NULL, NULL);

    EVENT_PROP("x", INT_TO_JSVAL(x));
    EVENT_PROP("y", INT_TO_JSVAL(y));
    EVENT_PROP("clientX", INT_TO_JSVAL(x));
    EVENT_PROP("clientY", INT_TO_JSVAL(y));
    EVENT_PROP("which", INT_TO_JSVAL(button));

    jevent = OBJECT_TO_JSVAL(event);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "window", &window);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(window),
        (state ? "_onmousedown" : "_onmouseup"), &onclick) &&
        !JSVAL_IS_PRIMITIVE(onclick) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onclick))) {

        JS_CallFunctionValue(cx, event, onclick, 1, &jevent, &rval);
    }
}

void NativeJS::mouseMove(int x, int y, int xrel, int yrel)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)
    
    jsval rval, jevent, window, onmove;
    JSObject *event;

    rootHandler->mousePosition.x = x;
    rootHandler->mousePosition.y = y;
    rootHandler->mousePosition.xrel += xrel;
    rootHandler->mousePosition.yrel += yrel;

    event = JS_NewObject(cx, &mouseEvent_class, NULL, NULL);

    EVENT_PROP("x", INT_TO_JSVAL(x));
    EVENT_PROP("y", INT_TO_JSVAL(y));
    EVENT_PROP("xrel", INT_TO_JSVAL(xrel));
    EVENT_PROP("yrel", INT_TO_JSVAL(yrel));
    EVENT_PROP("clientX", INT_TO_JSVAL(x));
    EVENT_PROP("clientY", INT_TO_JSVAL(y));

    jevent = OBJECT_TO_JSVAL(event);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "window", &window);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(window), "_onmousemove", &onmove) &&
        !JSVAL_IS_PRIMITIVE(onmove) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onmove))) {

        JS_CallFunctionValue(cx, event, onmove, 1, &jevent, &rval);
    }

}

void NativeJS::assetReady(const NMLTag &tag)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
        val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

    jsval window, onassetready, rval, jevent;
    JSObject *event;

    event = JS_NewObject(cx, &NMLEvent_class, NULL, NULL);
    jevent = OBJECT_TO_JSVAL(event);

    EVENT_PROP("data", STRING_TO_JSVAL(JS_NewStringCopyN(cx,
        (const char *)tag.content.data, tag.content.len)));
    EVENT_PROP("tag", STRING_TO_JSVAL(JS_NewStringCopyZ(cx, (const char *)tag.tag)));
    EVENT_PROP("id", STRING_TO_JSVAL(JS_NewStringCopyZ(cx, (const char *)tag.id)));

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "window", &window);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(window), "onassetready", &onassetready) &&
        !JSVAL_IS_PRIMITIVE(onassetready) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onassetready))) {

        JS_CallFunctionValue(cx, event, onassetready, 1, &jevent, &rval);
    }

}

static JSBool native_load(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *script, *type = NULL;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S/S", &script, &type)) {
        return JS_TRUE;
    }

    JSAutoByteString scriptstr(cx, script);
    jsval ret = JSVAL_NULL;

    if (type == NULL && !NJS->LoadScript(scriptstr.ptr())) {
        return JS_TRUE;
    } else if (type != NULL &&
        !NativeJS_NativeJSLoadScriptReturn(cx, scriptstr.ptr(), &ret)) {
        return JS_TRUE;
    }

    JS_SET_RVAL(cx, vp, ret);

    return JS_TRUE;
}

static void gccb(JSRuntime *rt, JSGCStatus status)
{
    //printf("Gc TH1 callback?\n");
}

static void PrintGetTraceName(JSTracer* trc, char *buf, size_t bufsize)
{
    snprintf(buf, bufsize, "[0x%p].mJSVal", trc->debugPrintArg);
}

static void NativeTraceBlack(JSTracer *trc, void *data)
{
    class NativeJS *self = (class NativeJS *)data;

    if (self->shutdown) {
        return;
    }
    ape_htable_item_t *item ;

    for (item = self->rootedObj->first; item != NULL; item = item->lnext) {
#ifdef DEBUG
        JS_SET_TRACING_DETAILS(trc, PrintGetTraceName, item, 0);
#endif
        JS_CallObjectTracer(trc, (JSObject *)item->addrs, NULL);
        //printf("Tracing object at %p\n", item->addrs);
    }
}

/* Use obj address as key */
void NativeJS::rootObjectUntilShutdown(JSObject *obj)
{
    hashtbl_append64(this->rootedObj, (uint64_t)obj, obj);
}

void NativeJS::unrootObject(JSObject *obj)
{
    hashtbl_erase64(this->rootedObj, (uint64_t)obj);
}

void NativeJS::Loaded()
{
    jsval canvas, onready, rval;

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "window", &canvas);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(canvas), "onready", &onready) &&
        !JSVAL_IS_PRIMITIVE(onready) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onready))) {

        JS_CallFunctionValue(cx, JSVAL_TO_OBJECT(canvas), onready, 0, NULL, &rval);
    }

}

NativeJS::NativeJS(int width, int height, NativeUIInterface *inUI, ape_global *net)
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
    shutdown = false;

    debugHandler = NULL;

    this->net = NULL;

    this->stats.nframe = 0;
    this->stats.starttime = NativeUtils::getTick();
    this->stats.lastmeasuredtime = this->stats.starttime;
    this->stats.lastdifftime = 0;
    this->stats.cumulframe = 0;
    this->stats.cumultimems = 0.f;
    this->stats.fps = 0.f;
    this->stats.minfps = UINT32_MAX;
    this->stats.sampleminfps = 0.f;

    memset(this->stats.samples, 0, sizeof(this->stats.samples));

    rootedObj = hashtbl_init(APE_HASH_INT);

    if ((rt = JS_NewRuntime(128L * 1024L * 1024L,
        JS_NO_HELPER_THREADS)) == NULL) {
        
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
    JS_BeginRequest(cx);
    JS_SetVersion(cx, JSVERSION_LATEST);
    #ifdef NATIVE_DEBUG
    JS_SetOptions(cx, JSOPTION_VAROBJFIX);
    #else
    JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_METHODJIT | JSOPTION_METHODJIT_ALWAYS |
        JSOPTION_TYPE_INFERENCE | JSOPTION_ION | JSOPTION_ASMJS | JSOPTION_BASELINE);
    #endif
    JS_SetErrorReporter(cx, reportError);

    if ((gbl = JS_NewGlobalObject(cx, &global_class, NULL)) == NULL ||
        !JS_InitStandardClasses(cx, gbl)) {

        return;
    }

    //js::frontend::ion::js_IonOptions.gvnIsOptimistic = true;
    //JS_SetGCCallback(rt, gccb);
    JS_SetExtraGCRootsTracer(rt, NativeTraceBlack, this);

    /* TODO: HAS_CTYPE in clang */
    JS_InitCTypesClass(cx, gbl);

    JS_SetGlobalObject(cx, gbl);
    JS_DefineFunctions(cx, gbl, glob_funcs);

    this->UI = inUI;
    this->bindNetObject(net);

    /* surface containing the window frame buffer */
    rootHandler = new NativeCanvasHandler(width, height);
    rootHandler->context = new NativeCanvas2DContext(rootHandler, width, height);
    surface = rootHandler->context->skia;

    JS_SetRuntimePrivate(rt, this);
    LoadGlobalObjects(surface, width, height);

    messages = new NativeSharedMessages();

    this->LoadScriptContent(preload_js, strlen(preload_js), "__native.js");
    //this->LoadScriptContent(preload_js);
    
    //animationframeCallbacks = ape_new_pool(sizeof(ape_pool_t), 8);

    streamTest = new NativeStreamTest(net);

}

static bool test_extracting(const char *buf, int len,
    size_t offset, size_t total, void *user)
{
    //NativeJS *njs = (NativeJS *)user;

    printf("Got a packet of size %ld out of %ld\n", offset, total);
    return true;
}

int NativeJS::LoadApplication(const char *path)
{
    if (this->net == NULL) {
        printf("LoadApplication: bind a net object first\n");
        return 0;
    }
    NativeApp *app = new NativeApp("./demo.zip");
    if (app->open()) {
        this->UI->setWindowTitle(app->getTitle());
        app->runWorker(this->net);
        size_t size = app->extractFile("main.js", test_extracting, this);
        if (size == 0) {
            printf("Cant exctract file\n");
        } else {
            printf("size : %ld\n", size);
        }
    }

    return 0;
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
    shutdown = true;

    ape_global *net = (ape_global *)JS_GetContextPrivate(cx);

    JS_BeginRequest(cx);

    JS_RemoveValueRoot(cx, &gfunc);
    /* clear all non protected timers */
    del_timers_unprotected(&net->timersng);
    
    delete rootHandler->context;
    delete rootHandler;
#if 0
    rootHandler->unrootHierarchy();
    
    delete rootHandler;
#endif
    //JS_SetAllNonReservedSlotsToUndefined(cx, JS_GetGlobalObject(cx));

    JS_EndRequest(cx);

    NativeSkia::glcontext = NULL;

    JS_DestroyContext(cx);

    JS_DestroyRuntime(rt);
    JS_ShutDown();

    delete messages;
    hashtbl_free(rootedObj);
}

static int Native_handle_messages(void *arg)
{
#define MAX_MSG_IN_ROW 20

#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)

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

                jsval inval = JSVAL_NULL;

                if (!JS_ReadStructuredClone(cx, ptr->data, ptr->nbytes,
                    JS_STRUCTURED_CLONE_VERSION, &inval, NULL, NULL)) {

                    printf("Failed to read input data (readMessage)\n");

                    continue;
                }

                event = JS_NewObject(cx, &messageEvent_class, NULL, NULL);

                EVENT_PROP("data", inval);

                jevent = OBJECT_TO_JSVAL(event);
                JS_CallFunctionValue(cx, event, onmessage, 1, &jevent, &rval);          

            }
            delete ptr;
            break;
            case NATIVE_THREAD_COMPLETE:
            ptr = static_cast<struct native_thread_msg *>(msg.dataPtr());
            if (JS_GetProperty(cx, ptr->callee, "oncomplete", &onmessage) &&
                !JSVAL_IS_PRIMITIVE(onmessage) && 
                JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onmessage))) {

                jsval inval = JSVAL_NULL;

                if (ptr->nbytes && !JS_ReadStructuredClone(cx,
                        ptr->data, ptr->nbytes,
                        JS_STRUCTURED_CLONE_VERSION, &inval, NULL, NULL)) {

                    continue;

                }

                event = JS_NewObject(cx, &messageEvent_class, NULL, NULL);
                EVENT_PROP("data", inval);
                jevent = OBJECT_TO_JSVAL(event);

                JS_CallFunctionValue(cx, ptr->callee, onmessage, 1, &jevent, &rval);          
            }            
            delete ptr;
            break;
            case NATIVE_AV_THREAD_MESSAGE_CALLBACK: {
                NativeJSAVMessageCallback *cmsg = static_cast<struct NativeJSAVMessageCallback *>(msg.dataPtr());
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
            }
            break;
            default:break;
        }
    }

    return 1;
#undef MAX_MSG_IN_ROW
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
    this->net = net;

    ape_timer *timer = add_timer(&net->timersng, 1,
        Native_handle_messages, this);

    timer->flags &= ~APE_TIMER_IS_PROTECTED;

    //NativeFileIO *io = new NativeFileIO("/tmp/foobar", this, net);
    //io->open();
}

typedef js::Vector<char, 8, js::TempAllocPolicy> FileContents;

static bool
ReadCompleteFile(JSContext *cx, FILE *fp, FileContents &buffer)
{
    /* Get the complete length of the file, if possible. */
    struct stat st;
    int ok = fstat(fileno(fp), &st);
    if (ok != 0)
        return false;
    if (st.st_size > 0) {
        if (!buffer.reserve(st.st_size))
            return false;
    }

    // Read in the whole file. Note that we can't assume the data's length
    // is actually st.st_size, because 1) some files lie about their size
    // (/dev/zero and /dev/random), and 2) reading files in text mode on
    // Windows collapses "\r\n" pairs to single \n characters.
    for (;;) {
        int c = getc(fp);
        if (c == EOF)
            break;
        if (!buffer.append(c))
            return false;
    }

    return true;
}

class NativeAutoFile
{
    FILE *fp_;
  public:
    NativeAutoFile()
      : fp_(NULL)
    {}
    ~NativeAutoFile()
    {
        if (fp_ && fp_ != stdin)
            fclose(fp_);
    }
    FILE *fp() const { return fp_; }
    bool open(JSContext *cx, const char *filename);
    bool readAll(JSContext *cx, FileContents &buffer)
    {
        JS_ASSERT(fp_);
        return ReadCompleteFile(cx, fp_, buffer);
    }
};

/*
 * Open a source file for reading. Supports "-" and NULL to mean stdin. The
 * return value must be fclosed unless it is stdin.
 */
bool
NativeAutoFile::open(JSContext *cx, const char *filename)
{
    if (!filename || strcmp(filename, "-") == 0) {
        fp_ = stdin;
    } else {
        fp_ = fopen(filename, "r");
        if (!fp_) {
            JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSMSG_CANT_OPEN,
                                 filename, "No such file or directory");
            return false;
        }
    }
    return true;
}

static int NativeJS_NativeJSLoadScriptReturn(JSContext *cx,
    const char *filename, jsval *ret)
{   
    JSObject *gbl = JS_GetGlobalObject(cx);

    NativeAutoFile file;
    if (!file.open(cx, filename))
        return 0;
    FileContents buffer(cx);
    if (!ReadCompleteFile(cx, file.fp(), buffer))
        return 0;

    char *func = (char *)malloc(sizeof(char) * buffer.length() + 64);
    memset(func, 0, sizeof(char) * buffer.length() + 64);
    
    strcat(func, "return (");
    strncat(func, buffer.begin(), buffer.length());
    strcat(func, ");");

    JSFunction *cf = JS_CompileFunction(cx, gbl, NULL, 0, NULL, func,
        strlen(func), NULL, 0);

    free(func);
    if (cf == NULL) {
        printf("Cant load script %s\n", filename);
        return 0;
    }

    if (JS_CallFunction(cx, gbl, cf, 0, NULL, ret) == JS_FALSE) {
        printf("Got an error?\n"); /* or thread has ended */

        return 0;
    }

    return 1;
#if 0
    JSScript *script = JS::Compile(cx, rgbl, options,
        buffer.begin(), buffer.length());

    JS_SetOptions(cx, oldopts);

    if (script == NULL || !JS_ExecuteScript(cx, gbl, script, ret)) {
        if (JS_IsExceptionPending(cx)) {
            if (!JS_ReportPendingException(cx)) {
                JS_ClearPendingException(cx);
            }
        }
        return 0;
    }
    return 1;
#endif
}

int NativeJS::LoadScriptContent(const char *data, size_t len,
    const char *filename)
{
    uint32_t oldopts;
    
    JSObject *gbl = JS_GetGlobalObject(cx);
    oldopts = JS_GetOptions(cx);

    JS_SetOptions(cx, oldopts | JSOPTION_COMPILE_N_GO | JSOPTION_NO_SCRIPT_RVAL);
    JS::CompileOptions options(cx);
    options.setUTF8(true)
           .setFileAndLine(filename, 1);

    js::RootedObject rgbl(cx, gbl);
    JSScript *script = JS::Compile(cx, rgbl, options, data, len);

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
    #ifdef NATIVE_AUDIO_ENABLED
    NativeJSAudio::registerObject(cx);
    NativeJSAudioNode::registerObject(cx);
    NativeJSVideo::registerObject(cx);
    #endif
    /* WebGL*() object */
    #ifdef NATIVE_WEBGL_ENABLED
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
    /* console() object */
    NativeJSconsole::registerObject(cx);

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
