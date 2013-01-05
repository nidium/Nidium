#ifndef nativesharedmessages_h__
#define nativesharedmessages_h__

#include <pthread.h>
#include <stdint.h>

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
