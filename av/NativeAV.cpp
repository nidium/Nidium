#include "NativeAV.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include <Coro.h>

extern "C" {
#include <libavformat/avformat.h>
}

#include "NativeAudioNode.h"

pthread_mutex_t NativeAVSource::m_FfmpegLock = PTHREAD_MUTEX_INITIALIZER;

NativeAVBufferReader::NativeAVBufferReader(uint8_t *buffer, unsigned long bufferSize)
    : m_Buffer(buffer), m_BufferSize(bufferSize), m_Pos(0) {}

int NativeAVBufferReader::read(void *opaque, uint8_t *buffer, int size)
{
    NativeAVBufferReader *reader = static_cast<NativeAVBufferReader *>(opaque);

    if (reader->m_Pos + size > reader->m_BufferSize) {
        size = reader->m_BufferSize - reader->m_Pos;
    }

    if (size > 0) {
        memcpy(buffer, reader->m_Buffer + reader->m_Pos, size);
        reader->m_Pos += size;
    }

    return size;
}


int64_t NativeAVBufferReader::seek(void *opaque, int64_t offset, int whence)
{
    NativeAVBufferReader *reader = static_cast<NativeAVBufferReader *>(opaque);
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


#define STREAM_BUFFER_SIZE NATIVE_AVIO_BUFFER_SIZE*6
NativeAVStreamReader::NativeAVStreamReader(const char *src,
        NativeAVStreamReadCallback readCallback, void *callbackPrivate, NativeAVSource *source, ape_global *net)
    : m_Source(source), m_TotalRead(0), m_ReadCallback(readCallback),
      m_CallbackPrivate(callbackPrivate), m_Opened(false),
      m_StreamRead(STREAM_BUFFER_SIZE), m_StreamPacketSize(0), m_StreamErr(-1),
      m_StreamSeekPos(0), m_StreamSize(0), m_StreamBuffer(NULL), m_Error(0),
      m_HaveDataAvailable(false)
{
    m_Async = true;
    m_Stream = Nidium::IO::Stream::create(Nidium::Core::Path(src));
    //m_Stream->setAutoClose(false);
    m_Stream->start(STREAM_BUFFER_SIZE);
    m_Stream->setListener(this);
    NATIVE_PTHREAD_VAR_INIT(&m_ThreadCond);
}

int NativeAVStreamReader::read(void *opaque, uint8_t *buffer, int size)
{
    NativeAVStreamReader *thiz = static_cast<NativeAVStreamReader *>(opaque);
    if (thiz->m_StreamErr == AVERROR_EXIT) {
        SPAM(("NativeAVStreamReader, streamErr is EXIT\n"));
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
        thiz->postMessage(opaque, NativeAVStreamReader::MSG_READ);
        NATIVE_PTHREAD_WAIT(&thiz->m_ThreadCond);
        SPAM(("store streamBuffer=%p / size=%d / err=%d\n", thiz->m_StreamBuffer, thiz->m_StreamPacketSize, thiz->m_StreamErr));
        if (!thiz->m_StreamBuffer) {
            switch (thiz->m_StreamErr) {
                case AVERROR_EXIT:
                    SPAM(("Got EXIT\n"));
                    thiz->m_Pending = false;
                    thiz->m_NeedWakup = false;
                    return AVERROR_EXIT;
                case Nidium::IO::Stream::DATA_STATUS_END:
                case Nidium::IO::Stream::DATA_STATUS_ERROR:
                    thiz->m_Error = AVERROR_EOF;
                    SPAM(("Got EOF\n"));
                    thiz->m_Pending = false;
                    thiz->m_NeedWakup = false;
                    return copied > 0 ? copied : thiz->m_Error;
                break;
                case Nidium::IO::Stream::DATA_STATUS_EAGAIN:
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

int64_t NativeAVStreamReader::seek(void *opaque, int64_t offset, int whence)
{
    NativeAVStreamReader *thiz = static_cast<NativeAVStreamReader *>(opaque);
    int64_t pos = 0;
    off_t size = thiz->m_Stream->getFileSize();
    SPAM(("NativeAVStreamReader::seek to %llu / %d\n", offset, whence));

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

    if (Nidium::Core::Utils::isMainThread()) {
        thiz->m_Stream->seek(pos);
    } else {
        thiz->postMessage(opaque, NativeAVStreamReader::MSG_SEEK);
        NATIVE_PTHREAD_WAIT(&thiz->m_ThreadCond);
    }

    return pos;
}

void NativeAVStreamReader::onMessage(const Nidium::Core::SharedMessages::Message &msg)
{
    //NativeAVStreamReader *thiz = static_cast<NativeAVStreamReader *>(msg.dataPtr());

    switch (msg.event()) {
        case Nidium::IO::Stream::EVENT_AVAILABLE_DATA:
            this->onAvailableData(0);
            return;
        case Nidium::IO::Stream::EVENT_ERROR: {
            int err;
            int streamErr = msg.args[0].toInt();

            if (streamErr == Nidium::IO::Stream::ERROR_OPEN) {
                err = ERR_FAILED_OPEN;
            } else if (streamErr == Nidium::IO::Stream::ERROR_READ) {
                err = ERR_READING;
            } else {
                err = ERR_IO;
            }

            m_Source->sendEvent(SOURCE_EVENT_ERROR, err, false);

            return;
        }
        case Nidium::IO::Stream::EVENT_PROGRESS: {
            NativeAVSourceEvent *ev = m_Source->createEvent(SOURCE_EVENT_BUFFERING, false);
            ev->m_Args[0].set(msg.args[0].toInt64());
            ev->m_Args[1].set(msg.args[1].toInt64());
            ev->m_Args[2].set(msg.args[2].toInt64());
            m_Source->sendEvent(ev);
            return;
        }
        case Nidium::IO::Stream::EVENT_READ_BUFFER:
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

    NATIVE_PTHREAD_SIGNAL(&m_ThreadCond);
}

/*
void NativeAVStreamReader::onProgress(size_t buffered, size_t len)
{
    this->source->onProgress(buffered, len);
}
*/

/*
void NativeAVStreamReader::onError(NativeStream::StreamError err)
{
    int error;
    switch (err)
    {
        case NativeStream::STREAM_ERROR_OPEN:
            error = ERR_FAILED_OPEN;
        break;
        default:
            error = ERR_UNKNOWN;
        break;
    }

    this->source->sendEvent(SOURCE_EVENT_ERROR, error, 0, false);
}
*/

void NativeAVStreamReader::onAvailableData(size_t len)
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

void NativeAVStreamReader::finish()
{
    m_StreamBuffer = NULL;
    m_StreamErr = AVERROR_EXIT;

    // Clean pending messages
    // (we can have a MSG_READ/MSG_SEEK event if we were waiting for stream data/seek)
    this->delMessages();

    NATIVE_PTHREAD_SIGNAL(&m_ThreadCond);
}

NativeAVStreamReader::~NativeAVStreamReader()
{
    if (Nidium::Core::Utils::isMainThread()) {
        delete m_Stream;
    } else {
        this->postMessage(this, NativeAVStreamReader::MSG_STOP);
        NATIVE_PTHREAD_WAIT(&m_ThreadCond);
    }
}

NativeAVSource::NativeAVSource()
    : m_Opened(false), m_Eof(false), m_Container(NULL), m_Coro(NULL), m_MainCoro(NULL),
      m_Seeking(false), m_DoSemek(false), m_DoSeekTime(0.0f), m_SeekFlags(0),  m_Error(0),
      m_SourceDoOpen(false)
{
}

AVDictionary *NativeAVSource::getMetadata()
{
    if (!m_Opened) {
        return NULL;
    }

    return m_Container ? m_Container->metadata : NULL;
}

AVFormatContext *NativeAVSource::getAVFormatContext()
{
    if (!m_Opened) {
        return NULL;
    }
    return m_Container;
}

int NativeAVSource::getBitrate()
{
    return m_Container ? m_Container->bit_rate : 0;
}

double NativeAVSource::getDuration()
{
    if (!m_Opened) {
        return 0;
    }

    return m_Container->duration/AV_TIME_BASE;
}

int NativeAVSource::readError(int err)
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


void NativeAVSource::onMessage(const Nidium::Core::SharedMessages::Message &msg)
{
    switch (msg.event()) {
        case MSG_CLOSE:
            this->close();
        break;
    }
}

NativeAVSource::~NativeAVSource()
{
}

