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

#ifndef nativefile_h__
#define nativefile_h__

#include <stdio.h>
#include <stdint.h>
#include "NativeMessages.h"
#include "NativeTaskManager.h"
#include "NativeIStreamer.h"
#include <NativeEvents.h>

#define NATIVEFILE_MESSAGE_BITS(id) ((1 << 20) | id)

enum {
    NATIVEFILE_OPEN_ERROR =     NATIVEFILE_MESSAGE_BITS(1),
    NATIVEFILE_OPEN_SUCCESS =   NATIVEFILE_MESSAGE_BITS(2),
    NATIVEFILE_CLOSE_SUCCESS =  NATIVEFILE_MESSAGE_BITS(3),
    NATIVEFILE_READ_SUCCESS =   NATIVEFILE_MESSAGE_BITS(4),
    NATIVEFILE_READ_ERROR =     NATIVEFILE_MESSAGE_BITS(5),
    NATIVEFILE_WRITE_SUCCESS =  NATIVEFILE_MESSAGE_BITS(6),
    NATIVEFILE_WRITE_ERROR =    NATIVEFILE_MESSAGE_BITS(7),
    NATIVEFILE_SEEK_SUCCESS =   NATIVEFILE_MESSAGE_BITS(8),
    NATIVEFILE_SEEK_ERROR =     NATIVEFILE_MESSAGE_BITS(9),
};

class NativeFile : public NativeManaged, public NativeIStreamer, public NativeEvents
{
public:
    static const uint8_t EventID = 2;

    enum Events {
        OPEN_ERROR = 1,
        OPEN_SUCCESS,
        CLOSE_SUCCESS,
        READ_SUCCESS,
        READ_ERROR,
        WRITE_SUCCESS,
        WRITE_ERROR,
        SEEK_SUCCESS, 
        SEEK_ERROR
    };

    explicit NativeFile(const char *path);
    ~NativeFile();

    void open(const char *mode, void *arg = NULL);
    void close(void *arg = NULL);
    void read(size_t size, void *arg = NULL);
    void write(char *buf, size_t size, void *arg = NULL);
    void seek(size_t pos, void *arg = NULL);

    int openSync(const char *modes, int *err);
    ssize_t readSync(uint64_t len, char **buffer, int *err);
    ssize_t mmapSync(char **buffer, int *err);

    ssize_t writeSync(char *data, uint64_t len, int *err);
    int seekSync(size_t pos, int *err);
    void closeSync();

    void openTask(const char *mode, void *arg = NULL);
    void closeTask(void *arg = NULL);
    void readTask(size_t size, void *arg = NULL);
    void writeTask(char *buf, size_t buflen, void *arg = NULL);
    void seekTask(size_t pos, void *arg = NULL);

    void setAutoClose(bool close) { m_AutoClose = close; }
    void setListener(NativeMessages *listener) {
        m_Delegate = listener;
    }
    size_t getFileSize() const {
        return m_Filesize;
    }

    bool isOpen() const {
        return m_Fd != NULL;
    }

    bool eof() const {
        return m_Fd == NULL || m_Eof;
    }

    const char *getFullPath() const {
        return m_Path;
    }

    FILE *getFd() const {
        return m_Fd;
    }

    void onMessage(const NativeSharedMessages::Message &msg);
    void onMessageLost(const NativeSharedMessages::Message &msg);
private:
    bool checkEOF();
    void checkRead(bool async = true, void *arg = NULL);

    FILE *m_Fd;
    NativeMessages *m_Delegate;
    char *m_Path;
    size_t m_Filesize;
    bool m_AutoClose;
    bool m_Eof;
    bool m_OpenSync;

    struct {
        size_t size;
        void *addr;
    } m_Mmap;
};

#endif
