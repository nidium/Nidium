/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativeTaskManager_h__
#define nativeTaskManager_h__

#include <stdio.h>
#include <pthread.h>

#include "NativeMessages.h"
#include "NativeSharedMessages.h"

#define NATIVE_TASKMANAGER_MAX_IDLE_THREAD 8
#define NATIVE_TASKMANAGER_MAX_THREAD 16

class NativeTask;

class NativeTaskManager
{
public:
    class workerInfo {
    public:
        workerInfo();
        ~workerInfo();

        void *work();
        void stop();
        void run();
        void addTask(NativeTask *task);
        void waitTerminate();

        void setManager(NativeTaskManager *manager) {
            m_Manager = manager;
        }

        NativeSharedMessages *getMessages() {
            return &m_Messages;
        }
    private:
        pthread_t m_Handle;
        pthread_mutex_t m_Lock;
        pthread_cond_t m_Cond;
        bool m_Stop;
        NativeTaskManager *m_Manager;
        NativeSharedMessages m_Messages;
    };

    NativeTaskManager();
    ~NativeTaskManager();
    int createWorker(int count = 1);
    void stopAll();

    workerInfo *getAvailableWorker();
    static NativeTaskManager *getManager();
    static void createManager();

private:

    struct {
        int count;
        int size;

        workerInfo *worker;
    } m_Threadpool;
};

class NativeManaged;

class NativeTask
{
#define MAX_ARG 8
public:
    typedef void (* task_func)(NativeTask *arg);
    NativeTask() : m_Obj(NULL), m_Func(NULL) {}

    void setFunction(task_func func) {
        m_Func = func;
    }

    task_func getFunction() const {
        return m_Func;
    }

    NativeManaged *getObject() const {
        return m_Obj;
    }

    Nidium::Core::Args args;

    friend class NativeManaged;
private:
    void setObject(NativeManaged *obj) {
        m_Obj = obj;
    }

    NativeManaged *m_Obj;
    task_func m_Func;
#undef MAX_ARG
};

class NativeManaged : public NativeMessages
{
public:
    NativeManaged() : m_TaskQueued(0), m_Worker(NULL) {
        m_Manager = NativeTaskManager::getManager();
    }
    ~NativeManaged() {
        if (m_Worker) {
            m_Worker->getMessages()->delMessagesForDest(this);
        }
    };
    void addTask(NativeTask *task);
    int32_t m_TaskQueued;
private:
    NativeTaskManager *m_Manager;
    NativeTaskManager::workerInfo *m_Worker;
};

#endif

