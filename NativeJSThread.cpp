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

#include "NativeJSThread.h"
#include "NativeSharedMessages.h"
#include "NativeJS.h"

extern void reportError(JSContext *cx, const char *message,
	JSErrorReport *report);
static JSBool native_post_message(JSContext *cx, unsigned argc, jsval *vp);
static void Thread_Finalize(JSFreeOp *fop, JSObject *obj);
static JSBool native_thread_start(JSContext *cx, unsigned argc, jsval *vp);

#define NJS (NativeJS::getNativeClass(cx))

static JSClass global_Thread_class = {
    "_GLOBALThread", JSCLASS_GLOBAL_FLAGS | JSCLASS_IS_GLOBAL,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass Thread_class = {
    "Thread", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Thread_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass messageEvent_class = {
    "ThreadMessageEvent", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSFunctionSpec glob_funcs_threaded[] = {
    JS_FN("send", native_post_message, 1, 0),
    JS_FS_END
};

static JSFunctionSpec Thread_funcs[] = {
    JS_FN("start", native_thread_start, 0, 0),
    JS_FS_END
};

static void Thread_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSThread *nthread = (NativeJSThread *)JS_GetPrivate(obj);

    if (nthread != NULL) {
        delete nthread;
    }
}

static JSBool JSThreadCallback(JSContext *cx)
{
    NativeJSThread *nthread;

    if ((nthread = (NativeJSThread *)JS_GetContextPrivate(cx)) == NULL ||
        nthread->markedStop) {
        return JS_FALSE;
    }
    return JS_TRUE;
}

static void *native_thread(void *arg)
{
    NativeJSThread *nthread = (NativeJSThread *)arg;

    JSRuntime *rt;
    JSContext *tcx;
    jsval rval = JSVAL_VOID;
    JSObject *gbl;

    if ((rt = JS_NewRuntime(128L * 1024L * 1024L, JS_USE_HELPER_THREADS)) == NULL) {
        printf("Failed to init JS runtime\n");
        return NULL;
    }

    JS_SetGCParameter(rt, JSGC_MAX_BYTES, 0xffffffff);
    JS_SetGCParameter(rt, JSGC_MODE, JSGC_MODE_INCREMENTAL);
    JS_SetGCParameter(rt, JSGC_SLICE_TIME_BUDGET, 15);
    
    if ((tcx = JS_NewContext(rt, 8192)) == NULL) {
        printf("Failed to init JS context\n");
        return NULL;     
    }
    JS_SetGCParameterForThread(tcx, JSGC_MAX_CODE_CACHE_BYTES, 16 * 1024 * 1024);

    JS_SetStructuredCloneCallbacks(rt, NativeJS::jsscc);
    JS_SetOperationCallback(tcx, JSThreadCallback);

    nthread->jsRuntime = rt;
    nthread->jsCx      = tcx;

    JS_BeginRequest(tcx);

    gbl = JS_NewGlobalObject(tcx, &global_Thread_class, NULL);

    JS_SetVersion(tcx, JSVERSION_LATEST);

    JS_SetOptions(tcx, JSOPTION_VAROBJFIX | JSOPTION_METHODJIT |
        JSOPTION_TYPE_INFERENCE | JSOPTION_ION/* | JSOPTION_ASMJS*/);

    if (!JS_InitStandardClasses(tcx, gbl))
        return NULL;
    
    JS_SetErrorReporter(tcx, reportError);

    JS_SetGlobalObject(tcx, gbl);

    JS_DefineFunctions(tcx, gbl, glob_funcs_threaded);

    JSAutoByteString str(tcx, nthread->jsFunction);
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

    /* Hold the parent cx */
    JS_SetContextPrivate(tcx, nthread);

    JSFunction *cf = JS_CompileFunction(tcx, gbl, NULL, 0, NULL, scoped,
        strlen(scoped), NULL, 0);

    delete[] scoped;
    
    if (cf == NULL) {
        printf("Cant compile function\n");
        JS_EndRequest(tcx);
        return NULL;
    }
    
    jsval *arglst = new jsval[nthread->params.argc];

    for (int i = 0; i < nthread->params.argc; i++) {
        JS_ReadStructuredClone(tcx,
                    nthread->params.argv[i],
                    nthread->params.nbytes[i],
                    JS_STRUCTURED_CLONE_VERSION, &arglst[i], NULL, NULL);

        JS_ClearStructuredClone(nthread->params.argv[i], nthread->params.nbytes[i]);
    }

    if (JS_CallFunction(tcx, gbl, cf, nthread->params.argc,
        arglst, &rval) == JS_FALSE) {
        printf("Got an error?\n"); /* or thread has ended */
    }

    JS_EndRequest(tcx);

    free(nthread->params.argv);
    free(nthread->params.nbytes);

    delete[] arglst;

    nthread->onComplete(&rval);

    JS_DestroyContext(tcx);
    JS_DestroyRuntime(rt);

    nthread->jsRuntime = NULL;
    nthread->jsCx = NULL;

    return NULL;
}


static JSBool native_thread_start(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSThread *nthread;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);

    if (JS_InstanceOf(cx, caller, &Thread_class, JS_ARGV(cx, vp)) == JS_FALSE) {
        return JS_TRUE;
    }    

    if ((nthread = (NativeJSThread *)JS_GetPrivate(caller)) == NULL) {
        return JS_TRUE;
    }

    nthread->params.argv = (argc ?
        (uint64_t **)malloc(sizeof(*nthread->params.argv) * argc) : NULL);
    nthread->params.nbytes = (argc ?
        (size_t *)malloc(sizeof(*nthread->params.nbytes) * argc) : NULL);

    for (int i = 0; i < (int)argc; i++) {
        if (!JS_WriteStructuredClone(cx, JS_ARGV(cx, vp)[i],
            &nthread->params.argv[i], &nthread->params.nbytes[i],
            NULL, NULL, JSVAL_VOID)) {

            return false;
        }
    }

    nthread->params.argc = argc;

    /* TODO: check if already running */
    pthread_create(&nthread->threadHandle, NULL,
                            native_thread, nthread);

    nthread->njs->rootObjectUntilShutdown(caller);

    return JS_TRUE;
}

void native_thread_message(JSContext *cx, NativeSharedMessages::Message *msg)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE)
    struct native_thread_msg *ptr;
    int ev;
    char prop[16];

    jsval jscbk, jevent, rval;
    JSObject *event;

    ptr = static_cast<struct native_thread_msg *>(msg->dataPtr());
    ev = msg->event();

    memset(prop, 0, sizeof(prop));

    if (ev == NATIVE_THREAD_MESSAGE) {
        strcpy(prop, "onmessage");
    } else if (ev == NATIVE_THREAD_COMPLETE) {
        strcpy(prop, "oncomplete");
    }

    jsval inval = JSVAL_NULL;

    if (!JS_ReadStructuredClone(cx, ptr->data, ptr->nbytes,
        JS_STRUCTURED_CLONE_VERSION, &inval, NULL, NULL)) {

        printf("Failed to read input data (readMessage)\n");

        delete ptr;
        return;
    }

    if (JS_GetProperty(cx, ptr->callee, prop, &jscbk) &&
        !JSVAL_IS_PRIMITIVE(jscbk) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(jscbk))) {

        event = JS_NewObject(cx, &messageEvent_class, NULL, NULL);

        EVENT_PROP("data", inval);

        jevent = OBJECT_TO_JSVAL(event);
        JS_CallFunctionValue(cx, event, jscbk, 1, &jevent, &rval);

    }
    JS_ClearStructuredClone(ptr->data, ptr->nbytes);

    delete ptr;
