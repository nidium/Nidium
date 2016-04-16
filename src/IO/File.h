/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef io_file_h__
#define io_file_h__

#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/types.h>

#include "Core/Messages.h"
#include "Core/NativeTaskManager.h"
#include "Core/Events.h"

#include "NativeIStreamer.h"


namespace Nidium {
namespace IO {

#define NIDIUM_FILE_MESSAGE_BITS(id) ((1 << 20) | id)

enum {
    FILE_OPEN_ERROR =     NIDIUM_FILE_MESSAGE_BITS(1),
    FILE_OPEN_SUCCESS =   NIDIUM_FILE_MESSAGE_BITS(2),
    FILE_CLOSE_SUCCESS =  NIDIUM_FILE_MESSAGE_BITS(3),
    FILE_READ_SUCCESS =   NIDIUM_FILE_MESSAGE_BITS(4),
    FILE_READ_ERROR =     NIDIUM_FILE_MESSAGE_BITS(5),
    FILE_WRITE_SUCCESS =  NIDIUM_FILE_MESSAGE_BITS(6),
    FILE_WRITE_ERROR =    NIDIUM_FILE_MESSAGE_BITS(7),
    FILE_SEEK_SUCCESS =   NIDIUM_FILE_MESSAGE_BITS(8),
    FILE_SEEK_ERROR =     NIDIUM_FILE_MESSAGE_BITS(9),
    FILE_LISTFILES_ENTRIES = NIDIUM_FILE_MESSAGE_BITS(10),
};

class File : public NativeManaged, public NativeIStreamer, public Nidium::Core::Events
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

    explicit File(const char *path);
    ~File();

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
    void setListener(Messages *listener) {
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

    File *dup() {
        return new File(m_Path);
    }

    void onMessage(const Nidium::Core::SharedMessages::Message &msg);
    void onMessageLost(const Nidium::Core::SharedMessages::Message &msg);
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

    Nidium::Core::Messages *m_Delegate;
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

} // namespace IO
} // namespace Nidium

#endif

