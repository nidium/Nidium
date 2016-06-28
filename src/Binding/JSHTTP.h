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

    bool request(JSContext *cx, JS::HandleObject options, 
            JS::HandleValue callback=JS::NullHandleValue);
    void onRequest(Net::HTTP::HTTPData *h, Nidium::Net::HTTP::DataType);
    void onProgress(size_t offset, size_t len,
        Net::HTTP::HTTPData *h, Nidium::Net::HTTP::DataType);
    void onError(Net::HTTP::HTTPError err);
    void onError(const char *err);
    void onError(int code, const char *err);
    void onHeader();

	void fireJSEvent(const char *name, JS::MutableHandleValue ev);
	void parseOptions(JSContext *cx, JS::HandleObject options);
	Net::HTTPRequest *getRequest(JSContext *cx);

    Net::HTTP *m_HTTP = nullptr;

    JS::Heap<JS::Value> m_JSCallback;
    JS::Heap<JSObject *> m_JSObj;

    bool m_Eval = true;
    char *m_URL = nullptr;
	Net::HTTPRequest *m_HTTPRequest = nullptr;
  private:
	void headersToJSObject(JS::MutableHandleObject obj);
};

} // namespace Binding
} // namespace Nidium

#endif

