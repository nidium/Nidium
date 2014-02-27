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

#define NATIVEFILE_MESSAGE_BITS(id) ((1 << 20) | id)

enum {
    NATIVEFILE_OPEN_ERROR = NATIVEFILE_MESSAGE_BITS(1),
    NATIVEFILE_OPEN_SUCCESS = NATIVEFILE_MESSAGE_BITS(2),
    NATIVEFILE_CLOSE_SUCCESS = NATIVEFILE_MESSAGE_BITS(3),
    NATIVEFILE_READ_SUCCESS = NATIVEFILE_MESSAGE_BITS(4),
    NATIVEFILE_READ_ERROR = NATIVEFILE_MESSAGE_BITS(5),
    NATIVEFILE_WRITE_SUCCESS = NATIVEFILE_MESSAGE_BITS(6),
    NATIVEFILE_WRITE_ERROR = NATIVEFILE_MESSAGE_BITS(7),
};

class NativeFile : public NativeManaged
{
public:
    explicit NativeFile(const char *path);
    ~NativeFile();

    void open(const char *mode, void *arg = NULL);
    void close(void *arg = NULL);
    void read(size_t size, void *arg = NULL);
    void write(char *buf, size_t size, void *arg = NULL);
    void seek(size_t pos, void *arg = NULL);

    void openTask(const char *mode);
    void closeTask();
    void readTask(size_t size);
    void writeTask(char *buf, size_t buflen);
private:
    FILE *m_Fd;
    NativeMessages *m_Delegate;
    char *m_Path;
    size_t m_Filesize;
};

#endif
