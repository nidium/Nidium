/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/NidiumJS.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>


#include <jsprf.h>

#include <js/TracingAPI.h>
#include <js/Initialization.h>

#include "Binding/JSSocket.h"
#include "Binding/JSThread.h"
#include "Binding/JSHTTP.h"
#include "Binding/JSFile.h"
#include "Binding/JSModules.h"
#include "Binding/JSStream.h"
#include "Binding/JSWebSocket.h"
#include "Binding/JSWebSocketClient.h"
#include "Binding/JSHTTPServer.h"
#include "Binding/JSDebug.h"
#include "Binding/JSConsole.h"
#include "Binding/JSFS.h"
#include "Binding/JSDebugger.h"
#include "Binding/JSGlobal.h"
#include "Binding/JSDB.h"
#include "Binding/JSOS.h"
#include "Binding/JSVM.h"

#include "Binding/ThreadLocalContext.h"

#ifndef NIDIUM_DISABLE_WINDOW_GLOBAL
  #include "Binding/JSWindow.h"
#endif

#include "Core/Context.h"

#if defined(MOZ_ASAN) || (defined(DEBUG) && !defined(XP_WIN))
static const size_t gMaxStackSize = 2 * 128 * sizeof(size_t) * 1024;
#else
static const size_t gMaxStackSize = 128 * sizeof(size_t) * 1024;
#endif

using namespace Nidium::Core;
using namespace Nidium::IO;

extern class Nidium::Binding::NidiumJS *g_nidiumjs;

