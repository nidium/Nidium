#include "NativeFileIO.h"
#include "NativeSharedMessages.h"
#include <native_netlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

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
                    (unsigned char *)msg.dataPtr(), nfileio->filesize);
                    delete (unsigned char *)msg.dataPtr();
                break;
            default:break;
        }
    }
    return 1;
}

void NativeFileIO::open()
{
    pthread_create(&threadHandle, NULL, native_open_thread, this);
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

    timer = add_timer(&this->net->timersng, 1,
        Native_handle_file_messages, this);

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

