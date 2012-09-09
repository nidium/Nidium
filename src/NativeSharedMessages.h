#ifndef nativesharedmessages_h__
#define nativesharedmessages_h__

#include <pthread.h>

typedef struct _native_shared_message
{
	void *ptr;
	struct _native_shared_message *prev;
} native_shared_message;

class NativeSharedMessages
{
  private:

	struct
	{
		int count;
		struct _native_shared_message *head;
		struct _native_shared_message *queue;
		pthread_mutex_t lock;

	} messageslist;
  public:
  	NativeSharedMessages();
  	void postMessage(void *ptr);
  	void *readMessage();
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
