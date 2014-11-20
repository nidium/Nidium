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

#ifndef nativemessages_h__
#define nativemessages_h__

#include <NativeSharedMessages.h>
#include <NativeHash.h>
#include <pthread.h>

typedef struct _ape_global ape_global;
class NativeEvents;

#define CREATE_MESSAGE(var, ev) NativeSharedMessages::Message *var = new NativeSharedMessages::Message(ev);

class NativeMessages
{
public:
    friend class NativeEvents;
    NativeMessages();
    virtual ~NativeMessages()=0;

    /*
        Derived classes must implement this in order to catch messages
    */
    virtual void onMessage(const NativeSharedMessages::Message &msg){};
    virtual void onMessageLost(const NativeSharedMessages::Message &msg){};

    void postMessage(void *dataptr, int event, bool forceAsync = false);
    void postMessage(uint64_t dataint, int event, bool forceAsync = false);
    void postMessage(NativeSharedMessages::Message *msg, bool forceAsync = false);
    void delMessages(int event = -1);

    static void initReader(ape_global *ape);
    static void destroyReader();

    NativeSharedMessages *getSharedMessages();

private:
    void listenFor(NativeEvents *obj, bool enable);
    pthread_t m_GenesisThread;

    /* Keep track on which objects we are listening events */
    NativeHash64<NativeEvents *>m_Listening;
};

#endif