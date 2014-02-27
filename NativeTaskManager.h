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

#ifndef nativeTaskManager_h__
#define nativeTaskManager_h__

#include <pthread.h>
#include "NativeMessages.h"
#include "NativeSharedMessages.h"
#include <stdio.h>

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
    NativeTask() : m_Obj(NULL) {}
    void setObject(NativeManaged *obj) {
        m_Obj = obj;
    }

    void setArg(void *arg, int pos) {
        if (pos >= MAX_ARG) {
            return;
        }
        m_Arg[pos].ptr = arg;
    }

    void setArg(uint64_t arg, int pos) {
        if (pos >= MAX_ARG) {
            return;
        }
        m_Arg[pos].u64 = arg;        
    }

    void setFunction(task_func func) {
        m_Func = func;
    }

    task_func getFunction() const {
        return m_Func;
    }

    void getArg(int pos, void **out) const {
        *out = m_Arg[pos].ptr;
    }

    void getArg(int pos, uint64_t *out) const {
        *out = m_Arg[pos].u64;
    }   

    void *getObject() const {
        return m_Obj;
    }
private:
    union {
        void *ptr;
        uint64_t u64;
    } m_Arg[MAX_ARG];

    NativeManaged *m_Obj;
    task_func m_Func;
#undef MAX_ARG
};

class NativeManaged : public NativeMessages
{
public:
    NativeManaged() : m_Worker(NULL) {
        m_Manager = NativeTaskManager::getManager();
    }
    ~NativeManaged(){};
    void addTask(NativeTask *task);
private:
    NativeTaskManager *m_Manager;
    NativeTaskManager::workerInfo *m_Worker;
};

    
#endif
