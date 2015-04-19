#include "NativeAV.h"
#include "NativeAudioNode.h"
#include "pa_ringbuffer.h"
#include "Coro.h"
#include <NativeUtils.h>
#include <NativeSharedMessages.h>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

pthread_mutex_t NativeAVSource::ffmpegLock = PTHREAD_MUTEX_INITIALIZER;

NativeAVBufferReader::NativeAVBufferReader(uint8_t *buffer, unsigned long bufferSize) 
    : buffer(buffer), bufferSize(bufferSize), pos(0) {}

int NativeAVBufferReader::read(void *opaque, uint8_t *buffer, int size) 
{
    NativeAVBufferReader *reader = static_cast<NativeAVBufferReader *>(opaque);

    if (reader->pos + size > reader->bufferSize) {
        size = reader->bufferSize - reader->pos;
    }

    if (size > 0) {
        memcpy(buffer, reader->buffer + reader->pos, size);
        reader->pos += size;
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
            return reader->bufferSize;
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos = reader->pos + offset;
            break;
        case SEEK_END:
            pos = reader->bufferSize - offset;
            break;
        default:
            return -1;
    }

    if( pos < 0 || pos > reader->bufferSize) {
        return -1;
    }

    reader->pos = pos;

    return pos;
}


#define STREAM_BUFFER_SIZE NATIVE_AVIO_BUFFER_SIZE*6
NativeAVStreamReader::NativeAVStreamReader(const char *src,
        NativeAVStreamReadCallback readCallback, void *callbackPrivate, NativeAVSource *source, ape_global *net)
    : source(source), totalRead(0), readCallback(readCallback), callbackPrivate(callbackPrivate),
      opened(false), streamRead(STREAM_BUFFER_SIZE), streamPacketSize(0), streamErr(-1), streamSeekPos(0), streamSize(0),
      streamBuffer(NULL), error(0), m_HaveDataAvailable(false)
{
    this->async = true;
    this->stream = NativeBaseStream::create(NativePath(src));
    //this->stream->setAutoClose(false);
    this->stream->start(STREAM_BUFFER_SIZE);
    this->stream->setListener(this);
    NATIVE_PTHREAD_VAR_INIT(&this->m_ThreadCond);
}

