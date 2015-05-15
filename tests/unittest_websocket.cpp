#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeWebSocket.h>

TEST(NativeWebsocket, Simple)
{
	ape_global * g_ape;
	ape_socket *socket;

	g_ape = native_netlib_init();
	ape_running = g_ape->is_running = 0;
	socket = APE_socket_new(APE_SOCKET_PT_UDP, 0, g_ape);
	NativeWebSocketListener wsl(8080);
	//@FIXME: EXPECT_EQ(wsl.EventID, 4);
	
	//@TODO: NativeWebSocketClientConnection wsc(&wsl, socket);
	//@TODO: wsc.setData((void*)"eeee");
	//@TODO: dummy = (char*) wsc.getData();
	//@TODO: void onFrame(const char *data, size_t len, bool binary);
	//@TODO: void write(unsigned char *data, size_t len,
	//@TODO: virtual void onHeaderEnded();
	//@TODO: virtual void onDisconnect(ape_global *ape);
	//@TODO: virtual void onUpgrade(const char *to);
	//@TODO: virtual void onContent(const char *data, size_t len);

	//native_netlib_destroy(g_ape);
}

