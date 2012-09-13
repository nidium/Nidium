#include "NativeJSThread.h"
#include "NativeSharedMessages.h"
#include "NativeJS.h"

extern void reportError(JSContext *cx, const char *message,
	JSErrorReport *report);
static JSBool native_post_message(JSContext *cx, unsigned argc, jsval *vp);
static void Thread_Finalize(JSFreeOp *fop, JSObject *obj);

#define NJS ((class NativeJS *)JS_GetRuntimePrivate(JS_GetRuntime(cx)))

static JSClass global_Thread_class = {
    "_GLOBALThread", JSCLASS_GLOBAL_FLAGS | JSCLASS_IS_GLOBAL,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass thread_class = {
    "Thread", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Thread_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSFunctionSpec glob_funcs_threaded[] = {
    JS_FN("send", native_post_message, 1, 0),
    JS_FS_END
};


static void Thread_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSThread *nthread = (NativeJSThread *)JS_GetPrivate(obj);
    if (nthread != NULL) {
        delete nthread;
    }    
}

static void *native_thread(void *arg)
{
    NativeJSThread *nthread = (NativeJSThread *)arg;

    JSRuntime *rt;
    JSContext *tcx;
    jsval rval;
    JSObject *gbl;

    if ((rt = JS_NewRuntime(128L * 1024L * 1024L)) == NULL) {
        printf("Failed to init JS runtime\n");
        return NULL;
    }

    if ((tcx = JS_NewContext(rt, 8192)) == NULL) {
        printf("Failed to init JS context\n");
        return NULL;     
    }

    nthread->jsRuntime = rt;
    nthread->jsCx      = tcx;

    gbl = JS_NewGlobalObject(tcx, &global_Thread_class, NULL);

    JS_SetVersion(tcx, JSVERSION_LATEST);

    JS_SetOptions(tcx, JSOPTION_VAROBJFIX | JSOPTION_METHODJIT |
        JSOPTION_TYPE_INFERENCE | JSOPTION_ION);

    if (!JS_InitStandardClasses(tcx, gbl))
        return NULL;
    
    JS_SetErrorReporter(tcx, reportError);

    JS_SetGlobalObject(tcx, gbl);

    JS_DefineFunctions(tcx, gbl, glob_funcs_threaded);

    JSAutoByteString str(tcx, nthread->jsFunction);

    /* Hold the parent cx */
    JS_SetContextPrivate(tcx, nthread);

    JSFunction *cf = JS_CompileFunction(tcx, gbl, NULL, 0, NULL, str.ptr(),
        strlen(str.ptr()), NULL, 0);
    
    if (cf == NULL) {
        printf("Cant compile function\n");
        return NULL;
    }
    
    if (JS_CallFunction(tcx, gbl, cf, 0, NULL, &rval) == JS_FALSE) {
        printf("Got an error?\n");
    }

    return NULL;
}


static JSBool native_Thread_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *ret = JS_NewObjectForConstructor(cx, &thread_class, vp);

    NativeJSThread *nthread = new NativeJSThread();
    JSFunction *nfn;

    if ((nfn = JS_ValueToFunction(cx, JS_ARGV(cx, vp)[0])) == NULL ||
    	(nthread->jsFunction = JS_DecompileFunctionBody(cx, nfn, 0)) == NULL) {

    	printf("Failed to read Threaded function\n");
    	return JS_TRUE;
    }

    nthread->jsObject 	= ret;
    nthread->njs 		= NJS;

    pthread_create(&nthread->threadHandle, NULL,
                            native_thread, nthread);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

static JSBool native_post_message(JSContext *cx, unsigned argc, jsval *vp)
{
    uint64_t *datap;
    size_t nbytes;
    NativeJSThread *nthread = (NativeJSThread *)JS_GetContextPrivate(cx);

    if (nthread == NULL) {
        printf("Could not retrieve thread\n");
        return JS_TRUE;
    }

    struct native_thread_msg *msg;

    if (!JS_WriteStructuredClone(cx, JS_ARGV(cx, vp)[0], &datap, &nbytes,
        NULL, NULL)) {
        printf("Failed to write strclone\n");
        /* TODO: exception */
        return JS_TRUE;
    }

    msg = new struct native_thread_msg;

    msg->data   = datap;
    msg->nbytes = nbytes;
    msg->callee = nthread->jsObject;

    nthread->njs->messages->postMessage(msg);

    return JS_TRUE;
}

NativeJSThread::~NativeJSThread()
{

	if (this->jsCx) {
		JS_DestroyContext(this->jsCx);
	}
	if (this->jsRuntime) {
		JS_DestroyRuntime(this->jsRuntime);
	}

}

NativeJSThread::NativeJSThread()
	: jsFunction(NULL), jsRuntime(NULL), jsCx(NULL), jsObject(NULL), njs(NULL)
{
	/* cx hold the main context (caller) */
	/* jsCx hold the newly created context (along with jsRuntime) */
	cx = NULL;
}

void NativeJSThread::registerObject(JSContext *cx)
{
    JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, &thread_class,
    	native_Thread_constructor,
        0, NULL, NULL, NULL, NULL);
}
