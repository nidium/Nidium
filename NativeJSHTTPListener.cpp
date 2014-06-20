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

#include "NativeJSHTTPListener.h"
static void HTTPListener_Finalize(JSFreeOp *fop, JSObject *obj);

static JSClass HTTPListener_class = {
    "HTTPListener", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, HTTPListener_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static void HTTPListener_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSHTTPListener *server = (NativeJSHTTPListener *)JS_GetPrivate(obj);

    if (server != NULL) {
        delete server;
    }       
}

NativeJSHTTPListener::NativeJSHTTPListener(uint16_t port, const char *ip) :
    NativeHTTPListener(port, ip)

{

}

NativeJSHTTPListener::~NativeJSHTTPListener()
{

}

static JSBool native_HTTPListener_constructor(JSContext *cx,
    unsigned argc, jsval *vp)
{
    uint16_t port;
    JSString *ip_bind = NULL;
    NativeJSHTTPListener *listener;

    if (!JS_IsConstructing(cx, vp)) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    JSObject *ret = JS_NewObjectForConstructor(cx, &HTTPListener_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "c/S",
        &port, &ip_bind)) {
        return false;
    }

    if (ip_bind) {
        JSAutoByteString cip(cx, ip_bind);
        listener = new NativeJSHTTPListener(port, cip.ptr());
    } else {
        listener = new NativeJSHTTPListener(port);
    }

    if (!listener->start()) {
        JS_ReportError(cx, "HTTPListener() couldn't listener on %d", port);
        delete listener;
        return false;
    }

    listener->cx = cx;
    listener->jsobj = ret;

    JS_SetPrivate(ret, listener);

    NativeJSObj(cx)->rootObjectUntilShutdown(ret);

    return true;
}

NATIVE_OBJECT_EXPOSE(HTTPListener)


