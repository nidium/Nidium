#ifndef nativefileio_h__
#define nativefileio_h__

#include <pthread.h>
#include <stdio.h>

class NativeSharedMessages;
class NativeFileIODelegate;

class NativeFileIO
{
public:
    NativeFileIO(const char *filename, NativeFileIODelegate *delegate,
        struct _ape_global *net);
    ~NativeFileIO();
    void open();
    void close();
    void getContents();
    NativeFileIODelegate *getDelegate() const { return delegate; };
    char *filename;
    NativeSharedMessages *messages;
    FILE *fd;
    size_t filesize;
private:
    NativeFileIODelegate *delegate;
    pthread_t threadHandle;
    struct _ape_timer *timer;
    struct _ape_global *net;
};

class NativeFileIODelegate
{
  public:
    virtual void onNFIOOpen(NativeFileIO *)=0;
    virtual void onNFIOError(NativeFileIO *, int errno)=0;
    virtual void onNFIORead(NativeFileIO *, unsigned char *data, size_t len)=0;

    NativeFileIO *NFIOref;
};

enum {
    NATIVE_FILEOPEN_MESSAGE,
    NATIVE_FILEERROR_MESSAGE,
    NATIVE_FILEREAD_MESSAGE
};

#endif
