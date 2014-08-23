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

#include "NativeMessages.h"
#include "NativeHash.h"
#include "NativeSharedMessages.h"
#include <native_netlib.h>
#include <stdio.h>
#include <NativeEvents.h>

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

    ape_timer *timer = add_timer(&ape->timersng, 1,
        NativeMessages_handle, NULL);

    timer->flags &= ~APE_TIMER_IS_PROTECTED;
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
