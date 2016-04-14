/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativeevents_h__
#define nativeevents_h__

#include "NativeHash.h"
#include "NativeMessages.h"

#define NATIVE_EVENTS_MESSAGE_BITS(id) ((1 << 31) | id)
#define NATIVE_EVENT(classe, event) (NATIVE_EVENTS_MESSAGE_BITS(classe::event) | (classe::EventID << 16))

/*
    Implementation Note :

    Children of NativeEvents must define a static
    const property "static const uint8_t EventID" with an unique 8bit identifier
*/
class NativeEvents
{
public:
    void addListener(NativeMessages *listener) {
        m_Listeners.set((uint64_t)(NativeMessages *)listener, listener);

        listener->listenFor(this, true);
    }
    void removeListener(NativeMessages *listener, bool propagate = true) {
        m_Listeners.erase((uint64_t)listener);

        if (propagate) {
            listener->listenFor(this, false);
        }
    }
    template <typename T>
    bool fireEvent(typename T::Events event, const NativeArgs &args,
        bool forceAsync = false) {

        ape_htable_item_t *item;

        for (item = m_Listeners.accessCStruct()->first; item != NULL; item = item->lnext) {
            NativeMessages *receiver = (NativeMessages *)item->content.addrs;

            NativeSharedMessages::Message *msg = new NativeSharedMessages::Message(NATIVE_EVENTS_MESSAGE_BITS(event) |
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

    virtual ~NativeEvents() {
        ape_htable_item_t *item;

        for (item = m_Listeners.accessCStruct()->first; item != NULL; item = item->lnext) {
            NativeMessages *receiver = (NativeMessages *)item->content.addrs;

            receiver->listenFor(this, false);
        }
    }

private:

    NativeHash64<NativeMessages *> m_Listeners;
};

#endif