#undef EVENT_PROP
}

static JSBool native_Thread_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *ret = JS_NewObjectForConstructor(cx, &Thread_class, vp);

    NativeJSThread *nthread = new NativeJSThread();
    JSFunction *nfn;

    nthread->cx = cx;

    if ((nfn = JS_ValueToFunction(cx, JS_ARGV(cx, vp)[0])) == NULL ||
    	(nthread->jsFunction = JS_DecompileFunction(cx, nfn, 0)) == NULL) {
    	printf("Failed to read Threaded function\n");
    	return JS_TRUE;
    }

    nthread->jsObject 	= ret;
    nthread->njs 		= NJS;

    JS_AddStringRoot(cx, &nthread->jsFunction);

    JS_SetPrivate(ret, nthread);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    JS_DefineFunctions(cx, ret, Thread_funcs);

    return JS_TRUE;
}

void NativeJSThread::onComplete(jsval *vp)
{
    struct native_thread_msg *msg = new struct native_thread_msg;

    if (!JS_WriteStructuredClone(jsCx, *vp, &msg->data, &msg->nbytes,
        NULL, NULL, JSVAL_VOID)) {

        msg->data = NULL;
        msg->nbytes = 0;
    }

    msg->callee = jsObject;

    njs->messages->postMessage(msg, NATIVE_THREAD_COMPLETE);

    njs->unrootObject(jsObject);
}

static JSBool native_post_message(JSContext *cx, unsigned argc, jsval *vp)
{
    uint64_t *datap;
    size_t nbytes;

    NativeJSThread *nthread = (NativeJSThread *)JS_GetContextPrivate(cx);

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    NATIVE_CHECK_ARGS("postMessage", 1);

    if (nthread == NULL || nthread->markedStop) {
        JS_ReportError(cx, "thread.send() Could not retrieve thread (or marked for stopping)");
        return false;
    }

    struct native_thread_msg *msg;

    if (!JS_WriteStructuredClone(cx, args[0], &datap, &nbytes,
        NULL, NULL, JSVAL_VOID)) {
        JS_ReportError(cx, "Failed to write strclone");
        /* TODO: exception */
        return false;
    }

    msg = new struct native_thread_msg;

    msg->data   = datap;
    msg->nbytes = nbytes;
    msg->callee = nthread->jsObject;

    nthread->njs->messages->postMessage(msg, NATIVE_THREAD_MESSAGE);

    return true;
}

NativeJSThread::~NativeJSThread()
{
    if (jsFunction && this->cx) {
        JS_RemoveStringRoot(this->cx, &jsFunction);
    }
    this->markedStop = true;
    if (this->jsRuntime) {
        JS_TriggerOperationCallback(this->jsRuntime);
        pthread_join(this->threadHandle, NULL);
    }
}

NativeJSThread::NativeJSThread()
	: jsFunction(NULL), jsRuntime(NULL), jsCx(NULL),
    jsObject(NULL), njs(NULL), markedStop(false)
{
	/* cx hold the main context (caller) */
	/* jsCx hold the newly created context (along with jsRuntime) */
	cx = NULL;
}

NATIVE_OBJECT_EXPOSE(Thread)
