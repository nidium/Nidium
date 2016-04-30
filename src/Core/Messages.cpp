/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Messages.h"

#include <pthread.h>

#include <ape_netlib.h>

#include "Events.h"

namespace Nidium {
namespace Core {

/*
    TODO: make thread local storage
*/
static SharedMessages *g_MessagesList;
static SharedMessages::Message *g_PostingSyncMsg = nullptr;

static int Messages_handle(void *arg)
{
#define MAX_MSG_IN_ROW 256
    int nread = 0;
    bool stopOnAsync = false;
    SharedMessages::Message *msg;

    while (++nread < MAX_MSG_IN_ROW &&
            (msg = g_MessagesList->readMessage(stopOnAsync))) {

        Messages *obj = static_cast<Messages *>(msg->dest());
        obj->onMessage(*msg);

        delete msg;

        if (g_PostingSyncMsg == msg) {
            /*
                Found the message being synchronously processed.
                After this message, any async message must be
                deffered to the next event loop.
            */
            stopOnAsync = true;
        }
    }

    return 8;
}

static void Messages_lost(const SharedMessages::Message &msg)
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
        Events *sender = static_cast<Events *>(item->content.addrs);
        sender->removeListener(this, false);
    }
}

void Messages::postMessage(void *dataptr, int event, bool forceAsync)
{
    SharedMessages::Message *msg = new SharedMessages::Message(dataptr, event);
    this->postMessage(msg, forceAsync);
}

void Messages::postMessage(uint64_t dataint, int event, bool forceAsync)
{
    SharedMessages::Message *msg = new SharedMessages::Message(dataint, event);
    this->postMessage(msg, forceAsync);
}

/*
    Post message in a synchronous way.
    XXX : This method does not ensure FIFO with postMessage() queue
*/
void Messages::postMessageSync(SharedMessages::Message *msg)
{
    this->onMessage(*msg);
}

void Messages::postMessage(SharedMessages::Message *msg, bool forceAsync)
{
    msg->setDest(this);

    /*
       Check if the message can be posted synchronously :
        - Must be sent on the same thread
        - Must not recursively post sync message
        - Must not have forced async message in queue
    */
    if (!forceAsync && pthread_equal(m_GenesisThread, pthread_self()) &&
            g_MessagesList->hasAsyncMessages() && !g_PostingSyncMsg) {

        g_PostingSyncMsg = msg;

        // Ensure that we don't break the FIFO rule.
        // Post the message first and then read all pendings messages
        g_MessagesList->postMessage(msg);
        (void)Messages_handle(nullptr);

        g_PostingSyncMsg = nullptr;
    } else {
        if (forceAsync) {
            msg->setForceAsync();
        }

        g_MessagesList->postMessage(msg);
    }
}

void Messages::InitReader(ape_global *ape)
{
    g_MessagesList = new SharedMessages();

    g_MessagesList->setCleaner(Messages_lost);

    ape_timer_t *timer = APE_timer_create(ape, 1,
        Messages_handle, NULL);

    APE_timer_unprotect(timer);
}

void Messages::listenFor(Events *obj, bool enable)
{
    if (enable) {
        m_Listening.set(reinterpret_cast<uint64_t>(obj), obj);
    } else {
        m_Listening.erase(reinterpret_cast<uint64_t>(obj));
    }
}

void Messages::delMessages(int event)
{
    g_MessagesList->delMessagesForDest(this, event);
}

void Messages::DestroyReader()
{
    delete g_MessagesList;
}

SharedMessages *Messages::getSharedMessages()
{
    return g_MessagesList;
}

} // namespace Core
} // namespace Nidium

