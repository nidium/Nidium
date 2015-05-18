#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJSDebug.h>

TEST(NativeJSDebug, Simple)
{
	JSObject *globalObj;
	ape_global * g_ape = native_netlib_init();
	NativeJS njs(g_ape);
	jsval rval;
	bool success;

	globalObj = JS_GetGlobalObject(njs.cx);

	rval = JSVAL_VOID;
	success = JS_GetProperty(njs.cx, globalObj, "Debug", &rval);
	EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

	NativeJSDebug::registerObject(njs.cx);

	rval = JSVAL_VOID;
	success = JS_GetProperty(njs.cx, globalObj, "Debug", &rval);
	EXPECT_TRUE(success == true);
	EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);
	
	//native_netlib_destroy(g_ape);
}

TEST(NativeJSDebug, Init)
{
	ape_global * g_ape = native_netlib_init();
	NativeJS njs(g_ape);
	JSObject * globalObj = JS_GetGlobalObject(njs.cx);

	NativeJSDebug nd(globalObj, njs.cx);

	EXPECT_TRUE(nd.getJSObject() == globalObj);
	EXPECT_TRUE(nd.getJSContext() == njs.cx);

	EXPECT_TRUE(strcmp(nd.getJSObjectName(), "Debug") == 0);

	//native_netlib_destroy(g_ape);
}