int NativeAVStreamReader::read(void *opaque, uint8_t *buffer, int size) 
{
    NativeAVStreamReader *thiz = static_cast<NativeAVStreamReader *>(opaque);
    if (thiz->streamErr == AVERROR_EXIT) {
        SPAM(("NativeAVStreamReader, streamErr is EXIT\n"));
        thiz->pending = false;
        thiz->needWakup = false;
        return AVERROR_EXIT;
    }

    int copied = 0;
    int avail = (thiz->streamPacketSize - thiz->streamRead);

    // Have data inside buffer
    if (avail > 0) {
        int left = size - copied;
        int copy = avail > left ? left : avail;

        SPAM(("get streamBuffer=%p, totalRead=%lld, streamRead=%d, streamSize=%d, copy=%d, size=%d, avail=%d, left=%d\n", thiz->streamBuffer, thiz->totalRead, thiz->streamRead, thiz->streamPacketSize, copy, size, avail, left));

        memcpy(buffer + copied, thiz->streamBuffer + thiz->streamRead, copy);

        thiz->totalRead += copy;
        thiz->streamRead += copy;
        copied += copy;

        if (copied >= size) {
            SPAM(("Returning %d\n", copied));
            return copied;
        }
    }

    SPAM(("streamSize=%lld\n", thiz->streamSize));
    // No more data inside buffer, need to get more
    for(;;) {
        thiz->postMessage(opaque, NativeAVStreamReader::MSG_READ);
        NATIVE_PTHREAD_WAIT(&thiz->m_ThreadCond);
        SPAM(("store streamBuffer=%p / size=%d / err=%d\n", thiz->streamBuffer, thiz->streamPacketSize, thiz->streamErr));
        if (!thiz->streamBuffer) {
            switch (thiz->streamErr) {
                case AVERROR_EXIT:
                    SPAM(("Got EXIT\n"));
                    thiz->pending = false;
                    thiz->needWakup = false;
                    return AVERROR_EXIT;
                case NativeBaseStream::STREAM_END:
                case NativeBaseStream::STREAM_ERROR:
                    thiz->error = AVERROR_EOF;
                    SPAM(("Got EOF\n"));
                    thiz->pending = false;
                    thiz->needWakup = false;
                    return copied > 0 ? copied : thiz->error;
                break;
                case NativeBaseStream::STREAM_EAGAIN:
                    SPAM(("Got eagain\n"));
                    if (!thiz->m_HaveDataAvailable) {
                        // Got EAGAIN, switch back to main coro
                        // and wait for onDataAvailable callback
                        thiz->pending = true;
                        Coro_switchTo_(thiz->source->coro, thiz->source->mainCoro);
                    } else {
                        // Another packet is already available
                        // (Packet has been received while waiting for the MSG_READ reply)
                    }
                break;
                default:
                    printf("received unknown error (%d) and streamBuffer is null. Returning EOF, copied = %d\n", thiz->streamErr, copied);
                    thiz->error = AVERROR_EOF;
                    return copied > 0 ? copied : thiz->error;
            }
        } else {
            size_t copy = size - copied;
            if (thiz->streamPacketSize < copy) {
                copy = thiz->streamPacketSize;
            }

            SPAM(("Writting to buffer. copied=%d, copy=%d, size=%d\n", copied, copy, size));
            memcpy(buffer + copied, thiz->streamBuffer, copy);

            thiz->streamRead = copy;
            thiz->totalRead += copy;
            copied += copy;
            SPAM(("totalRead=%lld, streamSize=%lld\n", thiz->totalRead, thiz->streamSize));

            // Got enought data, return
            if (copied == size || (thiz->streamSize != 0 && thiz->totalRead >= thiz->streamSize)) {
                SPAM(("wrote enough, return %u \n", copied));
                thiz->error = 0;
                thiz->pending = false;
                thiz->needWakup = false;

                return copied;
            } 
        }
    }

    if (thiz->streamSize != 0 && thiz->totalRead > thiz->streamSize) {
          SPAM(("Oh shit, read after EOF\n"));
          exit(1);
    }

    return 0; 
}

