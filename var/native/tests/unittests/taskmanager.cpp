/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeTaskManager.h>

static int dummy;

void dummyTask(NativeTask *arg)
{
    dummy++;
}

TEST(NativeTaskManager, NativeTask)
{
    NativeTask::task_func fun;
    NativeManaged *obj;
    NativeManaged *nm = new NativeManaged();
    NativeTask *nt = new NativeTask();

    fun = nt->getFunction();
    EXPECT_TRUE(fun == NULL);
    obj = nt->getObject();
    EXPECT_TRUE(obj == NULL);

    nt->setFunction(dummyTask);
    fun = nt->getFunction();
    EXPECT_TRUE(fun == dummyTask);

    dummy = 0;
    delete nt;
    delete nm;
}
TEST(NativeTaskManager, NativeManaged)
{
    NativeTask nt;
    NativeManaged nm;

    EXPECT_EQ(nm.m_TaskQueued, 0);
    nm.addTask(&nt);
}

TEST(NativeTaskManager, WorkerInfo)
{
    NativeTaskManager::workerInfo *wi = new NativeTaskManager::workerInfo();
    NativeTask *t = new NativeTask();
    wi->addTask(t);

    delete t;
    delete wi;
}

TEST(NativeTaksManager,  TaskManager)
{
    int i;
    NativeTaskManager::workerInfo * wi;
    NativeTaskManager *tm = NativeTaskManager::getManager();

    tm->createManager();
    i = tm->createWorker();
    EXPECT_EQ(i, 1);

    tm->stopAll();
    wi = tm->getAvailableWorker();
}

