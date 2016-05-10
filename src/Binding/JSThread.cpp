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
#include <glob.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <js/OldDebugAPI.h>

#include "Binding/JSConsole.h"

namespace Nidium {
namespace Binding {

// {{{ Preamble
extern void reportError(JSContext *cx, const char *message,
    JSErrorReport *report);
static bool nidium_post_message(JSContext *cx, unsigned argc, JS::Value *vp);
static void Thread_Finalize(JSFreeOp *fop, JSObject *obj);
static bool nidium_thread_start(JSContext *cx, unsigned argc, JS::Value *vp);

#define NJS (Nidiumcore::GetObject(cx))

static JSClass global_Thread_class = {
    "_GLOBALThread", JSCLASS_GLOBAL_FLAGS | JSCLASS_IS_GLOBAL,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, nullptr,
    nullptr, nullptr, nullptr, JS_GlobalObjectTraceHook, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSClass Thread_class = {
    "Thread", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Thread_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *JSExposer<JSThread>::jsclass = &Thread_class;

static JSClass messageEvent_class = {
    "ThreadMessageEvent", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSFunctionSpec glob_funcs_threaded[] = {
    JS_FN("send", nidium_post_message, 1, NIDIUM_JS_FNPROPS),
    JS_FS_END
};

static JSFunctionSpec Thread_funcs[] = {
    JS_FN("start", nidium_thread_start, 0, NIDIUM_JS_FNPROPS),
    JS_FS_END
};

static bool JSThreadCallback(JSContext *cx)
{
    JSThread *nthread;

    if ((nthread = static_cast<JSThread *>(JS_GetContextPrivate(cx))) == NULL ||
        nthread->m_MarkedStop) {
        return false;
    }
    return true;
}

JSObject *_CreateJSGlobal(JSContext *cx)
{
    JS::CompartmentOptions options;
    options.setVersion(JSVERSION_LATEST);

    JS::RootedObject glob(cx, JS_NewGlobalObject(cx, &global_Thread_class, nullptr,
                                             JS::DontFireOnNewGlobalHook, options));
    JSAutoCompartment ac(cx, glob);
    JS_InitStandardClasses(cx, glob);
    JS_DefineDebuggerObject(cx, glob);
    JS_DefineFunctions(cx, glob, glob_funcs_threaded);
    JS_FireOnNewGlobalObject(cx, glob);

    return glob;
    //JS::RegisterPerfMeasurement(cx, glob);

    //https://bugzilla.mozilla.org/show_bug.cgi?id=880330
    // context option vs compile option?
}

static void *nidium_thread(void *arg)
{
    JSThread *nthread = static_cast<JSThread *>(arg);

    JSRuntime *rt;
    JSContext *tcx;

    if ((rt = JS_NewRuntime(128L * 1024L * 1024L, JS_USE_HELPER_THREADS)) == NULL) {
        printf("Failed to init JS runtime\n");
        return NULL;
    }

    Nidiumcore::SetJSRuntimeOptions(rt);

    if ((tcx = JS_NewContext(rt, 8192)) == NULL) {
        printf("Failed to init JS context\n");
        JS_DestroyRuntime(rt);
        return NULL;
    } else {
        JSAutoRequest ar(tcx);
        JS_SetGCParameterForThread(tcx, JSGC_MAX_CODE_CACHE_BYTES, 16 * 1024 * 1024);

        JS_SetStructuredCloneCallbacks(rt, Nidiumcore::m_JsScc);
        JS_SetInterruptCallback(rt, JSThreadCallback);

        nthread->m_JsRuntime = rt;
        nthread->m_JsCx      = tcx;

        /*
            repportError read the runtime private to use the logger
        */
        JS_SetRuntimePrivate(rt, nthread->m_Njs);
        JS_SetErrorReporter(tcx, reportError);

        JS::RootedObject gbl(tcx, _CreateJSGlobal(tcx));
        if (gbl.get()) {
            JSAutoCompartment ac(tcx, gbl);
            JS::RootedValue rval(tcx, JSVAL_VOID);

            js::SetDefaultObjectForContext(tcx, gbl);

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
            sprintf(scoped, "return %c%s%s", '(', str.ptr(), ").apply(this, Array.prototype.slice.apply(arguments));");

            JS_SetReservedSlot(nthread->getJSObject(), 0, JS::NullHandleValue);

            /* Hold the parent cx */
            JS_SetContextPrivate(tcx, nthread);

            JS::CompileOptions options(tcx);
            options.setFileAndLine(nthread->m_CallerFileName, nthread->m_CallerLineNo)
                   .setUTF8(true);

            JS::RootedFunction cf(tcx);
            cf = JS::CompileFunction(tcx, gbl, options, NULL, 0, NULL, scoped, strlen(scoped));
            delete[] scoped;

            if (cf == NULL) {
                printf("Cant compile function\n");
                return NULL;
            }

            JS::AutoValueVector arglst(tcx);
            arglst.resize(nthread->m_Params.argc);

            for (size_t i = 0; i < nthread->m_Params.argc; i++) {
                JS::RootedValue args(tcx);
                JS_ReadStructuredClone(tcx,
                            nthread->m_Params.argv[i],
                            nthread->m_Params.nbytes[i],
                            JS_STRUCTURED_CLONE_VERSION, &args, NULL, NULL);

                arglst[i] = args;
                JS_ClearStructuredClone(nthread->m_Params.argv[i], nthread->m_Params.nbytes[i], NULL, NULL);
            }

            if (JS_CallFunction(tcx, gbl, cf, arglst, &rval) == false) {
            }

            free(nthread->m_Params.argv);
            free(nthread->m_Params.nbytes);
            nthread->onComplete(rval);
        }
    }
    JS_DestroyContext(tcx);
    JS_DestroyRuntime(rt);
    nthread->m_JsRuntime = NULL;
    nthread->m_JsCx = NULL;

    return NULL;
}

// }}}

// {{{ JSThread
JSThread::JSThread(JS::HandleObject obj, JSContext *cx) :
    JSExposer<JSThread>(obj, cx),
    m_JsRuntime(NULL), m_JsCx(NULL),
    m_JsObject(NULL), m_Njs(NULL), m_Params({0, NULL, 0}), m_MarkedStop(false),
    m_CallerFileName(NULL), m_CallerLineNo(0)
{
    /* cx hold the main context (caller) */
    /* jsCx hold the newly created context (along with jsRuntime) */
    cx = NULL;
}

void JSThread::onMessage(const Core::SharedMessages::Message &msg)
{
#define EVENT_PROP(name, val) JS_DefineProperty(m_Cx, event, name, \
    val, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)
    struct nidium_thread_msg *ptr;
    char prop[16];
    int ev = msg.event();

    ptr = static_cast<struct nidium_thread_msg *>(msg.dataPtr());
    memset(prop, 0, sizeof(prop));

    if (ev == JSThread::kThread_Message) {
        strcpy(prop, "onmessage");
    } else if (ev == JSThread::kThread_Complete) {
        strcpy(prop, "oncomplete");
    }

    JS::RootedValue inval(m_Cx, JSVAL_NULL);
    if (!JS_ReadStructuredClone(m_Cx, ptr->data, ptr->nbytes,
        JS_STRUCTURED_CLONE_VERSION, &inval, NULL, NULL)) {

        printf("Failed to read input data (readMessage)\n");

        delete ptr;
        return;
    }

    JS::RootedValue jscbk(m_Cx);
    JS::RootedObject callee(m_Cx, ptr->callee);
    if (JS_GetProperty(m_Cx, callee, prop, &jscbk) && JS_TypeOfValue(m_Cx, jscbk) == JSTYPE_FUNCTION) {
        JS::RootedObject event(m_Cx, JS_NewObject(m_Cx, &messageEvent_class, JS::NullPtr(), JS::NullPtr()));
        EVENT_PROP("data", inval);

        JS::AutoValueArray<1> jevent(m_Cx);
        jevent[0].setObject(*event);

        JS::RootedValue rval(m_Cx);
        JS_CallFunctionValue(m_Cx, event, jscbk, jevent, &rval);
    }
    JS_ClearStructuredClone(ptr->data, ptr->nbytes, NULL, NULL);

    delete ptr;
#undef EVENT_PROP
}


void JSThread::onComplete(JS::HandleValue vp)
{
    struct nidium_thread_msg *msg = new struct nidium_thread_msg;

    if (!JS_WriteStructuredClone(m_JsCx, vp, &msg->data, &msg->nbytes,
        NULL, NULL, JS::NullHandleValue)) {

        msg->data = NULL;
        msg->nbytes = 0;
    }

    msg->callee = m_JsObject;

    this->postMessage(msg, JSThread::kThread_Complete);

    m_Njs->unrootObject(m_JsObject);
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

// }}}

// {{{ Implementation

static bool nidium_thread_start(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JSThread *nthread;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    if (JS_InstanceOf(cx, caller, &Thread_class, &args) == false) {
        return true;
    }

    if ((nthread = static_cast<JSThread *>(JS_GetPrivate(caller))) == NULL) {
        return true;
    }

    nthread->m_Params.argv = (argc ?
        (uint64_t **)malloc(sizeof(*nthread->m_Params.argv) * argc) : NULL);
    nthread->m_Params.nbytes = (argc ?
        (size_t *)malloc(sizeof(*nthread->m_Params.nbytes) * argc) : NULL);

    for (int i = 0; i < static_cast<int>(argc); i++) {

        if (!JS_WriteStructuredClone(cx, args[i],
            &nthread->m_Params.argv[i], &nthread->m_Params.nbytes[i],
            NULL, NULL, JS::NullHandleValue)) {

            return false;
        }
    }

    nthread->m_Params.argc = argc;

    /* TODO: check if already running */
    pthread_create(&nthread->m_ThreadHandle, NULL,
                            nidium_thread, nthread);

    nthread->m_Njs->rootObjectUntilShutdown(caller);

    return true;
}

static bool nidium_post_message(JSContext *cx, unsigned argc, JS::Value *vp)
{
    uint64_t *datap;
    size_t nbytes;

    JSThread *nthread = static_cast<JSThread *>(JS_GetContextPrivate(cx));

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    NIDIUM_JS_CHECK_ARGS("postMessage", 1);

    if (nthread == NULL || nthread->m_MarkedStop) {
        JS_ReportError(cx, "thread.send() Could not retrieve thread (or marked for stopping)");
        return false;
    }

    struct nidium_thread_msg *msg;

    if (!JS_WriteStructuredClone(cx, args[0], &datap, &nbytes,
        NULL, NULL, JS::NullHandleValue)) {
        JS_ReportError(cx, "Failed to write strclone");
        /* TODO: exception */
        return false;
    }

    msg = new struct nidium_thread_msg;

    msg->data   = datap;
    msg->nbytes = nbytes;
    msg->callee = nthread->m_JsObject;

    nthread->postMessage(msg, JSThread::kThread_Message);

    return true;
}


static bool nidium_Thread_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &Thread_class, args));

    JS::RootedScript parent(cx);

    JSThread *nthread = new JSThread(ret, cx);
    JS::RootedFunction nfn(cx);

    if ((nfn = JS_ValueToFunction(cx, args[0])) == NULL ||
        (nthread->m_JsFunction = JS_DecompileFunction(cx, nfn, 0)) == NULL) {
        printf("Failed to read Threaded function\n");
        return true;
    }

    nthread->m_JsObject = ret;
    nthread->m_Njs = NJS;

    JS::AutoFilename af;
    JS::DescribeScriptedCaller(cx, &af, &nthread->m_CallerLineNo);

    nthread->m_CallerFileName = strdup(af.get());

    JS_SetPrivate(ret, nthread);

    args.rval().setObject(*ret);

    JS_DefineFunctions(cx, ret, Thread_funcs);

    return true;
}

static void Thread_Finalize(JSFreeOp *fop, JSObject *obj)
{
    JSThread *nthread = static_cast<JSThread *>(JS_GetPrivate(obj));

    if (nthread != NULL) {
        delete nthread;
    }
}

// }}}

// {{{ Registration

NIDIUM_JS_OBJECT_EXPOSE(Thread)

// }}}

} // namespace Binding
} // namespace Nidium

