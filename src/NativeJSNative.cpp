/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "NativeJSNative.h"

static void Native_Finalize(JSFreeOp *fop, JSObject *obj);

static JSClass Native_class = {
    "native", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Native_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
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
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedObject NativeObj(cx, JS_DefineObject(cx, global,
        NativeJSNative::getJSObjectName(), &Native_class , nullptr, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY));

    NativeJSNative *jnative = new NativeJSNative(NativeObj, cx);
    JS_DefineFunctions(cx, NativeObj, Native_funcs);
    JS_SetPrivate(NativeObj, jnative);

    NativeJS::getNativeClass(cx)->jsobjects.set(NativeJSNative::getJSObjectName(), NativeObj);

    //JS::RootedObject titleBar(cx, NativeJSCanvas::generateJSObject(cx, width, 35));
    //((NativeCanvasHandler *)JS_GetPrivate(canvas))->translate(0, 35);

    /* Set the newly generated CanvasHandler as first child of rootHandler */
    //NJS->rootHandler->addChild((NativeCanvasHandler *)JS_GetPrivate(titleBar));

    /*JS::RootedValue titleVal(cx, OBJECT_TO_JSVAL(titleBar));
    JS_DefineProperty(cx, NativeObj, "titleBar", titleVal, JSPROP_READONLY | JSPROP_PERMANENT);*/
}

