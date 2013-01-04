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
    
    while (1) {
        pthread_mutex_lock(&NFIO->threadMutex);

        while (!NFIO->action.active) {
            pthread_cond_wait(&NFIO->threadCond, &NFIO->threadMutex);
        }

        switch (NFIO->action.type) {
            case NativeFileIO::FILE_ACTION_OPEN:
                NFIO->openAction();
                break;
            case NativeFileIO::FILE_ACTION_READ:
                NFIO->readAction(NFIO->action.u64);
                break;
            default:
                printf("unknown action\n");
                break;
        }
        NFIO->action.active = false;
        pthread_mutex_unlock(&NFIO->threadMutex);
    }
    printf("end\n");
    
    return NULL;
}

static void *native_open_thread(void *arg)
{
    NativeFileIO *nfileio = (NativeFileIO *)arg;

    if ((nfileio->fd = fopen(nfileio->filename, "r")) == NULL) {
        nfileio->messages->postMessage(errno, NATIVE_FILEERROR_MESSAGE);
        return NULL;
    }
    fseek(nfileio->fd, 0L, SEEK_END);
    nfileio->filesize = ftell(nfileio->fd);
    fseek(nfileio->fd, 0L, SEEK_SET);

    nfileio->messages->postMessage(nfileio->fd, NATIVE_FILEOPEN_MESSAGE);

    return NULL;
}

static void *native_read_thread(void *arg)
{
    NativeFileIO *nfileio = (NativeFileIO *)arg;
    unsigned char *data = new unsigned char[nfileio->filesize];

    if (fread(data, sizeof(char), nfileio->filesize,
        nfileio->fd) != nfileio->filesize) {
        nfileio->messages->postMessage((unsigned int)0, NATIVE_FILEERROR_MESSAGE);
        delete data;
        return NULL;
    }

    fseek(nfileio->fd, 0L, SEEK_SET);

    nfileio->messages->postMessage(data, NATIVE_FILEREAD_MESSAGE);

    return NULL;
}

static int Native_handle_file_messages(void *arg)
{
    NativeFileIO *nfileio = (NativeFileIO *)arg;
    NativeSharedMessages::Message msg;

    while (nfileio->messages->readMessage(&msg)) {
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
                    delete (unsigned char *)msg.dataPtr();
                break;
            default:break;
        }
    }
    return 1;
}

void NativeFileIO::readAction(uint64_t len)
{
    uint64_t clamped_len;
    clamped_len = native_min(filesize, len);

    if (this->fd == NULL) {
        return;
    }
    printf("Start reading %lld size instead of %lld\n", clamped_len, len);
    unsigned char *data = new unsigned char[clamped_len];

    if (fread(data, sizeof(char), clamped_len, fd) < 1) {
        messages->postMessage((unsigned int)0, NATIVE_FILEERROR_MESSAGE);
        delete data;
        return;
    }

    messages->postMessage(data, NATIVE_FILEREAD_MESSAGE);
}

void NativeFileIO::openAction()
{
    if ((this->fd = fopen(this->filename, "r")) == NULL) {
        this->messages->postMessage(errno, NATIVE_FILEERROR_MESSAGE);
        return;
    }
    fseek(this->fd, 0L, SEEK_END);
    this->filesize = ftell(this->fd);
    fseek(this->fd, 0L, SEEK_SET);

    this->messages->postMessage(this->fd, NATIVE_FILEOPEN_MESSAGE);    
}

void NativeFileIO::open()
{
    pthread_mutex_lock(&threadMutex);
    if (action.active) {
        return;
    }
    action.active = true;
    action.type = FILE_ACTION_OPEN;
    pthread_cond_signal(&threadCond);
    pthread_mutex_unlock(&threadMutex);
}

void NativeFileIO::read(uint64_t len)
{
    pthread_mutex_lock(&threadMutex);
    if (action.active) {
        return;
    }
    action.active = true;
    action.type = FILE_ACTION_READ;
    action.u64  = len;
    pthread_cond_signal(&threadCond);
    pthread_mutex_unlock(&threadMutex);    
}

void NativeFileIO::getContents()
{
    if (fd == NULL) {
        return;
    }

    pthread_create(&threadHandle, NULL, native_read_thread, this);
}

NativeFileIO::NativeFileIO(const char *filename, NativeFileIODelegate *delegate,
    ape_global *net) :
    fd(NULL)
{
    messages = new NativeSharedMessages();
    this->filename = strdup(filename);
    this->net = net;
    this->delegate = delegate;

    this->action.active = 0;

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
    del_timer(&this->net->timersng, this->timer);
    delete messages;
    free(filename);
    if (fd != NULL) {
        fclose(fd);
    }
}

