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
	FILE *fp;

	if ((fp = fopen(nfileio->filename, "r")) == NULL) {
		nfileio->messages->postMessage(errno, NATIVE_FILEERROR_MESSAGE);
	}

	return NULL;
}

static int Native_handle_file_messages(void *arg)
{
	NativeFileIO *nfileio = (NativeFileIO *)arg;
	NativeSharedMessages::Message msg;

	while (nfileio->messages->readMessage(&msg)) {
		switch (msg.event()) {
			case NATIVE_FILEOPEN_MESSAGE:
			printf("success open file (message)\n");
			break;
			case NATIVE_FILEERROR_MESSAGE:
			printf("Can't open file (message)\n");
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

NativeFileIO::NativeFileIO(const char *filename, ape_global *net)
{
	messages = new NativeSharedMessages();
    this->filename = strdup(filename);
    this->net = net;

    timer = add_timer(&this->net->timersng, 1,
        Native_handle_file_messages, this);

}

NativeFileIO::~NativeFileIO()
{
	del_timer(&this->net->timersng, this->timer);
	delete messages;
    free(filename);
}

