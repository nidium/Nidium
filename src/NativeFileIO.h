#ifndef nativefileio_h__
#define nativefileio_h__

#include <pthread.h>

class NativeSharedMessages;


class NativeFileIO
{
public:
    NativeFileIO(const char *filename, struct _ape_global *net);
    ~NativeFileIO();
    void open();
    void close();
    char *filename;
    NativeSharedMessages *messages;
private:
    pthread_t threadHandle;
    struct _ape_timer *timer;
    struct _ape_global *net;
};

enum {
    NATIVE_FILEOPEN_MESSAGE,
    NATIVE_FILEERROR_MESSAGE
};

#endif
