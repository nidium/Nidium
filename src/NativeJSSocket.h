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

typedef struct _native_socket
{
    const char *host;
    unsigned short port;
    ape_socket *socket;
    int flags;

} native_socket;


class NativeJSSocket : public NativeJSExposer
{
  public:
	static void registerObject(JSContext *cx);
	NativeJSSocket(const char *host, unsigned short port);
	~NativeJSSocket();

	void write(unsigned char *data, size_t len,
		ape_socket_data_autorelease data_type);

	void destroy();
	void shutdown();

	void dettach();
	bool isAttached();

	bool isJSCallable();

	JSObject *jsobject;
	const char *host;
	unsigned short port;
	ape_socket *socket;
	int flags;
};

#endif