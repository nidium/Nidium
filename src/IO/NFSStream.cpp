/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "IO/NFSStream.h"

#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#include "Port/MSWindows.h"
#endif

#include <ape_netlib.h>

#include "IO/NFS.h"

#include "Binding/NidiumJS.h"

namespace Nidium {
namespace IO {

// {{{ Implementation

#ifdef NIDIUM_PACKAGE_EMBED
#include NIDIUM_EMBED_FILE
#endif
NFSStream::NFSStream(const char *location) : Stream(location)
{
    static NFS *nfs = NULL;

#ifdef NIDIUM_PACKAGE_EMBED
    if (nfs == NULL) {
        nfs = new NFS(embed_bin, sizeof(embed_bin));
    }
#endif
    m_NFS      = nfs;
    m_File.pos = 0;
}

void NFSStream::stop()
{
    /*
        Do nothing
    */
}

int NFSStream_getContent(void *arg)
{
    static_cast<NFSStream *>(arg)->_getContent();

    return 0;
}

void NFSStream::getContent()
{
    ape_global *ape = Binding::NidiumJS::GetNet();
    timer_dispatch_async_unprotected(NFSStream_getContent, this);
}

void NFSStream::_getContent()
{
    m_File.data = reinterpret_cast<const unsigned char *>(
        m_NFS->readFile(m_Location, &m_File.len));
    m_File.pos = 0;

    if (m_File.data == NULL) {
        this->errorSync(Stream::kErrors_Open, 0);

        return;
    }

    buffer buf;
    buffer_init(&buf);

    // TODO: new style cast
    buf.data = (unsigned char *)(m_File.data);
    buf.size = buf.used = m_File.len;

    CREATE_MESSAGE(message, Stream::kEvents_ReadBuffer);
    message->m_Args[0].set(&buf);

    this->notifySync(message);
}

bool NFSStream::exists()
{
    return (m_NFS->exists(m_Location) != 0);
}

bool NFSStream::isDir()
{
    return (m_NFS->exists(m_Location) == 2);
}

bool NFSStream::getContentSync(char **data, size_t *len, bool mmap)
{
    *data = NULL;

    const char *ret = m_NFS->readFile(m_Location, len);

    if (!ret) {
        *len = 0;
        return false;
    }

    if (!mmap) {
        *data = static_cast<char *>(malloc(*len + 1));
        memcpy(*data, ret, *len);
        (*data)[*len] = '\0';
    } else {
        /* /!\ data is not null terminated */
        // TODO: new style cast
        *data = (char *)(ret);
    }

    return true;
}

size_t NFSStream::getFileSize() const
{
    return m_File.len;
}

void NFSStream::seek(size_t pos)
{
    if (pos > m_File.len) {
        this->errorSync(Stream::kErrors_Seek, -1);
        return;
    }
    m_File.pos = pos;
}

// }}}

// {{{ Callbacks

void NFSStream::onStart(size_t packets, size_t seek)
{
    m_File.data = reinterpret_cast<const unsigned char *>(
        m_NFS->readFile(m_Location, &m_File.len));
    m_File.pos = 0;

    if (m_File.data == NULL) {
        this->errorSync(Stream::kErrors_Open, 0);

        return;
    }

    CREATE_MESSAGE(message_available, Events::kEvents_AvailableData);
    message_available->m_Args[0].set(nidium_min(packets, m_File.len));

    this->notifySync(message_available);

    buffer buf;
    buffer_init(&buf);

    // TODO: new style cast
    buf.data = (unsigned char *)(m_File.data);
    buf.size = buf.used = m_File.len;

    CREATE_MESSAGE(message, Stream::kEvents_ReadBuffer);
    message->m_Args[0].set(&buf);

    this->notifySync(message);
}

const unsigned char *NFSStream::onGetNextPacket(size_t *len, int *err)
{
    const unsigned char *data;

    ssize_t byteLeft = m_File.len - m_File.pos;

    if (byteLeft <= 0) {
        *err = Stream::kDataStatus_End;
        return NULL;
    }

    data = m_File.data + m_File.pos;
    *len = nidium_min(m_PacketsSize, byteLeft);

    m_File.pos += *len;

    return data;
}

// }}}

} // namespace IO
} // namespace Nidium
