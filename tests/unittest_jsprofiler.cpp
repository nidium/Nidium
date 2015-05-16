#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJS.h>
#include <NativeJSProfiler.h>

//@TODO: ? RegisterObject("NativeProfile");

TEST(NativeJSProfiler, Init)
{
	ape_global * g_ape = native_netlib_init();
	NativeJS njs(g_ape);
	//JSObject * globalObj = JS_GetGlobalObject(njs.cx);
	JSObject * obj;
	jsval rval;

	NativeProfiler *np = NativeProfiler::getInstance(njs.cx);

	np->start("dummy");
	np->stop();

	obj = np->toJSObject();
	EXPECT_TRUE(obj != NULL);
	
	obj = np->getJSObject();
	EXPECT_TRUE(obj != NULL);

	rval = JSVAL_VOID;
	JS_GetProperty(njs.cx, obj, "toJSObject", &rval);
	EXPECT_TRUE(!JSVAL_IS_VOID(rval));

	rval = JSVAL_VOID;
	JS_GetProperty(njs.cx, obj, "dump", &rval);
	EXPECT_TRUE(!JSVAL_IS_VOID(rval));

	//@TODO: toCacheGrind

	//native_netlib_destroy(g_ape);
}

TEST(NativeJSProfiler, InitEntry)
{
	char * sigDummy = strdup("object");
	char * sig;
	ape_global * g_ape = native_netlib_init();
	NativeJS njs(g_ape);
	NativeProfileEntry pe("file.js", "madFunc", 12, sigDummy, NULL);
	EXPECT_EQ(pe.getTotalTime(), 0);
	EXPECT_EQ(pe.getTotalTSC(), 0);
	EXPECT_EQ(pe.getTotalCall(), 0);
	EXPECT_EQ(pe.getLine(), 12);
	EXPECT_TRUE(strcmp(pe.getScript(), "file.js") == 0);
	EXPECT_TRUE(strcmp(pe.getFunction(), "madFunc") == 0);
	EXPECT_TRUE(strcmp(pe.getSignature(), sigDummy) == 0);
	EXPECT_TRUE(pe.getParent() == NULL);
	sig = pe.generateSignature("di.js", "sub", 121 );
	EXPECT_TRUE(strcmp(sig, "di.js:sub@121") == 0);
	pe.enter();
	pe.exit();
	EXPECT_TRUE(pe.getTotalTime()> 0);
	EXPECT_TRUE(pe.getTotalTSC() > 0);
	EXPECT_TRUE(pe.getTotalCall()> 0);

	JSObject *obj = pe.toJSObject(njs.cx);
	EXPECT_TRUE(obj != NULL);

	//@TODO: addChild
	//@TODO: toCacheGrind
	
	free(sig);
	free(sigDummy);
	//native_netlib_destroy(g_ape);
} 

TEST(NativeJSProfiler, InitChild)
{
	char * sigDummy = strdup("object");
	ape_global * g_ape = native_netlib_init();
	NativeJS njs(g_ape);
	NativeProfileEntry pe("file.js", "madFunc", 12, sigDummy, NULL);
	NativeProfileChildEntry ce(&pe, 13);
	EXPECT_TRUE(ce.getEntry() == &pe);
	EXPECT_EQ(ce.getTotalTime(), 0);
	EXPECT_EQ(ce.getTotalTSC(), 0);
	EXPECT_EQ(ce.getTotalCall(), 0);
	EXPECT_EQ(ce.getCallLine(), 13);
	ce.update(14, 16);
	EXPECT_EQ(ce.getTotalTime(), 14);
	EXPECT_EQ(ce.getTotalTSC(), 16);
	EXPECT_EQ(ce.getTotalCall(), 1);

	JSObject *obj = pe.toJSObject(njs.cx);
	EXPECT_TRUE(obj != NULL);
	//@TODO:toCacheGrind

	free(sigDummy);
	//native_netlib_destroy(g_ape);
} 
