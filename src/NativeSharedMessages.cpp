#include "NativeSharedMessages.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

NativeSharedMessages::NativeSharedMessages()
{
	messageslist.count = 0;
	messageslist.head  = NULL;
	messageslist.queue = NULL;
	pthread_mutex_init(&messageslist.lock, NULL);
}

NativeSharedMessages::~NativeSharedMessages()
{
	while (readMessage(NULL));
}

void NativeSharedMessages::postMessage(void *dataptr, int event)
{
	Message *message;

	if (dataptr == NULL) {
		return;
	}

	message = new Message(dataptr, event);

	NSMAutoLock lock(&messageslist.lock);

	if (messageslist.head) {
		messageslist.head->prev = message;
	}

	if (messageslist.queue == NULL) {
		messageslist.queue = message;
	}

	messageslist.head = message;
	messageslist.count++;
}

void NativeSharedMessages::postMessage(unsigned int dataint, int event)
{
	Message *message;

	message = new Message(dataint, event);

	NSMAutoLock lock(&messageslist.lock);

	if (messageslist.head) {
		messageslist.head->prev = message;
	}

	if (messageslist.queue == NULL) {
		messageslist.queue = message;
	}

	messageslist.head = message;
	messageslist.count++;
}

int NativeSharedMessages::readMessage(NativeSharedMessages::Message *msg)
{
	NSMAutoLock lock(&messageslist.lock);

	Message *message = messageslist.queue;

	if (message == NULL) {
		return 0;
	}

	messageslist.queue = message->prev;

	if (messageslist.queue == NULL) {
		messageslist.head = NULL;
	}

	messageslist.count--;

	if (msg != NULL) {
		memcpy(msg, message, sizeof(Message));
	}

	delete message;

	return 1;
}
