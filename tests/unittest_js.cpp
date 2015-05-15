#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJS.h>

static int logcounter;
static int msgcounter;
void msg_cb_t(JSContext *cx, NativeSharedMessages::Message *msg)
{
	msgcounter++;
	ape_running = 0;
}

int dummyLogger(const char *format)
{
	return logcounter += strcmp(format, "tt %s") ;
}

int dummyVLogger(const char *format, va_list ap)
{
	return logcounter += strcmp(format, "tt %s");
}

int dummyLoggerClear()
{
	logcounter = -10;
	return 0;
}	

TEST(NativeJS, Simple)
{
	int i = 1, *p;
	JSObject *d, *dc;
	ape_global *g_ape;
	jsval rval;
	struct _ape_htable *table;

	g_ape = native_netlib_init();
	NativeJS njs(g_ape);

	//check the init
	EXPECT_TRUE(njs.net == g_ape);
	EXPECT_TRUE(njs.getNet() == g_ape);
	EXPECT_TRUE(njs.cx != NULL);
	EXPECT_TRUE(njs.getJSContext() == njs.cx);
	EXPECT_TRUE(njs.messages != NULL);
	table = njs.jsobjects.accessCStruct();
	EXPECT_TRUE(table != NULL);
	EXPECT_TRUE(njs.registeredMessages != NULL);
	EXPECT_EQ(njs.registeredMessagesIdx, 8);
	EXPECT_EQ(njs.registeredMessagesSize, 16);
	EXPECT_EQ(njs.isShuttingDown(), false);

	//check the private 
	njs.setPrivate(&i);
	p = (int*)njs.getPrivate();
	EXPECT_EQ(p, &i);
	EXPECT_EQ(*p, i);

	//check the path
	EXPECT_TRUE(njs.getPath() == NULL);
	njs.setPath("/tmp/");
	EXPECT_TRUE(strcmp(njs.getPath(), "/tmp/") == 0);

	//some others
	njs.loadGlobalObjects();
	njs.gc();

	//store objects
	d = JS_NewObject(njs.cx, NULL, NULL, NULL);
	rval = INT_TO_JSVAL(1);
	JS_SetProperty(njs.cx, d, "a", &rval);
	njs.rootObjectUntilShutdown(d);
	njs.unrootObject(d);
	dc =JS_NewObject(njs.cx, NULL, NULL, NULL);
	njs.copyProperties(njs.cx, d, dc);
	JS_GetProperty(njs.cx, dc, "a", &rval);
	EXPECT_EQ(JSVAL_TO_INT(rval), 1);

	//check the loggers
	logcounter = 0;
	njs.log("%s");
	EXPECT_EQ(logcounter, 0);

	njs.setLogger(dummyLogger);
	njs.logf("tt %s", "a");
	EXPECT_EQ(logcounter, 0);
	njs.log("%s");
	EXPECT_EQ(logcounter, -1);

	njs.setLogger(dummyVLogger);
	njs.logf("tt %s", "a");
	EXPECT_EQ(logcounter, -1);
	njs.log("%s");
	EXPECT_EQ(logcounter, -2);
	
	njs.logclear();
	EXPECT_EQ(logcounter, -2);
	njs.setLogger(dummyLoggerClear);
	njs.logclear();
	EXPECT_EQ(logcounter, -10);

	ape_running = 0;

	//native_netlib_destroy(g_ape);
}


TEST(NativeJS, Quick)
{
	ape_global *g_ape = NULL;
	g_ape = native_netlib_init();
	NativeJS njs(g_ape);

	njs.initNet(g_ape);
	EXPECT_TRUE(njs.net == g_ape);
	EXPECT_TRUE(njs.getNet() == g_ape);
	
	//native_netlib_destroy(g_ape);
}