namespace Nidium {
namespace Binding {

// {{{ Preamble

static pthread_key_t gAPE;
static pthread_key_t gJS;

JSStructuredCloneCallbacks *NidiumJS::m_JsScc = NULL;


void NidiumJS::gc()
{
    JS_GC(JS_GetRuntime(m_Cx));
}


JSObject *NidiumJS::readStructuredCloneOp(JSContext *cx,
                                          JSStructuredCloneReader *r,
                                          uint32_t tag,
                                          uint32_t data,
                                          void *closure)
{
    NidiumJS *js = static_cast<NidiumJS *>(closure);

    switch (tag) {
        case kSctag_Function: {
            const char pre[] = "return (";
            const char end[]
                = ").apply(this, Array.prototype.slice.apply(arguments));";

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

            JS::AutoObjectVector scopeChain(cx);

            bool cret = JS::CompileFunction(cx, scopeChain, options,
                            NULL, 0, NULL, pdata,
                            strlen(pdata), &cf);

            free(pdata);

            if (!cret) {
                /*
                    We don't want "JS_CompileFunction" to repport error
                */
                if (JS_IsExceptionPending(cx)) {
                    JS_ClearPendingException(cx);
                }
                return JS_NewPlainObject(cx);
            }

            return JS_GetFunctionObject(cf);
        }
        case kSctag_Hidden: {
            uint8_t nullbyte;
            if (!JS_ReadBytes(r, &nullbyte, data)) {
                return NULL;
            }
            return JS_NewPlainObject(cx);
        }
        default: {
            ReadStructuredCloneOp op;
            if (js && (op = js->getReadStructuredCloneAddition())) {
                return op(cx, r, tag, data, closure);
            }
        }
    }

    return JS_NewPlainObject(cx);
}

bool NidiumJS::writeStructuredCloneOp(JSContext *cx,
                                      JSStructuredCloneWriter *w,
                                      JS::HandleObject obj,
                                      void *closure)
{
    JS::RootedValue vobj(cx, JS::ObjectValue(*obj));
    JSType type  = JS_TypeOfValue(cx, vobj);
    NidiumJS *js = static_cast<NidiumJS *>(closure);

    switch (type) {
        /* Serialize function into a string */
        case JSTYPE_FUNCTION: {

            JS::RootedFunction fun(cx, JS_ValueToFunction(cx, vobj));
            JS::RootedString func(
                cx, JS_DecompileFunction(cx, fun, 0 | JS_DONT_PRETTY_PRINT));
            JSAutoByteString cfunc(cx, func);
            size_t flen = cfunc.length();

            JS_WriteUint32Pair(w, kSctag_Function, flen);
            JS_WriteBytes(w, cfunc.ptr(), flen);
            break;
        }
        default: {
            if (js && type == JSTYPE_OBJECT) {

                WriteStructuredCloneOp op;
                if ((op = js->getWriteStructuredCloneAddition())
                    && op(cx, w, obj, closure)) {

                    return true;
                }
            }
            const uint8_t nullbyte = '\0';

            JS_WriteUint32Pair(w, kSctag_Hidden, 1);
            JS_WriteBytes(w, &nullbyte, 1);

            break;
        }
    }

    return true;
}


NidiumJS *NidiumJS::GetObject(JSContext *cx)
{
    if (cx == NULL) {
        return g_nidiumjs;
    }

    return static_cast<class NidiumJS *>(
        JS_GetRuntimePrivate(JS_GetRuntime(cx)));
}

ape_global *NidiumJS::GetNet()
{
    return static_cast<ape_global *>(pthread_getspecific(gAPE));
}

void NidiumJS::InitNet(ape_global *net)
{
    if (pthread_key_create(&gAPE, NULL) != 0) {
        APE_ERROR("Binding", "[NidiumJS] pthread_key_create() error\n");
        exit(1);
        return;
    }

    pthread_setspecific(gAPE, net);
}

JSObject *NidiumJS::CreateJSGlobal(JSContext *cx, NidiumJS *njs)
{
    JS::CompartmentOptions options;
    options.setVersion(JSVERSION_LATEST);
    JSClass *jsglobal;

#ifndef NIDIUM_DISABLE_WINDOW_GLOBAL
    jsglobal = JSWindow::GetJSClass();
#else
    jsglobal = JSGlobal::GetJSClass();
#endif

    JS::RootedObject glob(cx,
        JS_NewGlobalObject(cx, jsglobal, nullptr,
                                                 JS::DontFireOnNewGlobalHook,
                                                 options));

    JSAutoCompartment ac(cx, glob);

    JS_InitStandardClasses(cx, glob);
    JS_InitCTypesClass(cx, glob);

    JSGlobal::RegisterObject(cx, glob, njs);

    JS_FireOnNewGlobalObject(cx, glob);

    return glob;
    // JS::RegisterPerfMeasurement(cx, glob);

    // https://bugzilla.mozilla.org/show_bug.cgi?id=880330
    // context option vs compile option?
}

void NidiumJS::SetJSRuntimeOptions(JSRuntime *rt, bool strictmode)
{
    JS::RuntimeOptionsRef(rt).setBaseline(true).setIon(true).setAsmJS(true).setStrictMode(strictmode);

    JS_SetGCParameter(rt, JSGC_MAX_BYTES, 0xffffffff);
    JS_SetGCParameter(rt, JSGC_SLICE_TIME_BUDGET, 15);
    JS_SetNativeStackQuota(rt, gMaxStackSize);

    // rt->profilingScripts For profiling?
}

#if 0
static void _gc_callback(JSRuntime *rt, JSGCStatus status, void *data)
{
    APE_DEBUG("Binding", "[NidiumJS] Got gcd\n");
}
#endif

void NidiumJS::Init()
{
    static bool _alreadyInit = false;
    if (!_alreadyInit) {
        if (!JS_Init()) {
            APE_ERROR("Binding", "[Nidium] Failed to init JSAPI (JS_Init())\n");
            return;
        }
        _alreadyInit = true;
    }
}

void reportError(JSContext *cx, const char *message, JSErrorReport *report)
{
    NidiumJS *js  = NidiumJS::GetObject(cx);
    Context *nctx = nullptr;
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));

    if (js == nullptr) {
        APE_ERROR("Binding", "[NidiumJS] Error reporter failed (wrong JSContext?) (%s:%d > %s)\n",
               report->filename, report->lineno, message);
        return;
    }

    nctx = js->getContext();

