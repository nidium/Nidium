#include "NativeFileIO.h"
#include "NativeSharedMessages.h"
#include <native_netlib.h>

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
            //printf("Thread ended 1\n");
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
                printf("unknown action\n");
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
    return 1;
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
    uint64_t clamped_len;
    clamped_len = native_min(filesize, len);

    if (this->fd == NULL) {
        return;
    }

    size_t readsize = 0;
    unsigned char *data = new unsigned char[clamped_len];

    if ((readsize = fread(data, sizeof(char), clamped_len, fd)) < 1) {
        if (!action.stop) {
            messages->postMessage((unsigned int)0, NATIVE_FILEERROR_MESSAGE);
        }
        delete[] data;
        return;
    }

    action.u64 = readsize;
    if (!action.stop) {
        messages->postMessage(data, NATIVE_FILEREAD_MESSAGE);
    }
}

void NativeFileIO::openAction(char *modes)
{
    if ((this->fd = fopen(this->filename, modes)) == NULL) {
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
    pthread_mutex_lock(&threadMutex);
    if (action.active) {
        pthread_mutex_unlock(&threadMutex);
        return;
    }
    action.active = true;
    action.type = FILE_ACTION_OPEN;
    action.ptr  = strdup(modes);
    pthread_cond_signal(&threadCond);
    pthread_mutex_unlock(&threadMutex);
}

void NativeFileIO::read(uint64_t len)
{
    pthread_mutex_lock(&threadMutex);
    if (action.active) {
        pthread_mutex_unlock(&threadMutex);
        return;
    }
    action.active = true;
    action.type = FILE_ACTION_READ;
    action.u64  = len;
    pthread_cond_signal(&threadCond);
    pthread_mutex_unlock(&threadMutex);    
}

void NativeFileIO::write(unsigned char *data, uint64_t len)
{
    pthread_mutex_lock(&threadMutex);
    if (action.active) {
        pthread_mutex_unlock(&threadMutex);
        return;
    }
    action.active = true;
    action.type = FILE_ACTION_WRITE;
    action.ptr  = data;
    action.u64  = len;
    pthread_cond_signal(&threadCond);
    pthread_mutex_unlock(&threadMutex);    
}

void NativeFileIO::seek(uint64_t pos)
{
    pthread_mutex_lock(&threadMutex);
    fseek(fd, pos, SEEK_SET);
    pthread_mutex_unlock(&threadMutex);  
}

void NativeFileIO::close()
{
    if (!fd) {
        return;
    }
    pthread_mutex_lock(&threadMutex);
    fclose(fd);
    fd = NULL;
    pthread_mutex_unlock(&threadMutex);  
}


NativeFileIO::NativeFileIO(const char *filename, NativeFileIODelegate *delegate,
    ape_global *net) :
    fd(NULL)
{
    messages = new NativeSharedMessages();
    this->filename = strdup(filename);
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

NativeFileIO::~NativeFileIO()
{
    action.stop = true;

    pthread_mutex_lock(&threadMutex);
    pthread_cond_signal(&threadCond);
    pthread_mutex_unlock(&threadMutex);

    pthread_join(threadHandle, NULL);
    del_timer(&this->net->timersng, this->timer);
    delete messages;
    free(filename);
    if (fd != NULL) {
        fclose(fd);
    }

}

