/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <ape_netlib.h>
#include <Binding/JSExposer.h>

static const char * srcA = "counter = 0;";
static const char * srcFun = "function counterInc() {counter++;};";

TEST(JSExposer, Event)
{
    ape_global * g_ape = native_netlib_init();
    Nidium::Binding::NidiumJS njs(g_ape);

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedValue func(njs.cx);
    njs.LoadScriptContent(srcA, strlen(srcA), __FILE__);
    njs.LoadScriptContent(srcFun, strlen(srcFun), __FILE__);
    JS_GetProperty(njs.cx, globObj, "counterInc", &func);

    Nidium::Binding::JSEvent ne(njs.cx, func);
    EXPECT_TRUE(ne.m_Cx == njs.cx);
    EXPECT_TRUE(ne.m_Function == func);
    EXPECT_TRUE(ne.once == false);
    EXPECT_TRUE(ne.next == NULL);
    EXPECT_TRUE(ne.prev == NULL);

    native_netlib_destroy(g_ape);
}

TEST(JSExposer, Events)
{
    ape_global * g_ape = native_netlib_init();
    Nidium::Binding::NidiumJS njs(g_ape);
    char * name = strdup("dummy");

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedValue func(njs.cx);
    njs.LoadScriptContent(srcA, strlen(srcA), __FILE__);
    njs.LoadScriptContent(srcFun, strlen(srcFun), __FILE__);
    JS_GetProperty(njs.cx, globObj, "counterInc", &func);

    Nidium::Binding::JSEvents nes(name);
    EXPECT_TRUE(nes.m_Head == NULL);
    EXPECT_TRUE(nes.m_Queue == NULL);
    EXPECT_TRUE(strcmp(nes.m_Name, name) == 0);

    free(name);
    native_netlib_destroy(g_ape);
}
TEST(JSExposer, Exposer)
{

}

class Dummy;

class Dummy : public Nidium::Binding::JSObjectMapper<Dummy>{
public:
    int counter;
    Dummy(JSContext *cx): Nidium::Binding::JSObjectMapper(cx, "dummy") {
        counter = 1;
    }
    ~Dummy() {
    }
};

TEST(JSExposer, ObjectMapper)
{
    ape_global * g_ape = native_netlib_init();
    Nidium::Binding::NidiumJS njs(g_ape);

    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    Dummy njm(njs.cx);
    JS::RootedObject obj(njs.cx, njm.getJSObject());
    EXPECT_TRUE(obj != NULL);

    Dummy *dummy = Nidium::Binding::JSObjectMapper<Dummy>::GetObject(obj);
    EXPECT_TRUE(dummy != NULL);
    EXPECT_EQ(dummy->counter, 1);

    native_netlib_destroy(g_ape);
}

TEST(JSExposer, ObjectBuilder)
{
    ape_global * g_ape = native_netlib_init();
    Nidium::Binding::NidiumJS njs(g_ape);
    JS::RootedValue rval(njs.cx);

    JS::RootedObject obj(njs.cx);
    JS::RootedObject jsobj(njs.cx);
    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    Nidium::Binding::JSObjectBuilder ob(njs.cx, jsobj);
    obj = ob.obj();
    rval = ob.jsval();
    EXPECT_TRUE(obj == JSVAL_TO_OBJECT(rval));

    char * cstr;
    JS::RootedValue valOne(njs.cx, INT_TO_JSVAL(1));
    ob.set("valOne", valOne);
    JS_GetProperty(njs.cx, obj, "valOne", &rval);
    EXPECT_TRUE(1 == JSVAL_TO_INT(rval));

    JS::RootedString jstring(njs.cx, JS_NewStringCopyZ(njs.cx, "nidium"));
    ob.set("string", jstring.get());
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
    native_netlib_destroy(g_ape);
}

TEST(JSExposer, ObjectBuilderObj)
{
    ape_global * g_ape = native_netlib_init();
    Nidium::Binding::NidiumJS njs(g_ape);
    JS::RootedObject globObj(njs.cx, JS::CurrentGlobalOrNull(njs.cx));
    JS::RootedObject jsobj(njs.cx, JS_NewObject(njs.cx, NULL, JS::NullPtr(), JS::NullPtr()));
    JS::RootedObject obj(njs.cx);

    JS::RootedValue rval(njs.cx, INT_TO_JSVAL(12));
    JS_SetProperty(njs.cx, jsobj, "tst", rval);
    Nidium::Binding::JSObjectBuilder ob(njs.cx, jsobj);
    obj = ob.obj();
    JS_GetProperty(njs.cx, obj, "tst", &rval);
    EXPECT_TRUE(12 == JSVAL_TO_INT(rval));

    native_netlib_destroy(g_ape);
}

