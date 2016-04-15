/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "NativeTaskManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "../../system/native_atom.h"

static pthread_key_t gManager = 0;

void *NativeTaskManager_Worker(void *arg)
{
    return ((NativeTaskManager::workerInfo *)arg)->work();
}

void *NativeTaskManager::workerInfo::work()
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

        Nidium::Core::SharedMessages::Message *msg;

        while ((msg = m_Messages.readMessage())) {
            NativeTask *task = (NativeTask *)msg->dataPtr();
            task->getFunction()(task);
            native_atomic_dec(&task->getObject()->m_TaskQueued);

            delete task;
            delete msg;
        }
    }
    return NULL;
}

void NativeTaskManager::workerInfo::stop()
{
    if (m_Stop) {
        return;
    }
    NativePthreadAutoLock npal(&m_Lock);
    m_Stop = true;
    pthread_cond_signal(&m_Cond);
}

void NativeTaskManager::workerInfo::waitTerminate()
{
    pthread_join(m_Handle, NULL);
}

void NativeTaskManager_workerInfo_MessageCleaner(const Nidium::Core::SharedMessages::Message &msg)
{
    NativeTask *task = (NativeTask *)msg.dataPtr();
    delete task;
}

NativeTaskManager::workerInfo::workerInfo() :
    m_Stop(false), m_Manager(NULL)
{
    pthread_mutex_init(&m_Lock, NULL);
    pthread_cond_init(&m_Cond, NULL);

    m_Messages.setCleaner(NativeTaskManager_workerInfo_MessageCleaner);
}

NativeTaskManager::workerInfo::~workerInfo()
{
    this->stop();
    pthread_join(m_Handle, NULL);

    m_Messages.delMessagesForDest(NULL);
}

void NativeTaskManager::workerInfo::run()
{
    m_Stop = false;
    pthread_create(&m_Handle, NULL,
        NativeTaskManager_Worker, this);
}

void NativeTaskManager::workerInfo::addTask(NativeTask *task)
{
    Nidium::Core::SharedMessages::Message *msg = new Nidium::Core::SharedMessages::Message(task, 0);

    /*
        Set the caller as destination so that messages
        can be deleted "per caller" in a worker
        (in case a worker handle multiple caller/task)
    */
    msg->setDest(task->getObject());
    m_Messages.postMessage(msg);

    NativePthreadAutoLock npal(&m_Lock);
    pthread_cond_signal(&m_Cond);
}

////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////

NativeTaskManager::NativeTaskManager()
{
    m_Threadpool.count = 0;
    m_Threadpool.size = NATIVE_TASKMANAGER_MAX_THREAD;

    m_Threadpool.worker = new workerInfo[NATIVE_TASKMANAGER_MAX_THREAD];

    this->createWorker(NATIVE_TASKMANAGER_MAX_IDLE_THREAD);
}

int NativeTaskManager::createWorker(int count)
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
        actualCount = count - ((m_Threadpool.count + count) - m_Threadpool.size);
    }

    for (int i = 0; i < actualCount; i++) {

        workerInfo *worker = &m_Threadpool.worker[m_Threadpool.count+i];

        worker->setManager(this);
        worker->run();
    }

    m_Threadpool.count += actualCount;

    return actualCount;
}

void NativeTaskManager::stopAll()
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

NativeTaskManager::workerInfo *NativeTaskManager::getAvailableWorker()
{
    return &m_Threadpool.worker[rand() % m_Threadpool.count];
}

NativeTaskManager *NativeTaskManager::getManager()
{
    return (NativeTaskManager *)pthread_getspecific(gManager);
}

void NativeTaskManager::createManager()
{
    NativeTaskManager *manager = new NativeTaskManager();
    if (gManager == 0) {
        pthread_key_create(&gManager, NULL);
    }
    pthread_setspecific(gManager, manager);
}

NativeTaskManager::~NativeTaskManager()
{
    /*
        It's faster to stop all the
        worker at the same time
    */
    this->stopAll();
    delete[] m_Threadpool.worker;
    pthread_setspecific(gManager, NULL);
}

////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////

void NativeManaged::addTask(NativeTask *task)
{
    if (m_Manager == NULL) {
        printf("addTask() : Unknown manager\n");
        return;
    }
    if (m_Worker == NULL) {
        m_Worker = m_Manager->getAvailableWorker();
    }

    task->setObject(this);

    native_atomic_inc(&m_TaskQueued);

    m_Worker->addTask(task);
}

