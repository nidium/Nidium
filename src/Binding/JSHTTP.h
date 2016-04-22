/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jshttp_h__
#define binding_jshttp_h__

#include <ape_netlib.h>
#include <ape_array.h>

#include "JSExposer.h"
#include "Net/HTTP.h"

namespace Nidium {
namespace Binding {

class JSHTTP : public JSExposer<JSHTTP>, public Nidium::Net::HTTPDelegate
{
  public:
    static void registerObject(JSContext *cx);
    JSHTTP(JS::HandleObject obj, JSContext *cx, char *url);
    virtual ~JSHTTP();

    JS::Heap<JS::Value> request;
    JS::Heap<JSObject *>jsobj;

    Nidium::Net::HTTP *refHttp;

    void onRequest(Nidium::Net::HTTP::HTTPData *h, Nidium::Net::HTTP::DataType);
    void onProgress(size_t offset, size_t len,
        Nidium::Net::HTTP::HTTPData *h, Nidium::Net::HTTP::DataType);
    void onError(Nidium::Net::HTTP::HTTPError err);
    void onHeader() {};

    bool m_Eval;
    char *m_URL;
};

} // namespace Binding
} // namespace Nidium
#endif

