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

static JSBool native_httprequest_write(JSContext *cx,
    unsigned argc, jsval *vp);

static JSClass HTTPListener_class = {
    "HTTPListener", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, HTTPListener_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass HTTPClientConnection_class = {
    "HTTPClientConnection", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub
};

static JSClass HTTPRequest_class = {
    "HTTPRequest", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub
};


static JSFunctionSpec HTTPRequest_funcs[] = {
    JS_FN("write", native_httprequest_write, 1, 0),
    JS_FS_END
};

static JSPropertySpec HTTPRequest_props[] = {
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
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


void NativeJSHTTPListener::onClientDisconnect(NativeHTTPClientConnection *client)
{
    JS_RemoveObjectRoot(cx, (JSObject **)&client->m_Ctx);
    client->setContext(NULL);
}

void NativeJSHTTPListener::onData(NativeHTTPClientConnection *client,
    const char *buf, size_t len)
{
    // on progress
}

bool NativeJSHTTPListener::onEnd(NativeHTTPClientConnection *client)
{
    JS::Value oncallback, rval;
    buffer *k, *v;
    JSObject *objclient = JS_NewObject(cx, &HTTPClientConnection_class, NULL, NULL);
    JSObject *objrequest = JS_NewObject(cx, &HTTPRequest_class, NULL, NULL);

    client->setContext(objclient);

    JS_AddObjectRoot(cx, (JSObject **)&client->m_Ctx);

    JSObject *headers = JS_NewObject(cx, NULL, NULL, NULL);

    APE_A_FOREACH(client->getHTTPState()->headers.list, k, v) {
        JSString *jstr = JS_NewStringCopyN(cx, (char *)v->data,
            v->used-1);
        JSOBJ_SET_PROP(headers, k->data, STRING_TO_JSVAL(jstr));
    }

    JSOBJ_SET_PROP(objrequest, "headers", OBJECT_TO_JSVAL(headers));
    JSOBJ_SET_PROP(objrequest, "client", OBJECT_TO_JSVAL(objclient));

    if (JS_GetProperty(cx, this->jsobj, "onrequest", &oncallback) &&
        JS_TypeOfValue(cx, oncallback) == JSTYPE_FUNCTION) {
        JS::Value arg[2];
        arg[0].setObjectOrNull(objrequest);
        arg[1].setObjectOrNull(static_cast<NativeJSHTTPResponse*>(client->getResponse())->getJSObject());
        JS_CallFunctionValue(cx, this->jsobj, oncallback,
            2, arg, &rval);
    }

    return false;
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
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    NativeJSObj(cx)->rootObjectUntilShutdown(ret);

    return true;
}

static JSBool native_HTTPRequest_class_constructor(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JS_ReportError(cx, "Illegal constructor");
    return false;
}

static JSBool native_httprequest_write(JSContext *cx, unsigned argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    
    return true;
}

void NativeJSHTTPListener::registerObject(JSContext *cx)
{
    JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, &HTTPListener_class,
        native_HTTPListener_constructor,
        0, NULL, NULL, NULL, NULL);

    JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, &HTTPRequest_class,
                native_HTTPRequest_class_constructor,
                0, HTTPRequest_props, HTTPRequest_funcs, NULL, NULL);    
}
