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

#include "NativeHTTPStream.h"
#include "NativeUtils.h"
#include "NativeJS.h"

#include <errno.h>
#include <sys/mman.h>

NativeHTTPStream::NativeHTTPStream(const char *location) : 
    NativeBaseStream(location), m_StartPosition(0),
    m_BytesBuffered(0), m_LastReadUntil(0)
{

    m_Mapped.addr = NULL;
    m_Mapped.fd   = 0;
    m_Mapped.idx = 0;
    m_Mapped.size = 0;

    NativeHTTPRequest *req = new NativeHTTPRequest(location);
    m_Http = new NativeHTTP(req, NativeJS::getNet());
}

NativeBaseStream *NativeHTTPStream::createStream(const char *location)
{
    return new NativeHTTPStream(location);
}

NativeHTTPStream::~NativeHTTPStream()
{
    if (m_Mapped.addr) {
        munmap(m_Mapped.addr, m_Mapped.size);
    }
    if (m_Mapped.fd) {
        close(m_Mapped.fd);
    }

    if (m_DataBuffer.back) {
        free(m_DataBuffer.back);
    }
    if (m_DataBuffer.front) {
        free(m_DataBuffer.front);
    }

    delete m_Http;
}

void NativeHTTPStream::onStart(size_t packets, size_t seek)
{
    if (m_DataBuffer.back == NULL) {
        m_DataBuffer.back     = buffer_new(0);
        m_DataBuffer.front    = buffer_new(0);
    }

    char tmpfname[] = "/tmp/nidiumtmp.XXXXXXXX";
    m_Mapped.fd = mkstemp(tmpfname);
    if (m_Mapped.fd == -1) {
        printf("[NativeHTTPStream] Failed to create temporary file\n");
        return;
    }
    unlink(tmpfname);

    m_StartPosition = seek;
    m_BytesBuffered = 0;

    m_Http->request(this);
}

bool NativeHTTPStream::hasDataAvailable() const
{
    return (m_BytesBuffered - m_LastReadUntil >= m_PacketsSize);
}

const unsigned char *NativeHTTPStream::onGetNextPacket(size_t *len, int *err)
{
    unsigned char *data;

    if (!m_Mapped.addr) {
        *err = STREAM_ERROR;
        return NULL;
    }

    if (!this->hasDataAvailable()) {
        m_NeedToSendUpdate = true;
        *err = STREAM_EAGAIN;

        return NULL;
    }
    *len = m_PacketsSize;
    data = (unsigned char *)m_Mapped.addr + m_LastReadUntil;
    m_LastReadUntil += *len;

    return data;
}

void NativeHTTPStream::stop()
{
    m_Http->stopRequest();
}

void NativeHTTPStream::getContent()
{
    m_Http->request(this);
}

size_t NativeHTTPStream::getFileSize() const
{
    return m_Http->getFileSize();
}

void NativeHTTPStream::seek(size_t pos)
{

}

//////////////////
//////////////////
//////////////////


void NativeHTTPStream::onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType)
{
    this->m_DataBuffer.ended = true;
}

void NativeHTTPStream::onProgress(size_t offset, size_t len, NativeHTTP::HTTPData *h,
    NativeHTTP::DataType)
{
    /* overflow */
    if (!m_Mapped.fd || m_BytesBuffered + len > m_Mapped.size) {
        m_Http->resetData();
        return;
    }

    memcpy((char *)m_Mapped.addr + m_BytesBuffered, &h->data->data[offset], len);
    m_BytesBuffered += len;

    /* Reset the data buffer, so that data doesn't grow in memory */
    m_Http->resetData();

    if (m_NeedToSendUpdate && (m_LastReadUntil + m_BytesBuffered > m_PacketsSize)) {
        m_NeedToSendUpdate = false;

        CREATE_MESSAGE(message_available, NATIVESTREAM_AVAILABLE_DATA);
        message_available->args[0].set((char *)m_Mapped.addr + m_LastReadUntil);
        
        this->notify(message_available);        
    }
}

void NativeHTTPStream::onError(NativeHTTP::HTTPError err)
{

}

void NativeHTTPStream::onHeader()
{
    m_BytesBuffered = 0;

    if (!m_Mapped.fd) {
        return;
    }

    if (m_Mapped.addr) {
        munmap(m_Mapped.addr, m_Mapped.size);
    }

    m_Mapped.size = m_Http->http.contentlength;
    ftruncate(m_Mapped.fd, m_Mapped.size);

    m_Mapped.addr = mmap(NULL, m_Mapped.size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        m_Mapped.fd, 0);
}
