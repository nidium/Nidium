#ifndef nativejshttp_h__
#define nativejshttp_h__

#include "NativeJSExposer.h"
#include <native_netlib.h>
#include "ape_http_parser.h"
#include "ape_array.h"
#include "NativeHTTP.h"

class NativeJSHttp : public NativeJSExposer, public NativeHTTPDelegate
{
  public:
    static void registerObject(JSContext *cx);
    NativeJSHttp();
    ~NativeJSHttp();

    jsval request;

    void onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType);
    
};


#endif