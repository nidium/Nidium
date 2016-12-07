/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <ape_netlib.h>

#include <Core/Context.h>
#include <Binding/NidiumJS.h>

using namespace JS;

NIDIUMJS_FIXTURE(NidiumJS)

TEST_F(NidiumJS, Simple)
{
    struct _ape_htable *table;

    //check the init
    EXPECT_TRUE(njs->m_Net == ape);
    EXPECT_TRUE(njs->GetNet() == ape);
    EXPECT_TRUE(njs->m_Cx != NULL);
    EXPECT_TRUE(njs->getContext() == context);
    EXPECT_TRUE(njs->getJSContext() == njs->m_Cx);
    table = njs->m_JsObjects.accessCStruct();
    EXPECT_TRUE(table != NULL);

    //some others
    njs->loadGlobalObjects();
    njs->gc();
}

// XXX : Logger has been moved to Core/Context.cpp this test needs to be ported
#if 0
#define LOG_ARRAY_SIZE 1024
static char log_array[LOG_ARRAY_SIZE] = {'\0'};

int dummyVLogger(const char *format, va_list ap)
{
    size_t len = strlen(log_array);
    char * pos = log_array + len;
    vsnprintf(pos, LOG_ARRAY_SIZE - len, format, ap);
    return strlen(log_array);
}


int dummyLoggerClear()
{
    memset( log_array, '\0', sizeof(log_array));

    return 0;
}
TEST_F(NidiumJS, Loggers)
{
    memset( log_array, '\0', sizeof(log_array));
    njs->log("Normal logging to stdout");
    EXPECT_EQ(strlen(log_array), 0);

    njs->setLogger(dummyVLogger);
    njs->log("Logging" " " "Normal");
    njs->logf("Logging %s %s", "with", "args");
    EXPECT_EQ(strlen(log_array), 31);

    njs->logclear();
    EXPECT_EQ(strlen(log_array), 31);
    njs->setLogger(dummyLoggerClear);
    njs->logclear();
    EXPECT_EQ(strlen(log_array), 0);
}
#endif

TEST_F(NidiumJS, Quick)
{
    EXPECT_TRUE(njs->m_Net == ape);
    EXPECT_TRUE(njs->GetNet() == ape);
}

