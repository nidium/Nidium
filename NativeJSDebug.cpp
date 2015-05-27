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

#include "NativeJSDebug.h"
#include "NativeJS.h"

#include <native_netlib.h>
#include <jsapi.h>
#include <jsstr.h>

static void Debug_Finalize(JSFreeOp *fop, JSObject *obj);
static bool native_debug_serialize(JSContext *cx, unsigned argc, jsval *vp);
static bool native_debug_unserialize(JSContext *cx, unsigned argc, jsval *vp);

static JSClass debug_class = {
    "NativeDebug", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Debug_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

JSClass *NativeJSDebug::jsclass = &debug_class;

template<>
JSClass *NativeJSExposer<NativeJSDebug>::jsclass = &debug_class;


static JSFunctionSpec debug_funcs[] = {
    JS_FN("serialize", native_debug_serialize, 1, 0),
    JS_FN("unserialize", native_debug_unserialize, 1, 0),
    JS_FS_END
};

static void Debug_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSDebug *jdebug = NativeJSDebug::getNativeClass(obj);

    if (jdebug != NULL) {
        delete jdebug;
    }
}

static bool native_debug_serialize(JSContext *cx, unsigned argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    NATIVE_CHECK_ARGS("serialize", 1);
    uint64_t *data;
    size_t data_len;

    if (!JS_WriteStructuredClone(cx, args.array()[0], &data, &data_len,
        NULL, NativeJS::getNativeClass(cx), JSVAL_VOID)) {
        JS_ReportError(cx, "serialize() failed");
        return false;
    }

    void *content;
    uint8_t *cdata;

    if (!JS_AllocateArrayBufferContents(cx, data_len, &content, &cdata)) {
        JS_ReportOutOfMemory(cx);
        return false;
    }

    memcpy(cdata, data, data_len);

    JS_ClearStructuredClone(data, data_len);

    JS::RootedObject arraybuffer(cx, JS_NewArrayBufferWithContents(cx, content));

    args.rval().setObjectOrNull(arraybuffer);

    return true;
}

static bool native_debug_unserialize(JSContext *cx, unsigned argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject objdata(cx);
    uint32_t offset = 0;

    if (!JS_ConvertArguments(cx, args.length(), args.array(), "o/u", objdata.address(), &offset)) {
        return false;
    }

    if (!objdata || !JS_IsArrayBufferObject(objdata)) {
        JS_ReportError(cx, "unserialize() invalid data (not an ArrayBuffer)");
        return false;            
    }
    uint32_t len = JS_GetArrayBufferByteLength(objdata);
    uint8_t *data = JS_GetArrayBufferData(objdata);

    JS::RootedValue inval(cx);

    if (offset >= len) {
        JS_ReportError(cx, "unserialize() offset overflow");
        return false;              
    }

    if (!JS_ReadStructuredClone(cx, (uint64_t *)(data+offset), len-offset,
        JS_STRUCTURED_CLONE_VERSION, inval.address(), NULL, NativeJS::getNativeClass(cx))) {
        JS_ReportError(cx, "unserialize() invalid data");
        return false;             
    }

    args.rval().set(inval);

    return true;
}

void NativeJSDebug::registerObject(JSContext *cx)
{
    NativeJS *njs = NativeJS::getNativeClass(cx);

    JS::RootedObject debugObj(cx, JS_DefineObject(cx, JS_GetGlobalObject(cx),
        NativeJSDebug::getJSObjectName(),
        &debug_class , NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY));

    NativeJSDebug *jdebug = new NativeJSDebug(debugObj, cx);

    JS_SetPrivate(debugObj, jdebug);

    njs->jsobjects.set(NativeJSDebug::getJSObjectName(), debugObj);

    JS_DefineFunctions(cx, debugObj, debug_funcs);
}

