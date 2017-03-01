/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef io_file_h__
#define io_file_h__

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#include <prio.h>

#include "Core/Messages.h"
#include "Core/TaskManager.h"
#include "Core/Events.h"

#define NIDIUM_FILE_MESSAGE_BITS(id) ((1 << 20) | id)

namespace Nidium {
namespace IO {

class File : public Nidium::Core::Managed, public Nidium::Core::Events
{
public:
    static const uint8_t EventID = 2;

    enum Events
    {
        kEvents_OpenError    = NIDIUM_FILE_MESSAGE_BITS(1),
        kEvents_OpenSuccess  = NIDIUM_FILE_MESSAGE_BITS(2),
        kEvents_CloseSuccess = NIDIUM_FILE_MESSAGE_BITS(3),
        kEvents_ReadSuccess  = NIDIUM_FILE_MESSAGE_BITS(4),
        kEvents_ReadError    = NIDIUM_FILE_MESSAGE_BITS(5),
        kEvents_WriteSuccess = NIDIUM_FILE_MESSAGE_BITS(6),
        kEvents_WriteError   = NIDIUM_FILE_MESSAGE_BITS(7),
        kEvents_SeekSuccess  = NIDIUM_FILE_MESSAGE_BITS(8),
        kEvents_SeekError    = NIDIUM_FILE_MESSAGE_BITS(9),
        kEvents_ListFiles    = NIDIUM_FILE_MESSAGE_BITS(10),
    };

    struct DirEntries
    {
        int size;
        int allocated;
        PRDir *lst;
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
    int rm();

    /*
        Check whether a path points to an existing filename.
        Returns :

        0 : not found
        1 : file
        2 : directory
    */
    int exists();
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

    void setAutoClose(bool close)
    {
        m_AutoClose = close;
    }
    void setListener(Messages *listener)
    {
        m_Delegate = listener;
    }
    size_t getFileSize() const
    {
        return m_Filesize;
    }

    bool isDir() const
    {
        return m_isDir;
    }

    bool isOpen() const
    {
        return m_Fd || m_Dir;
    }

    bool eof() const
    {
        return m_Fd == NULL || m_Eof;
    }

    const char *getFullPath() const
    {
        return m_Path;
    }

    FILE *getFd() const
    {
        return m_Fd;
    }

    DIR *GetDir() const
    {
        return m_Dir;
    }

    File *dup()
    {
        return new File(m_Path);
    }

    void onMessage(const Nidium::Core::SharedMessages::Message &msg);
    void onMessageLost(const Nidium::Core::SharedMessages::Message &msg);

private:
    bool checkEOF();
    void checkRead(bool async = true, void *arg = NULL);
    void closeFd()
    {
        if (!isOpen()) {
            return;
        }
        if (m_isDir && m_Dir) {
            PR_CloseDir(m_Dir);
        } else if (m_Fd) {
            fclose(m_Fd);
        }

        m_Fd    = NULL;
        m_Dir   = NULL;
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

    struct
    {
        size_t size;
        void *addr;
    } m_Mmap;
};

} // namespace IO
} // namespace Nidium

#endif