    if (!report) {
        nctx->vlog("%s\n", message);
        return;
    }

    /*
        Enable log buffering
    */
    AutoContextLogBuffering aclb(nctx);

    char *prefix = NULL;
    if (report->filename) {
        prefix = JS_smprintf("%s:", report->filename);
    }

    if (report->lineno) {
        char *tmp = prefix;
        prefix = JS_smprintf("%s%u:%u ", tmp ? tmp : "", report->lineno,
                             report->column);
        JS_free(cx, tmp);
    }

    if (JSREPORT_IS_WARNING(report->flags)) {
        char *tmp = prefix;
        prefix
            = JS_smprintf("%s%swarning: ", tmp ? tmp : "",
                          JSREPORT_IS_STRICT(report->flags) ? "strict " : "");
        JS_free(cx, tmp);
    }

    /* embedded newlines -- argh! */
    const char *ctmp;
    while ((ctmp = strchr(message, '\n')) != 0) {
        ctmp++;
        if (prefix) {
            nctx->vlog("%s", prefix);
        }

        char *tmpwrite = static_cast<char *>(malloc((ctmp - message) + 1));
        memcpy(tmpwrite, message, ctmp - message);
        tmpwrite[ctmp - message] = '\0';
        nctx->vlog("%s", tmpwrite);
        free(tmpwrite);

        message = ctmp;
    }

    /* If there were no filename or lineno, the prefix might be empty */
    if (prefix) {
        nctx->vlog("%s", prefix);
    }
    nctx->vlog("%s", message);

    if (const char16_t* linebuf = report->linebuf()) {
        size_t n = report->linebufLength();

        nctx->log(":\n");
        if (prefix)
            nctx->log(prefix);

        for (size_t i = 0; i < n; i++)
            nctx->vlog("%c", static_cast<char>(linebuf[i]));

        // linebuf usually ends with a newline. If not, add one here.
        if (n == 0 || linebuf[n-1] != '\n')
            nctx->log("\n");

        if (prefix)
            nctx->log(prefix);

        n = report->tokenOffset();
        for (size_t i = 0, j = 0; i < n; i++) {
            if (linebuf[i] == '\t') {
                for (size_t k = (j + 8) & ~7; j < k; j++)
                    nctx->log(".");
                continue;
            }
            nctx->log(".");
            j++;
        }
        nctx->log("^");
    }

    nctx->log("\n");
    JS_free(cx, prefix);
}

NidiumJS::NidiumJS(ape_global *net, Context *context)
    : m_JSStrictMode(false), m_Context(context)
{
    JSRuntime *rt;
    m_Modules = NULL;

    m_StructuredCloneAddition.read  = NULL;
    m_StructuredCloneAddition.write = NULL;

    static int isUTF8 = 0;

    /* TODO: BUG */
    if (!isUTF8) {
        // JS_SetCStringsAreUTF8();
        isUTF8 = 1;
    }
    // APE_DEBUG("Binding", "[NidiumJS] New JS runtime\n");

    m_Net = NULL;

    m_RootedObj = hashtbl_init(APE_HASH_INT);

    NidiumJS::InitNet(net);

    pthread_key_create(&gJS, NULL);
    pthread_setspecific(gJS, this);

    NidiumJS::Init();
    NidiumLocalContext::Init();

    if ((rt = JS_NewRuntime(JS::DefaultHeapMaxBytes, JS::DefaultNurseryBytes))
        == NULL) {

        APE_ERROR("Binding", "[NidiumJS] Failed to init JS runtime\n");
        return;
    }

    NidiumJS::SetJSRuntimeOptions(rt, m_JSStrictMode);
    JS_SetErrorReporter(rt, reportError);

    if ((m_Cx = JS_NewContext(rt, 8192)) == NULL) {
        APE_ERROR("Binding", "[NidiumJS] Failed to init JS context\n");
        return;
    }

#if 0
    JS_SetGCZeal(m_Cx, 2, 5);
#endif

    JS_SetGCParameterForThread(m_Cx, JSGC_MAX_CODE_CACHE_BYTES,
                               16 * 1024 * 1024);

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

    gbl = NidiumJS::CreateJSGlobal(m_Cx, this);

    m_Compartment = JS_EnterCompartment(m_Cx, gbl);
// JSAutoCompartment ac(m_Cx, gbl);

    NidiumLocalContext::InitJSThread(rt, m_Cx);

    if (NidiumJS::m_JsScc == NULL) {
        NidiumJS::m_JsScc              = new JSStructuredCloneCallbacks();
        NidiumJS::m_JsScc->read        = NidiumJS::readStructuredCloneOp;
        NidiumJS::m_JsScc->write       = NidiumJS::writeStructuredCloneOp;
        NidiumJS::m_JsScc->reportError = NULL;
    }

    this->bindNetObject(net);

    JS_SetRuntimePrivate(rt, this);
}


