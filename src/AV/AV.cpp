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
    : m_Buffer(buffer), m_BufferSize(bufferSize), m_Pos(0) {}

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
    switch(whence)
    {
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
#define STREAM_BUFFER_SIZE NIDIUM_AVIO_BUFFER_SIZE*6
AVStreamReader::AVStreamReader(const char *src,
        AVStreamReadCallback readCallback, void *callbackPrivate, AVSource *source, ape_global *net)
    : m_Source(source), m_TotalRead(0), m_ReadCallback(readCallback),
      m_CallbackPrivate(callbackPrivate), m_Opened(false),
      m_StreamRead(STREAM_BUFFER_SIZE), m_StreamPacketSize(0), m_StreamErr(-1),
      m_StreamSeekPos(0), m_StreamSize(0), m_StreamBuffer(NULL), m_Error(0),
      m_HaveDataAvailable(false), m_GenesisThread(pthread_self())
{
    NIDIUM_PTHREAD_VAR_INIT(&m_ThreadCond);
    m_Async = true;
    m_Stream = Stream::Create(Path(src));
    if (!m_Stream) {
        m_Source->sendEvent(SOURCE_EVENT_ERROR, ERR_FAILED_OPEN, false);
        return;
    }
    //m_Stream->setAutoClose(false);
    m_Stream->start(STREAM_BUFFER_SIZE);
    m_Stream->setListener(this);
}

int AVStreamReader::read(void *opaque, uint8_t *buffer, int size)
{
    AVStreamReader *thiz = static_cast<AVStreamReader *>(opaque);
    if (thiz->m_StreamErr == AVERROR_EXIT) {
        SPAM(("AVStreamReader, streamErr is EXIT\n"));
        thiz->m_Pending = false;
        thiz->m_NeedWakup = false;
        return AVERROR_EXIT;
    }

    int copied = 0;
    int avail = (thiz->m_StreamPacketSize - thiz->m_StreamRead);

    // Have data inside buffer
    if (avail > 0) {
        int left = size - copied;
        int copy = avail > left ? left : avail;

        SPAM(("get streamBuffer = %p, totalRead = %lld, streamRead = %d, "
              "streamSize = %d, copy = %d, size = %d, avail = %d, left = %d\n",
            thiz->m_StreamBuffer, thiz->m_TotalRead, thiz->m_StreamRead, thiz->m_StreamPacketSize, copy, size, avail, left));

        memcpy(buffer + copied, thiz->m_StreamBuffer + thiz->m_StreamRead, copy);

        thiz->m_TotalRead += copy;
        thiz->m_StreamRead += copy;
        copied += copy;

        if (copied >= size) {
            SPAM(("Returning %d\n", copied));
            return copied;
        }
    }

    SPAM(("streamSize = %lld\n", thiz->m_StreamSize));
    // No more data inside buffer, need to get more
    for(; ;) {
        thiz->postMessage(opaque, AVStreamReader::MSG_READ);
        NIDIUM_PTHREAD_WAIT(&thiz->m_ThreadCond);
        SPAM(("store streamBuffer=%p / size=%d / err=%d\n", thiz->m_StreamBuffer, thiz->m_StreamPacketSize, thiz->m_StreamErr));
        if (!thiz->m_StreamBuffer) {
            switch (thiz->m_StreamErr) {
                case AVERROR_EXIT:
                    SPAM(("Got EXIT\n"));
                    thiz->m_Pending = false;
                    thiz->m_NeedWakup = false;
                    return AVERROR_EXIT;
                case Stream::kDataStatus_End:
                case Stream::kDataStatus_Error:
                    thiz->m_Error = AVERROR_EOF;
                    SPAM(("Got EOF\n"));
                    thiz->m_Pending = false;
                    thiz->m_NeedWakup = false;
                    return copied > 0 ? copied : thiz->m_Error;
                break;
                case Stream::kDataStatus_Again:
                    SPAM(("Got eagain\n"));
                    if (!thiz->m_HaveDataAvailable) {
                        // Got EAGAIN, switch back to main coro
                        // and wait for onDataAvailable callback
                        thiz->m_Pending = true;
                        Coro_switchTo_(thiz->m_Source->m_Coro, thiz->m_Source->m_MainCoro);
                    } else {
                        // Another packet is already available
                        // (Packet has been received while waiting for the MSG_READ reply)
                    }
                break;
                default:
                    fprintf(stderr, "received unknown error (%d) and streamBuffer is null. Returning EOF, copied = %u\n",
                       thiz->m_StreamErr, copied);
                    thiz->m_Error = AVERROR_EOF;
                    return copied > 0 ? copied : thiz->m_Error;
            }
        } else {
            size_t copy = size - copied;
            if (thiz->m_StreamPacketSize < copy) {
                copy = thiz->m_StreamPacketSize;
            }

            SPAM(("Writting to buffer. copied=%d, copy=%d, size=%d\n", copied, copy, size));
            memcpy(buffer + copied, thiz->m_StreamBuffer, copy);

            thiz->m_StreamRead = copy;
            thiz->m_TotalRead += copy;
            copied += copy;
            SPAM(("totalRead=%lld, streamSize=%lld\n", thiz->m_TotalRead, thiz->m_StreamSize));

            // Got enought data, return
            if (copied == size || (thiz->m_StreamSize != 0 && thiz->m_TotalRead >= thiz->m_StreamSize)) {
                SPAM(("wrote enough, return %u \n", copied));
                thiz->m_Error = 0;
                thiz->m_Pending = false;
                thiz->m_NeedWakup = false;

                return copied;
            }
        }
    }

    if (thiz->m_StreamSize != 0 && thiz->m_TotalRead > thiz->m_StreamSize) {
          SPAM(("Oh shit, read after EOF\n"));
          exit(1);
    }

    return 0;
}

int64_t AVStreamReader::seek(void *opaque, int64_t offset, int whence)
{
    AVStreamReader *thiz = static_cast<AVStreamReader *>(opaque);
    int64_t pos = 0;
    off_t size = thiz->m_Stream->getFileSize();
    SPAM(("AVStreamReader::seek to %llu / %d\n", offset, whence));

    switch(whence)
    {
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
        thiz->m_Error = AVERROR_EOF;
        return AVERROR_EOF;
    }

    SPAM(("SEEK pos=%lld, size=%lld\n", pos, size));

    thiz->m_StreamBuffer = NULL;
    thiz->m_StreamRead = STREAM_BUFFER_SIZE;
    thiz->m_TotalRead = pos;
    thiz->m_StreamSeekPos = pos;

    if (thiz->m_StreamErr == AVERROR_EXIT) {
        return pos;
    }

    if (thiz->isGenesisThread()) {
        thiz->m_Stream->seek(pos);
    } else {
        thiz->postMessage(opaque, AVStreamReader::MSG_SEEK);
        NIDIUM_PTHREAD_WAIT(&thiz->m_ThreadCond);
    }

    return pos;
}

void AVStreamReader::onMessage(const SharedMessages::Message &msg)
{
    //AVStreamReader *thiz = static_cast<AVStreamReader *>(msg.dataPtr());

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
            AVSourceEvent *ev = m_Source->createEvent(SOURCE_EVENT_BUFFERING, false);
            ev->m_Args[0].set(msg.m_Args[0].toInt64());
            ev->m_Args[1].set(msg.m_Args[1].toInt64());
            ev->m_Args[2].set(msg.m_Args[2].toInt64());
            m_Source->sendEvent(ev);
            return;
        }
        case Stream::kEvents_ReadBuffer:
            return;
        case MSG_SEEK:
            m_Stream->seek(m_StreamSeekPos);
            break;
        case MSG_READ:
            m_StreamBuffer = m_Stream->getNextPacket(&m_StreamPacketSize, &m_StreamErr);
            m_HaveDataAvailable = false;
            break;
        case MSG_STOP:
            delete m_Stream;
            break;
        default:
            return;
    }

    NIDIUM_PTHREAD_SIGNAL(&m_ThreadCond);
}

