/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "NFSStream.h"

#include <ape_netlib.h>
#include <ape_buffer.h>

#include "Core/Utils.h"

#include "NFS.h"

#include "Binding/NidiumJS.h"

namespace Nidium {
namespace IO {

// {{{ Implementation

#ifdef NIDIUM_EMBED_PRIVATE
  #include NIDIUM_EMBED_PRIVATE
#endif
NFSStream::NFSStream(const char *location) :
    Stream(location)
{
    static NFS *nfs = NULL;

#ifdef NIDIUM_EMBED_PRIVATE
    if (nfs == NULL) {
        nfs = new NFS(private_bin, sizeof(private_bin));
    }
#endif
    m_NFS = nfs;
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
    m_File.data = reinterpret_cast<const unsigned char *>(m_NFS->readFile(m_Location, &m_File.len));
    m_File.pos = 0;

    if (m_File.data == NULL) {
        this->errorSync(ERROR_OPEN, 0);

        return;
    }

    buffer buf;
    buffer_init(&buf);

    //TODO: new style cast
    buf.data = (unsigned char *)(m_File.data);
    buf.size = buf.used = m_File.len;

    CREATE_MESSAGE(message, EVENT_READ_BUFFER);
    message->args[0].set(&buf);

    this->notifySync(message);
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
        //TODO: new style cast
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
        this->errorSync(ERROR_SEEK, -1);
        return;
    }
    m_File.pos = pos;
}

// }}}

// {{{ Callbacks

void NFSStream::onStart(size_t packets, size_t seek)
{
    m_File.data = reinterpret_cast<const unsigned char *>(m_NFS->readFile(m_Location, &m_File.len));
    m_File.pos = 0;

    if (m_File.data == NULL) {
        this->errorSync(ERROR_OPEN, 0);

        return;
    }

    CREATE_MESSAGE(message_available,
        EVENT_AVAILABLE_DATA);
    message_available->args[0].set(nidium_min(packets, m_File.len));

    this->notifySync(message_available);

    buffer buf;
    buffer_init(&buf);

    // TODO: new style cast
    buf.data = (unsigned char *)(m_File.data);
    buf.size = buf.used = m_File.len;

    CREATE_MESSAGE(message, EVENT_READ_BUFFER);
    message->args[0].set(&buf);

    this->notifySync(message);
}

const unsigned char *NFSStream::onGetNextPacket(size_t *len, int *err)
{
    const unsigned char *data;

    ssize_t byteLeft = m_File.len - m_File.pos;

    if (byteLeft <= 0) {
        *err = DATA_STATUS_END;
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

