/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jshttp_h__
#define binding_jshttp_h__

#include <ape_netlib.h>
#include <ape_array.h>

#include "Binding/JSExposer.h"
#include "Net/HTTP.h"

namespace Nidium {
namespace Binding {

class JSHTTP : public JSExposer<JSHTTP>, public Nidium::Net::HTTPDelegate
{
  public:
    static void RegisterObject(JSContext *cx);
    JSHTTP(JS::HandleObject obj, JSContext *cx, char *url);
    virtual ~JSHTTP();

    JS::Heap<JS::Value> request;
    JS::Heap<JSObject *>jsobj;

    Net::HTTP *refHttp;

    void onRequest(Net::HTTP::HTTPData *h, Nidium::Net::HTTP::DataType);
    void onProgress(size_t offset, size_t len,
        Net::HTTP::HTTPData *h, Nidium::Net::HTTP::DataType);
    void onError(Net::HTTP::HTTPError err);
    void onHeader() {};

    bool m_Eval;
    char *m_URL;
};

} // namespace Binding
} // namespace Nidium

#endif

