/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include "JSOS.h"
#include "JSUtils.h"
#include "JSModules.h"
#ifdef NIDIUM_PRODUCT_FRONTEND
#include <SystemInterface.h>
#endif

namespace Nidium {
namespace Binding {

// {{{ Preamble
static bool jsos_platform(JSContext *cx, unsigned argc, JS::Value *vp);

static JSClass JSOS_class = { "OS",
                              JSCLASS_HAS_PRIVATE,
                              JS_PropertyStub,
                              JS_DeletePropertyStub,
                              JS_PropertyStub,
                              JS_StrictPropertyStub,
                              JS_EnumerateStub,
                              JS_ResolveStub,
                              JS_ConvertStub,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              JSCLASS_NO_INTERNAL_MEMBERS };

template <>
JSClass *JSExposer<JSOS>::jsclass = &JSOS_class;

static JSFunctionSpec JSOS_funcs[] = {
    JS_FS_END
};
// }}}

// {{{ Registration
static JSObject *registerCallback(JSContext *cx)
{
    JS::RootedObject gbl(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedObject exports(cx,
                         JS_NewObject(cx, NULL, JS::NullPtr(), JS::NullPtr()));

    // Platform
    JS::RootedString platformStr(cx, JSUtils::NewStringWithEncoding(cx,
                                      NIDIUM_PLATFORM, strlen(NIDIUM_PLATFORM), "utf8"));
    JS::RootedValue platform(cx, STRING_TO_JSVAL(platformStr));

    JS_DefineProperty(cx, exports, "platform", platform,
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);

    // Language
#ifdef NIDIUM_PRODUCT_FRONTEND
    SystemInterface* interface = SystemInterface::GetInstance();
    const char *clang = interface->getLanguage();
    JS::RootedString langStr(cx, JSUtils::NewStringWithEncoding(cx,
                                      lang, strlen(lang), "utf8"));
    JS::RootedValue lang(cx, STRING_TO_JSVAL(langStr));

    JS_DefineProperty(cx, exports, "language", lang,
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif

    return exports;
}

void JSOS::RegisterObject(JSContext *cx)
{
    JSModules::RegisterEmbedded(JSOS_class.name, registerCallback);
}
// }}}

} // namespace Binding
} // namespace Nidium

