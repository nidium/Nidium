/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Messages.h"

#include <pthread.h>

#include <native_netlib.h>

#include "Events.h"

namespace Nidium {
namespace Core {

/*
    TODO: make thread local storage
*/
static Nidium::Core::SharedMessages *g_MessagesList;

static int Messages_handle(void *arg)
{
#define MAX_MSG_IN_ROW 256
    int nread = 0;

    Nidium::Core::SharedMessages::Message *msg;

    /*
        TODO: need a lock for "obj"
    */
    while (++nread < MAX_MSG_IN_ROW && (msg = g_MessagesList->readMessage())) {
        Messages *obj = static_cast<Messages *>(msg->dest());
        obj->onMessage(*msg);

        delete msg;
    }

    return 8;
}

static void Messages_lost(const Nidium::Core::SharedMessages::Message &msg)
{
    Messages *obj = static_cast<Messages *>(msg.dest());
    obj->onMessageLost(msg);
}

Messages::Messages() :
    m_Listening(8)
{
    m_GenesisThread = pthread_self();
}

Messages::~Messages()
{
    g_MessagesList->delMessagesForDest(this);

    ape_htable_item_t *item;

    for (item = m_Listening.accessCStruct()->first; item != NULL; item = item->lnext) {
        Nidium::Core::Events *sender = (Nidium::Core::Events *)item->content.addrs;
        sender->removeListener(this, false);
    }
}

void Messages::postMessage(void *dataptr, int event, bool forceAsync)
{
    Nidium::Core::SharedMessages::Message *msg = new Nidium::Core::SharedMessages::Message(dataptr, event);
    this->postMessage(msg, forceAsync);
}

void Messages::postMessage(uint64_t dataint, int event, bool forceAsync)
{
    Nidium::Core::SharedMessages::Message *msg = new Nidium::Core::SharedMessages::Message(dataint, event);
    this->postMessage(msg, forceAsync);
}

void Messages::postMessage(Nidium::Core::SharedMessages::Message *msg, bool forceAsync)
{
    msg->setDest(this);

    /*
        Message sent from the same thread. Don't need
        to be sent in an asynchronous way
    */
    if (!forceAsync && pthread_equal(m_GenesisThread, pthread_self())) {
        // Make sure pending messagess are read so that we don't break the FIFO rule
        (void)Messages_handle(NULL);

        this->onMessage(*msg);
        delete msg;
    } else {
        g_MessagesList->postMessage(msg);
    }
}

void Messages::initReader(ape_global *ape)
{
    g_MessagesList = new Nidium::Core::SharedMessages();

    g_MessagesList->setCleaner(Messages_lost);

    ape_timer_t *timer = APE_timer_create(ape, 1,
        Messages_handle, NULL);

    APE_timer_unprotect(timer);
}

void Messages::listenFor(Nidium::Core::Events *obj, bool enable)
{
    if (enable) {
        m_Listening.set((uint64_t)obj, obj);
    } else {
        m_Listening.erase((uint64_t)obj);
    }
}

void Messages::delMessages(int event)
{
    g_MessagesList->delMessagesForDest(this, event);
}

void Messages::destroyReader()
{
    delete g_MessagesList;
}

Nidium::Core::SharedMessages *Messages::getSharedMessages()
{
    return g_MessagesList;
}

} // namespace Core
} // namespace Nidium

