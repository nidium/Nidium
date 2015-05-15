#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeJSWebSocket.h>

TEST(NativeJSWebsocket, Simple)
{

	JSObject *obj;
	ape_global * g_ape = native_netlib_init();
	NativeJS njs(g_ape);
	NativeJSWebSocketServer::registerObject(njs.cx);
	
	obj = JS_GetGlobalObject(njs.cx);
	NativeJSWebSocketServer wss(obj, njs.cx, "localhost", 8888);;
	
	wss.start();
	ape_running = 0;
	events_loop(g_ape);
	//@TODO: wss.onMessage;
		//native_netlib_destroy(g_ape);
}

