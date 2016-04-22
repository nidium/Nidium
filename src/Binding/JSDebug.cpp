/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "JSDebug.h"

#include <string.h>

namespace Nidium {
namespace Binding {

// {{{ Preamble

static void Debug_Finalize(JSFreeOp *fop, JSObject *obj);
static bool nidium_debug_serialize(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_debug_unserialize(JSContext *cx, unsigned argc, JS::Value *vp);

static JSClass debug_class = {
    "NativeDebug", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Debug_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

JSClass *JSDebug::jsclass = &debug_class;

template<>
JSClass *JSExposer<JSDebug>::jsclass = &debug_class;

static JSFunctionSpec debug_funcs[] = {
    JS_FN("serialize", nidium_debug_serialize, 1, NATIVE_JS_FNPROPS),
    JS_FN("unserialize", nidium_debug_unserialize, 1, NATIVE_JS_FNPROPS),
    JS_FS_END
};
// }}}

//  {{{ Implementation
static bool nidium_debug_serialize(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    NIDIUM_JS_CHECK_ARGS("serialize", 1);
    uint64_t *data;
    size_t data_len;

    if (!JS_WriteStructuredClone(cx, args[0], &data, &data_len,
        NULL, NidiumJS::GetObject(cx), JS::NullHandleValue)) {
        JS_ReportError(cx, "serialize() failed");
        return false;
    }

    void *content;

    if (!(content = JS_AllocateArrayBufferContents(cx, data_len))) {
        JS_ReportOutOfMemory(cx);
        return false;
    }

    memcpy(content, data, data_len);

    JS_ClearStructuredClone(data, data_len, nullptr, nullptr);

    JS::RootedObject arraybuffer(cx, JS_NewArrayBufferWithContents(cx, data_len, content));

    args.rval().setObjectOrNull(arraybuffer);

    return true;
}

static bool nidium_debug_unserialize(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject objdata(cx);
    uint32_t offset = 0;

    if (!JS_ConvertArguments(cx, args, "o/u", objdata.address(), &offset)) {
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
        JS_STRUCTURED_CLONE_VERSION, &inval, NULL, NidiumJS::GetObject(cx))) {
        JS_ReportError(cx, "unserialize() invalid data");
        return false;
    }

    args.rval().set(inval);

    return true;
}

static void Debug_Finalize(JSFreeOp *fop, JSObject *obj)
{
    JSDebug *jdebug = JSDebug::GetObject(obj);

    if (jdebug != NULL) {
        delete jdebug;
    }
}
// }}}

// {{{ Registration
void JSDebug::registerObject(JSContext *cx)
{
    NidiumJS *njs = NidiumJS::GetObject(cx);

    JS::RootedObject debugObj(cx, JS_DefineObject(cx, JS::CurrentGlobalOrNull(cx),
        JSDebug::getJSObjectName(),
        &debug_class , NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY));

    JSDebug *jdebug = new JSDebug(debugObj, cx);

    JS_SetPrivate(debugObj, jdebug);

    njs->jsobjects.set(JSDebug::getJSObjectName(), debugObj);

    JS_DefineFunctions(cx, debugObj, debug_funcs);
}
// }}}

} // namespace Binding
} // namespace Nidium

