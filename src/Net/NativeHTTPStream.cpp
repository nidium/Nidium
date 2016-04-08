/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "NativeHTTPStream.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "NativeJS.h"

using namespace Native::Core;

#define MMAP_SIZE_FOR_UNKNOWN_CONTENT_LENGTH (1024LL*1024LL*64LL)

NativeHTTPStream::NativeHTTPStream(const char *location) :
    NativeBaseStream(location), m_StartPosition(0),
    m_BytesBuffered(0), m_LastReadUntil(0)
{

    m_Mapped.addr = NULL;
    m_Mapped.fd   = 0;
    m_Mapped.size = 0;

    m_Http = new HTTP(NativeJS::getNet());
}

NativeHTTPStream::~NativeHTTPStream()
{
    if (m_Mapped.addr) {
        munmap(m_Mapped.addr, m_Mapped.size);
    }
    if (m_Mapped.fd) {
        close(m_Mapped.fd);
    }

    delete m_Http;
}

void NativeHTTPStream::onStart(size_t packets, size_t seek)
{
    if (m_Mapped.fd) {
        close(m_Mapped.fd);
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


    HTTPRequest *req = m_Http->getRequest();
    if (!req) {
        req = new HTTPRequest(m_Location);
    }

    m_Http->request(req, this);
}

const char *NativeHTTPStream::getPath() const
{
    if (!m_Http) {
        return NULL;
    }

    return m_Http->getPath();
}

bool NativeHTTPStream::hasDataAvailable() const
{
    /*
        Returns true if we have either enough data buffered or
        if the stream reached the end of file
    */
    return !m_PendingSeek && ((m_BytesBuffered - m_LastReadUntil >= m_PacketsSize ||
        (m_LastReadUntil != m_BytesBuffered && this->readComplete())));
}

const unsigned char *NativeHTTPStream::onGetNextPacket(size_t *len, int *err)
{
    unsigned char *data;

    if (!m_Mapped.addr) {
        *err = STREAM_ERROR;
        return NULL;
    }

    if (m_Mapped.size == m_LastReadUntil) {
        *err = STREAM_END;
        return NULL;
    }
    if (!this->hasDataAvailable()) {
        m_NeedToSendUpdate = true;
        *err = STREAM_EAGAIN;
        return NULL;
    }

    ssize_t byteLeft = m_Mapped.size - m_LastReadUntil;
    *len = native_min(m_PacketsSize, byteLeft);

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
    this->onStart(0, 0);
    m_NeedToSendUpdate = false;
}

size_t NativeHTTPStream::getFileSize() const
{
    return m_Http->getFileSize();
}

int NativeHTTPStream_notifyAvailable(void *arg)
{
    NativeHTTPStream *http = (NativeHTTPStream *)arg;

    http->notifyAvailable();
    return 0;
}

void NativeHTTPStream::seek(size_t pos)
{
    size_t max = m_StartPosition + m_BytesBuffered;

    /*
        We can read directly from our buffer
    */
    if (pos >= m_StartPosition && pos < max &&
        (pos <= max - m_PacketsSize || this->readComplete())) {

        ape_global *ape = NativeJS::getNet();
        m_LastReadUntil = pos - m_StartPosition;
        m_PendingSeek = true;
        m_NeedToSendUpdate = false;

        timer_dispatch_async(NativeHTTPStream_notifyAvailable, this);

        return;
    }

    /*
        We need to seek in HTTP
    */
    HTTPRequest *req = m_Http->getRequest();

    m_Http->stopRequest();

    m_PendingSeek = true;
    char seekstr[64];
    sprintf(seekstr, "bytes=%zu-", pos);

    req->recycle();

    req->setHeader("Range", seekstr);

    m_Http->request(req, this);

    m_StartPosition = pos;
    m_LastReadUntil = 0;
    m_BytesBuffered = 0;

    m_NeedToSendUpdate = true;
}

void NativeHTTPStream::notifyAvailable()
{
    m_PendingSeek = false;
    m_NeedToSendUpdate = false;

    CREATE_MESSAGE(message_available, NATIVESTREAM_AVAILABLE_DATA);
    message_available->args[0].set(m_BytesBuffered - m_LastReadUntil);

    this->notify(message_available);
}

void NativeHTTPStream::onRequest(HTTP::HTTPData *h, HTTP::DataType)
{
    this->m_DataBuffer.ended = true;

    buffer buf;
    buffer_init(&buf);

    buf.data = (unsigned char *)m_Mapped.addr;
    buf.size = buf.used = m_Mapped.size;

    CREATE_MESSAGE(message, NATIVESTREAM_READ_BUFFER);
    message->args[0].set(&buf);

    this->notify(message);
}

void NativeHTTPStream::onProgress(size_t offset, size_t len,
    HTTP::HTTPData *h, HTTP::DataType)
{
    /* overflow or invalid state */
    if (!m_Mapped.fd || !m_Mapped.addr ||
          m_BytesBuffered + len > m_Mapped.size) {
        m_Http->resetData();
        return;
    }

    memcpy((char *)m_Mapped.addr + m_BytesBuffered,
        &h->data->data[offset], len);

    m_BytesBuffered += len;

    /* Reset the data buffer, so that data doesn't grow in memory */
    m_Http->resetData();

    CREATE_MESSAGE(msg_progress, NATIVESTREAM_PROGRESS);
    msg_progress->args[0].set(m_Http->getFileSize());
    msg_progress->args[1].set(m_StartPosition);
    msg_progress->args[2].set(m_BytesBuffered);
    msg_progress->args[3].set(m_LastReadUntil);

    this->notify(msg_progress);

    if (m_NeedToSendUpdate && this->hasDataAvailable()) {
        this->notifyAvailable();
    }
}

void NativeHTTPStream::onError(HTTP::HTTPError err)
{
    this->cleanCacheFile();

    if (m_PendingSeek) {
        this->error(NATIVESTREAM_ERROR_SEEK, -1);
        m_PendingSeek = false;
        this->stop();
        return;
    }
    switch (err) {
        case HTTP::ERROR_HTTPCODE:
            this->error(NATIVESTREAM_ERROR_OPEN, m_Http->getStatusCode());
            break;
        case HTTP::ERROR_RESPONSE:
        case HTTP::ERROR_SOCKET:
        case HTTP::ERROR_TIMEOUT:
            this->error(NATIVESTREAM_ERROR_OPEN, -1);
            break;
        default:
            break;
    }
}

void NativeHTTPStream::cleanCacheFile()
{
    if (m_Mapped.addr) {
        munmap(m_Mapped.addr, m_Mapped.size);
        m_Mapped.addr = NULL;
        m_Mapped.size = 0;
    }
}

void NativeHTTPStream::onHeader()
{
    m_BytesBuffered = 0;

    if (!m_Mapped.fd) {
        return;
    }

    this->cleanCacheFile();
    /*
        HTTP didn't returned a partial content (HTTP 206) (seek failed?)
    */
    if (m_PendingSeek && m_Http->getStatusCode() != 206) {
        this->error(NATIVESTREAM_ERROR_SEEK, -1);

        m_PendingSeek = false;
        this->stop();
        return;
    }

    m_PendingSeek = false;

    m_Mapped.size = (!m_Http->http.contentlength ?
        MMAP_SIZE_FOR_UNKNOWN_CONTENT_LENGTH : m_Http->http.contentlength);

    if (ftruncate(m_Mapped.fd, m_Mapped.size) == -1) {
        m_Mapped.size = 0;
        this->stop();

        return;
    }

    m_Mapped.addr = mmap(NULL, m_Mapped.size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        m_Mapped.fd, 0);

    if (m_Mapped.addr == MAP_FAILED) {
        m_Mapped.addr = NULL;
        m_Mapped.size = 0;
        this->stop();

        return;
    }
}

