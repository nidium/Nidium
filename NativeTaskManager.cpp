/*
    NativeJS Core Library
    Copyright (C) 2013 Anthony Catel <paraboul@gmail.com>
    Copyright (C) 2013 Nicolas Trani <n.trani@weelya.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "NativeTaskManager.h"
#include "NativeUtils.h"

#include <stdio.h>
#include <stdlib.h>

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

        NativeSharedMessages::Message *msg;

        while ((msg = m_Messages.readMessage())) {
            NativeTask *task = (NativeTask *)msg->dataPtr();
            task->getFunction()(task);
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

void NativeTaskManager_workerInfo_MessageCleaner(const NativeSharedMessages::Message &msg)
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
    NativeSharedMessages::Message *msg = new NativeSharedMessages::Message(task, 0);

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
    m_Worker->addTask(task);
}
