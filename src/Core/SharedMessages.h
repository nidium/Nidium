/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef core_sharedmessages_h__
#define core_sharedmessages_h__

#include <stdint.h>

#include "Core/Args.h"

#ifdef _MSC_VER
#include "port/MSWindows.h"
#endif

/*
    TODO: Add "max messages in queue" to guard memory congestion in case of
   allocation
         (i.e. Nidium::Core:App extractor)
*/

namespace Nidium {
namespace Core {

typedef struct _nidium_shared_message
{
    void *ptr;
    struct _nidium_shared_message *prev;
} nidium_shared_message;

class SharedMessages
{
public:
    class Message
    {
    public:
        Message(void *ptr, int type, void *dest = NULL)
            : prev(NULL), m_Type(type), m_Dest(dest)
        {
            m_Msgdata.dataptr = ptr;
        }
        Message(uint64_t dataint, int type, void *dest = NULL)
            : prev(NULL), m_Type(type), m_Dest(dest)
        {
            m_Msgdata.dataint = dataint;
        }

        Message(int type) : prev(NULL), m_Type(type), m_Dest(NULL)
        {
        }

        ~Message()
        {
        }

        Message()
        {
        }

        void *dataPtr() const
        {
            return m_Msgdata.dataptr;
        }

        void *dest() const
        {
            return m_Dest;
        }

        void setDest(void *dest)
        {
            m_Dest = dest;
        }

        void setForceAsync()
        {
            m_ForceAsync = true;
        }

        bool forceAsync()
        {
            return m_ForceAsync;
        }

        uint64_t dataUInt() const
        {
            return m_Msgdata.dataint;
        }

        int event() const
        {
            return m_Type;
        }

        Message *prev;
        Args m_Args;
        uint32_t m_Priv;

    private:
        union
        {
            void *dataptr;
            uint64_t dataint;
        } m_Msgdata;

        int m_Type;
        void *m_Dest;
        bool m_ForceAsync = false;
    };

    typedef void (*nidium_shared_message_cleaner)(
        const SharedMessages::Message &msg);

    SharedMessages();
    ~SharedMessages();

    void postMessage(Message *msg);
    void postMessage(void *dataptr, int event);
    void postMessage(uint64_t dataint, int event);
    Message *readMessage(bool stopOnAsync = false);
    void delMessagesForDest(void *dest, int event = -1);
    void setCleaner(nidium_shared_message_cleaner cleaner)
    {
        m_Cleaner = cleaner;
    }

    bool hasAsyncMessages()
    {
        return m_MessagesList.asyncCount != 0;
    }

    bool hasPendingMessages() const
    {
        return m_MessagesList.count != 0;
    }

private:
    struct
    {
        int count;
        int asyncCount;
        Message *head;
        Message *queue;
        pthread_mutex_t lock;

    } m_MessagesList;

    nidium_shared_message_cleaner m_Cleaner;

    void addMessage(Message *msg);
};

} // namespace Core
} // namespace Nidium

#endif
