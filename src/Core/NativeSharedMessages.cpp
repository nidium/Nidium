/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "NativeSharedMessages.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "NativeUtils.h"

NativeSharedMessages::NativeSharedMessages() :
    m_Cleaner(NULL)
{
    messageslist.count = 0;
    messageslist.asyncCount = 0;
    messageslist.head  = NULL;
    messageslist.queue = NULL;

    pthread_mutex_init(&messageslist.lock, NULL);
}

NativeSharedMessages::~NativeSharedMessages()
{
    this->delMessagesForDest(NULL);
}

void NativeSharedMessages::postMessage(Message *message)
{
    this->addMessage(message);
}

void NativeSharedMessages::postMessage(void *dataptr, int event)
{
    this->addMessage(new Message(dataptr, event));
}

void NativeSharedMessages::postMessage(uint64_t dataint, int event)
{
    this->addMessage(new Message(dataint, event));
}

void NativeSharedMessages::addMessage(Message *msg) 
{
    NativePthreadAutoLock lock(&messageslist.lock);

    if (messageslist.head) {
        messageslist.head->prev = msg;
    }

    if (messageslist.queue == NULL) {
        messageslist.queue = msg;
    }

    if (msg->forceAsync()) {
        messageslist.asyncCount++;
    }

    messageslist.head = msg;
    messageslist.count++;
}

NativeSharedMessages::Message *NativeSharedMessages::readMessage()
{
    NativePthreadAutoLock lock(&messageslist.lock);

    Message *message = messageslist.queue;

    if (message == NULL) {
        return NULL;
    }

    messageslist.queue = message->prev;

    if (messageslist.queue == NULL) {
        messageslist.head = NULL;
    }

    if (message->forceAsync()) {
        messageslist.asyncCount--;
    }

    messageslist.count--;

    return message;
}

void NativeSharedMessages::delMessagesForDest(void *dest, int event)
{
    NativePthreadAutoLock lock(&messageslist.lock);

    Message *message = messageslist.queue;
    Message *next = NULL;

    while (message != NULL) {
        Message *tmp = message->prev;

        if ((dest == NULL || message->dest() == dest) &&
            (event == -1 || event == message->event())) {

            if (next != NULL) {
                next->prev = message->prev;
            } else {
                messageslist.queue = message->prev;
            }

            if (messageslist.queue == NULL) {
                messageslist.head = NULL;
            }

            if (message->forceAsync()) {
                messageslist.asyncCount--;
            }

            if (m_Cleaner) {
                m_Cleaner(*message);
            }

            delete message;
            messageslist.count--;
        } else {
            next = message;
        }

        message = tmp;
    }
}
