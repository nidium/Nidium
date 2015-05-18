#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJSConsole.h>

TEST(NativeJSConsole, Simple)
{
	JSObject *globalObj;
	ape_global * g_ape = native_netlib_init();
	NativeJS njs(g_ape);
	jsval rval;
	bool success;

	globalObj = JS_GetGlobalObject(njs.cx);

	rval = JSVAL_VOID;
	success = JS_GetProperty(njs.cx, globalObj, "console", &rval);
	EXPECT_TRUE(JSVAL_IS_VOID(rval) == true);

	//FIXME: naming convention NativeJS-C-onsole and uniform constructor
	NativeJSconsole::registerObject(njs.cx);

	rval = JSVAL_VOID;
	success = JS_GetProperty(njs.cx, globalObj, "console", &rval);
	EXPECT_TRUE(success == true);
	EXPECT_TRUE(JSVAL_IS_VOID(rval) == false);

	//native_netlib_destroy(g_ape);
}

