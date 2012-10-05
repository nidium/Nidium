#ifndef nativejssocket_h__
#define nativejssocket_h__

#include "NativeJSExposer.h"
#include <native_netlib.h>

enum {
    NATIVE_SOCKET_ISBINARY = 1 << 0
};

enum {
    SOCKET_PROP_BINARY
};


class NativeJSSocket : public NativeJSExposer
{
  public:
    static void registerObject(JSContext *cx);
    NativeJSSocket(const char *host, unsigned short port);
    ~NativeJSSocket();

    void write(unsigned char *data, size_t len,
        ape_socket_data_autorelease data_type);

    void disconnect();
    void shutdown();

    void dettach();
    bool isAttached();

    bool isJSCallable();

    JSObject *jsobject;
    char *host;
    unsigned short port;
    ape_socket *socket;
    int flags;
};

#endif