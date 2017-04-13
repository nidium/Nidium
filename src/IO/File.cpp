/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "IO/File.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#include "Port/MSWindows.h"
#include <prmem.h>
#else
#include <fts.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

#include <prerror.h>

#include <ape_buffer.h>

#include "Core/Utils.h"

using Nidium::Core::Task;
using Nidium::Core::SharedMessages;

namespace Nidium {
namespace IO {

// {{{ Preamble
static void NPR_Modes(const char *modes, PRIntn *mode, PRIntn *flags) {
    *mode = 700;
    *flags = PR_CREATE_FILE;
    size_t i;

    if (modes) {
        for (i = 0; i < strlen(modes); i++) {
            switch (modes[i]) {
#if __cplusplus >= 201103L || defined(__GNUC__ )
            case 'x': *flags |= PR_EXCL;
                break;
#endif
#if defined(__GNUC__ )
            case 'c': //
                break;
            case 'm': //
                break;
            case 'e': //
                break;
#endif
            case 'b':
                assert(i != 0);
                break;
            case 'r': *flags |= PR_RDONLY;
                assert(i == 0);
                break;
            case 'w': *flags |= PR_WRONLY;
                assert(i == 0);
                break;
            case 'a': *flags |= PR_CREATE_FILE;
                assert(i == 0);
                break;
            case '+': *flags = *flags & ~PR_RDONLY;
                *flags = *flags & ~PR_WRONLY;
                *flags |= PR_RDWR;
                break;
            default:
                break;
            }
        }
    } else {
        *flags = PR_CREATE_FILE | PR_RDWR;
    }
}

static PRStatus NPR_ftell(PRFileDesc * fdesc, PRUint32 *size, int * err)
{
    PRFileInfo info;

    if (PR_SUCCESS != PR_GetOpenFileInfo(fdesc, &info)) {
        *err = PR_GetError();
        return PR_FAILURE;
    }
    *size = info.size;

    return PR_SUCCESS;
}

#define NIDIUM_FILE_NOTIFY(param, event, arg)                                \
    do {                                                                     \
        SharedMessages::Message *__msg = new SharedMessages::Message(event); \
        __msg->m_Args[0].set(param);                                         \
        __msg->m_Args[7].set(arg);                                           \
        this->postMessage(__msg);                                            \
    } while (0);

enum FileTask
{
    kFileTask_Open,
    kFileTask_Close,
    kFileTask_Read,
    kFileTask_Write,
    kFileTask_Seek,
    kFileTask_Listfiles
};
// }}}

// {{{ Implementation
File::File(const char *name)
    : m_Dir(NULL), m_Fdesc(NULL), m_Delegate(NULL), m_Filesize(0),
      m_AutoClose(true), m_Eof(false), m_OpenSync(false), m_isDir(false)
{
    m_Mmap.addr = NULL;
    m_Mmap.fmap = NULL;
    m_Mmap.size = 0;
    m_Path      = strdup(name);
}

bool File::checkEOF()
{
    if (m_Fdesc && ((m_Eof = PR_Available( m_Fdesc )) == 0)) {
        if (m_AutoClose) {
            this->closeTask();
        }
    }

    return m_Eof;
}

void File::checkRead(bool async, void *arg)
{
    int err = -1;

    if (PR_Available64(m_Fdesc) == -1) {
        err = PR_GetError();
    } else if (this->checkEOF()) {
        err = 0;
    }

    if (async && err != -1) {
        NIDIUM_FILE_NOTIFY(err, File::kEvents_ReadError, arg);
    }
}

int File::rm()
{
    int ret = unlink(m_Path);

    closeFd();

    return ret;
}

#ifndef _MSC_VER
static int File_compare(const FTSENT **one, const FTSENT **two)
{
    return (strcmp((*one)->fts_name, (*two)->fts_name));
}
#endif

void File::rmrf()
{
    /*
      windows-x86: Fixme: this is a hack to get quick results
                   TODO: changes the fts version with a
                         nspr version
    */
#ifdef _MSC_VER
#define RMCOMMAND "\"%s\" / S / Q"
    ssize_t len;
    char *cmd;

    len = strlen(m_Path) + strlen(RMCOMMAND) + 1; // + 1 and the '%s' is more then enough
    cmd = (char*)malloc(len);
    snprintf(cmd, len, RMCOMMAND, m_Path);

    system(cmd);

    free(cmd);
#undef RMCOMMAND
#else
    FTS *tree;
    FTSENT *f;

    char *path[] = { m_Path, NULL };

    tree = fts_open(path, FTS_COMFOLLOW | FTS_NOCHDIR, File_compare);

    if (!tree) {
        ndm_log(NDM_LOG_ERROR, "File", "Failed to fts_open()");
        return;
    }
    while ((f = fts_read(tree))) {
        switch (f->fts_info) {
            case FTS_F:
            case FTS_NS:
            case FTS_SL:
            case FTS_SLNONE:
                PR_Delete(f->fts_path);
                break;
            case FTS_DP:
                PR_RmDir(f->fts_path);
                break;
            default:
                break;
        }
    }

    fts_close(tree);

    closeFd();
#endif
}

File::~File()
{
    Core::PthreadAutoLock tasksLock(&getManagedLock());

    if (m_Mmap.addr) {
        PR_MemUnmap(m_Mmap.addr, m_Mmap.size);
    }
    if (m_Mmap.fmap) {
        PR_CloseFileMap(m_Mmap.fmap);
    }

    if (this->isOpen()) {
        this->closeFd();
    }

    free(m_Path);

}

// }}}

// {{{ Tasks implementation

/*
    /!\ Exec in a worker thread
*/
void File_dispatchTask(Task *task)
{
    File *file    = static_cast<File *>(task->getObject());
    uint64_t type = task->m_Args[0].toInt64();
    void *arg     = task->m_Args[7].toPtr();

    switch (type) {
        case kFileTask_Open: {
            char *modes = static_cast<char *>(task->m_Args[1].toPtr());
            file->openTask(modes, arg);
            free(modes);
            break;
        }
        case kFileTask_Close: {
            file->closeTask();
            break;
        }
        case kFileTask_Read: {
            uint64_t size = task->m_Args[1].toInt64();
            file->readTask(size, arg);
            break;
        }
        case kFileTask_Write: {
            uint64_t buflen = task->m_Args[1].toInt64();
            char *buf       = static_cast<char *>(task->m_Args[2].toPtr());

            file->writeTask(buf, buflen, arg);

            free(buf);
            break;
        }
        case kFileTask_Seek: {
            uint64_t pos = task->m_Args[1].toInt64();
            file->seekTask(pos, arg);
            break;
        }
        case kFileTask_Listfiles: {
            file->listFilesTask(arg);
            break;
        }
        default:
            break;
    }
}

/*
    /!\ Exec in a worker thread
*/
void File::openTask(const char *modes, void *arg)
{
    if (this->isOpen()) {
        // seek(0)?
        NIDIUM_FILE_NOTIFY(m_Fdesc, File::kEvents_OpenSuccess, arg);
        return;
    }

    bool readOnly = !modes || (modes && strlen(modes) == 1 && modes[0] == 'r');
    struct stat s;
    int ret;

    ret = stat(m_Path, &s);
    if (ret != 0 && readOnly) {
        // Opened in read-only, but file does not exists
        NIDIUM_FILE_NOTIFY(errno, File::kEvents_OpenError, arg);
        return;
    }

    if (S_ISDIR(s.st_mode)) {
        if (!readOnly) {
            ndm_logf(NDM_LOG_ERROR, "File", "Can't open directory %s for writing", m_Path);
            NIDIUM_FILE_NOTIFY(EISDIR, File::kEvents_OpenError, arg);
            return;
        }

        m_Dir = PR_OpenDir(m_Path);
        if (!m_Dir) {
            ndm_logf(NDM_LOG_ERROR, "File", "Failed to open dir %s : %s", m_Path, strerror(errno));
           NIDIUM_FILE_NOTIFY(errno, File::kEvents_OpenError, arg);
            return;
        }
        m_isDir    = true;
        m_Filesize = 0;
    } else {
        PRIntn mode;
        PRIntn flags;

        NPR_Modes(modes, &mode, &flags);
        m_Fdesc = PR_Open(m_Path, flags, mode);
        if (m_Fdesc == NULL) {
            NIDIUM_FILE_NOTIFY(errno, File::kEvents_OpenError, arg);
            return;
        }

        m_Filesize = s.st_size;
        m_isDir    = false;
    }

    NIDIUM_FILE_NOTIFY(m_Fdesc, File::kEvents_OpenSuccess, arg);
}

/*
    /!\ Exec in a worker thread
*/
void File::closeTask(void *arg)
{
    closeFd();

    if (!m_OpenSync) {
        NIDIUM_FILE_NOTIFY(static_cast<void *>(NULL),
                           File::kEvents_CloseSuccess, arg);
    }
}

/*
    /!\ Exec in a worker thread
*/
void File::readTask(size_t size, void *arg)
{
    if (!this->isOpen() || this->isDir()) {
        NIDIUM_FILE_NOTIFY(static_cast<void *>(NULL), File::kEvents_ReadError,
                           arg);
        return;
    }

    uint64_t clamped_len;
    clamped_len = nidium_min(m_Filesize, size);

    buffer *buf = buffer_new(clamped_len + 1);

    /*
        Read an empty file
    */
    if (clamped_len == 0) {
        NIDIUM_FILE_NOTIFY(static_cast<void *>(buf), File::kEvents_ReadSuccess,
                           arg);
        buf->data[0] = '\0';
        return;
    }

    if ((buf->used = PR_Read(m_Fdesc, buf->data, clamped_len)) <= 0) {
        this->checkRead(true, arg);
        buffer_destroy(buf);
        return;
    }

    this->checkEOF();

    buf->data[buf->used] = '\0';

    NIDIUM_FILE_NOTIFY(static_cast<void *>(buf), File::kEvents_ReadSuccess,
                       arg);
}

/*
    /!\ Exec in a worker thread
*/
void File::writeTask(char *buf, size_t buflen, void *arg)
{
    if (!this->isOpen() || this->isDir()) {
        NIDIUM_FILE_NOTIFY(static_cast<void *>(NULL), File::kEvents_WriteError,
                           arg);
        return;
    }

    size_t writelen = PR_Write(m_Fdesc, buf, buflen);
    /*
        TODO:
        save cursor position,
        position cursor at the end,
        restore position
    */
    int dummy;

    NPR_ftell(m_Fdesc, &m_Filesize, &dummy);
    NIDIUM_FILE_NOTIFY(writelen, File::kEvents_WriteSuccess, arg);
}

/*
    /!\ Exec in a worker thread
*/
void File::seekTask(size_t pos, void *arg)
{
    if (!this->isOpen() || this->isDir()) {
        int err = 0;
        NIDIUM_FILE_NOTIFY(err, File::kEvents_SeekError, arg);
        return;
    }

    if (PR_Seek64(m_Fdesc, pos, PR_SEEK_SET) == -1) {
        NIDIUM_FILE_NOTIFY(errno, File::kEvents_SeekError, arg);
        return;
    }

    this->checkEOF();

    NIDIUM_FILE_NOTIFY(static_cast<void *>(NULL), File::kEvents_SeekSuccess,
                       arg);
}

/*
    /!\ Exec in a worker thread
*/
void File::listFilesTask(void *arg)
{
    if (!this->isOpen() || !this->isDir()) {
        return;
    }

    PRDirEntry *cur;
    DirEntries *entries = static_cast<DirEntries *>(malloc(sizeof(*entries)));
    entries->allocated  = 64;
    entries->lst
        = static_cast<PRDirEntry *>(malloc(sizeof(PRDirEntry) * entries->allocated));

    entries->size = 0;

    while ((cur = PR_ReadDir(m_Dir, PR_SKIP_BOTH)) != NULL) {
        memcpy(&entries->lst[entries->size], cur, sizeof(PRDirEntry));
        entries->size++;

        if (entries->size == entries->allocated) {
            entries->allocated *= 2;
            entries->lst = static_cast<PRDirEntry *>(
                realloc(entries->lst, sizeof(PRDirEntry) * entries->allocated));
        }
    }

    NIDIUM_FILE_NOTIFY(entries, File::kEvents_ListFiles, arg);

    //FIXME windows-x86 branch: need crossplatform code for rewinddir(m_Dir);
    //rewinddir(m_Dir);
}

// }}}

// {{{ Async operations
void File::open(const char *mode, void *arg)
{
    Task *task = new Task();
    task->m_Args[0].set(kFileTask_Open);
    task->m_Args[1].set(strdup(mode));
    task->m_Args[7].set(arg);

    task->setFunction(File_dispatchTask);

    this->addTask(task);
}

void File::close(void *arg)
{
    Task *task = new Task();
    task->m_Args[0].set(kFileTask_Close);
    task->m_Args[7].set(arg);

    task->setFunction(File_dispatchTask);

    this->addTask(task);
}

void File::read(size_t size, void *arg)
{
    Task *task = new Task();
    task->m_Args[0].set(kFileTask_Read);
    task->m_Args[1].set(size);
    task->m_Args[7].set(arg);

    task->setFunction(File_dispatchTask);

    this->addTask(task);
}

void File::write(char *buf, size_t size, void *arg)
{
    unsigned char *newbuf = static_cast<unsigned char *>(malloc(size));
    memcpy(newbuf, buf, size);

    Task *task = new Task();
    task->m_Args[0].set(kFileTask_Write);
    task->m_Args[1].set(size);
    task->m_Args[2].set(newbuf);
    task->m_Args[7].set(arg);

    task->setFunction(File_dispatchTask);

    this->addTask(task);
}

void File::seek(size_t pos, void *arg)
{
    Task *task = new Task();
    task->m_Args[0].set(kFileTask_Seek);
    task->m_Args[1].set(pos);
    task->m_Args[7].set(arg);

    task->setFunction(File_dispatchTask);

    this->addTask(task);
}

void File::listFiles(void *arg)
{
    Task *task = new Task();
    task->m_Args[0].set(kFileTask_Listfiles);
    task->m_Args[7].set(arg);

    task->setFunction(File_dispatchTask);

    this->addTask(task);
}

// }}}

// {{{ Sync operations

int File::exists()
{
    Core::PthreadAutoLock tasksLock(&getManagedLock());
    struct stat s;
    int ret;

    ret = stat(m_Path, &s);

    if (ret == -1) {
        return 0;
    }

    return (S_ISDIR(s.st_mode)) ? 2 : 1;
}

int File::openSync(const char *modes, int *err)
{
    Core::PthreadAutoLock tasksLock(&getManagedLock());

    *err = 0;

    if (this->isOpen()) {
        return 1;
    }

    struct stat s;
    int ret;
    bool readOnly = !modes || (modes && strlen(modes) == 1 && modes[0] == 'r');

    ret = stat(m_Path, &s);
    if (ret != 0 && readOnly) {
        // Opened in read-only, but file does not exists
        ndm_logf(NDM_LOG_ERROR, "File", "Failed to open : %s errno=%d", m_Path, errno);
        *err = errno;
        return 0;
    }

    if (S_ISDIR(s.st_mode)) {
        if (!readOnly) {
            *err = EISDIR;
            ndm_logf(NDM_LOG_ERROR, "File", "Can't open directory %s for writing", m_Path);
            return 0;
        }

        m_Dir = PR_OpenDir(m_Path);
        if (!m_Dir) {
            ndm_logf(NDM_LOG_ERROR, "File", "Failed to open : %s errno=%d", m_Path, errno);

            *err = errno;
            return 0;
        }

        m_isDir    = true;
        m_Filesize = 0;
    } else {
        PRIntn mode;
        PRIntn flags;

        NPR_Modes(modes, &mode, &flags);
        if ((m_Fdesc = PR_Open(m_Path, flags, mode)) == NULL) {
            ndm_logf(NDM_LOG_ERROR, "File", "Failed to open : %s errno=%d", m_Path, errno);
            *err = errno;
            return 0;
        }

        m_Filesize = s.st_size;
    }

    m_OpenSync = true;

    return 1;
}

ssize_t File::writeSync(char *data, uint64_t len, int *err)
{
    Core::PthreadAutoLock tasksLock(&getManagedLock());

    *err = 0;

    if (!this->isOpen() || this->isDir()) {
        return -1;
    }
    int ret;

    ret = PR_Write(m_Fdesc, data, len);

    if (ret < len) {
        *err = PR_GetError();
    } else {
        if (NPR_ftell(m_Fdesc, &m_Filesize, err) == PR_FAILURE) {
            return -1;
        }
    }

    return ret;
}

ssize_t File::mmapSync(char **buffer, int *err)
{
    Core::PthreadAutoLock tasksLock(&getManagedLock());

    *err = 0;

    if (!this->isOpen() || this->isDir()) {
        return -1;
    }
    size_t size = this->getFileSize();

    m_Mmap.fmap = PR_CreateFileMap(m_Fdesc, size, PR_PROT_READWRITE);
    if (m_Mmap.fmap == NULL) {
        *err = PR_GetError();
        return -1;
    }
    m_Mmap.addr = PR_MemMap(m_Mmap.fmap, 0, size);

    if (m_Mmap.addr == NULL) {
        PR_CloseFileMap(m_Mmap.fmap);
        *err = errno;
        return -1;
    }

    m_Mmap.size = size;

    *buffer = static_cast<char *>(m_Mmap.addr);

    return this->getFileSize();
}

ssize_t File::readSync(uint64_t len, char **buffer, int *err)
{
    Core::PthreadAutoLock tasksLock(&getManagedLock());

    *err = 0;

    *buffer = NULL;

    if (!this->isOpen() || this->isDir()) {
        return -1;
    }

    uint64_t clamped_len;
    clamped_len = nidium_min(m_Filesize, len);

    /*
        Read an empty file
    */
    if (clamped_len == 0) {
        return 0;
    }

    char *data      = static_cast<char *>(malloc(clamped_len + 1));
    size_t readsize = 0;

    if ((readsize = PR_Read(m_Fdesc, data, clamped_len)) <= 0) {

        this->checkRead(false);

        free(data);
        *err = PR_GetError();
        return -1;
    }

    data[readsize] = '\0';

    *buffer = data;

    this->checkEOF();

    return readsize;
}

void File::closeSync()
{
    Core::PthreadAutoLock tasksLock(&getManagedLock());

    if (!this->isOpen()) {
        return;
    }

    closeFd();
}

int File::seekSync(size_t pos, int *err)
{
    Core::PthreadAutoLock tasksLock(&getManagedLock());

    *err = 0;
    if (!this->isOpen() || this->isDir()) {
        return -1;
    }

    if (PR_Seek64(m_Fdesc, pos, PR_SEEK_SET) == -1) {
        *err = PR_GetError();

        return -1;
    }

    return 0;
}

// }}}

// {{{ Events

void File::onMessage(const SharedMessages::Message &msg)
{
    if (m_Delegate) {
        m_Delegate->onMessage(msg);
    }

    switch (msg.event()) {
        case File::kEvents_ReadSuccess: {
            buffer *buf = static_cast<buffer *>(msg.m_Args[0].toPtr());
            buffer_delete(buf);
            break;
        }
        case File::kEvents_ListFiles: {
            DirEntries *entries
                = static_cast<DirEntries *>(msg.m_Args[0].toPtr());
            free(entries->lst);
            free(entries);
            break;
        }
    }
}

void File::onMessageLost(const SharedMessages::Message &msg)
{
    switch (msg.event()) {
        case File::kEvents_ReadSuccess: {
            buffer *buf = static_cast<buffer *>(msg.m_Args[0].toPtr());
            buffer_delete(buf);
            break;
        }
        case File::kEvents_ListFiles: {
            DirEntries *entries
                = static_cast<DirEntries *>(msg.m_Args[0].toPtr());
            free(entries->lst);
            free(entries);
            break;
        }
    }
}
// }}}

} // namespace IO
} // namespace Nidium
