/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "JSVM.h"
#include "JSModules.h"
#include <js/CharacterEncoding.h>

namespace Nidium {
namespace Binding {

bool JSVM::JSStatic_runInScope(JSContext *cx, JS::CallArgs &args)
{
    if (!args[0].isString()) {
        JS_ReportError(cx, "runInScope() first argument must be a string");
        return false;
    }

    if (!args[1].isObject()) {
        JS_ReportError(cx, "runInScope() second argument must be an object");
        return false;        
    }

    JS::AutoObjectVector scopeChain(cx);

    scopeChain.append(&args[1].toObject());

    JSAutoByteString code;
    code.encodeUtf8(cx, JS::RootedString(cx, args[0].toString()));

    JS::CompileOptions options(cx);
    options.setUTF8(true)
        .setFileAndLine("VM.runInScope()", 1);

    size_t length = code.length();
    char16_t* chars = JS::UTF8CharsToNewTwoByteCharsZ(cx,
        JS::UTF8Chars(code.ptr(), length), &length).get();

    JS::RootedValue rval(cx);

    return JS::Evaluate(cx, scopeChain, options, chars, length, args.rval());
}



JSFunctionSpec *JSVM::ListStaticMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN_STATIC(JSVM, runInScope, 2),
        JS_FS_END
    };

    return funcs;
}

void JSVM::RegisterObject(JSContext *cx)
{
    JSModules::RegisterEmbedded("vm", JSVM::RegisterModule);
}

JSObject *JSVM::RegisterModule(JSContext *cx)
{
    return JSVM::ExposeObject(cx, "vm");
}

} // namespace Binding
} // namespace Nidium

