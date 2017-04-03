/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSThread.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>


#include "Binding/JSConsole.h"
#include <js/StructuredClone.h>

namespace Nidium {
namespace Binding {

struct nidium_thread_msg
{
    uint64_t *data;
    size_t nbytes;
};

// {{{ Thread management
#define NJS (NidiumJS::GetObject(cx))
extern void
reportError(JSContext *cx, const char *message, JSErrorReport *report);
static bool nidium_post_message(JSContext *cx, unsigned argc, JS::Value *vp);

static JSClass global_Thread_class = { "_GLOBALThread",
                                       JSCLASS_GLOBAL_FLAGS | JSCLASS_IS_GLOBAL,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       JS_GlobalObjectTraceHook};

static JSFunctionSpec glob_funcs_threaded[]
    = { JS_FN("send", nidium_post_message, 1, NIDIUM_JS_FNPROPS), JS_FS_END };

static bool JSThreadCallback(JSContext *cx)
{
    JSThread *nthread;

    if ((nthread = static_cast<JSThread *>(JS_GetContextPrivate(cx))) == NULL
        || nthread->m_MarkedStop) {
        return false;
    }
    return true;
}

JSObject *_CreateJSGlobal(JSContext *cx)
{
    JS::CompartmentOptions options;
    options.setVersion(JSVERSION_LATEST);

    JS::RootedObject glob(
        cx, JS_NewGlobalObject(cx, &global_Thread_class, nullptr,
                               JS::DontFireOnNewGlobalHook, options));
    JSAutoCompartment ac(cx, glob);
    JS_InitStandardClasses(cx, glob);
    JS_DefineDebuggerObject(cx, glob);
    JS_DefineFunctions(cx, glob, glob_funcs_threaded);
    JS_FireOnNewGlobalObject(cx, glob);

    return glob;
    // JS::RegisterPerfMeasurement(cx, glob);

    // https://bugzilla.mozilla.org/show_bug.cgi?id=880330
    // context option vs compile option?
}

static void *nidium_thread(void *arg)
{
    JSThread *nthread = static_cast<JSThread *>(arg);

    JSRuntime *rt;
    JSContext *tcx;

    if ((rt = JS_NewRuntime(JS::DefaultHeapMaxBytes,
        JS::DefaultNurseryBytes, nthread->m_ParentRuntime))
        == NULL) {
        fprintf(stderr, "Failed to init JS runtime");
        return NULL;
    }

    NidiumJS::SetJSRuntimeOptions(rt);

    if ((tcx = JS_NewContext(rt, 8192)) == NULL) {
        fprintf(stderr, "Failed to init JS context");
        JS_DestroyRuntime(rt);
        return NULL;
    } else {
        JSAutoRequest ar(tcx);
        JS_SetGCParameterForThread(tcx, JSGC_MAX_CODE_CACHE_BYTES,
                                   16 * 1024 * 1024);

        JS_SetInterruptCallback(rt, JSThreadCallback);

        NidiumLocalContext::InitJSThread(rt, tcx);

        nthread->m_JsRuntime = rt;
        nthread->m_JsCx      = tcx;

        /*
            repportError read the runtime private to use the logger
        */
        JS_SetRuntimePrivate(rt, nthread->m_Njs);
        JS_SetErrorReporter(rt, reportError);

        JS::RootedObject gbl(tcx, _CreateJSGlobal(tcx));
        if (gbl.get()) {
            JSAutoCompartment ac(tcx, gbl);
            JS::RootedValue rval(tcx, JS::NullValue());

            JSConsole::RegisterObject(tcx);

            JSAutoByteString str(tcx, nthread->m_JsFunction);
            char *scoped = new char[strlen(str.ptr()) + 128];
            /*
                JS_CompileFunction takes a function body.
                This is a hack in order to catch the arguments name, etc...

                function() {
                    (function (a) {
                        this.send("hello" + a);
                    }).apply(this, Array.prototype.slice.apply(arguments));
                };
            */
            sprintf(scoped, "return %c%s%s", '(', str.ptr(),
                    ").apply(this, Array.prototype.slice.apply(arguments));");

            /* Hold the parent cx */
            JS_SetContextPrivate(tcx, nthread);

            JS::CompileOptions options(tcx);
            options.setFileAndLine(nthread->m_CallerFileName,
                                   nthread->m_CallerLineNo)
                .setUTF8(true);

            JS::RootedFunction cf(tcx);

            JS::AutoObjectVector scopeChain(tcx);

            bool cret = JS::CompileFunction(tcx, scopeChain, options,
                            NULL, 0, NULL, scoped,
                            strlen(scoped), &cf);

            NidiumLocalContext::UnrootObject(nthread->m_JsFunction);

            delete[] scoped;

            if (!cret) {
                fprintf(stderr, "Can't compile function");
                return NULL;
            }

            JS::AutoValueVector arglst(tcx);
            arglst.resize(nthread->m_Params.argc);

            for (size_t i = 0; i < nthread->m_Params.argc; i++) {
                JS::RootedValue args(tcx);
                JS_ReadStructuredClone(
                    tcx, nthread->m_Params.argv[i], nthread->m_Params.nbytes[i],
                    JS_STRUCTURED_CLONE_VERSION, &args, NidiumJS::m_JsScc, NULL);

                arglst[i].set(args);
                JS_ClearStructuredClone(nthread->m_Params.argv[i],
                                        nthread->m_Params.nbytes[i], NidiumJS::m_JsScc,
                                        NULL);
            }

            if (JS_CallFunction(tcx, gbl, cf, arglst, &rval) == false) {
            }

            free(nthread->m_Params.argv);
            free(nthread->m_Params.nbytes);
            nthread->onComplete(rval);
        }
    }
    NidiumLocalContext *nlc = NidiumLocalContext::Get();
    nlc->shutdown();

    JS_DestroyContext(tcx);
    JS_DestroyRuntime(rt);
    nthread->m_JsRuntime = NULL;
    nthread->m_JsCx      = NULL;

    return NULL;
}
// }}}

// {{{ JSThread
JSThread::JSThread()
    : m_JsRuntime(NULL), m_JsCx(NULL),
      m_Njs(NULL), m_Params({ 0, NULL, 0 }),
      m_MarkedStop(false), m_CallerFileName(NULL), m_CallerLineNo(0)
{
    /* cx hold the main context (caller) */
    /* jsCx hold the newly created context (along with jsRuntime) */
    m_Cx = NULL;
}

void JSThread::onMessage(const Core::SharedMessages::Message &msg)
{
    struct nidium_thread_msg *ptr;
    char prop[16];
    int ev = msg.event();
    static const char *eventName[2] = {"message", "complete"};

    ptr = static_cast<struct nidium_thread_msg *>(msg.dataPtr());
    memset(prop, 0, sizeof(prop));

    JS::RootedValue inval(m_Cx, JS::NullValue());
    if (!JS_ReadStructuredClone(m_Cx, ptr->data, ptr->nbytes,
                                JS_STRUCTURED_CLONE_VERSION, &inval, NidiumJS::m_JsScc,
                                NULL)) {

        ndm_log(NDM_LOG_ERROR, "JSThread", "Failed to read input data (readMessage)");

        delete ptr;
        return;
    }

    JS::RootedObject event(m_Cx, JSEvents::CreateEventObject(m_Cx));
    JS::RootedValue eventVal(m_Cx, JS::ObjectValue(*event));

    NIDIUM_JSOBJ_SET_PROP(event, "data", inval);

    this->fireJSEvent(eventName[ev], &eventVal);

    JS_ClearStructuredClone(ptr->data, ptr->nbytes, NidiumJS::m_JsScc, NULL);

    delete ptr;
}


void JSThread::onComplete(JS::HandleValue vp)
{
    struct nidium_thread_msg *msg = new struct nidium_thread_msg;

    if (!JS_WriteStructuredClone(m_JsCx, vp, &msg->data, &msg->nbytes,
                                 NidiumJS::m_JsScc,
                                 NULL, JS::NullHandleValue)) {

        msg->data   = NULL;
        msg->nbytes = 0;
    }

    this->postMessage(msg, JSThread::kThread_Complete);

    this->unroot();
}


JSThread::~JSThread()
{
    this->m_MarkedStop = true;
    if (m_JsRuntime) {
        JS_RequestInterruptCallback(m_JsRuntime);
        pthread_join(this->m_ThreadHandle, NULL);
    }

    if (m_CallerFileName) {
        free(m_CallerFileName);
    }
}

bool JSThread::JS_start(JSContext *cx, JS::CallArgs &args)
{
    int argc = args.length();

    this->m_Params.argv
        = (argc ? (uint64_t **)malloc(sizeof(*this->m_Params.argv) * argc)
                : NULL);
    this->m_Params.nbytes
        = (argc ? (size_t *)malloc(sizeof(*this->m_Params.nbytes) * argc)
                : NULL);

    for (int i = 0; i < static_cast<int>(argc); i++) {
        if (!JS_WriteStructuredClone(cx, args[i], &this->m_Params.argv[i],
                                     &this->m_Params.nbytes[i], NidiumJS::m_JsScc, NULL,
                                     JS::NullHandleValue)) {

            return false;
        }
    }

    this->m_Params.argc = argc;

    /* TODO: check if already running */
    pthread_create(&this->m_ThreadHandle, NULL, nidium_thread, this);

    this->root();

    return true;
}

static bool nidium_post_message(JSContext *cx, unsigned argc, JS::Value *vp)
{
    uint64_t *datap;
    size_t nbytes;

    JSThread *nthread = static_cast<JSThread *>(JS_GetContextPrivate(cx));

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!args.requireAtLeast(cx, "postMessage", 1)) {
        return false;
    }

