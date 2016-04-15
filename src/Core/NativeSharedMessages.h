/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativesharedmessages_h__
#define nativesharedmessages_h__

#include <pthread.h>
#include <stdint.h>

#include "Args.h"

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
            Nidium::Core::Args args;
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

