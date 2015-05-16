#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeHTTPStream.h>

TEST(NativeHttpStream, Simple)
{
	ape_global * g_ape;

	g_ape = native_netlib_init();
	ape_running = g_ape->is_running = 1;
#if 0
	NativeHTTPStream http("http://www.nidium.com");

	//EXPECT_TRUE(strcmp(http.getBaseDir() == NULL);
	//EXPECT_TRUE(strcmp(http.allowLocalFileStream() == NULL);
	//EXPECT_TRUE(strcmp(http.allowSyncStream() == NULL);
	//@TODO: EXPECT_TRUE(strcmp(http.getPath(), "/") == 0);
	
	//@TODO: http.createStream();
	//@TODO: http.stop();
	//@TODO: http.getContent();
	//@TODO: http.seek();
	//@TODO: http.getFileSize();
	//@TODO: http.hasDataAvailable();
#endif
	//native_netlib_destroy(g_ape);
}

