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

#include "NativeFileIO.h"
#include "NativeSharedMessages.h"
#include <native_netlib.h>
#include "NativeUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#define native_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))
#define native_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))

static void *native_fileio_thread(void *arg)
{
    NativeFileIO *NFIO = (NativeFileIO *)arg;
    
    while (!NFIO->action.stop) {
        pthread_mutex_lock(&NFIO->threadMutex);

        while (!NFIO->action.active && !NFIO->action.stop) {
            pthread_cond_wait(&NFIO->threadCond, &NFIO->threadMutex);
        }
        if (NFIO->action.stop) {
            pthread_mutex_unlock(&NFIO->threadMutex);
            return NULL;
        }

        switch (NFIO->action.type) {
            case NativeFileIO::FILE_ACTION_OPEN:
                NFIO->openAction((char *)NFIO->action.ptr);
                free(NFIO->action.ptr);
                break;
            case NativeFileIO::FILE_ACTION_READ:
                NFIO->readAction(NFIO->action.u64);
                break;
            case NativeFileIO::FILE_ACTION_WRITE:
                NFIO->writeAction((unsigned char *)NFIO->action.ptr,
                                    NFIO->action.u64);
                free(NFIO->action.ptr);
                break;
            default:
                break;
        }
        NFIO->action.active = false;
        pthread_mutex_unlock(&NFIO->threadMutex);
    }
    //printf("Thread ended 2\n");
    return NULL;
}

static int Native_handle_file_messages(void *arg)
{
#define MAX_MSG_IN_ROW 32
    NativeFileIO *nfileio = (NativeFileIO *)arg;
    NativeSharedMessages::Message msg;
    int nread = 0;

    while (++nread < MAX_MSG_IN_ROW && nfileio->messages->readMessage(&msg)) {
        switch (msg.event()) {
            case NATIVE_FILEOPEN_MESSAGE:
                nfileio->getDelegate()->onNFIOOpen(nfileio);
                break;
            case NATIVE_FILEERROR_MESSAGE:
                nfileio->getDelegate()->onNFIOError(nfileio, msg.dataUInt());
                break;
            case NATIVE_FILEREAD_MESSAGE:
                nfileio->getDelegate()->onNFIORead(nfileio,
                    (unsigned char *)msg.dataPtr(), nfileio->action.u64);
                delete[] (unsigned char *)msg.dataPtr();
                break;
            case NATIVE_FILEWRITE_MESSAGE:
                nfileio->getDelegate()->onNFIOWrite(nfileio, msg.dataUInt());
                break;
            default:break;
        }
    }
    return 4;
#undef MAX_MSG_IN_ROW
}

void NativeFileIO::writeAction(unsigned char *data, uint64_t len)
{
    size_t ret;

    if (this->fd == NULL) {
        return;
    }

    ret = fwrite(data, sizeof(char), len, fd);

    messages->postMessage(ret, NATIVE_FILEWRITE_MESSAGE);
}

void NativeFileIO::readAction(uint64_t len)
{
    if (this->fd == NULL) {
        return;
    }

    uint64_t clamped_len;
    clamped_len = native_min(filesize, len);

    /*
        Read an empty file
    */
    if (clamped_len == 0) {
        action.u64 = 0;

        if (!action.stop) {
            messages->postMessage((void *)NULL, NATIVE_FILEREAD_MESSAGE);
            return;
        }
    }

    unsigned char *data = new unsigned char[clamped_len + 1];
    size_t readsize = 0;

    if ((readsize = fread(data, sizeof(char), clamped_len, fd)) == 0) {

        this->checkRead();

        delete[] data;
        return;
    }

    /*
        Never emit error (can closes the file and invalid |fd|)
    */
    this->checkEOF();

    /* Always null-terminate the returned data (doesn't impact returned size) */
    data[readsize] = '\0';

    action.u64 = readsize;
    if (!action.stop) {
        messages->postMessage(data, NATIVE_FILEREAD_MESSAGE);
    }
}

void NativeFileIO::openAction(char *modes)
{
    if ((this->fd = fopen(this->filename, modes)) == NULL) {
        printf("Failed to open : %s errno=%d\n", this->filename, errno);
        if (!action.stop) {
            this->messages->postMessage(errno, NATIVE_FILEERROR_MESSAGE);
        }
        return;
    }

    fseek(this->fd, 0L, SEEK_END);
    this->filesize = ftell(this->fd);
    fseek(this->fd, 0L, SEEK_SET);

    if (!action.stop) {
        this->messages->postMessage(this->fd, NATIVE_FILEOPEN_MESSAGE);
    }
}

