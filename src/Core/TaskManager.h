/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef core_taskmanager_h__
#define core_taskmanager_h__

#include <stdio.h>

#ifdef _MSC_VER
#include "port/MSWindows.h"
#endif

#include "Core/Messages.h"
#include "Core/SharedMessages.h"

namespace Nidium {
namespace Core {

#define NIDIUM_TASKMANAGER_MAX_IDLE_THREAD 8
#define NIDIUM_TASKMANAGER_MAX_THREAD 16

// {{{ TaskManager
class Task;
class TaskManager
{
public:
    class workerInfo
    {
    public:
        workerInfo();
        ~workerInfo();

        void *work();
        void stop();
        void run();
        void addTask(Task *task);
        void waitTerminate();

        void setManager(TaskManager *manager)
        {
            m_Manager = manager;
        }

        SharedMessages *getMessages()
        {
            return &m_Messages;
        }

    private:
        pthread_t m_Handle;
        pthread_mutex_t m_Lock;
        pthread_cond_t m_Cond;
        bool m_Stop;
        TaskManager *m_Manager;
        SharedMessages m_Messages;
    };

    TaskManager();
    ~TaskManager();
    int createWorker(int count = 1);
    void stopAll();

    workerInfo *getAvailableWorker();
    static TaskManager *GetManager();
    static void CreateManager();

private:
    struct
    {
        int count;
        int size;

        workerInfo *worker;
    } m_Threadpool;
};
// }}}

// {{{ Task
class Managed;
class Task
{
#define MAX_ARG 8
public:
    typedef void (*task_func)(Task *arg);
    Task() : m_Obj(NULL), m_Func(NULL)
    {
    }

    void setFunction(task_func func)
    {
        m_Func = func;
    }

    task_func getFunction() const
    {
        return m_Func;
    }

    Managed *getObject() const
    {
        return m_Obj;
    }

    Args m_Args;

    friend class Managed;

private:
    void setObject(Managed *obj)
    {
        m_Obj = obj;
    }

    Managed *m_Obj;
    task_func m_Func;
#undef MAX_ARG
};
// }}}

// {{{ Managed
class Managed : public Messages
{
public:
    Managed() : m_TaskQueued(0), m_Worker(NULL)
    {
        m_Manager = TaskManager::GetManager();
        pthread_mutex_init(&m_Lock, NULL);
    }

    ~Managed()
    {
        if (m_Worker) {
            m_Worker->getMessages()->delMessagesForDest(this);
        }
    };

    inline pthread_mutex_t &getManagedLock() {
        return m_Lock;
    }

    bool hasTaskOrMessagePending() const {
        return m_TaskQueued || _m_CountMessagePending;
    }

    void addTask(Task *task);
    void lockTasks();
    void unlockTasks();
    int32_t m_TaskQueued;

private:
    TaskManager *m_Manager;
    TaskManager::workerInfo *m_Worker;
    pthread_mutex_t m_Lock;
};
// }}}


} // namespace Core
} // namespace Nidium

#endif
