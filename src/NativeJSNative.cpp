#include "NativeJSNative.h"
#include "NativeJSCanvas.h"
#include "NativeJS.h"
#include "NativeCanvasHandler.h"
#include "NativeContext.h"
#include "NativeUIInterface.h"
#include "NativeSystemInterface.h"
#include <jsstr.h>


static void Native_Finalize(JSFreeOp *fop, JSObject *obj);

static JSClass Native_class = {
    "native", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Native_Finalize,
	JSCLASS_NO_OPTIONAL_MEMBERS
};

JSClass *NativeJSNative::jsclass = &Native_class;
template<>
JSClass *NativeJSExposer<NativeJSNative>::jsclass = &Native_class;


static JSFunctionSpec Native_funcs[] = {
    JS_FS_END
};

void Native_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSNative *jnative = NativeJSNative::getNativeClass(obj);

    if (jnative != NULL) {
        delete jnative;
    }
}

void NativeJSNative::registerObject(JSContext *cx)
{
    //JS::RootedObject titleBar(cx);
    JS::RootedObject NativeObj(cx, JS_DefineObject(cx, JS_GetGlobalObject(cx),
        NativeJSNative::getJSObjectName(), &Native_class , NULL,
        JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY));

    NativeJSNative *jnative = new NativeJSNative(NativeObj, cx);

    JS_SetPrivate(NativeObj, jnative);

    NativeJS::getNativeClass(cx)->jsobjects.set(
        NativeJSNative::getJSObjectName(), NativeObj);

    //titleBar = NativeJSCanvas::generateJSObject(cx, width, 35);
    //((NativeCanvasHandler *)JS_GetPrivate(canvas))->translate(0, 35);

    /* Set the newly generated CanvasHandler as first child of rootHandler */
    //NJS->rootHandler->addChild((NativeCanvasHandler *)JS_GetPrivate(titleBar));

    JS_DefineFunctions(cx, NativeObj, Native_funcs);
    /*JS_DefineProperty(cx, NativeObj, "titleBar",
        OBJECT_TO_JSVAL(titleBar), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);*/
}

