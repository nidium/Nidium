/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef core_messages_h__
#define core_messages_h__

#include <pthread.h>

#include "SharedMessages.h"
#include "Hash.h"

typedef struct _ape_global ape_global;

namespace Nidium {
	namespace Core {
		class Events;
	}
}

namespace Nidium {
namespace Core {

#define CREATE_MESSAGE(var, ev) Nidium::Core::SharedMessages::Message *var = new Nidium::Core::SharedMessages::Message(ev);

class Messages
{
public:
    friend class Nidium::Core::Events;
    Messages();
    virtual ~Messages()=0;

    /*
        Derived classes must implement this in order to catch messages
    */
    virtual void onMessage(const Nidium::Core::SharedMessages::Message &msg) {};
    virtual void onMessageLost(const Nidium::Core::SharedMessages::Message &msg) {};

    void postMessage(void *dataptr, int event, bool forceAsync = false);
    void postMessage(uint64_t dataint, int event, bool forceAsync = false);
    void postMessage(Nidium::Core::SharedMessages::Message *msg, bool forceAsync = false);
    void delMessages(int event = -1);

    static void initReader(ape_global *ape);
    static void destroyReader();

    Nidium::Core::SharedMessages *getSharedMessages();

private:
    void listenFor(Nidium::Core::Events *obj, bool enable);
    pthread_t m_GenesisThread;

    /* Keep track on which objects we are listening events */
    Nidium::Core::Hash64<Nidium::Core::Events *>m_Listening;
};

} // namespace Core
} // namespace Nidium

#endif

