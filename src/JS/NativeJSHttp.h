/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejshttp_h__
#define nativejshttp_h__

#include <native_netlib.h>
#include <ape_array.h>

#include "NativeJSExposer.h"
#include "Net/NativeHTTP.h"


class NativeJSHttp : public NativeJSExposer<NativeJSHttp>, public Native::Core::HTTPDelegate
{
  public:
    static void registerObject(JSContext *cx);
    NativeJSHttp(JS::HandleObject obj, JSContext *cx, char *url);
    virtual ~NativeJSHttp();

    JS::Heap<JS::Value> request;
    JS::Heap<JSObject *>jsobj;

    Native::Core::HTTP *refHttp;

    void onRequest(Native::Core::HTTP::HTTPData *h, Native::Core::HTTP::DataType);
    void onProgress(size_t offset, size_t len,
        Native::Core::HTTP::HTTPData *h, Native::Core::HTTP::DataType);
    void onError(Native::Core::HTTP::HTTPError err);
    void onHeader() {};

    bool m_Eval;
    char *m_URL;
};

#endif