    if (nthread == NULL || nthread->m_MarkedStop) {
        JS_ReportError(
            cx,
            "thread.send() Could not retrieve thread (or marked for stopping)");
        return false;
    }

    struct nidium_thread_msg *msg;

    if (!JS_WriteStructuredClone(cx, args[0], &datap, &nbytes, NidiumJS::m_JsScc, NULL,
                                 JS::NullHandleValue)) {
        JS_ReportError(cx, "Failed to write strclone");
        /* TODO: exception */
        return false;
    }

    msg = new struct nidium_thread_msg;

    msg->data   = datap;
    msg->nbytes = nbytes;

    nthread->postMessage(msg, JSThread::kThread_Message);

    return true;
}


JSThread * JSThread::Constructor(JSContext *cx, JS::CallArgs &args,
    JS::HandleObject obj)
{
    JS::RootedScript parent(cx);

    JSThread *nthread = new JSThread();
    JS::RootedFunction nfn(cx);

    if ((nfn = JS_ValueToFunction(cx, args[0])) == NULL
        || (nthread->m_JsFunction = JS_DecompileFunction(cx, nfn, 0)) == NULL) {
        ndm_log(NDM_LOG_ERROR, "JSThread", "Failed to read Threaded function");
        return nullptr;
    }

    NidiumLocalContext::RootObjectUntilShutdown(nthread->m_JsFunction);

    nthread->m_ParentRuntime = JS_GetRuntime(cx);
    nthread->m_Njs      = NJS;

    JS::AutoFilename af;
    JS::DescribeScriptedCaller(cx, &af, &nthread->m_CallerLineNo);

    nthread->m_CallerFileName = strdup(af.get());

    return nthread;
}

JSFunctionSpec *JSThread::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSThread, start, 0),
        JS_FS_END
    };

    return funcs;
}

void JSThread::RegisterObject(JSContext *cx)
{
    JSThread::ExposeClass<1>(cx, "Thread");
}
// }}}

} // namespace Binding
} // namespace Nidium
