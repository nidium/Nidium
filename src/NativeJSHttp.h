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

    void requestEnded();

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
        buffer *data;
    } http;
};

typedef enum _native_http_data_type {
    NATIVE_DATA_STRING = 1,
    NATIVE_DATA_BINARY,
    NATIVE_DATA_IMAGE,
    NATIVE_DATA_JSON,
    NATIVE_DATA_NULL,
    NATIVE_DATA_END
} native_http_data_type;


#endif