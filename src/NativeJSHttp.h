#ifndef nativejshttp_h__
#define nativejshttp_h__

#include "NativeJSExposer.h"
#include <native_netlib.h>
#include "ape_http_parser.h"
#include "ape_array.h"

class NativeJSHttp : public NativeJSExposer
{
  public:
    static void registerObject(JSContext *cx);
    NativeJSHttp();
    ~NativeJSHttp();

    char *host;
    char *path;
    u_short port;

    jsval request;
    
    struct {
        http_parser parser;
        struct {
            ape_array_t *list;
            buffer *tkey;
            buffer *tval;
        } headers;
    } http;
};


#endif