NidiumJS::~NidiumJS()
{
    JSRuntime *rt;
    rt         = JS_GetRuntime(m_Cx);

    NidiumLocalContext *nlc = NidiumLocalContext::Get();
    nlc->shutdown();

    ape_global *net = static_cast<ape_global *>(JS_GetContextPrivate(m_Cx));

    /* clear all non protected timers */
    APE_timers_destroy_unprotected(net);

    JS_LeaveCompartment(m_Cx, m_Compartment);
    JS_EndRequest(m_Cx);
    // JS_LeaveCompartment(m_Cx);

    JS_DestroyContext(m_Cx);

    JS_DestroyRuntime(rt);

    if (m_Modules) {
        delete m_Modules;
    }

    pthread_setspecific(gJS, NULL);

    hashtbl_free(m_RootedObj);
}


void NidiumJS::bindNetObject(ape_global *net)
{
    JS_SetContextPrivate(m_Cx, net);
    m_Net = net;

    // NidiumFileIO *io = new NidiumFileIO("/tmp/foobar", this, net);
    // io->open();
}

void NidiumJS::CopyProperties(JSContext *cx,
                              JS::HandleObject source,
                              JS::MutableHandleObject into)
{

    JS::Rooted<JS::IdVector> ida(cx, JS::IdVector(cx));

    JS_Enumerate(cx, source, &ida);

    for (size_t i = 0; i < ida.length(); i++) {
        JS::RootedId id(cx, ida[i]);

        JS::RootedValue val(cx);
        JS::RootedString prop(cx, JSID_TO_STRING(id));
        JSAutoByteString cprop(cx, prop);

        if (!JS_GetPropertyById(cx, source, id, &val)) {
            break;
        }
        /* TODO : has own property */
        switch (JS_TypeOfValue(cx, val)) {
            case JSTYPE_OBJECT: {
                JS::RootedValue oldval(cx);

                if (!JS_GetPropertyById(cx, into, id, &oldval)
                    || oldval.isNullOrUndefined() || oldval.isPrimitive()) {
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

int NidiumJS::LoadScriptReturn(JSContext *cx,
                               const char *data,
                               size_t len,
                               const char *filename,
                               JS::MutableHandleValue ret)
{
    JS::RootedObject gbl(cx, JS::CurrentGlobalOrNull(cx));

    char *func = static_cast<char *>(malloc(sizeof(char) * (len + 64)));
    memset(func, 0, sizeof(char) * (len + 64));

    strcat(func, "return (");
    strncat(func, data, len);
    strcat(func, ");");

    JS::RootedFunction cf(cx);

    JS::CompileOptions options(cx);
    options.setFileAndLine(filename, 1).setUTF8(true);

    JS::AutoObjectVector scopeChain(cx);

    /*
        The scope chain is empty, we only need global

        scopeChain.append(gbl) <= not needed
    */
    bool cret = JS::CompileFunction(cx, scopeChain, options,
        NULL, 0, NULL, func,
        strlen(func), &cf);

    free(func);

    if (!cret) {
        APE_ERROR("Binding", "[NidiumJS] Can not load script %s\n", filename);
        return 0;
    }

    if (JS_CallFunction(cx, gbl, cf, JS::HandleValueArray::empty(), ret)
        == false) {
        APE_ERROR("Binding", "[NidiumJS] Got an error?\n"); /* or thread has ended */

        return 0;
    }

    return 1;
}

int NidiumJS::LoadScriptReturn(JSContext *cx,
                               const char *filename,
                               JS::MutableHandleValue ret)
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

int NidiumJS::LoadScriptContent(const char *data,
                                size_t len,
                                const char *filename,
                                JS::HandleObject scope)
{
    if (!len) {
        /* silently success */
        return 1;
    }
    /*
        Detect JSBytecode using XDR magic number as defined in vm/Xdr.h
    */
    if ((*(uint32_t *)data) == (0xb973c0de - 330)) {
        return this->LoadBytecode((void *)(data), len, filename);
    }

    JS::RootedScript script(m_Cx);
#if 0
    /* RAII helper that resets to origin options state */
    JS::AutoSaveContextOptions asco(m_Cx);

    JS::ContextOptionsRef(m_Cx).setStrictMode(
        m_JSStrictMode);
#endif
    JS::CompileOptions options(m_Cx);
    options.setUTF8(true)
        .setFileAndLine(filename, 1)
        .setNoScriptRval(true);

    bool state = JS::Compile(m_Cx, options, data, len, &script);

    JS::AutoObjectVector scopeChain(m_Cx);
    if (scope != nullptr) {
        scopeChain.append(scope);
    }

    if (!state || !JS_ExecuteScript(m_Cx, scopeChain, script)) {
        if (JS_IsExceptionPending(m_Cx)) {
            if (!JS_ReportPendingException(m_Cx)) {
                JS_ClearPendingException(m_Cx);
            }
        }
        return 0;
    }
    return 1;
}

char *NidiumJS::LoadScriptContentAndGetResult(const char *data,
                                              size_t len,
                                              const char *filename)
{
    if (!len) {
        return NULL;
    }

    JS::RootedObject gbl(m_Cx, JS::CurrentGlobalOrNull(m_Cx));

    if (!gbl) {
        APE_ERROR("Binding", "[Nidium] Failed to load global object\n");
        return NULL;
    }

    JS::RootedScript script(m_Cx);

#if 0
    /* RAII helper that resets to origin options state */
    JS::AutoSaveContextOptions asco(m_Cx);

    JS::ContextOptionsRef(m_Cx).setVarObjFix(true).setStrictMode(
        m_JSStrictMode);

#endif

    JS::CompileOptions options(m_Cx);
    options.setUTF8(true)
        .setFileAndLine(filename, 1)
        .setNoScriptRval(false);

    bool state = JS::Compile(m_Cx, options, data, len, &script);

    JS::RootedValue rval(m_Cx);

    if (!state || !JS_ExecuteScript(m_Cx, script, &rval)) {
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
    // TODO: new style cast
    return this->LoadBytecode((void *)(script->data), script->size,
                              script->name);
}

int NidiumJS::LoadBytecode(void *data, int size, const char *filename)
{
    JS::RootedObject gbl(m_Cx, JS::CurrentGlobalOrNull(m_Cx));
    JS::RootedScript script(m_Cx, JS_DecodeScript(m_Cx, data, size));

    if (script == NULL || !JS_ExecuteScript(m_Cx, script)) {
        if (JS_IsExceptionPending(m_Cx)) {
            if (!JS_ReportPendingException(m_Cx)) {
                JS_ClearPendingException(m_Cx);
            }
        }
        return 0;
    }
    return 1;
}

void NidiumJS::loadGlobalObjects()
{
    JSFile::RegisterObject(m_Cx);
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
    JSDebuggerCompartment::RegisterObject(m_Cx);
    JSDB::RegisterObject(m_Cx);
    JSOS::RegisterObject(m_Cx);
    JSVM::RegisterObject(m_Cx);

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


// }}}
} // namespace Binding
} // namespace Nidium
