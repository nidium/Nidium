/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef core_messages_h__
#define core_messages_h__

#include <set>

#include "Core/SharedMessages.h"
#include "Core/Hash.h"

typedef struct _ape_global ape_global;

namespace Nidium {
namespace Core {

class Events;

#define CREATE_MESSAGE(var, ev)                \
    Nidium::Core::SharedMessages::Message *var \
        = new Nidium::Core::SharedMessages::Message(ev);

class Messages
{
public:
    friend class Events;
    Messages();
    virtual ~Messages() = 0;

    /*
        Derived classes must implement this in order to catch messages
    */
    virtual void onMessage(const SharedMessages::Message &msg)
    {
    }
    virtual void onMessageLost(const SharedMessages::Message &msg)
    {
    }

    void postMessage(void *dataptr, int event, bool forceAsync = false);
    void postMessage(uint64_t dataint, int event, bool forceAsync = false);
    void postMessage(SharedMessages::Message *msg, bool forceAsync = false);
    void postMessageSync(SharedMessages::Message *msg);
    void delMessages(int event = -1);

    bool hasPendingMessages() const {
        return _m_CountMessagePending;
    }

    static void InitReader(ape_global *ape);
    static void DestroyReader();

    SharedMessages *getSharedMessages();


    int32_t _m_CountMessagePending = 0;
protected:
    void cleanupMessages();

private:
    void listenFor(Events *obj, bool enable);
    pthread_t m_GenesisThread;

    /* Keep track on which objects we are listening events */
    std::set<Events *> m_Listening_s;

};

} // namespace Core
} // namespace Nidium

#endif
