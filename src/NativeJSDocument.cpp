#include "NativeJSDocument.h"
#include "NativeJS.h"
#include "NativeContext.h"
#include "NativeUIInterface.h"
#include <native_netlib.h>
#include <jsapi.h>

#define NJSDOC_GETTER(obj) ((class NativeJSdocument *)JS_GetPrivate(obj))

static JSBool native_document_run(JSContext *cx, unsigned argc, jsval *vp);
static void Document_Finalize(JSFreeOp *fop, JSObject *obj);

static JSPropertySpec document_props[] = {
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSClass document_class = {
    "NativeDocument", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Document_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

JSClass *NativeJSdocument::jsclass = &document_class;

static JSFunctionSpec document_funcs[] = {
    JS_FN("run", native_document_run, 1, 0),
    JS_FS_END
};

struct _native_document_restart_async
{
    NativeUIInterface *ui;
    char *location;
};

static int native_document_restart(void *param)
{
    struct _native_document_restart_async *ndra = (struct _native_document_restart_async *)param;

    ndra->ui->restartApplication(ndra->location);

    free(ndra->location);
    free(ndra);

    return 0;
}

static JSBool native_document_run(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *location;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &location)) {
        return JS_TRUE;
    }

    NativeUIInterface *NUI = NativeContext::getNativeClass(cx)->getUI();
    JSAutoByteString locationstr(cx, location);

    struct _native_document_restart_async *ndra = (struct _native_document_restart_async *)malloc(sizeof(*ndra));

    ndra->location = strdup(locationstr.ptr());
    ndra->ui = NUI;
    ape_global *ape = NativeJS::getNativeClass(cx)->net;

    timer_dispatch_async(native_document_restart, ndra);

    return true;
}

static void Document_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSdocument *jdoc = NativeJSdocument::getNativeClass(obj);

    if (jdoc != NULL) {
        delete jdoc;
    }
}

bool NativeJSdocument::populateStyle(JSContext *cx, const char *data,
    size_t len, const char *filename)
{
    JS::Value ret;
    if (!NativeJS::LoadScriptReturn(cx, data, len, filename, &ret)) {
        return false;
    }
    JSObject *jret = ret.toObjectOrNull();

    NativeJS::copyProperties(cx, jret, this->stylesheet);

    return true;
}

void NativeJSdocument::registerObject(JSContext *cx)
{
    JSObject *documentObj;
    NativeJSdocument *jdoc = new NativeJSdocument();
    NativeJS *njs = NativeJS::getNativeClass(cx);

    documentObj = JS_DefineObject(cx, JS_GetGlobalObject(cx),
        NativeJSdocument::getJSObjectName(),
        &document_class , NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE);

    JS_SetPrivate(documentObj, jdoc);
    jdoc->jsobj = documentObj;

    /* We have to root it since the user can replace the document object */
    njs->rootObjectUntilShutdown(documentObj);
    njs->jsobjects.set(NativeJSdocument::getJSObjectName(), documentObj);

    jdoc->stylesheet = JS_NewObject(cx, NULL, NULL, NULL);
    jsval obj = OBJECT_TO_JSVAL(jdoc->stylesheet);
    JS_SetProperty(cx, documentObj, "stylesheet", &obj);
    JS_DefineFunctions(cx, documentObj, document_funcs);
    JS_DefineProperties(cx, documentObj, document_props);
}

