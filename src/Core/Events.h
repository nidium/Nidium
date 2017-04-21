/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef core_events_h__
#define core_events_h__

#include "Core/Hash.h"
#include "Core/Messages.h"

#include <set>

namespace Nidium {
namespace Core {

#define NIDIUM_EVENTS_MESSAGE_BITS(id) ((1 << 31) | id)
#define NIDIUM_EVENT(classe, event) \
    (NIDIUM_EVENTS_MESSAGE_BITS(classe::event) | (classe::EventID << 16))

/*
    Implementation Note :

    Children of Events must define a static
    const property "static const uint8_t EventID" with an unique 8bit identifier
*/

class Events
{
public:
    enum EventStateFlags {
        kEventStateFlag_stopped   = 1,
        kEventStateFlag_prevented = 1 << 1
    };

    struct EventState {
        bool stopped          = false;
        bool defaultPrevented = false;
    };

    void addListener(Messages *listener)
    {
        m_Listeners_s.insert(listener);

        listener->listenFor(this, true);
    }
    void removeListener(Messages *listener, bool propagate = true)
    {
        m_Listeners_s.erase(listener);

        if (propagate) {
            listener->listenFor(this, false);
        }
    }

    template <typename T>
    bool fireEventSync(typename T::Events event,
                       const Args &args,
                       EventState *state = nullptr)
    {
        return this->fireEventImpl<T>(event, args, state,
                                      Events::kPropagationMode_Sync);
    }

    template <typename T>
    bool fireEvent(typename T::Events event,
                   const Args &args,
                   bool forceAsync = false)
    {

        return this->fireEventImpl<T>(
            event, args, nullptr,
            forceAsync ? Events::kPropagationMode_Async
                       : Events::kPropagationMode_Auto);
    }

    virtual ~Events()
    {
        for (Messages *const &receiver : m_Listeners_s) {
            receiver->listenFor(this, false);
        }
    }

private:
    enum PropagationMode
    {
        kPropagationMode_Auto,
        kPropagationMode_Sync,
        kPropagationMode_Async
    };

    std::set<Messages *> m_Listeners_s;

    template <typename T>
    bool fireEventImpl(typename T::Events event,
                       const Args &args,
                       EventState *state,
                       PropagationMode propagation
                       = Events::kPropagationMode_Auto)
    {
        for (Messages *const &receiver : m_Listeners_s) {
            SharedMessages::Message *msg = new SharedMessages::Message(
                NIDIUM_EVENTS_MESSAGE_BITS(event) | (T::EventID << 16));

            msg->m_Args[0].set(static_cast<T *>(this));

            for (int i = 0; i < args.size(); i++) {
                msg->m_Args[i + 1].set(args[i].toInt64());
            }
            msg->m_Priv = 0;

            if (propagation == Events::kPropagationMode_Sync) {
                Core::PtrAutoDelete<SharedMessages::Message *> _msg(msg);
                receiver->postMessageSync(msg, false /* deleteMsg */);

                if (msg->m_Priv & kEventStateFlag_stopped) {
                    if (state) state->stopped = true;
                    return false;
                }

                if(msg->m_Priv & kEventStateFlag_prevented) {
                    if (state) state->defaultPrevented = true;
                    return false;
                }
            } else {
                receiver->postMessage(
                    msg, propagation == Events::kPropagationMode_Async ? true
                                                                       : false);
            }
        }


        return true;
    }
};

} // namespace Core
} // namespace Nidium

#endif
