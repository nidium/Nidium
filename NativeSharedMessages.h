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
            Message(void *ptr, int type)
            : prev(NULL), type(type) {
                msgdata.dataptr = ptr;
            }
            Message(uint64_t dataint, int type)
            : prev(NULL), type(type) {
                msgdata.dataint = dataint;
            }

            Message(){};

            void *dataPtr() const {
                return msgdata.dataptr;
            }

            uint64_t dataUInt() const {
                return msgdata.dataint;
            }

            int event() const {
                return type;
            }
            Message *prev;
        private:

            union {
                void *dataptr;
                uint64_t dataint;
            } msgdata;

            int type;
    };
    NativeSharedMessages();
    ~NativeSharedMessages();
    void postMessage(void *dataptr, int event);
    void postMessage(unsigned int dataint, int event);
    int readMessage(NativeSharedMessages::Message *msg);
  private:

    struct
    {
        int count;
        Message *head;
        Message *queue;
        pthread_mutex_t lock;

    } messageslist;
};

class NSMAutoLock {
  public:
    NSMAutoLock(pthread_mutex_t *mutex)
      : lock(mutex) {

        pthread_mutex_lock(lock);
    }

    ~NSMAutoLock() {
        pthread_mutex_unlock(lock);
    }
  private:
    pthread_mutex_t *lock;
};

#endif