TEST(NativeJS, Code)
{	
	ape_global *g_ape = native_netlib_init();
	NativeJS njs(g_ape);
	const char * srcA = "var a = 11*11;";
	const char * srcB = "b = a - 21";
	jsval rval;
	int success;

	success = njs.LoadScriptContent(srcA, strlen(srcA), __FILE__);
	EXPECT_EQ(success, 1);
	JS_GetProperty(njs.cx, JS_GetGlobalObject(njs.cx), "a", &rval);
	EXPECT_EQ(JSVAL_TO_INT(rval), 121);

	success = njs.LoadScriptReturn(njs.cx, srcB, strlen(srcB), __FILE__, &rval);
	EXPECT_EQ(success, 1);
	EXPECT_EQ(JSVAL_TO_INT(rval), 100);
	JS_GetProperty(njs.cx, JS_GetGlobalObject(njs.cx), "b", &rval);
	EXPECT_EQ(JSVAL_TO_INT(rval), 100);

#if 0
	const char * srcC = "var c = b * a";
	const char * srcD = "var d = c - a";
	NativeBytecodeScript bc;

	success = njs.LoadBytecode((void*)srcC, strlen(srcC), __FILE__);
	EXPECT_EQ(success, 0);  // bad script XDR, but comilies
	JS_GetProperty(njs.cx, JS_GetGlobalObject(njs.cx), "c", &rval);
	EXPECT_EQ(JSVAL_TO_INT(rval), 12100);

	bc.name = "dummy";
	bc.size = strlen(srcD);
	bc.data = (const unsigned char*) srcD;
	success = njs.LoadBytecode(&bc);
	EXPECT_EQ(success, 1);
	JS_GetProperty(njs.cx, JS_GetGlobalObject(njs.cx), "d", &rval);
	EXPECT_EQ(JSVAL_TO_INT(rval), 12079);
	
#endif
//@TODO: static int LoadScriptReturn(JSContext *cx, const char *filename, JS::Value *ret);
//@TODO: int LoadScript(const char *filename);

	//native_netlib_destroy(g_ape);
}
TEST(NativeJS, Messages)
{
	ape_global *g_ape = native_netlib_init();
	NativeJS njs(g_ape);
	size_t i;
	
	EXPECT_EQ(njs.registeredMessagesIdx, 8);
	EXPECT_EQ(njs.registeredMessagesSize, 16);
	for( i = 0; i < 8; i++) {	
		EXPECT_TRUE(njs.registeredMessages[i] == NULL);
	}
	njs.registerMessage( msg_cb_t);
	EXPECT_EQ(njs.registeredMessagesIdx, 9);
	EXPECT_EQ(njs.registeredMessagesSize, 16);
	for( i = 10; i < 18; i++) {	
		printf("%d %d %d\n", i, njs.registeredMessagesIdx, njs.registeredMessagesSize);
		njs.registerMessage( msg_cb_t);
		EXPECT_TRUE(njs.registeredMessages[i] != NULL);
	}
	EXPECT_EQ(njs.registeredMessagesIdx, 17);
	EXPECT_EQ(njs.registeredMessagesSize, 32);

	njs.registerMessage( msg_cb_t, 0);
	//@FIXME: njs.registerMessage( msg_cb_t, 8);
	//@FIXME: njs.registerMessage(msg_cb_t, 222);	
#if 0
	msgcounter = 0;
	ape_running = 1;
	void postMessage(void *dataPtr, int ev);
	events_loop(g_ape);
	EXPECT_EQ(msgcounter, 1);
#endif
	//native_netlib_destroy(g_ape);
}
/*
static JSStructuredCloneCallbacks *jsscc;
static JSObject *readStructuredCloneOp(JSContext *cx, JSStructuredCloneReader *r, uint32_t tag, uint32_t data, void *closure);
static JSBool writeStructuredCloneOp(JSContext *cx, JSStructuredCloneWriter *w, JSObject *obj, void *closure);
void setStructuredCloneAddition(WriteStructuredCloneOp write, ReadStructuredCloneOp read)
ReadStructuredCloneOp getReadStructuredCloneAddition() const {
WriteStructuredCloneOp getWriteStructuredCloneAddition() const {
*/
