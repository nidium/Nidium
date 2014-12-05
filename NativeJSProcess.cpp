/*
    NativeJS Core Library
    Copyright (C) 2014 Anthony Catel <paraboul@gmail.com>

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

#include "NativeJSProcess.h"
#include "NativeJS.h"

static void Process_Finalize(JSFreeOp *fop, JSObject *obj);


static JSClass Process_class = {
    "NativeProcess", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Process_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

JSClass *NativeJSProcess::jsclass = &Process_class;

template<>
JSClass *NativeJSExposer<NativeJSProcess>::jsclass = &Process_class;


static JSFunctionSpec Process_funcs[] = {

    JS_FS_END
};

static void Process_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSProcess *jProcess = NativeJSProcess::getNativeClass(obj);

    if (jProcess != NULL) {
        delete jProcess;
    }
}


void NativeJSProcess::registerObject(JSContext *cx, char **argv, int argc)
{
    JSObject *ProcessObj;
    
    NativeJS *njs = NativeJS::getNativeClass(cx);

    ProcessObj = JS_DefineObject(cx, JS_GetGlobalObject(cx),
        NativeJSProcess::getJSObjectName(),
        &Process_class , NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    NativeJSProcess *jProcess = new NativeJSProcess(ProcessObj, cx);

    JS_SetPrivate(ProcessObj, jProcess);

    njs->jsobjects.set(NativeJSProcess::getJSObjectName(), ProcessObj);

    JS_DefineFunctions(cx, ProcessObj, Process_funcs);

    JSObject *jsargv = JS_NewArrayObject(cx, argc, NULL);

    for (int i = 0; i < argc; i++) {
        jsval jelem = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, argv[i]));

        JS_SetElement(cx, jsargv, i, &jelem);
    }

    JS::Value jsargv_v = OBJECT_TO_JSVAL(jsargv);
    JS_SetProperty(cx, ProcessObj, "argv", &jsargv_v);
}

