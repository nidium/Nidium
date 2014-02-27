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

#include "NativeFile.h"
#include "NativeUtils.h"
#include <ape_buffer.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define NATIVE_FILE_NOTIFY(arg, event) \
    do {   \
        this->postMessage(arg, event); \
    } while(0);

enum {
    NATIVEFILE_TASK_OPEN,
    NATIVEFILE_TASK_CLOSE,
    NATIVEFILE_TASK_READ,
    NATIVEFILE_TASK_WRITE,
    NATIVEFILE_TASK_SEEK
};

NativeFile::NativeFile(const char *name) :
    m_Fd(NULL), m_Delegate(NULL), m_Filesize(0), m_AutoClose(true), m_Eof(false)
{
    m_Path = strdup(name);
}

/*
    /!\ Exec in a worker thread
*/
void NativeFile_dispatchTask(NativeTask *task)
{
    NativeFile *file = (NativeFile *)task->getObject();
    uint64_t type;
    task->getArg(0, &type);

    switch (type) {
        case NATIVEFILE_TASK_OPEN:
        {
            char *modes;
            task->getArg(1, (void **)&modes);
            file->openTask(modes);
            free(modes);
            break;
        }
        case NATIVEFILE_TASK_CLOSE:
        {
            file->closeTask();
            break;
        }
        case NATIVEFILE_TASK_READ:
        {
            uint64_t readsize;
            task->getArg(1, &readsize);
            file->readTask(readsize);
            break;
        }
        case NATIVEFILE_TASK_WRITE:
        {
            uint64_t buflen;
            char *buf;

            task->getArg(1, &buflen);
            task->getArg(2, (void **)&buf);

            file->writeTask(buf, buflen);
            break;
        }
        case NATIVEFILE_TASK_SEEK:
        {
            uint64_t pos;
            task->getArg(1, &pos);

            file->seekTask(pos);
            break;
        }
        default:
            break;
    }
}

/*
    /!\ Exec in a worker thread
*/
void NativeFile::openTask(const char *mode)
{
    m_Fd = fopen(m_Path, mode);

    if (m_Fd == NULL) {
        NATIVE_FILE_NOTIFY(errno, NATIVEFILE_OPEN_ERROR);
        return;
    }
    
    fseek(m_Fd, 0L, SEEK_END);
    m_Filesize = ftell(m_Fd);
    fseek(m_Fd, 0L, SEEK_SET);

    NATIVE_FILE_NOTIFY(m_Fd, NATIVEFILE_OPEN_SUCCESS);
}

/*
    /!\ Exec in a worker thread
*/
void NativeFile::closeTask()
{
    if (m_Fd == NULL) {
        return;
    }

    fclose(m_Fd);
    NATIVE_FILE_NOTIFY((void *)NULL, NATIVEFILE_CLOSE_SUCCESS);
    m_Fd = NULL;
}

/*
    /!\ Exec in a worker thread
*/
void NativeFile::readTask(size_t size)
{
    if (m_Fd == NULL) {
        NATIVE_FILE_NOTIFY((void *)NULL, NATIVEFILE_READ_ERROR);
        return;
    }

    uint64_t clamped_len;
    clamped_len = native_min(m_Filesize, size);

    /*
        Read an empty file
    */
    if (clamped_len == 0) {
        return;
    }

    buffer *buf = buffer_new(clamped_len + 1);

    if ((buf->used = fread(buf->data, 1, buf->size, m_Fd)) == 0) {        
        this->checkRead();
        buffer_destroy(buf);
        return;
    }

    this->checkEOF();

    buf->data[buf->used] = '\0';

    NATIVE_FILE_NOTIFY(buf, NATIVEFILE_READ_SUCCESS);
}