void NativeFileIO::open(const char *modes)
{
    NativePthreadAutoLock npal(&threadMutex);

    if (action.active) {
        return;
    }
    action.active = true;
    action.type = FILE_ACTION_OPEN;
    action.ptr  = strdup(modes);
    pthread_cond_signal(&threadCond);
}

void NativeFileIO::read(uint64_t len)
{
    NativePthreadAutoLock npal(&threadMutex);

    if (action.active) {
        return;
    }
    action.active = true;
    action.type = FILE_ACTION_READ;
    action.u64  = len;
    pthread_cond_signal(&threadCond);  
}

void NativeFileIO::write(unsigned char *data, uint64_t len)
{
    NativePthreadAutoLock npal(&threadMutex);

    if (action.active) {
        return;
    }
    action.active = true;
    action.type = FILE_ACTION_WRITE;
    action.ptr  = data;
    action.u64  = len;
    pthread_cond_signal(&threadCond); 
}

void NativeFileIO::seek(uint64_t pos)
{
    if (!fd) {
        return;
    }

    NativePthreadAutoLock npal(&threadMutex);

    fseek(fd, pos, SEEK_SET);

    // Discard all message of type NATIVE_FILEREAD_MESSAGE,
    // because after a seek we expect to only read new data
    NativeSharedMessages::Message msg;
    while (messages->readMessage(&msg, NATIVE_FILEREAD_MESSAGE)) {
        if (msg.dataPtr() != NULL) {
            delete[] (unsigned char *)msg.dataPtr();
        }
    }
}

void NativeFileIO::close()
{
    if (!fd) {
        return;
    }
    NativePthreadAutoLock npal(&threadMutex);
    fclose(fd);
    fd = NULL;
}


NativeFileIO::NativeFileIO(const char *filename, NativeFileIODelegate *delegate,
    ape_global *net, const char *prefix) :
    fd(NULL), autoClose(true), m_eof(false)
{
    messages = new NativeSharedMessages();
    if (prefix) {
        this->filename = (char *)malloc(sizeof(char) *
            (strlen(filename) + strlen(prefix) + 1));

        memcpy(this->filename, prefix, strlen(prefix)+1);
        strcat(this->filename, filename);
    } else {
        this->filename = strdup(filename);
    }
    this->net = net;
    this->delegate = delegate;
    this->filesize = 0;

    this->action.active = false;
    this->action.stop = false;

    timer = add_timer(&this->net->timersng, 1,
        Native_handle_file_messages, this);

    pthread_mutex_init(&threadMutex, NULL);
    pthread_cond_init(&threadCond, NULL);

    pthread_create(&threadHandle, NULL, native_fileio_thread, this);

    pthread_mutex_lock(&threadMutex);
        pthread_cond_signal(&threadCond);
    pthread_mutex_unlock(&threadMutex);
}

bool NativeFileIO::checkEOF()
{
    if (fd && ((m_eof = (bool)feof(fd)) == true ||
        (m_eof = (ftell(fd) == this->filesize))) && autoClose) {
        
        fclose(fd);
        fd = NULL;
    }

    return m_eof;
}

void NativeFileIO::checkRead()
{
    int err = -1;

    if (ferror(fd)) {
        err = errno;
    } else if (this->checkEOF()) {
        err = 0;
    }

    if (!action.stop && err != -1) {
        messages->postMessage((unsigned int)err, NATIVE_FILEERROR_MESSAGE);
    }
}

NativeFileIO::~NativeFileIO()
{
    action.stop = true;

    pthread_mutex_lock(&threadMutex);
    pthread_cond_signal(&threadCond);
    pthread_mutex_unlock(&threadMutex);

    pthread_join(threadHandle, NULL);
    del_timer(&this->net->timersng, this->timer);

    NativeSharedMessages::Message msg;
    while (messages->readMessage(&msg)) {
        if (msg.event() == NATIVE_FILEREAD_MESSAGE && msg.dataPtr() != NULL) {
            delete[] (unsigned char *)msg.dataPtr();
        }
    }
    delete messages;

    free(filename);
    if (fd != NULL) {
        fclose(fd);
    }
}

