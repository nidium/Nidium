/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "SharedMessages.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Utils.h"

namespace Nidium {
namespace Core {

SharedMessages::SharedMessages() :

    m_Cleaner(NULL)
{
    messageslist.count = 0;
    messageslist.asyncCount = 0;
    messageslist.head  = NULL;
    messageslist.queue = NULL;

    pthread_mutex_init(&messageslist.lock, NULL);
}

SharedMessages::~SharedMessages()
{
    this->delMessagesForDest(NULL);
}

void SharedMessages::postMessage(Message *message)
{
    this->addMessage(message);
}

void SharedMessages::postMessage(void *dataptr, int event)
{
    this->addMessage(new Message(dataptr, event));
}

void SharedMessages::postMessage(uint64_t dataint, int event)
{
    this->addMessage(new Message(dataint, event));
}

void SharedMessages::addMessage(Message *msg) 
{
    PthreadAutoLock lock(&messageslist.lock);

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

SharedMessages::Message *SharedMessages::readMessage(bool stopOnAsync)
{
    PthreadAutoLock lock(&messageslist.lock);

    Message *message = messageslist.queue;

    if (message == NULL) {
        return NULL;
    }

    if (stopOnAsync && message->forceAsync()) {
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

void SharedMessages::delMessagesForDest(void *dest, int event)
{
    PthreadAutoLock lock(&messageslist.lock);

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

} // namespace Core
} // namespace Nidium

