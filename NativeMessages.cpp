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

static NativeSharedMessages *g_MessagesList;

static int NativeMessages_handle(void *arg)
{
#define MAX_MSG_IN_ROW 128
    int nread = 0;

    NativeSharedMessages::Message msg;

    while (++nread < MAX_MSG_IN_ROW && g_MessagesList->readMessage(&msg)) {
        NativeMessages *obj = static_cast<NativeMessages *>(msg.dest());
        obj->onMessage(msg);
    }

    return 8;
}

static void NativeMessages_lost(const NativeSharedMessages::Message &msg)
{
    NativeMessages *obj = static_cast<NativeMessages *>(msg.dest());
    obj->onMessageLost(msg);
}

NativeMessages::~NativeMessages()
{
    g_MessagesList->delMessagesForDest(this);
}

void NativeMessages::postMessage(void *dataptr, int event)
{
    NativeSharedMessages::Message *msg = new NativeSharedMessages::Message(dataptr, event, this);
    g_MessagesList->postMessage(msg);
}

void NativeMessages::postMessage(uint64_t dataint, int event)
{
    NativeSharedMessages::Message *msg = new NativeSharedMessages::Message(dataint, event, this);
    g_MessagesList->postMessage(msg);
}

void NativeMessages::postMessage(NativeSharedMessages::Message *msg)
{
    msg->setDest(this);
    g_MessagesList->postMessage(msg);
}

void NativeMessages::onMessage(const NativeSharedMessages::Message &msg)
{
    printf("Unhandled message sent to %p\n", this);
}

void NativeMessages::onMessageLost(const NativeSharedMessages::Message &msg)
{
    
}

void NativeMessages::initReader(ape_global *ape)
{
    g_MessagesList = new NativeSharedMessages();

    g_MessagesList->setCleaner(NativeMessages_lost);

    ape_timer *timer = add_timer(&ape->timersng, 1,
        NativeMessages_handle, NULL);

    timer->flags &= ~APE_TIMER_IS_PROTECTED;
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
