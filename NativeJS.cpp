/*
    NativeJS Core Library
    Copyright (C) 2013 Anthony Catel <paraboul@gmail.com>
    Copyright (C) 2013 Nicolas Trani <n.trani@weelya.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "NativeJS.h"

#include "NativeSharedMessages.h"

#include "NativeJSSocket.h"
#include "NativeJSThread.h"
#include "NativeJSHttp.h"
#include "NativeJSFileIO.h"
#include "NativeJSModules.h"
#include "NativeJSStream.h"
#include "NativeJSWebSocket.h"
#include "NativeJSHTTPListener.h"
#include "NativeJSDebug.h"
#include "NativeJSConsole.h"
#include "NativeJSFS.h"

#include "NativeUtils.h"
#include "NativeMessages.h"

#include <ape_hash.h>

#include <jsapi.h>
#include <jsfriendapi.h>
#include <jsdbgapi.h>
#include <jsprf.h>

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#include "NativeTaskManager.h"
#include "NativeFile.h"

#include "NativeWebSocket.h"
#include "NativePath.h"

#include <NativeNFS.h>
#include <NativeStreamInterface.h>

static pthread_key_t gAPE = 0;
static pthread_key_t gJS = 0;

/* Assume that we can not use more than 5e5 bytes of C stack by default. */
#if (defined(DEBUG) && defined(__SUNPRO_CC))  || defined(JS_CPU_SPARC)
/* Sun compiler uses larger stack space for js_Interpret() with debug
   Use a bigger gMaxStackSize to make "make check" happy. */
#define DEFAULT_MAX_STACK_SIZE 5000000
#else
#define DEFAULT_MAX_STACK_SIZE 500000
#endif

size_t gMaxStackSize = DEFAULT_MAX_STACK_SIZE;

struct _native_sm_timer
{
    JSContext *cx;
    JSObject *global;
    ape_timer *timerng;
    jsval *argv;
    jsval func;

    unsigned argc;
    int ms;
    int cleared;
    struct _ticks_callback *timer;
};

enum {
    GLOBAL_PROP___DIRNAME,
    GLOBAL_PROP___FILENAME,
    GLOBAL_PROP_GLOBAL,
    GLOBAL_PROP_WINDOW
};

JSStructuredCloneCallbacks *NativeJS::jsscc = NULL;

