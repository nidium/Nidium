/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "JSKeyboard.h"

#include "JSModules.h"
#include "Interface/SystemInterface.h"

namespace Nidium {
namespace Binding {

void JSKeyboard::RegisterObject(JSContext *cx)
{
    JSModules::RegisterEmbedded("Keyboard", JSKeyboard::RegisterModule);
}

JSObject *JSKeyboard::RegisterModule(JSContext *cx)
{
    JS::RootedObject exports(cx, JSKeyboard::ExposeObject(cx, "Keyboard"));

    JS_DefineConstDoubles(cx, exports, JSKeyboard::ListConstDoubles());

    return exports;
}

JSConstDoubleSpec *JSKeyboard::ListConstDoubles()
{
    static JSConstDoubleSpec constDoubles[] = {
        {"OPTION_NORMAL",      kOption_Normal},
        {"OPTION_NUMERIC",     kOption_Numeric},
        {"OPTION_TEXT_ONLY",   kOption_TextOnly},
        { nullptr, 0}
    };

    return constDoubles;
}

JSFunctionSpec *JSKeyboard::ListStaticMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN_STATIC(JSKeyboard, show, 0),
        CLASSMAPPER_FN_STATIC(JSKeyboard, hide, 0),
        JS_FS_END
    };

    return funcs;
}


bool JSKeyboard::JSStatic_show(JSContext *cx, JS::CallArgs &args)
{
    Interface::SystemInterface *system = Interface::SystemInterface::GetInstance();
    int flags = 0;

    if (args.length() > 0 && args[0].isNumber()) {
        flags = args[0].toNumber();
    }

    system->showVirtualKeyboard(flags);

    return true;
}

bool JSKeyboard::JSStatic_hide(JSContext *cx, JS::CallArgs &args)
{
    Interface::SystemInterface::GetInstance()->showVirtualKeyboard(false);

    return true;
}

} // namespace Binding
} // namespace Nidium

