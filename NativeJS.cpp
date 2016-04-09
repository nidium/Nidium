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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <glob.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <js/OldDebugAPI.h>
#include <jsprf.h>

#include "NativeJSSocket.h"
#include "NativeJSThread.h"
#include "NativeJSHttp.h"
#include "NativeJSFileIO.h"
#include "NativeJSModules.h"
#include "NativeJSStream.h"
#include "NativeJSWebSocket.h"
#include "NativeJSWebSocketClient.h"
#include "NativeJSHTTPListener.h"
#include "NativeJSDebug.h"
#include "NativeJSConsole.h"
#include "NativeJSFS.h"
#include "NativeStreamInterface.h"
#include "NativeJSDebugger.h"

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

struct native_sm_timer
{
    JSContext *cx;

    
    JS::PersistentRootedObject global;
    JS::PersistentRootedValue **argv;
    JS::PersistentRootedValue func;

    unsigned argc;
    int ms;

    native_sm_timer(JSContext *cx) : global(cx), func(cx) { }
    ~native_sm_timer() { }
};

enum {
    GLOBAL_PROP___DIRNAME,
    GLOBAL_PROP___FILENAME,
    GLOBAL_PROP_GLOBAL,
    GLOBAL_PROP_WINDOW
};

JSStructuredCloneCallbacks *NativeJS::jsscc = NULL;

JSClass global_class = {
    "global", JSCLASS_GLOBAL_FLAGS_WITH_SLOTS(16) | JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, JS_GlobalObjectTraceHook
};

static bool native_global_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t, JS::MutableHandleValue vp);

