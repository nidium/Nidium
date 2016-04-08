/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativemessages_h__
#define nativemessages_h__

#include <pthread.h>

#include "NativeSharedMessages.h"
#include "NativeHash.h"

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
    virtual void onMessage(const NativeSharedMessages::Message &msg) {};
    virtual void onMessageLost(const NativeSharedMessages::Message &msg) {};

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

