/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "AV.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include <Coro.h>

extern "C" {
#include <libavformat/avformat.h>
}

#include "Macros.h"
#include "AudioNode.h"

using Nidium::Core::Utils;
using Nidium::Core::Path;
using Nidium::Core::SharedMessages;
using Nidium::IO::Stream;

namespace Nidium {
namespace AV {

pthread_mutex_t AVSource::m_FfmpegLock = PTHREAD_MUTEX_INITIALIZER;

// {{{ AVBufferReader
AVBufferReader::AVBufferReader(uint8_t *buffer, unsigned long bufferSize)
    : m_Buffer(buffer), m_BufferSize(bufferSize), m_Pos(0)
{
}

int AVBufferReader::read(void *opaque, uint8_t *buffer, int size)
{
    AVBufferReader *reader = static_cast<AVBufferReader *>(opaque);

    if (reader->m_Pos + size > reader->m_BufferSize) {
        size = reader->m_BufferSize - reader->m_Pos;
    }

    if (size > 0) {
        memcpy(buffer, reader->m_Buffer + reader->m_Pos, size);
        reader->m_Pos += size;
    }

    return size;
}

int64_t AVBufferReader::seek(void *opaque, int64_t offset, int whence)
{
    AVBufferReader *reader = static_cast<AVBufferReader *>(opaque);
    int64_t pos = 0;
    switch (whence) {
        case AVSEEK_SIZE:
            return reader->m_BufferSize;
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos = reader->m_Pos + offset;
            break;
        case SEEK_END:
            pos = reader->m_BufferSize - offset;
            break;
        default:
            return -1;
    }

    if (pos < 0 || pos > reader->m_BufferSize) {
        return -1;
    }

    reader->m_Pos = pos;

    return pos;
}
// }}}

// {{{ AVStreamReader
AVStreamReader::AVStreamReader(const char *src,
                               AVStreamReadCallback readCallback,
                               void *callbackPrivate,
                               AVSource *source,
                               ape_global *net)
    : m_Source(source), m_ReadCallback(readCallback),
      m_CallbackPrivate(callbackPrivate),  m_GenesisThread(pthread_self())
{
    m_Async  = true;
    m_Stream = Stream::Create(Path(src));
    if (!m_Stream) {
        m_Source->sendEvent(SOURCE_EVENT_ERROR, ERR_FAILED_OPEN, false);
        return;
    }
    m_Stream->start(NIDIUM_AVIO_BUFFER_SIZE * 4);
    m_Stream->setListener(this);
}

int AVStreamReader::read(void *opaque, uint8_t *buffer, int size)
{
    AVStreamReader *reader = static_cast<AVStreamReader *>(opaque);
    return reader->_read(buffer, size);
}

bool AVStreamReader::fillBuffer(uint8_t *buffer, int size, int *outCopied)
{
    int avail  = m_StreamPacketSize - m_StreamRead;
    if (avail <= 0 || !m_StreamBuffer) {
        return false;
    }

    int copied = *outCopied;
    int left   = size - copied;
    int copy   = avail > left ? left : avail;

    SPAM(("Filling ffmpeg buffer : "
         " - Stream (%p) : totalRead=%lld, currentRead=%d, currentSize=%d"
         " - Buffer : size=%d copy=%d, copied=%d (avail=%d, left=%d)",
         m_StreamBuffer, m_TotalRead, m_StreamRead, m_StreamPacketSize,
         size, copy, copied, avail, left));

    memcpy(buffer + copied, m_StreamBuffer + m_StreamRead, copy);

    m_TotalRead  += copy;
    m_StreamRead += copy;
    copied       += copy;

    *outCopied   = copied;

    if (copied >= size
        || (m_StreamSize != 0
            && m_TotalRead >= m_StreamSize)) {
        return true;
    }

    return false;
}

void AVStreamReader::streamMessage(AVStreamReader::StreamMessage ev)
{
    std::unique_lock<std::mutex> lock(m_PostMessageMutex);

    this->postMessage(this, ev);

    while (m_MessagePosted == false) {
        m_PostMessageCond.wait(lock);
    }
    m_MessagePosted = false;
}

int AVStreamReader::_read(uint8_t *buffer, int size)
{
    SPAM(("%p / Read called", this));
    if (m_StreamErr == AVERROR_EXIT) {
        SPAM(("AVStreamReader, streamErr is EXIT"));
        m_Pending   = false;
        m_NeedWakup = false;
        return AVERROR_EXIT;
    }

    int copied = 0;

    // First, try to fill the buffer with what is already in memory
    SPAM(("%p / fillBuffer", this));
    if (this->fillBuffer(buffer, size, &copied)) {
        SPAM(("Buffer filled with data already in memory"));
        return copied;
    }
    SPAM(("%p / fillBuffer is over", this));

    // If we reach this point, there is no more data inside
    // the stream buffer. Let's get more.
    for (;;) {
        SPAM(("%p / Asking for more data", this));
        this->streamMessage(kStream_Read);

        SPAM(("%p / store streamBuffer=%p / size=%d / err=%d", this,
              m_StreamBuffer, m_StreamPacketSize,
              m_StreamErr));

        if (m_StreamErr ==  Stream::kDataStatus_Again) {
            std::unique_lock<std::mutex> lock(m_DataAvailMutex);

            if (!m_HaveDataAvailable) {
                m_Pending = true;
                lock.unlock();

                SPAM(("Got EAGAIN, no data available, switching back to main coro"));

                Coro_switchTo_(m_Source->m_Coro,
                               m_Source->m_MainCoro);

                SPAM(("After EAGAIN"));

                lock.lock();
                m_Pending   = false;
                m_NeedWakup = false;
                lock.unlock();
            } else {
                // Another packet is already available
                // (Packet has been received while waiting for the
                // MSG_READ reply)
                lock.unlock();
                continue;
            }
        } else if (!m_StreamBuffer) {
            std::lock_guard<std::mutex> lock(m_DataAvailMutex);

            switch (m_StreamErr) {
                case AVERROR_EXIT:
                    m_Pending   = false;
                    m_NeedWakup = false;
                    return AVERROR_EXIT;
                case Stream::kDataStatus_End:
                case Stream::kDataStatus_Error:
                    SPAM(("Got EOF"));
                    m_Pending   = false;
                    m_NeedWakup = false;
                    return copied > 0 ? copied : AVERROR_EOF;
                default:
                    ndm_logf(NDM_LOG_ERROR, "AVStream",
                            "Received unknown error (%d) and streamBuffer is "
                            "null. Returning EOF, "
                            "copied = %u",
                            m_StreamErr, copied);
                    return copied > 0 ? copied : AVERROR_EOF;
            }
        } else {
            if (this->fillBuffer(buffer, size, &copied)) {
                return copied;
            }
        }
    }

    return 0;
}

int64_t AVStreamReader::seek(void *opaque, int64_t offset, int whence)
{
    AVStreamReader *thiz = static_cast<AVStreamReader *>(opaque);
    int64_t pos          = 0;
    off_t size = thiz->m_Stream->getFileSize();
    SPAM(("AVStreamReader::seek to %llu / %d", offset, whence));

    switch (whence) {
        case AVSEEK_SIZE:
            return size;
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos = (thiz->m_TotalRead) + offset;
            break;
        case SEEK_END:
            if (size != 0) {
                pos = size - offset;
            } else {
                pos = 0;
            }
            break;
        default:
            return -1;
    }

    if (pos < 0 || pos > size) {
        return AVERROR_EOF;
    }

    SPAM(("SEEK pos=%lld, size=%lld", pos, size));

    thiz->m_StreamBuffer  = NULL;
    thiz->m_StreamRead    = 0;
    thiz->m_TotalRead     = pos;
    thiz->m_StreamSeekPos = pos;

    if (thiz->m_StreamErr == AVERROR_EXIT) {
        return pos;
    }

    if (thiz->isGenesisThread()) {
        thiz->m_Stream->seek(pos);
    } else {
        thiz->streamMessage(kStream_Seek);
    }

    return pos;
}

void AVStreamReader::onMessage(const SharedMessages::Message &msg)
{
    switch (msg.event()) {
        case Stream::kEvents_AvailableData:
            this->onAvailableData(0);
            return;
        case Stream::kEvents_Error: {
            int err;
            int streamErr = msg.m_Args[0].toInt();

            if (streamErr == Stream::kErrors_Open) {
                err = ERR_FAILED_OPEN;
            } else if (streamErr == Stream::kErrors_Read) {
                err = ERR_READING;
            } else {
                err = ERR_IO;
            }

            m_Source->sendEvent(SOURCE_EVENT_ERROR, err, false);

            return;
        }
        case Stream::kEvents_Progress: {
            AVSourceEvent *ev
                = m_Source->createEvent(SOURCE_EVENT_BUFFERING, false);
            ev->m_Args[0].set(msg.m_Args[0].toInt64());
            ev->m_Args[1].set(msg.m_Args[1].toInt64());
            ev->m_Args[2].set(msg.m_Args[2].toInt64());
            m_Source->sendEvent(ev);
            return;
        }
        case Stream::kEvents_ReadBuffer:
            return;
        case kStream_Seek:
            m_Stream->seek(m_StreamSeekPos);
            break;
        case kStream_Read:
            {
                std::lock_guard<std::mutex> lock(m_DataAvailMutex);
                m_StreamBuffer
                    = m_Stream->getNextPacket(&m_StreamPacketSize, &m_StreamErr);
                m_HaveDataAvailable = false;
                m_StreamRead        = 0;
            }
            break;
        case kStream_Stop:
            delete m_Stream;
            break;
        default:
            return;
    }

    std::lock_guard<std::mutex> lock(m_PostMessageMutex);
    m_MessagePosted = true;
    m_PostMessageCond.notify_one();
}

bool AVStreamReader::isGenesisThread()
{
    return pthread_equal(m_GenesisThread, pthread_self());
}

/*
void AVStreamReader::onProgress(size_t buffered, size_t len)
{
    this->source->onProgress(buffered, len);
}
*/

/*
void AVStreamReader::onError(Stream::StreamError err)
{
    int error;
    switch (err)
    {
        case Stream::STREAM_ERROR_OPEN:
            error = ERR_FAILED_OPEN;
        break;
        default:
            error = ERR_UNKNOWN;
        break;
    }

    this->source->sendEvent(SOURCE_EVENT_ERROR, error, 0, false);
}
*/

void AVStreamReader::onAvailableData(size_t len)
{
    SPAM(("%p / onAvailableData len=%d opened=%d pending=%d", this, len, m_Opened, m_Pending));
    {
        std::lock_guard<std::mutex> lock(m_DataAvailMutex);
        SPAM(("%p / Got lock", this));

        m_HaveDataAvailable = true;

        if (m_Pending) {
            m_NeedWakup = true;
            m_ReadCallback(m_CallbackPrivate);
            SPAM(("%p / Lock is releaased", this));
            return;
        }
    }
    SPAM(("%p / Lock is releaased", this));

    if (!m_Opened) {
        m_StreamSize = m_Stream->getFileSize();
        m_Opened = true;
        m_Source->openInit();
    }
}

void AVStreamReader::finish()
{
    m_StreamBuffer = NULL;
    m_StreamErr    = AVERROR_EXIT;

    // Clean pending messages
    // (we can have a MSG_READ/MSG_SEEK event if we were waiting for stream
    // data/seek)
    this->delMessages();

    // Wakup any thread waiting for an operation
    std::lock_guard<std::mutex> lock(m_PostMessageMutex);
    m_MessagePosted = true;
    m_PostMessageCond.notify_one();
}

AVStreamReader::~AVStreamReader()
{
    if (this->isGenesisThread()) {
        delete m_Stream;
    } else {
        this->streamMessage(kStream_Stop);
    }
}
// }}}

// {{{ AVSource
AVSource::AVSource()
    : m_Opened(false), m_Eof(false), m_Container(NULL), m_Coro(NULL),
      m_MainCoro(NULL), m_Seeking(false), m_DoSemek(false), m_DoSeekTime(0.0f),
      m_SeekFlags(0), m_Error(0), m_SourceDoOpen(false)
{
}

AVDictionary *AVSource::getMetadata()
{
    if (!m_Opened) {
        return NULL;
    }

    return m_Container ? m_Container->metadata : NULL;
}

AVFormatContext *AVSource::getAVFormatContext()
{
    if (!m_Opened) {
        return NULL;
    }
    return m_Container;
}

int AVSource::getBitrate()
{
    return m_Container ? m_Container->bit_rate : 0;
}

double AVSource::getDuration()
{
    if (!m_Opened) {
        return 0;
    }

    if (m_Container) {
        return m_Container->duration / AV_TIME_BASE;
    }

    return 0;
}

int AVSource::readError(int err)
{
    SPAM(("readError Got error %d/%d", err, AVERROR_EOF));
    if (err == AVERROR_EOF
        || (m_Container->pb && m_Container->pb->eof_reached)) {
        m_Error = AVERROR_EOF;
        return AVERROR_EOF;
    } else if (err != AVERROR(EAGAIN)) {
        m_Error = AVERROR(err);
        this->sendEvent(SOURCE_EVENT_ERROR, ERR_READING, true);
        return -1;
    }

    return 0;
}


void AVSource::onMessage(const SharedMessages::Message &msg)
{
    switch (msg.event()) {
        case MSG_CLOSE:
            this->close();
            break;
    }
}
// }}}

} // namespace AV
} // namespace Nidium
