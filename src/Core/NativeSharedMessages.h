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

#ifndef nativesharedmessages_h__
#define nativesharedmessages_h__

#include <pthread.h>
#include <stdint.h>

#include "NativeArgs.h"

/*
    TODO: Add "max messages in queue" to guard memory congestion in case of allocation
         (i.e. NativeApp extractor)
*/

typedef struct _native_shared_message
{
    void *ptr;
    struct _native_shared_message *prev;
} native_shared_message;

class NativeSharedMessages
{
  public:

    class Message
    {
        public:
            Message(void *ptr, int type, void *dest = NULL)
            : prev(NULL), type(type), m_Dest(dest) {
                msgdata.dataptr = ptr;
            }
            Message(uint64_t dataint, int type, void *dest = NULL)
            : prev(NULL), type(type), m_Dest(dest) {
                msgdata.dataint = dataint;
            }

            Message(int type) : prev(NULL), type(type), m_Dest(NULL) {
            }

            ~Message() {}

            Message() {};

            void *dataPtr() const {
                return msgdata.dataptr;
            }

            void *dest() const {
                return m_Dest;
            }

            void setDest(void *dest) {
                m_Dest = dest;
            }

            uint64_t dataUInt() const {
                return msgdata.dataint;
            }

            int event() const {
                return type;
            }

            Message *prev;
            NativeArgs args;
            uint32_t priv;
        private:

            union {
                void *dataptr;
                uint64_t dataint;
            } msgdata;

            int type;
            void *m_Dest;
    };

    typedef void (*native_shared_message_cleaner)(const NativeSharedMessages::Message &msg);

    NativeSharedMessages();
    ~NativeSharedMessages();

    void postMessage(Message *msg);
    void postMessage(void *dataptr, int event);
    void postMessage(uint64_t dataint, int event);
    Message *readMessage();
    Message *readMessage(int ev);
    void delMessagesForDest(void *dest, int event = -1);
    void setCleaner(native_shared_message_cleaner cleaner) {
        m_Cleaner = cleaner;
    }

    int hasPendingMessages() const {
        return (messageslist.count != 0);
    }
  private:

    struct
    {
        int count;
        Message *head;
        Message *queue;
        pthread_mutex_t lock;

    } messageslist;

    native_shared_message_cleaner m_Cleaner;
};
#endif

