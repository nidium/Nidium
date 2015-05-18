#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJSHttp.h>

TEST(NativeJSHTTP, Simple)
{
	JSObject *globalObj;
	ape_global * g_ape = native_netlib_init();
	NativeJS njs(g_ape);
	jsval rval;
	bool success;

	globalObj = JS_GetGlobalObject(njs.cx);

	rval = JSVAL_VOID;
	success = JS_GetProperty(njs.cx, globalObj, "Http", &rval);
	EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

	NativeJSHttp::registerObject(njs.cx);

	rval = JSVAL_VOID;
	success = JS_GetProperty(njs.cx, globalObj, "Http", &rval);
	EXPECT_TRUE(success == true);
	EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);
	
	//@TODO: onRequest
	//@TODO: onProgress
	//@TODO: onError
	//@TODO: onHeader

	//native_netlib_destroy(g_ape);
}

TEST(NativeJSHTTP, init)
{
	ape_global * g_ape = native_netlib_init();
	NativeJS njs(g_ape);
	JSObject * globalObj = JS_GetGlobalObject(njs.cx);
	char * url = strdup("http://nidium.com:80/new.html");

	NativeJSHttp ht(globalObj, njs.cx, url);

	EXPECT_TRUE(ht.getJSObject() == globalObj);
	EXPECT_TRUE(ht.getJSContext() == njs.cx);
	
	EXPECT_TRUE(ht.request == JSVAL_NULL);
	EXPECT_TRUE(ht.refHttp == NULL);
	EXPECT_TRUE(ht.m_Eval == true);
	EXPECT_TRUE(strcmp(ht.m_URL, url) == 0);
	
	free(url);
	//native_netlib_destroy(g_ape);
}
