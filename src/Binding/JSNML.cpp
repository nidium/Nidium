/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSNML.h"
#include "Frontend/NML.h"


namespace Nidium {
namespace Binding {

bool JSNML::JSStatic_parse(JSContext *cx, JS::CallArgs &args)
{
    if (!args[0].isString()) {
        JS_ReportError(cx, "parse() first argument must be a string");
        return false;
    }

    JSAutoByteString str;

    str.encodeUtf8(cx, JS::RootedString(cx, args[0].toString()));

    JS::RootedObject tree(cx, Frontend::NML::BuildLST(cx, str.ptr()));

    NDM_REPORT_PENDING_EXCEPTION(cx)

    args.rval().setObjectOrNull(tree);

    return true;
}

JSFunctionSpec *JSNML::ListStaticMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN_STATIC(JSNML, parse, 1),
        JS_FS_END
    };

    return funcs;
}


void JSNML::RegisterObject(JSContext *cx)
{
    JSNML::ExposeClass<0>(cx, "NML");
}


} // namespace Binding
} // namespace Nidium
