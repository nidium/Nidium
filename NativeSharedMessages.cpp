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

#include "NativeSharedMessages.h"
#include "NativeUtils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

NativeSharedMessages::NativeSharedMessages() : 
    m_Cleaner(NULL)
{
    messageslist.count = 0;
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
    NativePthreadAutoLock lock(&messageslist.lock);

    if (messageslist.head) {
        messageslist.head->prev = message;
    }

    if (messageslist.queue == NULL) {
        messageslist.queue = message;
    }

    messageslist.head = message;
    messageslist.count++;
}

void NativeSharedMessages::postMessage(void *dataptr, int event)
{
    Message *message;

    message = new Message(dataptr, event);

    NativePthreadAutoLock lock(&messageslist.lock);

    if (messageslist.head) {
        messageslist.head->prev = message;
    }

    if (messageslist.queue == NULL) {
        messageslist.queue = message;
    }

    messageslist.head = message;
    messageslist.count++;
}

void NativeSharedMessages::postMessage(uint64_t dataint, int event)
{
    Message *message;

    message = new Message(dataint, event);

    NativePthreadAutoLock lock(&messageslist.lock);

    if (messageslist.head) {
        messageslist.head->prev = message;
    }

    if (messageslist.queue == NULL) {
        messageslist.queue = message;
    }

    messageslist.head = message;
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

    messageslist.count--;

    return message;
}

NativeSharedMessages::Message *NativeSharedMessages::readMessage(int type)
{
    NativePthreadAutoLock lock(&messageslist.lock);

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

