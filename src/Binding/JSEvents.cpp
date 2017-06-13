/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSEvents.h"

#include "Binding/JSMacros.h"

namespace Nidium {
namespace Binding {


JSObject *JSEvents::CreateEventObject(JSContext *cx)
{
    return JSEvent::CreateObject(cx);
}

JSObject * JSEvents::CreateErrorEventObject(JSContext *cx,
    int code, const char *err)
{
    JSContext *m_Cx = cx;
    JS::RootedObject ret(cx, JSEvent::CreateObject(cx));

    NIDIUM_JSOBJ_SET_PROP_INT(ret, "errorCode", code);
    NIDIUM_JSOBJ_SET_PROP_CSTR(ret, "error", err);

    return ret;
}

bool JSEvent::JS_stopPropagation(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedObject inst(cx, m_Instance);

    JS::RootedValue cancelBubble(cx, JS::BooleanValue(true));
    if (!JS_SetProperty(cx, inst, "cancelBubble", cancelBubble)) {
        return false;
    }

    return true;
}

bool JSEvent::JS_preventDefault(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedObject inst(cx, m_Instance);
    JS::RootedValue defaultPrevented(cx, JS::BooleanValue(true));

    if (!JS_SetProperty(cx, inst, "defaultPrevented", defaultPrevented)) {
        return false;
    }

    return true;
}

bool JSEvent::JS_forcePropagation(JSContext *cx, JS::CallArgs &args)
{
    return true;
}


void JSEvent::RegisterObject(JSContext *cx)
{
    JSEvent::ExposeClass(cx, "JSEvent");
}

JSFunctionSpec *JSEvent::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSEvent, stopPropagation, 0),
        CLASSMAPPER_FN(JSEvent, forcePropagation, 0),
        CLASSMAPPER_FN(JSEvent, preventDefault, 0),
        JS_FS_END
    };

    return funcs;
}

} // namespace Binding
} // namespace Nidium
