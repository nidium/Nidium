/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Core/SharedMessages.h"

#include <pthread.h>

#include "Utils.h"

namespace Nidium {
namespace Core {

SharedMessages::SharedMessages() : m_Cleaner(NULL)
{
    m_MessagesList.count      = 0;
    m_MessagesList.asyncCount = 0;
    m_MessagesList.head       = NULL;
    m_MessagesList.queue      = NULL;

    pthread_mutex_init(&m_MessagesList.lock, NULL);
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
    PthreadAutoLock lock(&m_MessagesList.lock);

    if (m_MessagesList.head) {
        m_MessagesList.head->prev = msg;
    }

    if (m_MessagesList.queue == NULL) {
        m_MessagesList.queue = msg;
    }

    if (msg->forceAsync()) {
        m_MessagesList.asyncCount++;
    }

    m_MessagesList.head = msg;
    m_MessagesList.count++;
}

SharedMessages::Message *SharedMessages::readMessage(bool stopOnAsync)
{
    PthreadAutoLock lock(&m_MessagesList.lock);

    Message *message = m_MessagesList.queue;

    if (message == NULL) {
        return NULL;
    }

    if (stopOnAsync && message->forceAsync()) {
        return NULL;
    }

    m_MessagesList.queue = message->prev;

    if (m_MessagesList.queue == NULL) {
        m_MessagesList.head = NULL;
    }

    if (message->forceAsync()) {
        m_MessagesList.asyncCount--;
    }

    m_MessagesList.count--;

    return message;
}

void SharedMessages::delMessagesForDest(void *dest, int event)
{
    PthreadAutoLock lock(&m_MessagesList.lock);

    Message *message = m_MessagesList.queue;
    Message *next    = NULL;

    while (message != NULL) {
        Message *tmp = message->prev;

        if ((dest == NULL || message->dest() == dest)
            && (event == -1 || event == message->event())) {

            if (next != NULL) {
                next->prev = message->prev;
            } else {
                m_MessagesList.queue = message->prev;
            }

            if (message == m_MessagesList.head) {
                m_MessagesList.head = next;
            }

            if (m_MessagesList.queue == NULL) {
                m_MessagesList.head = NULL;
            }

            if (message->forceAsync()) {
                m_MessagesList.asyncCount--;
            }

            if (m_Cleaner) {
                m_Cleaner(*message);
            }

            delete message;
            m_MessagesList.count--;
        } else {
            next = message;
        }

        message = tmp;
    }
}

} // namespace Core
} // namespace Nidium