bool AVStreamReader::isGenesisThread() {
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
    m_Error = 0;
    m_HaveDataAvailable = true;
    SPAM(("onAvailableData=%d/%d\n", len, m_Opened));

    if (m_Pending) {
        m_NeedWakup = true;
        m_ReadCallback(m_CallbackPrivate);
        return;
    }

    if (!m_Opened) {
        m_StreamSize = m_Stream->getFileSize();
        m_Opened = true;
        m_Source->openInit();
    }
}

void AVStreamReader::finish()
{
    m_StreamBuffer = NULL;
    m_StreamErr = AVERROR_EXIT;

    // Clean pending messages
    // (we can have a MSG_READ/MSG_SEEK event if we were waiting for stream data/seek)
    this->delMessages();

    NIDIUM_PTHREAD_SIGNAL(&m_ThreadCond);
}

AVStreamReader::~AVStreamReader()
{
    if (this->isGenesisThread()) {
        delete m_Stream;
    } else {
        this->postMessage(this, AVStreamReader::MSG_STOP);
        NIDIUM_PTHREAD_WAIT(&m_ThreadCond);
    }
}
// }}}

// {{{ AVSource
AVSource::AVSource()
    : m_Opened(false), m_Eof(false), m_Container(NULL), m_Coro(NULL), m_MainCoro(NULL),
      m_Seeking(false), m_DoSemek(false), m_DoSeekTime(0.0f), m_SeekFlags(0),  m_Error(0),
      m_SourceDoOpen(false)
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

    return m_Container->duration/AV_TIME_BASE;
}

int AVSource::readError(int err)
{
    SPAM(("readError Got error %d/%d\n", err, AVERROR_EOF));
    if (err == AVERROR_EOF || (m_Container->pb && m_Container->pb->eof_reached)) {
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

AVSource::~AVSource()
{
}
// }}}

} // namespace AV
} // namespace Nidium

