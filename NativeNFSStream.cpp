/*
    NativeJS Core Library
    Copyright (C) 2014 Anthony Catel <paraboul@gmail.com>

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

#include "NativeNFSStream.h"
#include "NativeUtils.h"
#include "NativeJS.h"
#include <NativeNFS.h>
#include <native_netlib.h>
#include <ape_buffer.h>


#ifdef NATIVE_EMBED_PRIVATE
  #include NATIVE_EMBED_PRIVATE
#endif
NativeNFSStream::NativeNFSStream(const char *location) : 
    NativeBaseStream(location)
{
    static NativeNFS *nfs = NULL;

#ifdef NATIVE_EMBED_PRIVATE
    if (nfs == NULL) {
        nfs = new NativeNFS(private_bin, sizeof(private_bin));
    }
#endif
    m_NFS = nfs;
    m_File.pos = 0;
}

void NativeNFSStream::onStart(size_t packets, size_t seek)
{
    m_File.data = (const unsigned char *)m_NFS->readFile(m_Location, &m_File.len);
    m_File.pos = 0;

    if (m_File.data == NULL) {
        this->error(NATIVESTREAM_ERROR_OPEN, 0);

        return;
    }

    CREATE_MESSAGE(message_available,
        NATIVESTREAM_AVAILABLE_DATA);
    message_available->args[0].set(native_min(packets, m_File.len));

    this->notify(message_available);

    buffer buf;
    buffer_init(&buf);

    buf.data = (unsigned char *)m_File.data;
    buf.size = buf.used = m_File.len;

    CREATE_MESSAGE(message, NATIVESTREAM_READ_BUFFER);
    message->args[0].set(&buf);

    /*
        The underlying object is notified in a sync way 
        since it's on the same thread.
    */
    this->notify(message);    
}

const unsigned char *NativeNFSStream::onGetNextPacket(size_t *len, int *err)
{
    const unsigned char *data;

    ssize_t byteLeft = m_File.len - m_File.pos;

    if (byteLeft <= 0) {
        *err = STREAM_END;
        return NULL;
    }

    data = m_File.data + m_File.pos;
    *len = native_min(m_PacketsSize, byteLeft);

    m_File.pos += *len;

    return data;
}

void NativeNFSStream::stop()
{
    /*
        Do nothing
    */
}

int NativeNFSStream_getContent(void *arg)
{
    ((NativeNFSStream *)arg)->_getContent();

    return 0;
}

void NativeNFSStream::getContent()
{
    ape_global *ape = NativeJS::getNet();
    timer_dispatch_async_unprotected(NativeNFSStream_getContent, this);
}

void NativeNFSStream::_getContent()
{
    m_File.data = (const unsigned char *)m_NFS->readFile(m_Location, &m_File.len);
    m_File.pos = 0;

    if (m_File.data == NULL) {
        this->error(NATIVESTREAM_ERROR_OPEN, 0);

        return;
    }

    buffer buf;
    buffer_init(&buf);

    buf.data = (unsigned char *)m_File.data;
    buf.size = buf.used = m_File.len;

    CREATE_MESSAGE(message, NATIVESTREAM_READ_BUFFER);
    message->args[0].set(&buf);

    this->notify(message);
}

bool NativeNFSStream::getContentSync(char **data, size_t *len, bool mmap)
{
    const char *ret = m_NFS->readFile(m_Location, len);

    if (!ret) {
        *len = 0;
        return false;
    }

    if (!mmap) {
        *data = (char *)malloc(*len + 1);
        memcpy(*data, ret, *len);
        *data[*len] = '\0';
    } else {
        /* /!\ data is not null terminated */
        *data = (char *)ret;
    }

    return true;
}

size_t NativeNFSStream::getFileSize() const
{
    return m_File.len;
}

void NativeNFSStream::seek(size_t pos)
{
    if (pos > m_File.len) {
        this->error(NATIVESTREAM_ERROR_SEEK, -1);
        return;
    }
    m_File.pos = pos;
}
