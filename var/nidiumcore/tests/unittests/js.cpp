/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <ape_netlib.h>
#include <Binding/NidiumJS.h>

namespace {

using namespace JS;

NIDIUMJS_FIXTURE(NidiumJS)

static int logcounter;
static int msgcounter;
void msg_cb_t(JSContext *cx, Nidium::Core::SharedMessages::Message *msg)
{
    msgcounter++;
    APE_loop_stop();
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

TEST_F(NidiumJS, Simple)
{
    int i = 1, *p;
    struct _ape_htable *table;

    //check the init
    EXPECT_TRUE(njs->m_Net == ape);
    EXPECT_TRUE(njs->GetNet() == ape);
    EXPECT_TRUE(njs->m_Cx != NULL);
    EXPECT_TRUE(njs->getJSContext() == njs->m_Cx);
    EXPECT_TRUE(njs->m_Messages != NULL);
    table = njs->m_JsObjects.accessCStruct();
    EXPECT_TRUE(table != NULL);
    EXPECT_TRUE(njs->m_RegisteredMessages != NULL);
    EXPECT_EQ(njs->m_RegisteredMessagesIdx, 7);
    EXPECT_EQ(njs->m_RegisteredMessagesSize, 16);
    EXPECT_EQ(njs->isShuttingDown(), false);

    //check the private
    njs->setPrivate(&i);
    p = (int*)njs->getPrivate();
    EXPECT_EQ(p, &i);
    EXPECT_EQ(*p, i);

    //check the path
    EXPECT_TRUE(njs->getPath() == NULL);
    njs->setPath("/tmp/");
    EXPECT_TRUE(strcmp(njs->getPath(), "/tmp/") == 0);

    //some others
    njs->loadGlobalObjects();
    njs->gc();

    //store objects
    JS::RootedObject d(njs->m_Cx, JS_NewObject(njs->m_Cx, NULL, JS::NullPtr(), JS::NullPtr()));
    JS::RootedValue rval(njs->m_Cx, INT_TO_JSVAL(1));
    JS_SetProperty(njs->m_Cx, d, "a", rval);
    njs->rootObjectUntilShutdown(d);
    njs->unrootObject(d);
    JS::RootedObject dc(njs->m_Cx, JS_NewObject(njs->m_Cx, NULL, JS::NullPtr(), JS::NullPtr()));
    njs->CopyProperties(njs->m_Cx, d, &dc);
    JS_GetProperty(njs->m_Cx, dc, "a", &rval);
    EXPECT_EQ(JSVAL_TO_INT(rval), 1);

    //check the loggers
    logcounter = 0;
    njs->log("%s");
    EXPECT_EQ(logcounter, 0);

    njs->setLogger(dummyLogger);
    njs->logf("tt %s", "a");
    EXPECT_EQ(logcounter, 0);
    njs->log("%s");
    EXPECT_EQ(logcounter, -79);

    njs->setLogger(dummyVLogger);
    njs->logf("tt %s", "a");
    EXPECT_EQ(logcounter, -79);
    njs->log("%s");
    EXPECT_EQ(logcounter, -158);

    njs->logclear();
    EXPECT_EQ(logcounter, -158);
    njs->setLogger(dummyLoggerClear);
    njs->logclear();
    EXPECT_EQ(logcounter, -10);
}


TEST_F(NidiumJS, Quick)
{
    njs->InitNet(ape);
    EXPECT_TRUE(njs->m_Net == ape);
    EXPECT_TRUE(njs->GetNet() == ape);
}

TEST_F(NidiumJS, Code)
{
    const char * srcA = "a = 11*11;";
    const char * srcB = "b = a - 21";
    int success;

    success = njs->LoadScriptContent(srcA, strlen(srcA), __FILE__);
    EXPECT_EQ(success, 1);

    JS::RootedValue rval(njs->m_Cx);
    JS::RootedObject globObj(njs->m_Cx, JS::CurrentGlobalOrNull(njs->m_Cx));
    JS_GetProperty(njs->m_Cx, globObj, "a", &rval);
    EXPECT_EQ(JSVAL_TO_INT(rval), 121);

    success = njs->LoadScriptReturn(njs->m_Cx, srcB, strlen(srcB), __FILE__, &rval);
    EXPECT_EQ(success, 1);
    EXPECT_EQ(JSVAL_TO_INT(rval), 100);
    JS_GetProperty(njs->m_Cx, globObj, "b", &rval);
    EXPECT_EQ(JSVAL_TO_INT(rval), 100);
}

TEST_F(NidiumJS, Messages)
{
    size_t i;

    EXPECT_EQ(njs->m_RegisteredMessagesIdx, 7);
    EXPECT_EQ(njs->m_RegisteredMessagesSize, 16);
    for (i = 0; i < 8; i++) {
        EXPECT_TRUE(njs->m_RegisteredMessages[i] == NULL);
    }

    njs->registerMessage(msg_cb_t);
    EXPECT_EQ(njs->m_RegisteredMessagesIdx, 8);
    EXPECT_EQ(njs->m_RegisteredMessagesSize, 16);

    int start = njs->m_RegisteredMessagesIdx;
    int end = njs->m_RegisteredMessagesSize + 2;
    for (i = start; i < end; i++) {
        printf("index=%u njs->m_RegisteredMessagesIdx=%d njs->m_RegisteredMessagesSize=%d\n", i, njs->m_RegisteredMessagesIdx, njs->m_RegisteredMessagesSize);
        njs->registerMessage(msg_cb_t);
        EXPECT_TRUE(njs->m_RegisteredMessages[i] != NULL);
    }

    EXPECT_EQ(njs->m_RegisteredMessagesIdx, end);
    EXPECT_EQ(njs->m_RegisteredMessagesSize, 32);

    njs->registerMessage(msg_cb_t, 0);
    msgcounter = 0;
    APE_loop_stop();
    //void postMessage(void *dataPtr, int ev);
}

} // Namespace

