/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "NativeMessages.h"

#include <pthread.h>

#include <ape_netlib.h>

#include "NativeEvents.h"

/*
    TODO: make thread local storage
*/
static NativeSharedMessages *g_MessagesList;

static int NativeMessages_handle(void *arg)
{
#define MAX_MSG_IN_ROW 256
    int nread = 0;

    NativeSharedMessages::Message *msg;

    /*
        TODO: need a lock for "obj"
    */
    while (++nread < MAX_MSG_IN_ROW && (msg = g_MessagesList->readMessage())) {
        NativeMessages *obj = static_cast<NativeMessages *>(msg->dest());
        obj->onMessage(*msg);

        delete msg;
    }

    return 8;
}

static void NativeMessages_lost(const NativeSharedMessages::Message &msg)
{
    NativeMessages *obj = static_cast<NativeMessages *>(msg.dest());
    obj->onMessageLost(msg);
}

NativeMessages::NativeMessages() :
    m_Listening(8)
{
    m_GenesisThread = pthread_self();
}

NativeMessages::~NativeMessages()
{
    g_MessagesList->delMessagesForDest(this);

    ape_htable_item_t *item;

    for (item = m_Listening.accessCStruct()->first; item != NULL; item = item->lnext) {
        NativeEvents *sender = (NativeEvents *)item->content.addrs;
        sender->removeListener(this, false);
    }
}

void NativeMessages::postMessage(void *dataptr, int event, bool forceAsync)
{
    NativeSharedMessages::Message *msg = new NativeSharedMessages::Message(dataptr, event);
    this->postMessage(msg, forceAsync);
}

void NativeMessages::postMessage(uint64_t dataint, int event, bool forceAsync)
{
    NativeSharedMessages::Message *msg = new NativeSharedMessages::Message(dataint, event);
    this->postMessage(msg, forceAsync);
}

void NativeMessages::postMessage(NativeSharedMessages::Message *msg, bool forceAsync)
{
    msg->setDest(this);

    /*
        Message sent from the same thread. Don't need
        to be sent in an asynchronous way
    */
    if (!forceAsync && pthread_equal(m_GenesisThread, pthread_self())) {
        // Make sure pending messagess are read so that we don't break the FIFO rule
        (void)NativeMessages_handle(NULL);

        this->onMessage(*msg);
        delete msg;
    } else {
        g_MessagesList->postMessage(msg);
    }
}

void NativeMessages::initReader(ape_global *ape)
{
    g_MessagesList = new NativeSharedMessages();

    g_MessagesList->setCleaner(NativeMessages_lost);

    ape_timer_t *timer = APE_timer_create(ape, 1,
        NativeMessages_handle, NULL);
    
    APE_timer_unprotect(timer);
}

void NativeMessages::listenFor(NativeEvents *obj, bool enable)
{
    if (enable) {
        m_Listening.set((uint64_t)obj, obj);
    } else {
        m_Listening.erase((uint64_t)obj);
    }
}

void NativeMessages::delMessages(int event)
{
    g_MessagesList->delMessagesForDest(this, event);
}

void NativeMessages::destroyReader()
{
    delete g_MessagesList;
}

NativeSharedMessages *NativeMessages::getSharedMessages()
{
    return g_MessagesList;
}