static JSClass global_class = {
    "global", JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSBool native_global_prop_get(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSMutableHandleValue vp);

/******** Natives ********/
static JSBool native_pwd(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_load(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_set_timeout(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_set_interval(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_clear_timeout(JSContext *cx, unsigned argc, jsval *vp);
//static JSBool native_readData(JSContext *cx, unsigned argc, jsval *vp);
/*************************/
//static void native_timer_wrapper(struct _native_sm_timer *params, int *last);
static int native_timerng_wrapper(void *arg);

static JSFunctionSpec glob_funcs[] = {
    JS_FN("load", native_load, 2, 0),
    JS_FN("pwd", native_pwd, 0, 0),
    JS_FN("setTimeout", native_set_timeout, 2, 0),
    JS_FN("setInterval", native_set_interval, 2, 0),
    JS_FN("clearTimeout", native_clear_timeout, 1, 0),
    JS_FN("clearInterval", native_clear_timeout, 1, 0),
    JS_FS_END
};

static JSPropertySpec glob_props[] = {
    {"__filename", GLOBAL_PROP___FILENAME, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY,
        JSOP_WRAPPER(native_global_prop_get),
        JSOP_NULLWRAPPER},
   {"__dirname", GLOBAL_PROP___DIRNAME, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY,
        JSOP_WRAPPER(native_global_prop_get),
        JSOP_NULLWRAPPER},
   {"global", GLOBAL_PROP_GLOBAL, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY,
        JSOP_WRAPPER(native_global_prop_get),
        JSOP_NULLWRAPPER},
#ifndef NATIVE_DISABLE_WINDOW_GLOBAL
   {"window", GLOBAL_PROP_WINDOW, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY,
        JSOP_WRAPPER(native_global_prop_get),
        JSOP_NULLWRAPPER},
#endif
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSBool native_global_prop_get(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSMutableHandleValue vp)
{
    switch(JSID_TO_INT(id)) {
        case GLOBAL_PROP___FILENAME:
        {
            vp.setString(JS_NewStringCopyZ(cx, NativePath::currentJSCaller()));
            break;
        }
        case GLOBAL_PROP___DIRNAME:
        {
            NativePath path(NativePath::currentJSCaller(), false, true);
            vp.setString(JS_NewStringCopyZ(cx, path.dir()));
            break;
        }
        case GLOBAL_PROP_WINDOW:
        case GLOBAL_PROP_GLOBAL:
        {
            vp.setObjectOrNull(JS_GetGlobalObject(cx));
            break;
        }
        default:
            return false;
    }
    return true;
}

void reportError(JSContext *cx, const char *message, JSErrorReport *report)
{
    NativeJS *js = NativeJS::getNativeClass(cx);

    if (js == NULL) {
        printf("Error reporter failed (wrong JSContext?) (%s:%d > %s)\n", report->filename, report->lineno, message);
        return;
    }
    
    if (!report) {
        js->logf("%s\n", message);
        return;
    }

    char *prefix = NULL;
    if (report->filename)
        prefix = JS_smprintf("%s:", report->filename);
    if (report->lineno) {
        char *tmp = prefix;
        prefix = JS_smprintf("%s%u:%u ", tmp ? tmp : "", report->lineno, report->column);
        JS_free(cx, tmp);
    }
    if (JSREPORT_IS_WARNING(report->flags)) {
        char *tmp = prefix;
        prefix = JS_smprintf("%s%swarning: ",
                             tmp ? tmp : "",
                             JSREPORT_IS_STRICT(report->flags) ? "strict " : "");
        JS_free(cx, tmp);
    }

    /* embedded newlines -- argh! */
    const char *ctmp;
    while ((ctmp = strchr(message, '\n')) != 0) {
        ctmp++;
        if (prefix)
            js->logf("%s", prefix);

        char *tmpwrite = (char *)malloc((ctmp - message)+1);
        memcpy(tmpwrite, message, ctmp - message);
        tmpwrite[ctmp - message] = '\0';
        js->logf("%s", tmpwrite);
        free(tmpwrite);

        message = ctmp;
    }

    /* If there were no filename or lineno, the prefix might be empty */
    if (prefix)
        js->logf("%s", prefix);
    js->logf("%s", message);

    if (report->linebuf) {
        /* report->linebuf usually ends with a newline. */
        int n = strlen(report->linebuf);
        js->logf(":\n%s%s%s%s",
                prefix,
                report->linebuf,
                (n > 0 && report->linebuf[n-1] == '\n') ? "" : "\n",
                prefix);
        n = report->tokenptr - report->linebuf;
        for (int i = 0, j = 0; i < n; i++) {
            if (report->linebuf[i] == '\t') {
                for (int k = (j + 8) & ~7; j < k; j++) {
                    js->logf("%c", '.');
                }
                continue;
            }
            js->logf("%c", '.');
            j++;
        }
        js->logf("%c", '^');
    }
    js->logf("%c", '\n');
    fflush(stdout);
    JS_free(cx, prefix);
}

void NativeJS::logf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    if (m_vLogger == NULL) {
        vprintf(format, args);
    } else {
        m_vLogger(format, args);
    }
    va_end(args);
}

void NativeJS::log(const char *format)
{
    if (!m_Logger) {
        fwrite(format, sizeof(char), strlen(format), stdout);
    } else {
        m_Logger(format);
    }
}

void NativeJS::logclear()
{
    if (!m_LogClear) {
        return;
    }

    m_LogClear();
}

JSObject *NativeJS::readStructuredCloneOp(JSContext *cx, JSStructuredCloneReader *r,
                                           uint32_t tag, uint32_t data, void *closure)
{
    NativeJS *js = (NativeJS *)closure;

    switch(tag) {
        case NATIVE_SCTAG_FUNCTION:
        {
            const char pre[] = "return (";
            const char end[] = ").apply(this, Array.prototype.slice.apply(arguments));";

            char *pdata = (char *)malloc(data + 256);
            memcpy(pdata, pre, sizeof(pre));

            if (!JS_ReadBytes(r, pdata+(sizeof(pre)-1), data)) {
                free(pdata);
                return NULL;
            }

            memcpy(pdata+sizeof(pre)+data-1, end, sizeof(end));
            JSFunction *cf = JS_CompileFunction(cx, JS_GetGlobalObject(cx), NULL, 0, NULL, pdata,
                strlen(pdata), NULL, 0);

            free(pdata);

            if (cf == NULL) {
                /*
                    We don't want "JS_CompileFunction" to repport error
                */
                if (JS_IsExceptionPending(cx)) {
                    JS_ClearPendingException(cx);
                }
                return JS_NewObject(cx, NULL, NULL, NULL);
            }

            return JS_GetFunctionObject(cf);
        }
        case NATIVE_SCTAG_HIDDEN:
        {
            uint8_t nullbyte;
            if (!JS_ReadBytes(r, &nullbyte, data)) {
                return NULL;
            }
            return JS_NewObject(cx, NULL, NULL, NULL);
        }
        default:
        {
            ReadStructuredCloneOp op;
            if (js && (op = js->getReadStructuredCloneAddition())) {
                return op(cx, r, tag, data, closure);
            }
        }
    }

    return JS_NewObject(cx, NULL, NULL, NULL);
}

JSBool NativeJS::writeStructuredCloneOp(JSContext *cx, JSStructuredCloneWriter *w,
                                         JSObject *obj, void *closure)
{
    JS::Value vobj = OBJECT_TO_JSVAL(obj);
    JSType type = JS_TypeOfValue(cx, vobj);
    NativeJS *js = (NativeJS *)closure;

    switch(type) {
        /* Serialize function into a string */
        case JSTYPE_FUNCTION:
        {
            JSString *func = JS_DecompileFunction(cx,
                JS_ValueToFunction(cx, vobj), 0 | JS_DONT_PRETTY_PRINT);
            JSAutoByteString cfunc(cx, func);
            size_t flen = cfunc.length();

            JS_WriteUint32Pair(w, NATIVE_SCTAG_FUNCTION, flen);
            JS_WriteBytes(w, cfunc.ptr(), flen);
            break;
        }
        default:
        {
            if (js && type == JSTYPE_OBJECT) {
                
                WriteStructuredCloneOp op;
                if ((op = js->getWriteStructuredCloneAddition()) &&
                    op(cx, w, obj, closure)) {

                    return true;
                }

            }
            const uint8_t nullbyte = '\0';

            JS_WriteUint32Pair(w, NATIVE_SCTAG_HIDDEN, 1);
            JS_WriteBytes(w, &nullbyte, 1);

            break;
        }

    }

    return true;
}

static JSBool native_pwd(JSContext *cx, unsigned argc, jsval *vp)
{
    NativePath cur(NativePath::currentJSCaller(cx), false, true);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (cur.dir() == NULL) {
        args.rval().setUndefined();
        return true;
    }

    JSString *res = JS_NewStringCopyZ(cx, cur.dir());

    args.rval().setString(res);

    return true;
}

static JSBool native_load(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *script = NULL;
    char *content;
    size_t len;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    
    if (!JS_ConvertArguments(cx, argc, args.array(), "S", &script)) {
        return false;
    }
    
    NativeJS *njs = NativeJS::getNativeClass(cx);
    JSAutoByteString scriptstr(cx, script);
    NativePath scriptpath(scriptstr.ptr());

    NativePath::schemeInfo *schemePwd = NativePath::getPwdScheme();

    if (scriptpath.path() == NULL) {
        JS_ReportError(cx, "script error : invalid file location");
        return false;
    }

    /* only private are allowed in an http context */
    if (SCHEME_MATCH(schemePwd, "http") &&
        !URLSCHEME_MATCH(scriptstr.ptr(), "private")) {
        JS_ReportError(cx, "script access error : cannot load in this context");
        return false;
    }

    if (!scriptpath.getScheme()->allowSyncStream()) {
        JS_ReportError(cx, "script error : \"%s\" scheme can't load in a sync way", schemePwd->str);
        return false;
    }

    NativePtrAutoDelete<NativeBaseStream *> stream(scriptpath.createStream());

    if (!stream.ptr() || !stream.ptr()->getContentSync(&content, &len, true)) {
        JS_ReportError(cx, "load() failed read script");
        return false;        
    }

    if (!njs->LoadScriptContent(content, len, scriptpath.path())) {
        JS_ReportError(cx, "load() failed to load script");
        return false;
    }    

    return true;
}

#if 0
static void gccb(JSRuntime *rt, JSGCStatus status)
{
    //printf("Gc TH1 callback?\n");
}
#endif

#ifdef DEBUG
static void PrintGetTraceName(JSTracer* trc, char *buf, size_t bufsize)
{
    snprintf(buf, bufsize, "[0x%p].mJSVal", trc->debugPrintArg);
}
#endif

static void NativeTraceBlack(JSTracer *trc, void *data)
{
    class NativeJS *self = (class NativeJS *)data;

    if (self->isShuttingDown()) {
        return;
    }
    ape_htable_item_t *item ;

    for (item = self->rootedObj->first; item != NULL; item = item->lnext) {
#ifdef DEBUG
        JS_SET_TRACING_DETAILS(trc, PrintGetTraceName, item, 0);
#endif
        JS_CallObjectTracer(trc, (JSObject *)item->content.addrs, "nativeroot");
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

NativeJS *NativeJS::getNativeClass(JSContext *cx)
{
    if (cx == NULL) {
        return (NativeJS *)pthread_getspecific(gJS);
    }
    return ((class NativeJS *)JS_GetRuntimePrivate(JS_GetRuntime(cx)));
}

ape_global *NativeJS::getNet()
{
    if (gAPE == 0) {
        return NULL;
    }
    return (ape_global *)pthread_getspecific(gAPE);
}

void NativeJS::initNet(ape_global *net)
{
    if (gAPE == 0) {
        pthread_key_create(&gAPE, NULL);
    }

    pthread_setspecific(gAPE, net); 
}

NativeJS::NativeJS(ape_global *net) :
    m_Logger(NULL), m_vLogger(NULL), m_LogClear(NULL)
{
    JSRuntime *rt;
    JSObject *gbl;
    this->privateslot = NULL;
    this->relPath = NULL;

    m_StructuredCloneAddition.read = NULL;
    m_StructuredCloneAddition.write = NULL;

    static int isUTF8 = 0;
    
    /* TODO: BUG */
    if (!isUTF8) {
        //JS_SetCStringsAreUTF8();
        isUTF8 = 1;
    }
    //printf("New JS runtime\n");

    shutdown = false;

    this->net = NULL;

    rootedObj = hashtbl_init(APE_HASH_INT);

    NativeJS::initNet(net);

    if (gJS == 0) {
        pthread_key_create(&gJS, NULL);
    }
    pthread_setspecific(gJS, this);    

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

    if (NativeJS::jsscc == NULL) {
        NativeJS::jsscc = new JSStructuredCloneCallbacks();
        NativeJS::jsscc->read = NativeJS::readStructuredCloneOp;
        NativeJS::jsscc->write = NativeJS::writeStructuredCloneOp;
        NativeJS::jsscc->reportError = NULL;
    }

    JS_SetStructuredCloneCallbacks(rt, NativeJS::jsscc);

    /* TODO: HAS_CTYPE in clang */
    JS_InitCTypesClass(cx, gbl);
    JS_InitReflect(cx, gbl);

    JS_SetGlobalObject(cx, gbl);
    JS_DefineFunctions(cx, gbl, glob_funcs);
    JS_DefineProperties(cx, gbl, glob_props);

    this->bindNetObject(net);

    JS_SetRuntimePrivate(rt, this);

    messages = new NativeSharedMessages();
    registeredMessages = (native_thread_message_t*)calloc(16, sizeof(native_thread_message_t));
    registeredMessagesIdx = 8; // The 8 first slots are reserved for Native internals messages
    registeredMessagesSize = 16;

#if 0
    NativeBaseStream *stream = NativeBaseStream::create("nvfs:///libs/zip.lib.js");
    char *ret;
    size_t retlen;

    if (stream->getContentSync(&ret, &retlen)) {
        printf("Got the file\n");
        printf("ret : %s\n", ret);
    }


    NativeBaseStream *mov = NativeBaseStream::create("/tmp/test");

    char *content;
    size_t len;
    if (!mov->getContentSync(&content, &len, true)) {
        printf("Failed to open file\n");
    }

    NativeNFS *nfs = new NativeNFS((unsigned char *)content, len);


    const char *mp4file = nfs->readFile("/foo/toto", &len);

    printf("Got a file of len %ld\n", len);
    /*printf("Create header 1 %d\n", nfs->mkdir("/hello", strlen("/hello")));
    printf("Create header 1 %d\n", nfs->mkdir("/foo", strlen("/foo")));
    printf("Create header 2 %d\n", nfs->mkdir("/hello/th", strlen("/hello/th")));
    printf("Create header 2 %d\n", nfs->mkdir("/foo/bar", strlen("/foo/bar")));

    printf("Create header 2 %d\n", nfs->writeFile("/foo/toto", strlen("/foo/foo.mp4"), content, len));

    free(content);

    nfs->save("/tmp/test");

    */
#endif

}


#if 0
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
#endif


NativeJS::~NativeJS()
{
    JSRuntime *rt;
    rt = JS_GetRuntime(cx);
    shutdown = true;

    ape_global *net = (ape_global *)JS_GetContextPrivate(cx);

    /* clear all non protected timers */
    del_timers_unprotected(&net->timersng);

    JS_EndRequest(cx);

    JS_DestroyContext(cx);

    JS_DestroyRuntime(rt);
    JS_ShutDown();

    delete messages;
    delete modules;

    pthread_setspecific(gJS, NULL);
    
    hashtbl_free(rootedObj);
    free(registeredMessages);
}

static int Native_handle_messages(void *arg)
{
#define MAX_MSG_IN_ROW 20
    NativeJS *njs = (NativeJS *)arg;
    JSContext *cx = njs->cx;
    int nread = 0;

    NativeSharedMessages::Message *msg;
    JSAutoRequest ar(cx);

    while (++nread < MAX_MSG_IN_ROW && (msg = njs->messages->readMessage())) {
        int ev = msg->event();
        if (ev < 0 || ev > njs->registeredMessagesSize) {
            continue;
        }
        njs->registeredMessages[ev](cx, msg);
        delete msg;
    }

    return 8;
#undef MAX_MSG_IN_ROW
}


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

void NativeJS::copyProperties(JSContext *cx, JSObject *source, JSObject *into)
{

    js::AutoIdArray ida(cx, JS_Enumerate(cx, source));

    for (size_t i = 0; i < ida.length(); i++) {
        js::Rooted<jsid> id(cx, ida[i]);
        jsval val;

        JSString *prop = JSID_TO_STRING(id);
        JSAutoByteString cprop(cx, prop);

        if (!JS_GetPropertyById(cx, source, id, &val)) {
            break;
        }
        /* TODO : has own property */
        switch(JS_TypeOfValue(cx, val)) {
            case JSTYPE_OBJECT:
            {
                jsval oldval;
                if (!JS_GetPropertyById(cx, into, id, &oldval) ||
                    JSVAL_IS_VOID(oldval) || JSVAL_IS_PRIMITIVE(oldval)) {
                    JS_SetPropertyById(cx, into, id, &val);
                } else {
                    NativeJS::copyProperties(cx, JSVAL_TO_OBJECT(val), JSVAL_TO_OBJECT(oldval));
                }
                break;
            }
            default:
                JS_SetPropertyById(cx, into, id, &val);
                break;
        }
    }
}

int NativeJS::LoadScriptReturn(JSContext *cx, const char *data,
    size_t len, const char *filename, JS::Value *ret)
{
    JSObject *gbl = JS_GetGlobalObject(cx);

    char *func = (char *)malloc(sizeof(char) * (len + 64));
    memset(func, 0, sizeof(char) * (len + 64));
    
    strcat(func, "return (");
    strncat(func, data, len);
    strcat(func, ");");

    JSFunction *cf = JS_CompileFunction(cx, gbl, NULL, 0, NULL, func,
        strlen(func), filename, 1);

    free(func);
    if (cf == NULL) {
        printf("Cant load script %s\n", filename);
        return 0;
    }

    if (JS_CallFunction(cx, gbl, cf, 0, NULL, ret) == false) {
        printf("Got an error?\n"); /* or thread has ended */

        return 0;
    }

    return 1;    
}

int NativeJS::LoadScriptReturn(JSContext *cx,
    const char *filename, jsval *ret)
{   
    int err;
    char *data;
    size_t len;

    NativeFile file(filename);

    if (!file.openSync("r", &err)) {
        return 0;
    }

    if ((len = file.readSync(file.getFileSize(), &data, &err)) <= 0) {
        return 0;
    }

    int r = NativeJS::LoadScriptReturn(cx, data, len, filename, ret);

    free(data);

    return r;
}

int NativeJS::LoadScriptContent(const char *data, size_t len,
    const char *filename)
{
    if (!len) {
        /* silently success */
        return 1;
    }
    /*
        Detect JSBytecode using XDR magic number ad defined in xdr.h
    */
    if ((*(uint32_t *)data & 0xFFFFFF00) == 0xb973c000) {
        return this->LoadBytecode((void *)data, len, filename);
    }

    uint32_t oldopts;
    JSObject *gbl = JS_GetGlobalObject(cx);
    oldopts = JS_GetOptions(cx);

    JS_SetOptions(cx, oldopts | JSOPTION_COMPILE_N_GO |
        JSOPTION_NO_SCRIPT_RVAL | JSOPTION_VAROBJFIX);

    JS::CompileOptions options(cx);
    options.setUTF8(true)
           .setFileAndLine(filename, 1);

    js::RootedObject rgbl(cx, gbl);

    JSScript *script = JS::Compile(cx, rgbl, options, data, len);

    JS_SetOptions(cx, oldopts);

    if (script == NULL || !JS_ExecuteScript(cx, rgbl, script, NULL)) {
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
    int err;
    char *data;
    size_t len;

    NativeFile file(filename);

    if (!file.openSync("r", &err)) {
        return 0;
    }

    if ((len = file.readSync(file.getFileSize(), &data, &err)) <= 0) {
        return 0;
    }

    int ret = this->LoadScriptContent(data, len, filename);

    free(data);

    return ret;
}

int NativeJS::LoadBytecode(NativeBytecodeScript *script)
{
    return this->LoadBytecode((void *)script->data, script->size, script->name);
}

int NativeJS::LoadBytecode(void *data, int size, const char *filename)
{
    JSObject *gbl = JS_GetGlobalObject(cx);
    js::RootedObject rgbl(cx, gbl);

    JSScript *script = JS_DecodeScript(cx, data, size, NULL, NULL);

    if (script == NULL || !JS_ExecuteScript(cx, rgbl, script, NULL)) {
        if (JS_IsExceptionPending(cx)) {
            if (!JS_ReportPendingException(cx)) {
                JS_ClearPendingException(cx);
            }
        }
        return 0;
    }
    return 1;
}

void NativeJS::setPath(const char *path) {
    this->relPath = path;
    if (this->modules) {
        this->modules->setPath(path);
    }
}

void NativeJS::loadGlobalObjects()
{
    /* File() object */
    NativeJSFileIO::registerObject(cx);
    /* Socket() object */
    NativeJSSocket::registerObject(cx);
    /* Thread() object */
    NativeJSThread::registerObject(cx);
    /* Http() object */
    NativeJSHttp::registerObject(cx);
    /* Stream() object */
    NativeJSStream::registerObject(cx);
    /* WebSocket*() object */
    NativeJSWebSocketServer::registerObject(cx);
    /* HTTPListener object */
    NativeJSHTTPListener::registerObject(cx);
    /* Debug object */
    NativeJSDebug::registerObject(cx);
    /* console object */
    NativeJSconsole::registerObject(cx);
    /* fs object */
    NativeJSFS::registerObject(cx);

    modules = new NativeJSModules(cx);
    if (!modules) {
        JS_ReportOutOfMemory(cx);
        return;
    }
    if (!modules->init()) {
        JS_ReportError(cx, "Failed to init require()");
        if (!JS_ReportPendingException(cx)) {
            JS_ClearPendingException(cx);
        }
    }
}

void NativeJS::gc()
{
    JS_GC(JS_GetRuntime(cx));
}

int NativeJS::registerMessage(native_thread_message_t cbk)
{
    if (registeredMessagesIdx >= registeredMessagesSize) {
        void *ptr = realloc(registeredMessages, (registeredMessagesSize + 16) * sizeof(native_thread_message_t));
        if (ptr == NULL) {
            return -1;
        }

        registeredMessages = (native_thread_message_t *)ptr;
        registeredMessagesSize += 16;
    }

    registeredMessagesIdx++;

    registeredMessages[registeredMessagesIdx] = cbk;

    return registeredMessagesIdx;
}

void NativeJS::registerMessage(native_thread_message_t cbk, int id)
{
    if (id > 8) {
        printf("ERROR : You can't register a message with idx > 8.\n");
        return;
    }

    if (registeredMessages[id] != NULL) {
        printf("ERROR : Trying to register a shared message at idx %d but slot is already reserved\n", id);
        return;
    }

    registeredMessages[id] = cbk;
}

void NativeJS::postMessage(void *dataPtr, int ev)
{
    this->messages->postMessage(dataPtr, ev);
}

static int native_timer_deleted(void *arg)
{
    struct _native_sm_timer *params = (struct _native_sm_timer *)arg;

    JSAutoRequest ar(params->cx);

    if (params == NULL) {
        return 0;
    }

    JS_RemoveValueRoot(params->cx, &params->func);

    for (int i = 0; i < params->argc; i++) {
        JS_RemoveValueRoot(params->cx, &params->argv[i]);
    }

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

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject *obj = &args.thisv().toObject();

    params = (struct _native_sm_timer *)malloc(sizeof(*params));

    if (params == NULL || argc < 2) {
        if (params) free(params);
        return true;
    }

    params->cx = cx;
    params->global = obj;
    params->argc = argc-2;
    params->cleared = 0;
    params->timer = NULL;
    params->timerng = NULL;
    params->ms = 0;

    params->argv = (argc-2 ? (jsval *)malloc(sizeof(*params->argv) * argc-2) : NULL);

    if (!JS_ConvertValue(cx, args.array()[0], JSTYPE_FUNCTION, &params->func)) {
        free(params->argv);
        free(params);
        return true;
    }

    if (!JS_ConvertArguments(cx, 1, &args.array()[1], "i", &ms)) {
        free(params->argv);
        free(params);
        return false;
    }

    JS_AddValueRoot(cx, &params->func);

    for (i = 0; i < (int)argc-2; i++) {
        params->argv[i] = args.array()[i+2];
        JS_AddValueRoot(cx, &params->argv[i]);
    }

    params->timerng = add_timer(&((ape_global *)JS_GetContextPrivate(cx))->timersng,
        native_max(ms, 8), native_timerng_wrapper,
        (void *)params);

    params->timerng->flags &= ~APE_TIMER_IS_PROTECTED;
    params->timerng->clearfunc = native_timer_deleted;

    args.rval().setNumber((double)params->timerng->identifier);

    return true;
}

static JSBool native_set_interval(JSContext *cx, unsigned argc, jsval *vp)
{
    struct _native_sm_timer *params;
    int ms, i;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject *obj = &args.thisv().toObject();

    params = (struct _native_sm_timer *)malloc(sizeof(*params));

    if (params == NULL || argc < 2) {
        if (params) free(params);
        return true;
    }

    params->cx = cx;
    params->global = obj;
    params->argc = argc-2;
    params->cleared = 0;
    params->timer = NULL;

    params->argv = (argc-2 ? (jsval *)malloc(sizeof(*params->argv) * argc-2) : NULL);

    if (!JS_ConvertValue(cx, args.array()[0], JSTYPE_FUNCTION, &params->func)) {
        free(params->argv);
        free(params);
        return true;
    }

    if (!JS_ConvertArguments(cx, 1, &args.array()[1], "i", &ms)) {
        free(params->argv);
        free(params);
        return false;
    }

    params->ms = native_max(8, ms);

    JS_AddValueRoot(cx, &params->func);

    for (i = 0; i < (int)argc-2; i++) {
        params->argv[i] = args.array()[i+2];
        JS_AddValueRoot(cx, &params->argv[i]);
    }

    params->timerng = add_timer(&((ape_global *)JS_GetContextPrivate(cx))->timersng,
        params->ms, native_timerng_wrapper,
        (void *)params);

    params->timerng->flags &= ~APE_TIMER_IS_PROTECTED;
    params->timerng->clearfunc = native_timer_deleted;

    args.rval().setNumber((double)params->timerng->identifier);

    return true; 
}

static JSBool native_clear_timeout(JSContext *cx, unsigned argc, jsval *vp)
{
    double identifier;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!JS_ConvertArguments(cx, argc, args.array(), "d", &identifier)) {
        return false;
    }

    clear_timer_by_id(&((ape_global *)JS_GetContextPrivate(cx))->timersng,
        (uint64_t)identifier, 0);

    return true;    
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