int64_t NativeAVStreamReader::seek(void *opaque, int64_t offset, int whence) 
{
    NativeAVStreamReader *thiz = static_cast<NativeAVStreamReader *>(opaque);
    int64_t pos = 0;
    off_t size = thiz->stream->getFileSize();
    SPAM(("NativeAVStreamReader::seek to %llu / %d\n", offset, whence));

    switch(whence)
    {
        case AVSEEK_SIZE:
            return size;
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos = (thiz->totalRead) + offset;
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

    if( pos < 0 || pos > size) {
        thiz->error = AVERROR_EOF;
        return AVERROR_EOF;
    }

    SPAM(("SEEK pos=%lld, size=%lld\n", pos, size));

    thiz->streamBuffer = NULL;
    thiz->streamRead = STREAM_BUFFER_SIZE;
    thiz->totalRead = pos;
    thiz->streamSeekPos = pos;

    if (thiz->streamErr == AVERROR_EXIT) {
        return pos;
    }

    if (NativeUtils::isMainThread()) {
        thiz->stream->seek(pos);
    } else {
        thiz->postMessage(opaque, NativeAVStreamReader::MSG_SEEK);
        NATIVE_PTHREAD_WAIT(&thiz->m_ThreadCond);
    }

    return pos;
}

void NativeAVStreamReader::onMessage(const NativeSharedMessages::Message &msg)
{
    //NativeAVStreamReader *thiz = static_cast<NativeAVStreamReader *>(msg.dataPtr());

    switch (msg.event()) {
        case NATIVESTREAM_AVAILABLE_DATA:
            this->onAvailableData(0);
            return;
        case NATIVESTREAM_ERROR: {
            int err;
            int streamErr = msg.args[0].toInt();

            if (streamErr == NativeBaseStream::NATIVESTREAM_ERROR_OPEN) {
                err = ERR_FAILED_OPEN;
            } else if (streamErr == NativeBaseStream::NATIVESTREAM_ERROR_READ) {
                err = ERR_READING;
            } else {
                err = ERR_IO;
            }

            this->source->sendEvent(SOURCE_EVENT_ERROR, err, false);

            return;
        }
        case NATIVESTREAM_PROGRESS: {
            NativeAVSourceEvent *ev = this->source->createEvent(SOURCE_EVENT_BUFFERING, false);
            ev->args[0].set(msg.args[0].toInt64());
            ev->args[1].set(msg.args[1].toInt64());
            ev->args[2].set(msg.args[2].toInt64());
            this->source->sendEvent(ev);
            return;
        }
        case NATIVESTREAM_READ_BUFFER:
            return;
        case MSG_SEEK:
            this->stream->seek(this->streamSeekPos);
            break;
        case MSG_READ:
            this->streamBuffer = this->stream->getNextPacket(&this->streamPacketSize, &this->streamErr);
            m_HaveDataAvailable = false;
            break;
        case MSG_STOP:
            delete this->stream;
            break;
        default:
            return;
    }

    NATIVE_PTHREAD_SIGNAL(&this->m_ThreadCond);
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
    this->error = 0;
    m_HaveDataAvailable = true;
    SPAM(("onAvailableData=%d/%d\n", len, this->opened));

    if (this->pending) {
        this->needWakup = true;
        this->readCallback(this->callbackPrivate);
        return;
    }

    if (!this->opened) {
        this->streamSize = this->stream->getFileSize();
        this->opened = true;
        this->source->openInit();
    }
}

void NativeAVStreamReader::finish()
{
    this->streamBuffer = NULL;
    this->streamErr = AVERROR_EXIT;

    // Clean pending messages 
    // (we can have a MSG_READ/MSG_SEEK event if we were waiting for stream data/seek)
    this->delMessages();
    
    NATIVE_PTHREAD_SIGNAL(&this->m_ThreadCond);
}

NativeAVStreamReader::~NativeAVStreamReader() 
{
    if (NativeUtils::isMainThread()) {
        delete this->stream;
    } else {
        this->postMessage(this, NativeAVStreamReader::MSG_STOP);
        NATIVE_PTHREAD_WAIT(&this->m_ThreadCond);
    }
}

NativeAVSource::NativeAVSource()
    : eventCbk(NULL), eventCbkCustom(NULL),
      opened(false), eof(false), container(NULL), coro(NULL), mainCoro(NULL),
      seeking(false), doSeek(false),  error(0),
      m_SourceDoOpen(false)
{
}

void NativeAVSource::eventCallback(NativeAVSourceEventCallback cbk, void *custom) 
{
    this->eventCbk = cbk;
    this->eventCbkCustom = custom;
}

NativeAVSourceEvent *NativeAVSource::createEvent(int ev, bool fromThread)
{
    return new NativeAVSourceEvent(this, ev, this->eventCbkCustom, fromThread);
}

void NativeAVSource::sendEvent(int type, int value, bool fromThread) 
{
    NativeAVSourceEvent *ev = this->createEvent(type, fromThread);
    ev->args[0].set(value);
    this->sendEvent(ev);
}

void NativeAVSource::sendEvent(NativeAVSourceEvent *ev) 
{
    if (this->eventCbk != NULL) {
        this->eventCbk(ev);
    }
}

AVDictionary *NativeAVSource::getMetadata() 
{
    if (!this->opened) {
        return NULL;
    }

    return this->container ? this->container->metadata : NULL;
}
int NativeAVSource::getBitrate()
{
    return this->container ? this->container->bit_rate : 0;
}

double NativeAVSource::getDuration() 
{
    if (!this->opened) {
        return 0;
    }

    return this->container->duration/AV_TIME_BASE;
}

int NativeAVSource::readError(int err)
{
    SPAM(("readError Got error %d/%d\n", err, AVERROR_EOF));
    if (err == AVERROR_EOF || (this->container->pb && this->container->pb->eof_reached)) {
        this->error = AVERROR_EOF;
        return AVERROR_EOF;
    } else if (err != AVERROR(EAGAIN)) {
        this->error = AVERROR(err);
        this->sendEvent(SOURCE_EVENT_ERROR, ERR_READING, true);
        return -1;
    }

    return 0;
}


void NativeAVSource::onMessage(const NativeSharedMessages::Message &msg)
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

