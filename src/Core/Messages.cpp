/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Core/Messages.h"

#include <stdbool.h>
#include <pthread.h>

#include <assert.h>

#include <ape_netlib.h>

#include "Core/Atomic.h"
#include "Core/Events.h"

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
    int nread        = 0;
    bool stopOnAsync = false;
    SharedMessages::Message *msg;

    while (++nread < MAX_MSG_IN_ROW
           && (msg = g_MessagesList->readMessage(stopOnAsync))) {

        Messages *obj = static_cast<Messages *>(msg->dest());

        Atomic::Dec(&obj->_m_CountMessagePending);

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

Messages::Messages()
{
    m_GenesisThread = pthread_self();
}

Messages::~Messages()
{
    cleanupMessages();

    for (Events *const &sender : m_Listening_s) {
        sender->removeListener(this, false);
    }
}

void Messages::cleanupMessages()
{
    if (g_MessagesList == nullptr) {
        return;
    }

    g_MessagesList->delMessagesForDest(this);
}

void Messages::postMessage(void *dataptr, int event, bool forceAsync)
{
    SharedMessages::Message *msg = new SharedMessages::Message(dataptr, event);
    postMessage(msg, forceAsync);
}

void Messages::postMessage(uint64_t dataint, int event, bool forceAsync)
{
    SharedMessages::Message *msg = new SharedMessages::Message(dataint, event);
    postMessage(msg, forceAsync);
}

/*
    Post message in a synchronous way.
    XXX : This method does not ensure FIFO with postMessage() queue
*/
void Messages::postMessageSync(SharedMessages::Message *msg)
{
    this->onMessage(*msg);
    delete msg;
}

void Messages::postMessageSync(void *dataptr, int event)
{
    SharedMessages::Message *msg = new SharedMessages::Message(dataptr, event);
    postMessageSync(msg);
}


void Messages::postMessage(SharedMessages::Message *msg, bool forceAsync)
{
    /* Don't postMessage after DestroyReader() */
    assert(g_MessagesList != nullptr);

    msg->setDest(this);

    Atomic::Inc(&_m_CountMessagePending);

    /*
       Check if the message can be posted synchronously :
        - Must be sent on the same thread
        - Must not recursively post sync message
        - Must not have forced async message in queue
    */
    if (!forceAsync && pthread_equal(m_GenesisThread, pthread_self())
        && !g_MessagesList->hasAsyncMessages() && !g_PostingSyncMsg) {

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

    ape_timer_t *timer = APE_timer_create(ape, 1, Messages_handle, NULL);

    APE_timer_unprotect(timer);
}

void Messages::listenFor(Events *obj, bool enable)
{
    if (enable) {
        m_Listening_s.insert(obj);
    } else {
        m_Listening_s.erase(obj);
    }
}

void Messages::delMessages(int event)
{
    g_MessagesList->delMessagesForDest(this, event);
}

void Messages::DestroyReader()
{
    delete g_MessagesList;

    g_MessagesList = nullptr;
}

SharedMessages *Messages::getSharedMessages()
{
    return g_MessagesList;
}

} // namespace Core
} // namespace Nidium
