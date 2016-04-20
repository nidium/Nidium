/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <Core/TaskManager.h>

static int dummy;

void dummyTask(Nidium::Core::Task *arg)
{
    dummy++;
}

TEST(TaskManager, Task)
{
    Nidium::Core::Task::task_func fun;
    Nidium::Core::Managed *obj;
    Nidium::Core::Managed *nm = new Nidium::Core::Managed();
    Nidium::Core::Task *nt = new Nidium::Core::Task();

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
TEST(TaskManager, Managed)
{
    Nidium::Core::Task nt;
    Nidium::Core::Managed nm;

    EXPECT_EQ(nm.m_TaskQueued, 0);
    nm.addTask(&nt);
}

TEST(TaskManager, WorkerInfo)
{
    Nidium::Core::TaskManager::workerInfo *wi = new Nidium::Core::TaskManager::workerInfo();
    Nidium::Core::Task *t = new Nidium::Core::Task();
    wi->addTask(t);

    delete t;
    delete wi;
}

TEST(TaksManager,  TaskManager)
{
    int i;
    Nidium::Core::TaskManager::workerInfo * wi;
    Nidium::Core::TaskManager *tm = Nidium::Core::TaskManager::getManager();

    tm->createManager();
    i = tm->createWorker();
    EXPECT_EQ(i, 1);

    tm->stopAll();
    wi = tm->getAvailableWorker();
}

