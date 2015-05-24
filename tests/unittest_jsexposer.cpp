#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <NativeJSExposer.h>

static const char * srcA = "counter = 0;";
static const char * srcFun = "function counterInc() {counter++;};";

TEST(NativeJSExposer, Event)
{
    JSObject *globalObj;
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    globalObj = JS_GetGlobalObject(njs.cx);
    jsval func;

    njs.LoadScriptContent(srcA, strlen(srcA), __FILE__);
    njs.LoadScriptContent(srcFun, strlen(srcFun), __FILE__);
    JS_GetProperty(njs.cx, globalObj, "counterInc", &func);

    NativeJSEvent ne(njs.cx, func);
    EXPECT_TRUE(ne.m_Cx == njs.cx);
    EXPECT_TRUE(ne.m_Function == func);
    EXPECT_TRUE(ne.once == false);
    EXPECT_TRUE(ne.next == NULL);
    EXPECT_TRUE(ne.prev == NULL);

    native_netlib_destroy(g_ape);
}

TEST(NativeJSExposer, Events)
{
    JSObject *globalObj;
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    globalObj = JS_GetGlobalObject(njs.cx);
    jsval func;
    char * name = strdup("dummy");

    njs.LoadScriptContent(srcA, strlen(srcA), __FILE__);
    njs.LoadScriptContent(srcFun, strlen(srcFun), __FILE__);
    JS_GetProperty(njs.cx, JS_GetGlobalObject(njs.cx), "counterInc", &func);



    NativeJSEvents nes(name);
    EXPECT_TRUE(nes.m_Head == NULL);
    EXPECT_TRUE(nes.m_Queue == NULL);
    EXPECT_TRUE(strcmp(nes.m_Name, name) == 0);

#if 0
    NativeJSEvent *ne(njs.cx, func); invallid free..
    nes.add(&ne);

    jsvar rval
    //@TODO nes.fire(evobj, globalObj);

    JS_GetProperty(njs.cx, JS_GetGlobalObject(njs.cx), "counter", &rval);
    EXPECT_EQ(JSVAL_TO_INT(rval), 1);
#endif

    free(name);
    native_netlib_destroy(g_ape);
}
TEST(NativeJSExposer, Exposer)
{
}
#if 0
class NativeJSAsyncDummy : public NativeJSAsyncHandler
{
public:
    NativeJSAsyncDummy(JSContext *ctx) : NativeJSAsyncHandler(ctx) {}
    void onMessage(const NativeSharedMessages::Message &msg) {
    }
};

TEST(NativeJSExposer, AsyncHandler)
{
    JSObject *globalObj;
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    globalObj = JS_GetGlobalObject(njs.cx);

    NativeJSAsyncDummy na(njs.cx);
    EXPECT_TRUE(na.getJSContext() == njs.cx);

    JSObject *obj = NULL;

    EXPECT_TRUE(na.getCallback(0) == NULL);
    EXPECT_TRUE(na.getCallback(1) == NULL);
    EXPECT_TRUE(na.getCallback(2) == NULL);
    EXPECT_TRUE(na.getCallback(3) == NULL);
    obj = JS_NewObject(njs.cx, NULL, NULL, NULL);
    na.setCallback(0, obj);
    EXPECT_TRUE(na.getCallback(0) == obj);

    //@TODO: onMessage

    native_netlib_destroy(g_ape);
}
#endif
class Dummy;

class Dummy : public NativeJSObjectMapper<Dummy>{
public:
    int counter;
    Dummy(JSContext *cx): NativeJSObjectMapper(cx, "dummy") {
        counter = 1;
    }
    ~Dummy() {
    }
};

TEST(NativeJSExposer, ObjectMapper)
{
    JSObject *globalObj;
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    globalObj = JS_GetGlobalObject(njs.cx);
    JSObject *obj = NULL;

    Dummy njm(njs.cx);
    obj = njm.getJSObject();
    EXPECT_TRUE(obj != NULL);

    Dummy *dummy = NativeJSObjectMapper<Dummy>::getObject(obj);
    EXPECT_TRUE(dummy != NULL);
    EXPECT_EQ(dummy->counter, 1);

    native_netlib_destroy(g_ape);
}

TEST(NativeJSExposer, ObjectBuilder)
{
    JSObject *globalObj;
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    globalObj = JS_GetGlobalObject(njs.cx);
    jsval rval;
    JSObject *obj, *jsobj;

    jsobj = NULL;
    NativeJSObjectBuilder ob(njs.cx, jsobj);
    obj = ob.obj();
    rval = ob.jsval();
    EXPECT_TRUE(obj == JSVAL_TO_OBJECT(rval));

/*
    @FIXME: crash
    char * cstr;
    jsval valOne = INT_TO_JSVAL(1);
    ob.set("valOne", valOne);
    JS_GetProperty(njs.cx, obj, "valOne", &rval);
    EXPECT_TRUE(1 == JSVAL_TO_INT(rval));

    JSString *jstring = JS_NewStringCopyZ(njs.cx, "nidium");
    ob.set("string", jstring);
    JS_GetProperty(njs.cx, obj, "valOne", &rval);
    cstr = JS_EncodeString(njs.cx, jstring);
    EXPECT_TRUE(strcmp(cstr, "nidium") == 0);
    free(cstr);

    ob.set("chars", "nidium");
    JS_GetProperty(njs.cx, obj, "chars", &rval);
    cstr = JS_EncodeString(njs.cx, jstring);
    EXPECT_TRUE(strcmp(cstr, "nidium") == 0);
    free(cstr);

    int32_t i = 1;
    ob.set("one", i);
    JS_GetProperty(njs.cx, obj, "one", &rval);
    EXPECT_TRUE(1 == JSVAL_TO_INT(rval));

    ob.set("minusone", -1);
    JS_GetProperty(njs.cx, obj, "minusone", &rval);
    EXPECT_TRUE(-1 == JSVAL_TO_INT(rval));

    ob.set("float", 3.1415961f);
    JS_GetProperty(njs.cx, obj, "float", &rval);
    EXPECT_TRUE((double)3.1415961f == JSVAL_TO_DOUBLE(rval));

    ob.set("bool", true);
    JS_GetProperty(njs.cx, obj, "bool", &rval);
    EXPECT_TRUE(true == JSVAL_TO_BOOLEAN(rval));
*/
    native_netlib_destroy(g_ape);
}
/*
TEST(NativeJSExposer, ObjectBuilderObj)
{
    JSObject *globalObj;
    ape_global * g_ape = native_netlib_init();
    NativeJS njs(g_ape);
    globalObj = JS_GetGlobalObject(njs.cx);
    jsval rval;
    JSObject *jsobj = JS_NewObject(njs.cx, NULL, NULL, NULL);
    JSObject *obj;

    rval = INT_TO_JSVAL(12);
    JS_SetProperty(njs.cx, jsobj, "tst", &rval);
    NativeJSObjectBuilder ob(njs.cx, jsobj);
    obj = ob.obj();
    JS_GetProperty(njs.cx, obj, "tst", &rval);
    EXPECT_TRUE(12 == JSVAL_TO_INT(rval));

    native_netlib_destroy(g_ape);
}
*/

