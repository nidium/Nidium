#ifndef nativejshttp_h__
#define nativejshttp_h__

#include "NativeJSExposer.h"
#include <native_netlib.h>

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

};


#endif