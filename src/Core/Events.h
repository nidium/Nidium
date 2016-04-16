/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef core_events_h__
#define core_events_h__

#include "Hash.h"
#include "Messages.h"

namespace Nidium {
namespace Core {

#define NIDIUM_EVENTS_MESSAGE_BITS(id) ((1 << 31) | id)
#define NIDIUM_EVENT(classe, event) (NIDIUM_EVENTS_MESSAGE_BITS(classe::event) | (classe::EventID << 16))

/*
    Implementation Note :

    Children of Nidium::Core::Events must define a static
    const property "static const uint8_t EventID" with an unique 8bit identifier
*/
class Events
{
public:
    void addListener(Messages *listener) {
        m_Listeners.set((uint64_t)(Messages *)listener, listener);

        listener->listenFor(this, true);
    }
    void removeListener(Messages *listener, bool propagate = true) {
        m_Listeners.erase((uint64_t)listener);

        if (propagate) {
            listener->listenFor(this, false);
        }
    }
    template <typename T>
    bool fireEvent(typename T::Events event, const Nidium::Core::Args &args,
        bool forceAsync = false) {

        ape_htable_item_t *item;

        for (item = m_Listeners.accessCStruct()->first; item != NULL; item = item->lnext) {
            Messages *receiver = (Messages *)item->content.addrs;

            Nidium::Core::SharedMessages::Message *msg = new Nidium::Core::SharedMessages::Message(NIDIUM_EVENTS_MESSAGE_BITS(event) |
                                                                                   (T::EventID << 16));

            msg->args[0].set(static_cast<T *>(this));

            for (int i = 0; i < args.size(); i++) {
                msg->args[i+1].set(args[i].toInt64());
            }
            msg->priv = 0;

            receiver->postMessage(msg, forceAsync);
#if 0
            /* TODO FIX : Use after free here */
            /* stop propagation */
            if (msg->priv) {
                return false;
            }
#endif
        }

        return true;
    }

    virtual ~Events() {
        ape_htable_item_t *item;

        for (item = m_Listeners.accessCStruct()->first; item != NULL; item = item->lnext) {
            Messages *receiver = (Messages *)item->content.addrs;

            receiver->listenFor(this, false);
        }
    }

private:

    Nidium::Core::Hash64<Messages *> m_Listeners;
};

} // namespace Core
} // namespace Nidium

#endif

