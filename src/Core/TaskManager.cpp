/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Core/TaskManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "Core/Atomic.h"

namespace Nidium {
namespace Core {

// {{{ Preamble
static pthread_key_t gManager = 0;

void *TaskManager_Worker(void *arg)
{
    return static_cast<TaskManager::workerInfo *>(arg)->work();
}
// }}}

// {{{ TaskManager::workerInfo
void *TaskManager::workerInfo::work()
{
    while (!m_Stop) {
        pthread_mutex_lock(&m_Lock);
        while (!m_Stop && !m_Messages.hasPendingMessages()) {
            pthread_cond_wait(&m_Cond, &m_Lock);
        }
        if (m_Stop) {
            return NULL;
        }
        pthread_mutex_unlock(&m_Lock);

        SharedMessages::Message *msg;

        while ((msg = m_Messages.readMessage())) {
            Task *task       = static_cast<Task *>(msg->dataPtr());
            Managed *managed = task->getObject();

            managed->lockTasks();
            task->getFunction()(task);
            managed->unlockTasks();

            Atomic::Dec(&managed->m_TaskQueued);

            delete task;
            delete msg;
        }
    }
    return NULL;
}

void TaskManager::workerInfo::stop()
{
    if (m_Stop) {
        return;
    }
    PthreadAutoLock npal(&m_Lock);
    m_Stop = true;
    pthread_cond_signal(&m_Cond);
}

void TaskManager::workerInfo::waitTerminate()
{
    pthread_join(m_Handle, NULL);
}

void TaskManager_workerInfo_MessageCleaner(const SharedMessages::Message &msg)
{
    Task *task = static_cast<Task *>(msg.dataPtr());
    delete task;
}

TaskManager::workerInfo::workerInfo() : m_Stop(false), m_Manager(NULL)
{
    pthread_mutex_init(&m_Lock, NULL);
    pthread_cond_init(&m_Cond, NULL);

    m_Messages.setCleaner(TaskManager_workerInfo_MessageCleaner);
}

TaskManager::workerInfo::~workerInfo()
{
    this->stop();
    pthread_join(m_Handle, NULL);

    m_Messages.delMessagesForDest(NULL);
}

void TaskManager::workerInfo::run()
{
    m_Stop = false;
    pthread_create(&m_Handle, NULL, TaskManager_Worker, this);
}

void TaskManager::workerInfo::addTask(Task *task)
{
    SharedMessages::Message *msg = new SharedMessages::Message(task, 0);

    /*
        Set the caller as destination so that messages
        can be deleted "per caller" in a worker
        (in case a worker handle multiple caller/task)
    */
    msg->setDest(task->getObject());
    m_Messages.postMessage(msg);

    PthreadAutoLock npal(&m_Lock);
    pthread_cond_signal(&m_Cond);
}
// }}}

// {{{ TaskManager
TaskManager::TaskManager()
{
    m_Threadpool.count = 0;
    m_Threadpool.size  = NIDIUM_TASKMANAGER_MAX_THREAD;

    m_Threadpool.worker = new workerInfo[NIDIUM_TASKMANAGER_MAX_THREAD];

    this->createWorker(NIDIUM_TASKMANAGER_MAX_IDLE_THREAD);
}

int TaskManager::createWorker(int count)
{
    int actualCount = count;

    /*
        Thread pool is full
    */
    if (m_Threadpool.size == m_Threadpool.count) {
        return 0;
    }
    /*
        Limit reached
    */
    if (m_Threadpool.count + count > m_Threadpool.size) {
        actualCount
            = count - ((m_Threadpool.count + count) - m_Threadpool.size);
    }

    for (int i = 0; i < actualCount; i++) {

        workerInfo *worker = &m_Threadpool.worker[m_Threadpool.count + i];

        worker->setManager(this);
        worker->run();
    }

    m_Threadpool.count += actualCount;

    return actualCount;
}

void TaskManager::stopAll()
{
    int count = m_Threadpool.count, i;
    for (i = 0; i < count; i++) {
        m_Threadpool.worker[i].stop();
    }

    for (i = 0; i < count; i++) {
        m_Threadpool.worker[i].waitTerminate();
        m_Threadpool.worker[i].getMessages()->delMessagesForDest(NULL);
    }
}

TaskManager::workerInfo *TaskManager::getAvailableWorker()
{
    return &m_Threadpool.worker[rand() % m_Threadpool.count];
}

TaskManager *TaskManager::GetManager()
{
    if (gManager == 0) {
        return NULL;
    }
    return static_cast<TaskManager *>(pthread_getspecific(gManager));
}

void TaskManager::CreateManager()
{
    /*
        Don't recreate if TaskManager
        if already setup in the current thread
    */
    if (GetManager()) {
        return;
    }
    TaskManager *manager = new TaskManager();
    if (gManager == 0) {
        pthread_key_create(&gManager, NULL);
    }
    pthread_setspecific(gManager, manager);
}

TaskManager::~TaskManager()
{
    /*
        It's faster to stop all the
        worker at the same time
    */
    this->stopAll();
    delete[] m_Threadpool.worker;
    pthread_setspecific(gManager, NULL);
}
// }}}

// {{{ Managed
void Managed::lockTasks()
{
    pthread_mutex_lock(&m_Lock);
}

void Managed::unlockTasks()
{
    pthread_mutex_unlock(&m_Lock);
}

void Managed::addTask(Task *task)
{
    if (m_Manager == NULL) {
        ndm_log(NDM_LOG_WARN, "TaskManager", "addTask() : Unknown manager");
        return;
    }
    if (m_Worker == NULL) {
        m_Worker = m_Manager->getAvailableWorker();
    }

    task->setObject(this);

    Atomic::Inc(&m_TaskQueued);

    m_Worker->addTask(task);
}
// }}}

} // namespace Core
} // namespace Nidium
