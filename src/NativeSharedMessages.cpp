#include "NativeSharedMessages.h"

#include <stdlib.h>
#include <stdio.h>


NativeSharedMessages::NativeSharedMessages()
{
	messageslist.count = 0;
	messageslist.head  = NULL;
	pthread_mutex_init(&messageslist.lock, NULL);
}

void NativeSharedMessages::postMessage(void *ptr)
{
	native_shared_message *message;

	message = new native_shared_message;

	message->ptr  = ptr;
	message->prev = NULL;

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

void *NativeSharedMessages::readMessage()
{
	NSMAutoLock lock(&messageslist.lock);

	native_shared_message *message = messageslist.queue;

	if (message == NULL) {
		return NULL;
	}

	messageslist.queue = message->prev;

	if (messageslist.queue == NULL) {
		messageslist.head = NULL;
	}
	messageslist.count--;
	return message->ptr;
}
