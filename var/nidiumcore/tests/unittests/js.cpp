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
    EXPECT_TRUE(njs->net == ape);
    EXPECT_TRUE(njs->GetNet() == ape);
    EXPECT_TRUE(njs->cx != NULL);
    EXPECT_TRUE(njs->getJSContext() == njs->cx);
    EXPECT_TRUE(njs->messages != NULL);
    table = njs->jsobjects.accessCStruct();
    EXPECT_TRUE(table != NULL);
    EXPECT_TRUE(njs->registeredMessages != NULL);
    EXPECT_EQ(njs->registeredMessagesIdx, 7);
    EXPECT_EQ(njs->registeredMessagesSize, 16);
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
    JS::RootedObject d(njs->cx, JS_NewObject(njs->cx, NULL, JS::NullPtr(), JS::NullPtr()));
    JS::RootedValue rval(njs->cx, INT_TO_JSVAL(1));
    JS_SetProperty(njs->cx, d, "a", rval);
    njs->rootObjectUntilShutdown(d);
    njs->unrootObject(d);
    JS::RootedObject dc(njs->cx, JS_NewObject(njs->cx, NULL, JS::NullPtr(), JS::NullPtr()));
    njs->CopyProperties(njs->cx, d, &dc);
    JS_GetProperty(njs->cx, dc, "a", &rval);
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
    EXPECT_TRUE(njs->net == ape);
    EXPECT_TRUE(njs->GetNet() == ape);
}

TEST_F(NidiumJS, Code)
{
    const char * srcA = "a = 11*11;";
    const char * srcB = "b = a - 21";
    int success;

    success = njs->LoadScriptContent(srcA, strlen(srcA), __FILE__);
    EXPECT_EQ(success, 1);

    JS::RootedValue rval(njs->cx);
    JS::RootedObject globObj(njs->cx, JS::CurrentGlobalOrNull(njs->cx));
    JS_GetProperty(njs->cx, globObj, "a", &rval);
    EXPECT_EQ(JSVAL_TO_INT(rval), 121);

    success = njs->LoadScriptReturn(njs->cx, srcB, strlen(srcB), __FILE__, &rval);
    EXPECT_EQ(success, 1);
    EXPECT_EQ(JSVAL_TO_INT(rval), 100);
    JS_GetProperty(njs->cx, globObj, "b", &rval);
    EXPECT_EQ(JSVAL_TO_INT(rval), 100);
}

TEST_F(NidiumJS, Messages)
{
    size_t i;

    EXPECT_EQ(njs->registeredMessagesIdx, 7);
    EXPECT_EQ(njs->registeredMessagesSize, 16);
    for (i = 0; i < 8; i++) {
        EXPECT_TRUE(njs->registeredMessages[i] == NULL);
    }

    njs->registerMessage(msg_cb_t);
    EXPECT_EQ(njs->registeredMessagesIdx, 8);
    EXPECT_EQ(njs->registeredMessagesSize, 16);

    int start = njs->registeredMessagesIdx;
    int end = njs->registeredMessagesSize + 2;
    for (i = start; i < end; i++) {
        printf("index=%u njs->registeredMessagesIdx=%d njs->registeredMessagesSize=%d\n", i, njs->registeredMessagesIdx, njs->registeredMessagesSize);
        njs->registerMessage(msg_cb_t);
        EXPECT_TRUE(njs->registeredMessages[i] != NULL);
    }

    EXPECT_EQ(njs->registeredMessagesIdx, end);
    EXPECT_EQ(njs->registeredMessagesSize, 32);

    njs->registerMessage(msg_cb_t, 0);
    msgcounter = 0;
    APE_loop_stop();
    void postMessage(void *dataPtr, int ev);
}

} // Namespace

