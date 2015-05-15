#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJS.h>
#include <NativeJSUtils.h>

TEST(NativeJSUtils, String)
{
	ape_global *g_ape;
	jsval rval;
	char * cstr;
	const char *text = " .c89032 pg98u 2.dhu982 89";
	const char * te = text; //@TODO: encode as utf8;
	JSString *jstr;
	char target[32];
	bool success;

	memset(&target[0], '\0', sizeof(target));
	g_ape = native_netlib_init();
	NativeJS njs(g_ape);

	success = NativeJSUtils::strToJsval(njs.cx, text, strlen(text), &rval, "utf8");
	EXPECT_TRUE(success == true);
	jstr = rval.toString();
	cstr = JS_EncodeString(njs.cx, jstr);
	EXPECT_TRUE(strcmp(te, cstr) == 0);
	free(cstr);

	success = NativeJSUtils::strToJsval(njs.cx, text, strlen(text), &rval, "asciiiiii");
	EXPECT_TRUE(success == true);
	jstr = rval.toString();
	cstr = JS_EncodeString(njs.cx, jstr);
	EXPECT_TRUE(strcmp(te, cstr) == 0);
	free(cstr);

	//@TODO: strToJsVal encoding = NULL => arraybuffer
	
	jstr = NativeJSUtils::newStringWithEncoding(njs.cx, text, strlen(text), "utf8");
	cstr = JS_EncodeString(njs.cx, jstr);
	EXPECT_TRUE(strcmp(te, cstr) == 0);
	free(cstr);
	
	jstr = NativeJSUtils::newStringWithEncoding(njs.cx, text, strlen(text), "asciiiiii");
	cstr = JS_EncodeString(njs.cx, jstr);
	EXPECT_TRUE(strcmp(te, cstr) == 0);
	free(cstr);

	//@TODO: newStringWithEncoding encoding = NULL => arraybuffer
	//@TODO: check buf = NULL or len = 0

	//native_netlib_destroy(g_ape);
}

