#ifndef nativefileio_h__
#define nativefileio_h__

#include <pthread.h>
#include <stdio.h>
#include <stdint.h>

class NativeSharedMessages;
class NativeFileIODelegate;

class NativeFileIO
{
public:

    enum ACTION_TYPE {
        FILE_ACTION_OPEN,
        FILE_ACTION_READ,
        FILE_ACTION_WRITE
    };

    NativeFileIO(const char *filename, NativeFileIODelegate *delegate,
        struct _ape_global *net);
    ~NativeFileIO();
    void open(const char *modes);
    void openAction(char *modes);
    void read(uint64_t len);
    void readAction(uint64_t len);
    void write(unsigned char *data, uint64_t len);
    void writeAction(unsigned char *data, uint64_t len);
    void close();
    void seek(uint64_t pos);
    NativeFileIODelegate *getDelegate() const { return delegate; };
    char *filename;
    NativeSharedMessages *messages;
    FILE *fd;
    size_t filesize;

    pthread_t threadHandle;
    pthread_mutex_t threadMutex;
    pthread_cond_t threadCond;

    struct {
        uint64_t u64;
        void *ptr;
        enum ACTION_TYPE type;
        bool active;
        uint32_t u32;
        uint8_t  u8;
    } action;

private:
    NativeFileIODelegate *delegate;

    struct _ape_timer *timer;
    struct _ape_global *net;

};

class NativeFileIODelegate
{
  public:
    virtual void onNFIOOpen(NativeFileIO *)=0;
    virtual void onNFIOError(NativeFileIO *, int errno)=0;
    virtual void onNFIORead(NativeFileIO *, unsigned char *data, size_t len)=0;
    virtual void onNFIOWrite(NativeFileIO *, size_t written)=0;

};

enum {
    NATIVE_FILEOPEN_MESSAGE,
    NATIVE_FILEERROR_MESSAGE,
    NATIVE_FILEREAD_MESSAGE,
    NATIVE_FILEWRITE_MESSAGE
};

#endif
