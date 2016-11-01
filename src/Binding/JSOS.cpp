/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include "JSOS.h"
#include "JSUtils.h"
#include "JSModules.h"
#ifdef NIDIUM_PRODUCT_FRONTEND
#include "Interface/SystemInterface.h"
#endif

namespace Nidium {
namespace Binding {


void JSOS::RegisterObject(JSContext *cx)
{
    JSModules::RegisterEmbedded("OS", JSOS::RegisterModule);
}

JSObject *JSOS::RegisterModule(JSContext *cx)
{
    JS::RootedObject exports(cx, JS_NewObject(cx, nullptr));

    JSOS::ExposeClass<1>(cx, "OS", 0, JSOS::kEmpty_ExposeFlag, exports);

    // Platform
    JS::RootedString platformStr(cx, JSUtils::NewStringWithEncoding(cx,
                                      NIDIUM_PLATFORM, strlen(NIDIUM_PLATFORM), "utf8"));
    JS::RootedValue platform(cx, JS::StringValue(platformStr));

    JS_DefineProperty(cx, exports, "platform", platform,
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);

    // Language
#ifdef NIDIUM_PRODUCT_FRONTEND
    Interface::SystemInterface* interface = Interface::SystemInterface::GetInstance();
    const char *clang = interface->getLanguage();
    JS::RootedString langStr(cx, JSUtils::NewStringWithEncoding(cx,
                                      clang, strlen(clang), "utf8"));
    JS::RootedValue lang(cx, JS::StringValue(langStr));

    JS_DefineProperty(cx, exports, "language", lang,
                      JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
#endif

    return exports;
}

} // namespace Binding
} // namespace Nidium

