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

#ifndef nativefileio_h__
#define nativefileio_h__

#include <NativeIStreamer.h>

#include <pthread.h>
#include <stdio.h>
#include <stdint.h>

class NativeSharedMessages;
class NativeFileIODelegate;

class NativeFileIO : public NativeIStreamer
{
public:

    enum ACTION_TYPE {
        FILE_ACTION_OPEN,
        FILE_ACTION_READ,
        FILE_ACTION_WRITE
    };

    NativeFileIO(const char *filename, NativeFileIODelegate *delegate,
        struct _ape_global *net, const char *prefix = NULL);
    ~NativeFileIO();
    void open(const char *modes);
    void openAction(char *modes);
    void read(uint64_t len);
    void readAction(uint64_t len);
    void write(unsigned char *data, uint64_t len);
    void writeAction(unsigned char *data, uint64_t len);
    void close();
    void setAutoClose(bool close) { autoClose = close; }
    void seek(uint64_t pos);

    const char *getFileName() const {
        return filename;
    }

    bool eof() const {
        return m_eof;
    }
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
        bool stop;
        uint32_t u32;
        uint8_t  u8;
    } action;

private:
    NativeFileIODelegate *delegate;

    struct _ape_timer *timer;
    struct _ape_global *net;

    bool autoClose;
    bool m_eof;
    void checkRead();

};

class NativeFileIODelegate
{
  public:
    virtual void onNFIOOpen(NativeFileIO *)=0;
    virtual void onNFIOError(NativeFileIO *, int errno)=0;
    virtual void onNFIORead(NativeFileIO *, unsigned char *data, size_t len)=0;
    virtual void onNFIOWrite(NativeFileIO *, size_t written)=0;
    virtual ~NativeFileIODelegate(){};

};

enum {
    NATIVE_FILEOPEN_MESSAGE,
    NATIVE_FILEERROR_MESSAGE,
    NATIVE_FILEREAD_MESSAGE,
    NATIVE_FILEWRITE_MESSAGE
};

#endif
