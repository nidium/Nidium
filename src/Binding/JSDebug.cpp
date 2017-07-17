/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSDebug.h"
#include "Binding/NidiumJS.h"

#include <string.h>

namespace Nidium {
namespace Binding {


//  {{{ Implementation
bool JSDebug::JS_serialize(JSContext *cx, JS::CallArgs &args)
{
    uint64_t *data;
    size_t data_len;

    if (!JS_WriteStructuredClone(cx, args[0], &data, &data_len, NidiumJS::m_JsScc,
                                 NidiumJS::GetObject(cx),
                                 JS::NullHandleValue)) {
        JS_ReportError(cx, "serialize() failed");
        return false;
    }

    /*
        JSAPI will takes ownership of this and free it
    */
    char *content = (char *)malloc(data_len);
    memcpy(content, data, data_len);

    JS_ClearStructuredClone(data, data_len, NidiumJS::m_JsScc, nullptr);

    JS::RootedObject arraybuffer(
        cx, JS_NewArrayBufferWithContents(cx, data_len, content));

    args.rval().setObjectOrNull(arraybuffer);

    return true;
}

bool JSDebug::JS_unserialize(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedObject objdata(cx);
    uint32_t offset = 0;

    if (!JS_ConvertArguments(cx, args, "o/u", objdata.address(), &offset)) {
        return false;
    }

    if (!objdata || !JS_IsArrayBufferObject(objdata)) {
        JS_ReportError(cx, "unserialize() invalid data (not an ArrayBuffer)");
        return false;
    }
    uint32_t len  = JS_GetArrayBufferByteLength(objdata);
    

    JS::RootedValue inval(cx);

    if (offset >= len) {
        JS_ReportError(cx, "unserialize() offset overflow");
        return false;
    }

    uint8_t *data;
    {
        /*
            New scope here to comply with AutoCheckCannotGC.
            We're not using the data after JS_ReadStructuredClone()
        */
        JS::AutoCheckCannotGC nogc;
        bool shared;
        data = JS_GetArrayBufferData(objdata, &shared, nogc);
    }

    if (!JS_ReadStructuredClone(cx, (uint64_t *)(data + offset), len - offset,
                                JS_STRUCTURED_CLONE_VERSION, &inval,
                                NidiumJS::m_JsScc,
                                NidiumJS::GetObject(cx))) {
        JS_ReportError(cx, "unserialize() invalid data");
        return false;
    }

    args.rval().set(inval);

    return true;
}


// }}}

JSFunctionSpec *JSDebug::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSDebug, serialize, 1),
        CLASSMAPPER_FN(JSDebug, unserialize, 1),
        JS_FS_END
    };

    return funcs;
}

// {{{ Registration
void JSDebug::RegisterObject(JSContext *cx)
{
    JSDebug::ExposeClass(cx, "NidiumDebug");
    JSDebug::CreateUniqueInstance(cx, new JSDebug(), "Debug");
}
// }}}

} // namespace Binding
} // namespace Nidium
