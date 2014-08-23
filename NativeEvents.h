/*
    NativeJS Core Library
    Copyright (C) 2014 Anthony Catel <paraboul@gmail.com>

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

#ifndef nativeevents_h__
#define nativeevents_h__

#include <NativeHash.h>
#include <NativeArgs.h>

#include <NativeMessages.h>

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
    void fireEvent(typename T::Events event, const NativeArgs &args,
        bool forceAsync = false) {
        
        ape_htable_item_t *item;

        for (item = m_Listeners.accessCStruct()->first; item != NULL; item = item->lnext) {
            NativeMessages *receiver = (NativeMessages *)item->content.addrs;

            NativeSharedMessages::Message *msg = new NativeSharedMessages::Message(NATIVE_EVENTS_MESSAGE_BITS(event) | (T::EventID << 16));

            msg->args[0].set(static_cast<T *>(this));

            for (int i = 0; i < args.size(); i++) {
                msg->args[i+1].set(args[i].toInt64());
            }

            receiver->postMessage(msg, forceAsync);
        }
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
