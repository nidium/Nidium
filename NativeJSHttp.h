/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejshttp_h__
#define nativejshttp_h__

#include <native_netlib.h>
#include <ape_array.h>

#include "NativeHTTP.h"
#include "NativeJSExposer.h"

class NativeJSHttp : public NativeJSExposer<NativeJSHttp>, public NativeHTTPDelegate
{
  public:
    static void registerObject(JSContext *cx);
    NativeJSHttp(JS::HandleObject obj, JSContext *cx, char *url);
    virtual ~NativeJSHttp();

    JS::Heap<JS::Value> request;
    JS::Heap<JSObject *>jsobj;

    NativeHTTP *refHttp;

    void onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType);
    void onProgress(size_t offset, size_t len,
        NativeHTTP::HTTPData *h, NativeHTTP::DataType);
    void onError(NativeHTTP::HTTPError err);
    void onHeader() {};

    bool m_Eval;
    char *m_URL;
};

#endif

