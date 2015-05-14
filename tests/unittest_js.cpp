#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJS.h>

static int counter;

int dummyLogger(const char *format)
{
	return counter += strcmp(format, "tt %s") ;
}

int dummyVLogger(const char *format, va_list ap)
{
	return counter += strcmp(format, "tt %s");
}

int dummyLoggerClear()
{
	counter = -10;
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
	JS_SetProperty(njs.cx, d, "nidium", &rval);
	njs.rootObjectUntilShutdown(d);
	njs.unrootObject(d);
	dc =JS_NewObject(njs.cx, NULL, NULL, NULL);
	njs.copyProperties(njs.cx, d, dc);
	JS_GetProperty(njs.cx, dc, "nidium", &rval);
	EXPECT_EQ(JSVAL_TO_INT(rval), 1);

	//check the loggers
	counter = 0;
	njs.log("%s");
	EXPECT_EQ(counter, 0);

	njs.setLogger(dummyLogger);
	njs.logf("tt %s", "nidium");
	EXPECT_EQ(counter, 0);
	njs.log("%s");
	EXPECT_EQ(counter, -1);

	njs.setLogger(dummyVLogger);
	njs.logf("tt %s", "nidium");
	EXPECT_EQ(counter, -1);
	njs.log("%s");
	EXPECT_EQ(counter, -2);
	
	njs.logclear();
	EXPECT_EQ(counter, -2);
	njs.setLogger(dummyLoggerClear);
	njs.logclear();
	EXPECT_EQ(counter, -10);

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
	
}
/*
static int LoadScriptReturn(JSContext *cx, const char *data, size_t len, const char *filename, JS::Value *ret);
static int LoadScriptReturn(JSContext *cx, const char *filename, JS::Value *ret);

int LoadScriptContent(const char *data, size_t len, const char *filename);
int LoadScript(const char *filename);
int LoadBytecode(NativeBytecodeScript *script);
int LoadBytecode(void *data, int size, const char *filename);

int registerMessage(native_thread_message_t cbk);
void registerMessage(native_thread_message_t cbk, int id);
void postMessage(void *dataPtr, int ev);

static JSStructuredCloneCallbacks *jsscc;
static JSObject *readStructuredCloneOp(JSContext *cx, JSStructuredCloneReader *r, uint32_t tag, uint32_t data, void *closure);
static JSBool writeStructuredCloneOp(JSContext *cx, JSStructuredCloneWriter *w, JSObject *obj, void *closure);
void setStructuredCloneAddition(WriteStructuredCloneOp write, ReadStructuredCloneOp read)
ReadStructuredCloneOp getReadStructuredCloneAddition() const {
WriteStructuredCloneOp getWriteStructuredCloneAddition() const {
*/
