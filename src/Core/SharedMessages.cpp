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
    Nidium::Core::PthreadAutoLock lock(&messageslist.lock);

    if (messageslist.head) {
        messageslist.head->prev = message;
    }

    if (messageslist.queue == NULL) {
        messageslist.queue = message;
    }

    messageslist.head = message;
    messageslist.count++;
}

void SharedMessages::postMessage(void *dataptr, int event)
{
    Message *message;

    message = new Message(dataptr, event);

    Nidium::Core::PthreadAutoLock lock(&messageslist.lock);

    if (messageslist.head) {
        messageslist.head->prev = message;
    }

    if (messageslist.queue == NULL) {
        messageslist.queue = message;
    }

    messageslist.head = message;
    messageslist.count++;
}

void SharedMessages::postMessage(uint64_t dataint, int event)
{
    Message *message;

    message = new Message(dataint, event);

    Nidium::Core::PthreadAutoLock lock(&messageslist.lock);

    if (messageslist.head) {
        messageslist.head->prev = message;
    }

    if (messageslist.queue == NULL) {
        messageslist.queue = message;
    }

    messageslist.head = message;
    messageslist.count++;
}

SharedMessages::Message *SharedMessages::readMessage()
{
    Nidium::Core::PthreadAutoLock lock(&messageslist.lock);

    Message *message = messageslist.queue;

    if (message == NULL) {
        return NULL;
    }

    messageslist.queue = message->prev;

    if (messageslist.queue == NULL) {
        messageslist.head = NULL;
    }

    messageslist.count--;

    return message;
}

SharedMessages::Message *SharedMessages::readMessage(int type)
{
    Nidium::Core::PthreadAutoLock lock(&messageslist.lock);

    Message *message = messageslist.queue;
    Message *next = NULL;

    if (message == NULL) {
        printf("no message to delete\n");
        return NULL;
    }

    while (message != NULL && message->event() != type) {
        next = message;
        message = message->prev;
    }

    if (message == NULL) {
        return NULL;
    }

    if (message == messageslist.queue) {
        messageslist.queue = message->prev;
    } else {
        next->prev = message->prev;
    }

    if (messageslist.queue == NULL) {
        messageslist.head = NULL;
    }

    messageslist.count--;

    return message;
}

void SharedMessages::delMessagesForDest(void *dest, int event)
{
    Nidium::Core::PthreadAutoLock lock(&messageslist.lock);

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

