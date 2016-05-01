/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "NidiumJS.h"

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

#include "IO/Stream.h"

#include "JSSocket.h"
#include "JSThread.h"
#include "JSHTTP.h"
#include "JSFileIO.h"
#include "JSModules.h"
#include "JSStream.h"
#include "JSWebSocket.h"
#include "JSWebSocketClient.h"
#include "JSHTTPServer.h"
#include "JSDebug.h"
#include "JSConsole.h"
#include "JSUtils.h"
#include "JSFS.h"
#include "JSDebugger.h"

using namespace Nidium::Core;
using namespace Nidium::IO;

namespace Nidium {
namespace Binding {

// {{{ Preamble
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

struct nidium_sm_timer
{
    JSContext *cx;

    JS::PersistentRootedObject global;
    JS::PersistentRootedValue **argv;
    JS::PersistentRootedValue func;

    unsigned argc;
    int ms;

    nidium_sm_timer(JSContext *cx) : cx(cx), global(cx), argv(NULL), func(cx), argc(0), ms(0) { }
    ~nidium_sm_timer() { }
};

enum {
    GLOBAL_PROP___DIRNAME,
    GLOBAL_PROP___FILENAME,
    GLOBAL_PROP_GLOBAL,
    GLOBAL_PROP_WINDOW
};

JSStructuredCloneCallbacks *NidiumJS::m_JsScc = NULL;

JSClass global_class = {
    "global", JSCLASS_GLOBAL_FLAGS_WITH_SLOTS(16) | JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, JS_GlobalObjectTraceHook
};

static bool nidium_global_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t, JS::MutableHandleValue vp);

static bool nidium_cwd(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_load(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_set_immediate(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_set_timeout(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_set_interval(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_clear_timeout(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_btoa(JSContext *cx, unsigned argc, JS::Value *vp);
//static bool nidium_readData(JSContext *cx, unsigned argc, JS::Value *vp);
/*************************/
//static void nidium_timer_wrapper(struct _nidium_sm_timer *params, int *last);
static int nidium_timerng_wrapper(void *arg);

static JSFunctionSpec glob_funcs[] = {
    JS_FN("load", nidium_load, 2, NIDIUM_JS_FNPROPS),
    JS_FN("cwd", nidium_cwd, 0, NIDIUM_JS_FNPROPS),
    JS_FN("setTimeout", nidium_set_timeout, 2, NIDIUM_JS_FNPROPS),
    JS_FN("setImmediate", nidium_set_immediate, 1, NIDIUM_JS_FNPROPS),
    JS_FN("setInterval", nidium_set_interval, 2, NIDIUM_JS_FNPROPS),
    JS_FN("clearTimeout", nidium_clear_timeout, 1, NIDIUM_JS_FNPROPS),
    JS_FN("clearInterval", nidium_clear_timeout, 1, NIDIUM_JS_FNPROPS),
    JS_FN("btoa", nidium_btoa, 1, NIDIUM_JS_FNPROPS),
    JS_FS_END
};

static JSPropertySpec glob_props[] = {
    NIDIUM_JS_PSG("__filename", GLOBAL_PROP___FILENAME, nidium_global_prop_get),
    NIDIUM_JS_PSG("__dirname", GLOBAL_PROP___DIRNAME, nidium_global_prop_get),
    NIDIUM_JS_PSG("global", GLOBAL_PROP_GLOBAL, nidium_global_prop_get),
#ifndef NATIVE_DISABLE_WINDOW_GLOBAL
    NIDIUM_JS_PSG("window", GLOBAL_PROP_WINDOW, nidium_global_prop_get),
#endif
    JS_PS_END
};

// }}}

// {{{ NidiumJS

#if 0
static void gccb(JSRuntime *rt, JSGCStatus status)
{
    //printf("Gc TH1 callback?\n");
}
#endif

#if 1

static void NidiumTraceBlack(JSTracer *trc, void *data)
{
    class NidiumJS *self = static_cast<NidiumJS *>(data);

    if (self->isShuttingDown()) {
        return;
    }

    ape_htable_item_t *item;

    for (item = self->m_RootedObj->first; item != NULL; item = item->lnext) {
        uintptr_t oldaddr = reinterpret_cast<uintptr_t>(item->content.addrs);
        uintptr_t newaddr = oldaddr;

        JS_CallObjectTracer(trc, reinterpret_cast<JSObject **>(&newaddr), "NidiumRoot");

        if (oldaddr != newaddr) {
            printf("Address changed\n");
        }
        //printf("Tracing object at %p\n", item->addrs);
    }
}
#endif

void NidiumJS::gc()
{
    JS_GC(JS_GetRuntime(m_Cx));
}

/* Use obj address as key */
void NidiumJS::rootObjectUntilShutdown(JSObject *obj)
{
    //m_RootedSet->put(obj);
    //JS::AutoHashSetRooter<JSObject *> rooterhash(cx, 0);
    hashtbl_append64(this->m_RootedObj, reinterpret_cast<uint64_t>(obj), obj);
}

void NidiumJS::unrootObject(JSObject *obj)
{
    //m_RootedSet->remove(obj);
    hashtbl_erase64(this->m_RootedObj, reinterpret_cast<uint64_t>(obj));
}

JSObject *NidiumJS::readStructuredCloneOp(JSContext *cx, JSStructuredCloneReader *r,
                                           uint32_t tag, uint32_t data, void *closure)
{
    NidiumJS *js = static_cast<NidiumJS *>(closure);

    switch(tag) {
        case NIDIUM_SCTAG_FUNCTION:
        {
            const char pre[] = "return (";
            const char end[] = ").apply(this, Array.prototype.slice.apply(arguments));";

            char *pdata = static_cast<char *>(malloc(data + 256));
            memcpy(pdata, pre, sizeof(pre));

            if (!JS_ReadBytes(r, pdata + (sizeof(pre) - 1), data)) {
                free(pdata);
                return NULL;
            }

            memcpy(pdata + sizeof(pre) + data - 1, end, sizeof(end));
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
        case NIDIUM_SCTAG_HIDDEN:
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

bool NidiumJS::writeStructuredCloneOp(JSContext *cx, JSStructuredCloneWriter *w,
                                         JS::HandleObject obj, void *closure)
{
    JS::RootedValue vobj(cx, JS::ObjectValue(*obj));
    JSType type = JS_TypeOfValue(cx, vobj);
    NidiumJS *js = static_cast<NidiumJS *>(closure);

    switch(type) {
        /* Serialize function into a string */
        case JSTYPE_FUNCTION:
        {

            JS::RootedFunction fun(cx, JS_ValueToFunction(cx, vobj));
            JS::RootedString func(cx, JS_DecompileFunction(cx,
                fun, 0 | JS_DONT_PRETTY_PRINT));
            JSAutoByteString cfunc(cx, func);
            size_t flen = cfunc.length();

            JS_WriteUint32Pair(w, NIDIUM_SCTAG_FUNCTION, flen);
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

            JS_WriteUint32Pair(w, NIDIUM_SCTAG_HIDDEN, 1);
            JS_WriteBytes(w, &nullbyte, 1);

            break;
        }

    }

    return true;
}


NidiumJS *NidiumJS::GetObject(JSContext *cx)
{
    if (cx == NULL) {
        return static_cast<NidiumJS *>(pthread_getspecific(gJS));
    }
    return static_cast<class NidiumJS *>(JS_GetRuntimePrivate(JS_GetRuntime(cx)));
}

ape_global *NidiumJS::GetNet()
{
    if (gAPE == 0) {
        return NULL;
    }
    return static_cast<ape_global *>(pthread_getspecific(gAPE));
}

void NidiumJS::InitNet(ape_global *net)
{
    if (gAPE == 0) {
        pthread_key_create(&gAPE, NULL);
    }

    pthread_setspecific(gAPE, net);
}

JSObject *NidiumJS::CreateJSGlobal(JSContext *cx)
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

void NidiumJS::SetJSRuntimeOptions(JSRuntime *rt)
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

void NidiumJS::Init()
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

void reportError(JSContext *cx, const char *message, JSErrorReport *report)
{
    NidiumJS *js = NidiumJS::GetObject(cx);
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

        char *tmpwrite = static_cast<char *>(malloc((ctmp - message) + 1));
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

NidiumJS::NidiumJS(ape_global *net) :
    m_JSStrictMode(false), m_Logger(NULL), m_vLogger(NULL), m_LogClear(NULL)
{
    JSRuntime *rt;
    m_Privateslot = NULL;
    m_RelPath = NULL;
    m_Modules = NULL;

    m_StructuredCloneAddition.read = NULL;
    m_StructuredCloneAddition.write = NULL;

    static int isUTF8 = 0;

    /* TODO: BUG */
    if (!isUTF8) {
        //JS_SetCStringsAreUTF8();
        isUTF8 = 1;
    }
    //printf("New JS runtime\n");

    m_Shutdown = false;

    m_Net = NULL;

    m_RootedObj = hashtbl_init(APE_HASH_INT);

    NidiumJS::InitNet(net);

    if (gJS == 0) {
        pthread_key_create(&gJS, NULL);
    }
    pthread_setspecific(gJS, this);

    NidiumJS::Init();

    if ((rt = JS_NewRuntime(128L * 1024L * 1024L,
        JS_NO_HELPER_THREADS)) == NULL) {

        printf("Failed to init JS runtime\n");
        return;
    }

    NidiumJS::SetJSRuntimeOptions(rt);
    JS_SetGCParameterForThread(m_Cx, JSGC_MAX_CODE_CACHE_BYTES, 16 * 1024 * 1024);

    if ((m_Cx = JS_NewContext(rt, 8192)) == NULL) {
        printf("Failed to init JS context\n");
        return;
    }
#if 0
    JS_SetGCZeal(m_Cx, 2, 4);
#endif
    //JS_ScheduleGC(m_Cx, 1);
#if 0
    JS_SetGCCallback(rt, _gc_callback, NULL);
#endif

    JS_BeginRequest(m_Cx);
    JS::RootedObject gbl(m_Cx);
#if 0
    #ifdef NIDIUM_DEBUG
    JS_SetOptions(m_Cx, JSOPTION_VAROBJFIX);
    #else

    JS_SetOptions(m_Cx, JSOPTION_VAROBJFIX | JSOPTION_METHODJIT_ALWAYS |
        JSOPTION_TYPE_INFERENCE | JSOPTION_ION | JSOPTION_ASMJS | JSOPTION_BASELINE);
    #endif
#endif
    JS_SetErrorReporter(m_Cx, reportError);

    gbl = NidiumJS::CreateJSGlobal(m_Cx);

    m_Compartment = JS_EnterCompartment(m_Cx, gbl);
    //JSAutoCompartment ac(m_Cx, gbl);
#if 1
    JS_AddExtraGCRootsTracer(rt, NidiumTraceBlack, this);
#endif
    if (NidiumJS::m_JsScc == NULL) {
        NidiumJS::m_JsScc = new JSStructuredCloneCallbacks();
        NidiumJS::m_JsScc->read = NidiumJS::readStructuredCloneOp;
        NidiumJS::m_JsScc->write = NidiumJS::writeStructuredCloneOp;
        NidiumJS::m_JsScc->reportError = NULL;
    }

    JS_SetStructuredCloneCallbacks(rt, NidiumJS::m_JsScc);

    js::SetDefaultObjectForContext(m_Cx, gbl);

    this->bindNetObject(net);

    JS_SetRuntimePrivate(rt, this);

    m_Messages = new SharedMessages();
    m_RegisteredMessages = static_cast<nidium_thread_message_t*>(calloc(16, sizeof(nidium_thread_message_t)));
    m_RegisteredMessagesIdx = 7; // The 8 first slots (0 to 7) are reserved for Nidium internals messages
    m_RegisteredMessagesSize = 16;

#if 0
    Stream *stream = Stream::create("nvfs:///libs/zip.lib.js");
    char *ret;
    size_t retlen;

    if (stream->getContentSync(&ret, &retlen)) {
        printf("Got the file\n");
        printf("ret : %s\n", ret);
    }

    Stream *mov = Stream::create("/tmp/test");

    char *content;
    size_t len;
    if (!mov->getContentSync(&content, &len, true)) {
        printf("Failed to open file\n");
    }

    NFS *nfs = new NFS((unsigned char *)content, len);


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
    //NidiumJS *njs = static_cast<NidiumJS *>(user);

    printf("Got a packet of size %ld out of %ld\n", offset, total);
    return true;
}


int NidiumJS::LoadApplication(const char *path)
{
    if (m_Net == NULL) {
        printf("LoadApplication: bind a net object first\n");
        return 0;
    }
    NativeApp *app = new NativeApp("./demo.zip");
    if (app->open()) {
        this->UI->setWindowTitle(app->getTitle());
        app->runWorker(m_Net);
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

void NidiumJS::logf(const char *format, ...)
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

void NidiumJS::log(const char *format)
{
    if (!m_Logger) {
        fwrite(format, sizeof(char), strlen(format), stdout);
    } else {
        m_Logger(format);
    }
}

void NidiumJS::logclear()
{
    if (!m_LogClear) {
        return;
    }

    m_LogClear();
}


NidiumJS::~NidiumJS()
{
    JSRuntime *rt;
    rt = JS_GetRuntime(m_Cx);
    m_Shutdown = true;

    ape_global *net = static_cast<ape_global *>(JS_GetContextPrivate(m_Cx));

    /* clear all non protected timers */
    APE_timers_destroy_unprotected(net);

    JS_LeaveCompartment(m_Cx, m_Compartment);
    JS_EndRequest(m_Cx);
    //JS_LeaveCompartment(m_Cx);

    JS_DestroyContext(m_Cx);

    JS_DestroyRuntime(rt);

    delete m_Messages;
    if (m_Modules) {
        delete m_Modules;
    }

    pthread_setspecific(gJS, NULL);

    hashtbl_free(m_RootedObj);
    free(m_RegisteredMessages);
}

static int Nidium_handle_messages(void *arg)
{
#define MAX_MSG_IN_ROW 20
    NidiumJS *njs = static_cast<NidiumJS *>(arg);
    JSContext *cx = njs->m_Cx;
    int nread = 0;

    SharedMessages::Message *msg;
    JSAutoRequest ar(cx);

    while (++nread < MAX_MSG_IN_ROW && (msg = njs->m_Messages->readMessage())) {
        int ev = msg->event();
        if (ev < 0 || ev > njs->m_RegisteredMessagesSize) {
            continue;
        }
        njs->m_RegisteredMessages[ev](cx, msg);
        delete msg;
    }

    return 8;
#undef MAX_MSG_IN_ROW
}

void NidiumJS::bindNetObject(ape_global *net)
{
    JS_SetContextPrivate(m_Cx, net);
    m_Net = net;

    ape_timer_t *timer = APE_timer_create(net, 1,
        Nidium_handle_messages, this);

    APE_timer_unprotect(timer);

    //NidiumFileIO *io = new NidiumFileIO("/tmp/foobar", this, net);
    //io->open();
}

void NidiumJS::CopyProperties(JSContext *cx, JS::HandleObject source, JS::MutableHandleObject into)
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
                    NidiumJS::CopyProperties(cx, newvalobj, &oldvalobj);
                }
                break;
            }
            default:
                JS_SetPropertyById(cx, into, id, val);
                break;
        }
    }
}

int NidiumJS::LoadScriptReturn(JSContext *cx, const char *data,
    size_t len, const char *filename, JS::MutableHandleValue ret)
{
    JS::RootedObject gbl(cx, JS::CurrentGlobalOrNull(cx));

    char *func = static_cast<char *>(malloc(sizeof(char) * (len + 64)));
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

int NidiumJS::LoadScriptReturn(JSContext *cx,
    const char *filename, JS::MutableHandleValue ret)
{
    int err;
    char *data;
    size_t len;

    File file(filename);

    if (!file.openSync("r", &err)) {
        return 0;
    }

    if ((len = file.readSync(file.getFileSize(), &data, &err)) <= 0) {
        return 0;
    }

    int r = NidiumJS::LoadScriptReturn(cx, data, len, filename, ret);

    free(data);

    return r;
}

int NidiumJS::LoadScriptContent(const char *data, size_t len,
    const char *filename)
{
    if (!len) {
        /* silently success */
        return 1;
    }
    /*
        Detect JSBytecode using XDR magic number ad defined in xdr.h
    */
    //TODO: static cast...
    if ((*(uint32_t *)data & 0xFFFFFF00) == 0xb973c000) {
        return this->LoadBytecode((void *)(data), len, filename);
    }

    JS::RootedObject gbl(m_Cx, JS::CurrentGlobalOrNull(m_Cx));

    if (!gbl) {
        fprintf(stderr, "Failed to load global object\n");
        return 0;
    }

    JS::RootedScript script(m_Cx);
    /* RAII helper that resets to origin options state */
    JS::AutoSaveContextOptions asco(m_Cx);

    JS::ContextOptionsRef(m_Cx).setVarObjFix(true).setStrictMode(m_JSStrictMode);

    JS::CompileOptions options(m_Cx);
    options.setUTF8(true)
           .setFileAndLine(filename, 1)
           .setCompileAndGo(true).setNoScriptRval(true);

    script = JS::Compile(m_Cx, gbl, options, data, len);

    if (!script || !JS_ExecuteScript(m_Cx, gbl, script)) {
        if (JS_IsExceptionPending(m_Cx)) {
            if (!JS_ReportPendingException(m_Cx)) {
                JS_ClearPendingException(m_Cx);
            }
        }
        return 0;
    }
    return 1;
}

char *NidiumJS::LoadScriptContentAndGetResult(const char *data, size_t len,
    const char *filename)
{
    if (!len) {
        return NULL;
    }

    JS::RootedObject gbl(m_Cx, JS::CurrentGlobalOrNull(m_Cx));

    if (!gbl) {
        fprintf(stderr, "Failed to load global object\n");
        return NULL;
    }

    JS::RootedScript script(m_Cx);
    /* RAII helper that resets to origin options state */
    JS::AutoSaveContextOptions asco(m_Cx);

    JS::ContextOptionsRef(m_Cx).setVarObjFix(true).setStrictMode(m_JSStrictMode);

    JS::CompileOptions options(m_Cx);
    options.setUTF8(true)
           .setFileAndLine(filename, 1)
           .setCompileAndGo(true).setNoScriptRval(false);

    script = JS::Compile(m_Cx, gbl, options, data, len);

    JS::RootedValue rval(m_Cx);

    if (!script || !JS_ExecuteScript(m_Cx, gbl, script, &rval)) {
        if (JS_IsExceptionPending(m_Cx)) {
            if (!JS_ReportPendingException(m_Cx)) {
                JS_ClearPendingException(m_Cx);
            }
        }
        return NULL;
    }

    JS::RootedString rvalstr(m_Cx, JS::ToString(m_Cx, rval));
    JSAutoByteString cstr;

    cstr.encodeUtf8(m_Cx, rvalstr);

    return strdup(cstr.ptr());
}

int NidiumJS::LoadScript(const char *filename)
{
    int err;
    char *data;
    size_t len;

    File file(filename);

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

int NidiumJS::LoadBytecode(NidiumBytecodeScript *script)
{
    //TODO: new style cast
    return this->LoadBytecode((void *)(script->data), script->size, script->name);
}

int NidiumJS::LoadBytecode(void *data, int size, const char *filename)
{
    JS::RootedObject gbl(m_Cx, JS::CurrentGlobalOrNull(m_Cx));
    JS::RootedScript script(m_Cx, JS_DecodeScript(m_Cx, data, size, NULL));

    if (script == NULL || !JS_ExecuteScript(m_Cx, gbl, script)) {
        if (JS_IsExceptionPending(m_Cx)) {
            if (!JS_ReportPendingException(m_Cx)) {
                JS_ClearPendingException(m_Cx);
            }
        }
        return 0;
    }
    return 1;
}

void NidiumJS::setPath(const char *path) {
    m_RelPath = path;
    if (m_Modules) {
        m_Modules->setPath(path);
    }
}

void NidiumJS::loadGlobalObjects()
{
    JSFileIO::RegisterObject(m_Cx);
    JSSocket::RegisterObject(m_Cx);
    JSThread::RegisterObject(m_Cx);
    JSHTTP::RegisterObject(m_Cx);
    JSStream::RegisterObject(m_Cx);
    JSWebSocketServer::RegisterObject(m_Cx);
    JSWebSocket::RegisterObject(m_Cx);
    JSHTTPServer::RegisterObject(m_Cx);
    JSConsole::RegisterObject(m_Cx);
    JSFS::RegisterObject(m_Cx);
    JSDebug::RegisterObject(m_Cx);
    JSDebugger::RegisterObject(m_Cx);

    m_Modules = new JSModules(m_Cx);
    if (!m_Modules) {
        JS_ReportOutOfMemory(m_Cx);
        return;
    }
    if (!m_Modules->init()) {
        JS_ReportError(m_Cx, "Failed to init require()");
        if (!JS_ReportPendingException(m_Cx)) {
            JS_ClearPendingException(m_Cx);
        }
    }
}

int NidiumJS::registerMessage(nidium_thread_message_t cbk)
{
    if (m_RegisteredMessagesIdx >= m_RegisteredMessagesSize - 1) {
        void *ptr = realloc(m_RegisteredMessages, (m_RegisteredMessagesSize + 16) * sizeof(nidium_thread_message_t));
        if (ptr == NULL) {
            return -1;
        }

        m_RegisteredMessages = static_cast<nidium_thread_message_t *>(ptr);
        m_RegisteredMessagesSize += 16;
    }

    m_RegisteredMessagesIdx++;

    m_RegisteredMessages[m_RegisteredMessagesIdx] = cbk;

    return m_RegisteredMessagesIdx;
}

void NidiumJS::registerMessage(nidium_thread_message_t cbk, int id)
{
    if (id < 0 || id > 7) {
        printf("ERROR : Message id must be between 0 and 7.\n");
        return;
    }

    if (m_RegisteredMessages[id] != NULL) {
        printf("ERROR : Trying to register a shared message at idx %d but slot is already reserved\n", id);
        return;
    }

    m_RegisteredMessages[id] = cbk;
}

void NidiumJS::postMessage(void *dataPtr, int ev)
{
    m_Messages->postMessage(dataPtr, ev);
}
// }}}

// {{{ Implementation
static bool nidium_global_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp)
{
    switch(id) {
        case GLOBAL_PROP___FILENAME:
        {
            char *filename = JSUtils::CurrentJSCaller(cx);
            vp.setString(JS_NewStringCopyZ(cx, filename));
            free(filename);
            break;
        }
        case GLOBAL_PROP___DIRNAME:
        {
            Path path(JSUtils::CurrentJSCaller(cx), false, true);
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

static bool nidium_cwd(JSContext *cx, unsigned argc, JS::Value *vp)
{
    Path cur(JSUtils::CurrentJSCaller(cx), false, true);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (cur.dir() == NULL) {
        args.rval().setUndefined();
        return true;
    }

    JS::RootedString res(cx, JS_NewStringCopyZ(cx, cur.dir()));

    args.rval().setString(res);

    return true;
}

static bool nidium_load(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString script(cx);
    char *content;
    size_t len;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!JS_ConvertArguments(cx, args, "S", script.address())) {
        return false;
    }

    NidiumJS *njs = NidiumJS::GetObject(cx);
    JSAutoByteString scriptstr(cx, script);
    Path scriptpath(scriptstr.ptr());

    Path::schemeInfo *schemeCwd = Path::GetCwdScheme();

    if (scriptpath.path() == NULL) {
        JS_ReportError(cx, "script error : invalid file location");
        return false;
    }

    /* only private are allowed in an http context */
    if (SCHEME_MATCH(schemeCwd, "http") &&
        !URLSCHEME_MATCH(scriptstr.ptr(), "private")) {
        JS_ReportError(cx, "script access error : cannot load in this context");
        return false;
    }

    if (!scriptpath.GetScheme()->AllowSyncStream()) {
        JS_ReportError(cx, "script error : \"%s\" scheme can't load in a sync way", schemeCwd->str);
        return false;
    }

    PtrAutoDelete<Stream *> stream(scriptpath.CreateStream());

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

// {{{ Timers
static int nidium_timer_deleted(void *arg)
{
    struct nidium_sm_timer *params = static_cast<struct nidium_sm_timer *>(arg);

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

static bool nidium_set_immediate(JSContext *cx, unsigned argc, JS::Value *vp)
{
    struct nidium_sm_timer *params;
    int i;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject *obj = JS_THIS_OBJECT(cx, vp);

    params = new nidium_sm_timer(cx);

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

    for (i = 0; i < static_cast<int>(argc) - 1; i++) {
        params->argv[i]->set(args[i + 1]);
    }

    ape_timer_async_t *async = APE_async(static_cast<ape_global *>(JS_GetContextPrivate(cx)),
                nidium_timerng_wrapper, static_cast<void *>(params));

    APE_async_setclearfunc(async, nidium_timer_deleted);

    args.rval().setNull();

    return true;
}

static bool nidium_set_timeout(JSContext *cx, unsigned argc, JS::Value *vp)
{
    struct nidium_sm_timer *params;
    int ms, i;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JSObject *obj = JS_THIS_OBJECT(cx, vp);

    params = new nidium_sm_timer(cx);

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

    for (i = 0; i < static_cast<int>(argc) - 2; i++) {
        params->argv[i]->set(args[i + 2]);
    }

    ape_timer_t *timer = APE_timer_create(static_cast<ape_global *>(JS_GetContextPrivate(cx)),
        nidium_max(ms, 8), nidium_timerng_wrapper,
        static_cast<void *>(params));

    APE_timer_unprotect(timer);
    APE_timer_setclearfunc(timer, nidium_timer_deleted);

    args.rval().setNumber(static_cast<double>(APE_timer_getid(timer)));

    return true;
}

static bool nidium_set_interval(JSContext *cx, unsigned argc, JS::Value *vp)
{
    struct nidium_sm_timer *params;
    int ms, i;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    JSObject *obj = JS_THIS_OBJECT(cx, vp);

    params = new nidium_sm_timer(cx);

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

    params->ms = nidium_max(8, ms);

    for (i = 0; i < static_cast<int>(argc) - 2; i++) {
        params->argv[i]->set(args.array()[i + 2]);
    }

    ape_timer_t *timer = APE_timer_create(static_cast<ape_global *>(JS_GetContextPrivate(cx)),
        params->ms, nidium_timerng_wrapper, static_cast<void *>(params));

    APE_timer_unprotect(timer);
    APE_timer_setclearfunc(timer, nidium_timer_deleted);

    args.rval().setNumber(static_cast<double>(APE_timer_getid(timer)));

    return true;
}

static bool nidium_clear_timeout(JSContext *cx, unsigned argc, JS::Value *vp)
{
    double identifier;

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!JS_ConvertArguments(cx, args, "d", &identifier)) {
        return false;
    }

    APE_timer_clearbyid(static_cast<ape_global *>(JS_GetContextPrivate(cx)),
        static_cast<uint64_t>(identifier), 0);

    return true;
}

static int nidium_timerng_wrapper(void *arg)
{
    struct nidium_sm_timer *params = static_cast<struct nidium_sm_timer *>(arg);

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
// }}}
// {{{ Conversions
static bool nidium_btoa(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_CHECK_ARGS("btoa", 1);

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (args[0].isString()) {

        JSAutoByteString cdata;
        JS::RootedString str(cx, args[0].toString());
        cdata.encodeUtf8(cx, str);

        char *ret = Utils::B64Encode(reinterpret_cast<unsigned char *>(cdata.ptr()), cdata.length());

        args.rval().setString(JS_NewStringCopyZ(cx, ret));

        free(ret);

    } else {
        args.rval().setNull();
        JS_ReportWarning(cx, "btoa() non-string given");
    }

    return true;
}
// }}}
// }}}

} // namespace Binding
} // namespace Nidium