/*
    /!\ Exec in a worker thread
*/
void NativeFile::writeTask(char *buf, size_t buflen)
{
    if (m_Fd == NULL) {
        NATIVE_FILE_NOTIFY((void *)NULL, NATIVEFILE_WRITE_ERROR);
        return;
    }

    size_t writelen = fwrite(buf, 1, buflen, m_Fd);
    /*
        TODO:
        save curosor pisition,
        position cursor at the end,
        restore position
    */
    m_Filesize = ftell(m_Fd);

    NATIVE_FILE_NOTIFY(writelen, NATIVEFILE_WRITE_SUCCESS);
}

/*
    /!\ Exec in a worker thread
*/
void NativeFile::seekTask(size_t pos)
{
    int res;

    if (m_Fd == NULL) {
        return;
    }

    if ((res = fseek(m_Fd, pos, SEEK_SET)) == -1) {
        NATIVE_FILE_NOTIFY(errno, NATIVEFILE_SEEK_ERROR);
        return;
    }

    NATIVE_FILE_NOTIFY((void *)NULL, NATIVEFILE_SEEK_SUCCESS);
}

void NativeFile::open(const char *mode, void *arg)
{
    NativeTask *task = new NativeTask();
    task->setObject(this);
    task->setArg(NATIVEFILE_TASK_OPEN, 0);
    task->setArg(strdup(mode), 1);
    task->setFunction(NativeFile_dispatchTask);

    this->addTask(task);
}

void NativeFile::close(void *arg)
{
    NativeTask *task = new NativeTask();
    task->setObject(this);
    task->setArg(NATIVEFILE_TASK_CLOSE, 0);
    task->setFunction(NativeFile_dispatchTask);

    this->addTask(task);
}

void NativeFile::read(size_t size, void *arg)
{
    NativeTask *task = new NativeTask();
    task->setObject(this);
    task->setArg(NATIVEFILE_TASK_READ, 0);
    task->setArg(size, 1);
    task->setFunction(NativeFile_dispatchTask);

    this->addTask(task);
}

void NativeFile::write(char *buf, size_t size, void *arg)
{
    NativeTask *task = new NativeTask();
    task->setObject(this);
    task->setArg(NATIVEFILE_TASK_WRITE, 0);
    task->setArg(size, 1);
    task->setArg(buf, 2);
    task->setFunction(NativeFile_dispatchTask);

    this->addTask(task);
}

void NativeFile::seek(size_t pos, void *arg)
{
    NativeTask *task = new NativeTask();
    task->setObject(this);
    task->setArg(NATIVEFILE_TASK_SEEK, 0);
    task->setArg(pos, 1);
    task->setFunction(NativeFile_dispatchTask);

    this->addTask(task);
}

bool NativeFile::checkEOF()
{
    if (m_Fd && ((m_Eof = (bool)feof(m_Fd)) == true ||
        (m_Eof = (ftell(m_Fd) == this->m_Filesize))) && m_AutoClose) {
        
        this->closeTask();
    }

    return m_Eof;
}

void NativeFile::checkRead(bool async)
{
    int err = -1;

    if (ferror(m_Fd)) {
        err = errno;
    } else if (this->checkEOF()) {
        err = 0;
    }

    if (async && err != -1) {
        NATIVE_FILE_NOTIFY(err, NATIVEFILE_READ_ERROR);
    }
}

NativeFile::~NativeFile()
{
    free(m_Path);
}

void NativeFile::onMessage(const NativeSharedMessages::Message &msg)
{
    if (m_Delegate) {
        m_Delegate->onMessage(msg);
    }

    switch(msg.event()) {
        case NATIVEFILE_READ_SUCCESS:
        {
            buffer *buf = (buffer *)msg.dataPtr();
            buffer_delete(buf);
            break;
        }
    }
}

void NativeFile::onMessageLost(const NativeSharedMessages::Message &msg)
{
    switch(msg.event()) {
        case NATIVEFILE_READ_SUCCESS:
        {
            buffer *buf = (buffer *)msg.dataPtr();
            buffer_delete(buf);
            break;
        }
    }
}