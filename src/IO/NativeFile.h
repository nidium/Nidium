/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativefile_h__
#define nativefile_h__

#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/types.h>

#include "Core/NativeMessages.h"
#include "Core/NativeTaskManager.h"
#include "Core/NativeEvents.h"

#include "NativeIStreamer.h"


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
    NATIVEFILE_LISTFILES_ENTRIES = NATIVEFILE_MESSAGE_BITS(10),
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

    struct DirEntries {
        int size;
        int allocated;
        dirent *lst;
    };

    explicit NativeFile(const char *path);
    ~NativeFile();

    void open(const char *mode, void *arg = NULL);
    void close(void *arg = NULL);
    void read(size_t size, void *arg = NULL);
    void write(char *buf, size_t size, void *arg = NULL);
    void seek(size_t pos, void *arg = NULL);
    void listFiles(void *arg = NULL);
    void rmrf();

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
    void listFilesTask(void *arg = NULL);

    void setAutoClose(bool close) { m_AutoClose = close; }
    void setListener(NativeMessages *listener) {
        m_Delegate = listener;
    }
    size_t getFileSize() const {
        return m_Filesize;
    }

    bool isDir() const {
        return m_isDir;
    }

    bool isOpen() const {
        return m_Fd || m_Dir;
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

    DIR *getDir() const {
        return m_Dir;
    }

    NativeFile *dup() {
        return new NativeFile(m_Path);
    }

    void onMessage(const NativeSharedMessages::Message &msg);
    void onMessageLost(const NativeSharedMessages::Message &msg);
private:
    bool checkEOF();
    void checkRead(bool async = true, void *arg = NULL);
    void closeFd() {
        if (!isOpen()) {
            return;
        }
        if (m_isDir && m_Dir) {
            closedir(m_Dir);
        } else if (m_Fd) {
            fclose(m_Fd);
        }

        m_Fd = NULL;
        m_Dir = NULL;
        m_isDir = false;
    }

    DIR *m_Dir;
    FILE *m_Fd;

    NativeMessages *m_Delegate;
    char *m_Path;
    size_t m_Filesize;
    bool m_AutoClose;
    bool m_Eof;
    bool m_OpenSync;
    bool m_isDir;

    struct {
        size_t size;
        void *addr;
    } m_Mmap;
};

#endif