/******** Natives ********/
static bool native_pwd(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_load(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_set_immediate(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_set_timeout(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_set_interval(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_clear_timeout(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_btoa(JSContext *cx, unsigned argc, JS::Value *vp);
//static bool native_readData(JSContext *cx, unsigned argc, JS::Value *vp);
/*************************/
//static void native_timer_wrapper(struct _native_sm_timer *params, int *last);
static int native_timerng_wrapper(void *arg);

static JSFunctionSpec glob_funcs[] = {
    JS_FN("load", native_load, 2, NATIVE_JS_FNPROPS),
    JS_FN("pwd", native_pwd, 0, NATIVE_JS_FNPROPS),
    JS_FN("setTimeout", native_set_timeout, 2, NATIVE_JS_FNPROPS),
    JS_FN("setImmediate", native_set_immediate, 1, NATIVE_JS_FNPROPS),
    JS_FN("setInterval", native_set_interval, 2, NATIVE_JS_FNPROPS),
    JS_FN("clearTimeout", native_clear_timeout, 1, NATIVE_JS_FNPROPS),
    JS_FN("clearInterval", native_clear_timeout, 1, NATIVE_JS_FNPROPS),
    JS_FN("btoa", native_btoa, 1, NATIVE_JS_FNPROPS),
    JS_FS_END
};

static JSPropertySpec glob_props[] = {

    NATIVE_PSG("__filename", GLOBAL_PROP___FILENAME, native_global_prop_get),
    NATIVE_PSG("__dirname", GLOBAL_PROP___DIRNAME, native_global_prop_get),
    NATIVE_PSG("global", GLOBAL_PROP_GLOBAL, native_global_prop_get),
#ifndef NATIVE_DISABLE_WINDOW_GLOBAL    
    NATIVE_PSG("window", GLOBAL_PROP_WINDOW, native_global_prop_get),
#endif
    JS_PS_END
};

static bool native_global_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp)
{
    switch(id) {
        case GLOBAL_PROP___FILENAME:
        {
            char *filename = NativePath::currentJSCaller(cx);
            vp.setString(JS_NewStringCopyZ(cx, filename));
            free(filename);
            break;
        }
        case GLOBAL_PROP___DIRNAME:
        {
            NativePath path(NativePath::currentJSCaller(cx), false, true);
            vp.setString(JS_NewStringCopyZ(cx, path.dir()));
            break;
        }
        case GLOBAL_PROP_WINDOW:
        case GLOBAL_PROP_GLOBAL:
        {
            JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
            vp.setObjectOrNull(global);
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
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));

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

            memcpy(pdata+sizeof(pre) + data-1, end, sizeof(end));
            JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
            JS::CompileOptions options(cx);
            options.setUTF8(true);

            JS::RootedFunction cf(cx);

            cf = JS::CompileFunction(cx, global, options, NULL, 0, NULL, pdata, strlen(pdata));

            free(pdata);

            if (cf == NULL) {
                /*
                    We don't want "JS_CompileFunction" to repport error
                */
                if (JS_IsExceptionPending(cx)) {
                    JS_ClearPendingException(cx);
                }
                return JS_NewObject(cx, NULL, JS::NullPtr(), JS::NullPtr());
            }

            return JS_GetFunctionObject(cf);
        }
        case NATIVE_SCTAG_HIDDEN:
        {
            uint8_t nullbyte;
            if (!JS_ReadBytes(r, &nullbyte, data)) {
                return NULL;
            }
            return JS_NewObject(cx, NULL, JS::NullPtr(), JS::NullPtr());
        }
        default:
        {
            ReadStructuredCloneOp op;
            if (js && (op = js->getReadStructuredCloneAddition())) {
                return op(cx, r, tag, data, closure);
            }
        }
    }

    return JS_NewObject(cx, NULL, JS::NullPtr(), JS::NullPtr());
}

bool NativeJS::writeStructuredCloneOp(JSContext *cx, JSStructuredCloneWriter *w,
                                         JS::HandleObject obj, void *closure)
{
    JS::RootedValue vobj(cx, JS::ObjectValue(*obj));
    JSType type = JS_TypeOfValue(cx, vobj);
    NativeJS *js = (NativeJS *)closure;

    switch(type) {
        /* Serialize function into a string */
        case JSTYPE_FUNCTION:
        {

            JS::RootedFunction fun(cx, JS_ValueToFunction(cx, vobj));
            JS::RootedString func(cx, JS_DecompileFunction(cx,
                fun, 0 | JS_DONT_PRETTY_PRINT));
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

static bool native_pwd(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NativePath cur(NativePath::currentJSCaller(cx), false, true);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (cur.dir() == NULL) {
        args.rval().setUndefined();
        return true;
    }

    JS::RootedString res(cx, JS_NewStringCopyZ(cx, cur.dir()));

    args.rval().setString(res);

    return true;
}

static bool native_load(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString script(cx);
    char *content;
    size_t len;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!JS_ConvertArguments(cx, args, "S", script.address())) {
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

#if 1

static void NativeTraceBlack(JSTracer *trc, void *data)
{
    class NativeJS *self = (class NativeJS *)data;

    if (self->isShuttingDown()) {
        return;
    }

    ape_htable_item_t *item;

    for (item = self->rootedObj->first; item != NULL; item = item->lnext) {
        uintptr_t oldaddr = (uintptr_t)item->content.addrs;
        uintptr_t newaddr = oldaddr;

        JS_CallObjectTracer(trc, (JSObject **)&newaddr, "nativeroot");

        if (oldaddr != newaddr) {
            printf("Address changed\n");
        }
        //printf("Tracing object at %p\n", item->addrs);
    }
}
#endif

/* Use obj address as key */
void NativeJS::rootObjectUntilShutdown(JSObject *obj)
{
    //m_RootedSet->put(obj);
    //JS::AutoHashSetRooter<JSObject *> rooterhash(cx, 0);
    hashtbl_append64(this->rootedObj, (uint64_t)obj, obj);
}

void NativeJS::unrootObject(JSObject *obj)
{
    //m_RootedSet->remove(obj);
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

JSObject *NativeJS::CreateJSGlobal(JSContext *cx)
{
    JS::CompartmentOptions options;
    options.setVersion(JSVERSION_LATEST);

    JS::RootedObject glob(cx, JS_NewGlobalObject(cx, &global_class, nullptr,
                                             JS::DontFireOnNewGlobalHook, options));

    JSAutoCompartment ac(cx, glob);

    JS_InitStandardClasses(cx, glob);
    JS_InitCTypesClass(cx, glob);

    JS_DefineFunctions(cx, glob, glob_funcs);
    JS_DefineProperties(cx, glob, glob_props);

    JS_FireOnNewGlobalObject(cx, glob);

    return glob;
    //JS::RegisterPerfMeasurement(cx, glob);

    //https://bugzilla.mozilla.org/show_bug.cgi?id=880330
    // context option vs compile option?
}

void NativeJS::SetJSRuntimeOptions(JSRuntime *rt)
{
    JS::RuntimeOptionsRef(rt).setBaseline(true)
                             .setIon(true)
                             .setAsmJS(true);

    JS_SetGCParameter(rt, JSGC_MAX_BYTES, 0xffffffff);
    JS_SetGCParameter(rt, JSGC_SLICE_TIME_BUDGET, 15);
    JS_SetNativeStackQuota(rt, gMaxStackSize);

    //rt->profilingScripts For profiling?
}

#if 0
static void _gc_callback(JSRuntime *rt, JSGCStatus status, void *data)
{
    printf("Got gcd\n");
}
#endif

void NativeJS::Init()
{
    static bool _alreadyInit = false;
    if (!_alreadyInit) {
        if (!JS_Init()) {
            fprintf(stderr, "Failed to init JSAPI (JS_Init())\n");
            return;
        }
        _alreadyInit = true;
    }
}

NativeJS::NativeJS(ape_global *net) :
    m_JSStrictMode(false), m_Logger(NULL), m_vLogger(NULL), m_LogClear(NULL)
{
    JSRuntime *rt;
    this->privateslot = NULL;
    this->relPath = NULL;
    this->modules = NULL;

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

    NativeJS::Init();

    if ((rt = JS_NewRuntime(128L * 1024L * 1024L,
        JS_NO_HELPER_THREADS)) == NULL) {

        printf("Failed to init JS runtime\n");
        return;
    }

    NativeJS::SetJSRuntimeOptions(rt);
    JS_SetGCParameterForThread(cx, JSGC_MAX_CODE_CACHE_BYTES, 16 * 1024 * 1024);

    if ((cx = JS_NewContext(rt, 8192)) == NULL) {
        printf("Failed to init JS context\n");
        return;
    }
#if 0
    JS_SetGCZeal(cx, 2, 4);
#endif
    //JS_ScheduleGC(cx, 1);
#if 0
    JS_SetGCCallback(rt, _gc_callback, NULL);
#endif

    JS_BeginRequest(cx);
    JS::RootedObject gbl(cx);
#if 0
    #ifdef NATIVE_DEBUG
    JS_SetOptions(cx, JSOPTION_VAROBJFIX);
    #else

    JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_METHODJIT_ALWAYS |
        JSOPTION_TYPE_INFERENCE | JSOPTION_ION | JSOPTION_ASMJS | JSOPTION_BASELINE);
    #endif
#endif
    JS_SetErrorReporter(cx, reportError);

    gbl = NativeJS::CreateJSGlobal(cx);

    m_Compartment = JS_EnterCompartment(cx, gbl);
    //JSAutoCompartment ac(cx, gbl);
#if 1
    JS_AddExtraGCRootsTracer(rt, NativeTraceBlack, this);
#endif
    if (NativeJS::jsscc == NULL) {
        NativeJS::jsscc = new JSStructuredCloneCallbacks();
        NativeJS::jsscc->read = NativeJS::readStructuredCloneOp;
        NativeJS::jsscc->write = NativeJS::writeStructuredCloneOp;
        NativeJS::jsscc->reportError = NULL;
    }

    JS_SetStructuredCloneCallbacks(rt, NativeJS::jsscc);

    js::SetDefaultObjectForContext(cx, gbl);

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

    JS_LeaveCompartment(cx, m_Compartment);
    JS_EndRequest(cx);
    //JS_LeaveCompartment(cx);

    JS_DestroyContext(cx);

    JS_DestroyRuntime(rt);

    delete messages;
    if (this->modules) {
        delete this->modules;
    }

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

void NativeJS::copyProperties(JSContext *cx, JS::HandleObject source, JS::MutableHandleObject into)
{

    JS::AutoIdArray ida(cx, JS_Enumerate(cx, source));

    for (size_t i = 0; i < ida.length(); i++) {
        JS::RootedId id(cx, ida[i]);

        JS::RootedValue val(cx);
        JS::RootedString prop(cx, JSID_TO_STRING(id));
        JSAutoByteString cprop(cx, prop);

        if (!JS_GetPropertyById(cx, source, id, &val)) {
            break;
        }
        /* TODO : has own property */
        switch(JS_TypeOfValue(cx, val)) {
            case JSTYPE_OBJECT:
            {
                JS::RootedValue oldval(cx);

                if (!JS_GetPropertyById(cx, into, id, &oldval) ||
                    oldval.isNullOrUndefined() || oldval.isPrimitive()) {
                    JS_SetPropertyById(cx, into, id, val);
                } else {
                    JS::RootedObject oldvalobj(cx, &oldval.toObject());
                    JS::RootedObject newvalobj(cx, &val.toObject());
                    NativeJS::copyProperties(cx, newvalobj, &oldvalobj);
                }
                break;
            }
            default:
                JS_SetPropertyById(cx, into, id, val);
                break;
        }
    }
}

int NativeJS::LoadScriptReturn(JSContext *cx, const char *data,
    size_t len, const char *filename, JS::MutableHandleValue ret)
{
    JS::RootedObject gbl(cx, JS::CurrentGlobalOrNull(cx));

    char *func = (char *)malloc(sizeof(char) * (len + 64));
    memset(func, 0, sizeof(char) * (len + 64));

    strcat(func, "return (");
    strncat(func, data, len);
    strcat(func, ");");

    JS::RootedFunction cf(cx);

    JS::CompileOptions options(cx);
    options.setFileAndLine(filename, 1)
           .setUTF8(true);

    cf = JS::CompileFunction(cx, gbl, options, NULL, 0, NULL, func, strlen(func));

    free(func);
    if (cf == NULL) {
        printf("Cant load script %s\n", filename);
        return 0;
    }

    if (JS_CallFunction(cx, gbl, cf, JS::HandleValueArray::empty(), ret) == false) {
        printf("Got an error?\n"); /* or thread has ended */

        return 0;
    }

    return 1;
}

int NativeJS::LoadScriptReturn(JSContext *cx,
    const char *filename, JS::MutableHandleValue ret)
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

    JS::RootedObject gbl(cx, JS::CurrentGlobalOrNull(cx));

    if (!gbl) {
        fprintf(stderr, "Failed to load global object\n");
        return 0;
    }

    JS::RootedScript script(cx);
    /* RAII helper that resets to origin options state */
    JS::AutoSaveContextOptions asco(cx);

    JS::ContextOptionsRef(cx).setVarObjFix(true).setStrictMode(m_JSStrictMode);

    JS::CompileOptions options(cx);
    options.setUTF8(true)
           .setFileAndLine(filename, 1)
           .setCompileAndGo(true).setNoScriptRval(true);

    script = JS::Compile(cx, gbl, options, data, len);

    if (!script || !JS_ExecuteScript(cx, gbl, script)) {
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
    JS::RootedObject gbl(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedScript script(cx, JS_DecodeScript(cx, data, size, NULL));

    if (script == NULL || !JS_ExecuteScript(cx, gbl, script)) {
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
    NativeJSWebSocket::registerObject(cx);
    /* HTTPListener object */
    NativeJSHTTPListener::registerObject(cx);
    /* Debug object */
    NativeJSDebug::registerObject(cx);
    /* console object */
    NativeJSconsole::registerObject(cx);
    /* fs object */
    NativeJSFS::registerObject(cx);
    /* Debugger object */
    NativeJSDebugger::registerObject(cx);

    this->modules = new NativeJSModules(cx);
    if (!this->modules) {
        JS_ReportOutOfMemory(cx);
        return;
    }
    if (!this->modules->init()) {
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
    struct native_sm_timer *params = (struct native_sm_timer *)arg;

    if (params == NULL) {
        return 0;
    }

    JSAutoRequest ar(params->cx);
    for (int i = 0; i < params->argc; i++) {
        delete params->argv[i];
    }
    delete[] params->argv;
    delete params;

    return 1;
}

static bool native_set_immediate(JSContext *cx, unsigned argc, JS::Value *vp)
{
    struct native_sm_timer *params;
    int i;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject *obj = JS_THIS_OBJECT(cx, vp);

    params = new native_sm_timer(cx);

    if (params == NULL || argc < 1) {
        if (params) delete params;
        return true;
    }

    params->cx = cx;
    params->global = obj;
    params->argc = argc-1;
    params->ms = 0;

    params->argv = new JS::PersistentRootedValue*[argc-1];

    for (i = 0; i < argc-1; i++) {
        params->argv[i] = new JS::PersistentRootedValue(cx);
    }

    JS::RootedValue func(cx);

    if (!JS_ConvertValue(cx, args[0], JSTYPE_FUNCTION, &func)) {
        delete[] params->argv;
        delete params;
        return true;
    }

    params->func.set(func);

    for (i = 0; i < (int)argc-1; i++) {
        params->argv[i]->set(args[i+1]);
    }

    ape_async *async = add_async(&((ape_global *)JS_GetContextPrivate(cx))->timersng,
                native_timerng_wrapper, (void *)params);

    async->clearfunc = native_timer_deleted;

    args.rval().setNull();

    return true;
}

static bool native_set_timeout(JSContext *cx, unsigned argc, JS::Value *vp)
{
    struct native_sm_timer *params;
    int ms, i;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject *obj = JS_THIS_OBJECT(cx, vp);

    params = new native_sm_timer(cx);

    if (params == NULL || argc < 2) {
        if (params) delete params;
        return true;
    }

    params->cx = cx;
    params->global = obj;
    params->argc = argc-2;
    params->ms = 0;

    params->argv = new JS::PersistentRootedValue*[argc-2];

    for (i = 0; i < argc-2; i++) {
        params->argv[i] = new JS::PersistentRootedValue(cx);
    }

    JS::RootedValue func(cx);

    if (!JS_ConvertValue(cx, args[0], JSTYPE_FUNCTION, &func)) {
        delete[] params->argv;
        delete params;
        return true;
    }

    if (!JS::ToInt32(cx, args[1], &ms)) {
        free(params->argv);
        delete params;
        return false;
    }

    params->func.set(func);

    for (i = 0; i < (int)argc-2; i++) {
        params->argv[i]->set(args[i+2]);
    }

    ape_timer *timer = add_timer(&((ape_global *)JS_GetContextPrivate(cx))->timersng,
        native_max(ms, 8), native_timerng_wrapper,
        (void *)params);

    timer->flags &= ~APE_TIMER_IS_PROTECTED;
    timer->clearfunc = native_timer_deleted;

    args.rval().setNumber((double)timer->identifier);

    return true;
}

static bool native_set_interval(JSContext *cx, unsigned argc, JS::Value *vp)
{
    struct native_sm_timer *params;
    int ms, i;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JSObject *obj = JS_THIS_OBJECT(cx, vp);

    params = new native_sm_timer(cx);

    if (params == NULL || argc < 2) {
        if (params) delete params;
        return true;
    }

    params->cx = cx;
    params->global = obj;
    params->argc = argc-2;

    params->argv = new JS::PersistentRootedValue*[argc-2];

    for (i = 0; i < argc-2; i++) {
        params->argv[i] = new JS::PersistentRootedValue(cx);
    }

    JS::RootedValue func(cx);
    if (!JS_ConvertValue(cx, args[0], JSTYPE_FUNCTION, &func)) {
        delete[] params->argv;
        delete params;
        return true;
    }

    params->func.set(func);

    if (!JS::ToInt32(cx, args[1], &ms)) {
        delete[] params->argv;
        delete params;
        return false;
    }

    params->ms = native_max(8, ms);

    for (i = 0; i < (int)argc-2; i++) {
        params->argv[i]->set(args.array()[i+2]);
    }

    ape_timer *timer = add_timer(&((ape_global *)JS_GetContextPrivate(cx))->timersng,
        params->ms, native_timerng_wrapper,
        (void *)params);

    timer->flags &= ~APE_TIMER_IS_PROTECTED;
    timer->clearfunc = native_timer_deleted;

    args.rval().setNumber((double)timer->identifier);

    return true;
}

static bool native_clear_timeout(JSContext *cx, unsigned argc, JS::Value *vp)
{
    double identifier;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!JS_ConvertArguments(cx, args, "d", &identifier)) {
        return false;
    }

    clear_timer_by_id(&((ape_global *)JS_GetContextPrivate(cx))->timersng,
        (uint64_t)identifier, 0);

    return true;
}

static bool native_btoa(JSContext *cx, unsigned argc, JS::Value *vp)
{
    double identifier;

    NATIVE_CHECK_ARGS("btoa", 1);

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (args[0].isString()) {

        JSAutoByteString cdata;
        JS::RootedString str(cx, args[0].toString());
        cdata.encodeUtf8(cx, str);

        char *ret = NativeUtils::b64Encode((unsigned char *)cdata.ptr(), cdata.length());

        args.rval().setString(JS_NewStringCopyZ(cx, ret));

        free(ret);

    } else {
        args.rval().setNull();
        JS_ReportWarning(cx, "btoa() non-string given");
    }

    return true;
}

static int native_timerng_wrapper(void *arg)
{
    struct native_sm_timer *params = (struct native_sm_timer *)arg;

    JSAutoRequest       ar(params->cx);
    JS::RootedValue     rval(params->cx);
    JS::AutoValueVector arr(params->cx);
    JS::RootedValue     func(params->cx, params->func);
    JS::RootedObject    global(params->cx, params->global);

    arr.resize(params->argc);
    for(size_t i = 0; i< params->argc; i++) {
        arr[i] = params->argv[i]->get();
    }
    JS_CallFunctionValue(params->cx, global, func, arr, &rval);

    //timers_stats_print(&((ape_global *)JS_GetContextPrivate(params->cx))->timersng);

    return params->ms;
}

