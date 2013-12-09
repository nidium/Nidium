#include "NativeAV.h"
#include "NativeAudioNode.h"
#include "pa_ringbuffer.h"
#include "Coro.h"

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
NativeAVStreamReader::NativeAVStreamReader(const char *chroot, const char *src, bool *readFlag, 
        pthread_cond_t *bufferCond, NativeAVSource *source, ape_global *net) 
    : source(source), readFlag(readFlag), opened(false), bufferCond(bufferCond), totalRead(0), 
      streamRead(STREAM_BUFFER_SIZE), streamPacketSize(0), streamSize(0), streamBuffer(NULL), error(0)
{
    this->async = true;
    if (chroot != NULL) {
        this->stream = new NativeStream(net, src, chroot);
    } else {
        this->stream = new NativeStream(net, src);
    }
    this->stream->setAutoClose(false);
    this->stream->start(STREAM_BUFFER_SIZE);
    this->stream->setDelegate(this);
}

int NativeAVStreamReader::read(void *opaque, uint8_t *buffer, int size) 
{
    NativeAVStreamReader *thiz = static_cast<NativeAVStreamReader *>(opaque);

    int err;
    size_t copied = 0;
    int avail = (thiz->streamPacketSize - thiz->streamRead);

    // Have data inside buffer
    if (avail > 0) {
        int copy = avail > size ? size : avail;
        SPAM(("get streamBuffer=%p, totalRead=%lld, streamRead=%d, streamSize=%d, copy=%d, size=%d, avail=%d\n", thiz->streamBuffer, thiz->totalRead, thiz->streamRead, thiz->streamPacketSize, copy, size, avail));
        memcpy(buffer + copied, thiz->streamBuffer + thiz->streamRead, copy);
        thiz->totalRead += copy;
        thiz->streamRead += copy;

        if (copy >= size) {
            SPAM(("Returning %d\n", size));
            return size;
        }

        copied += copy;
    }

    SPAM(("streamSize=%lld\n", thiz->streamSize));
    // No more data inside buffer, need to get more
    for(;;) {
        bool loopCond = true;
        thiz->streamBuffer = thiz->stream->getNextPacket(&thiz->streamPacketSize, &err);
        SPAM(("store streamBuffer=%p / size=%d / err=%d\n", thiz->streamBuffer, thiz->streamPacketSize, err));
        if (!thiz->streamBuffer) {
            switch (err) {
                case NativeStream::STREAM_EOF:
                case NativeStream::STREAM_END:
                case NativeStream::STREAM_ERROR:
                    thiz->error = AVERROR_EOF;
                    SPAM(("Got EOF\n"));
                    thiz->pending = false;
                    thiz->needWakup = false;
                    return copied > 0 ? copied : thiz->error;
                break;
                case NativeStream::STREAM_EAGAIN:
                    SPAM(("Got eagain\n"));
                    // Got EAGAIN, switch back to main coro
                    // and wait for onDataAvailable callback
                    thiz->pending = true;
                    Coro_switchTo_(thiz->source->coro, thiz->source->mainCoro);
                break;
                default:
                    printf("received unknown error (%d) and streamBuffer is null. Returning EOF\n", err);
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
            if (copied == size || thiz->totalRead >= thiz->streamSize) {
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
    thiz->stream->seek(pos);

    return pos;
}

void NativeAVStreamReader::onProgress(size_t buffered, size_t len)
{
    this->source->onProgress(buffered, len);
}

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

void NativeAVStreamReader::onAvailableData(size_t len) 
{
    this->error = 0;
    SPAM(("onAvailableData=%d/%d\n", len, this->opened));

    if (this->pending) {
        this->needWakup = true;
        *this->readFlag = true;
        pthread_cond_signal(this->bufferCond);
        return;
    }

    if (!this->opened) {
        this->streamSize = this->stream->getFileSize();
        if (this->streamSize == 0) {
            this->source->sendEvent(SOURCE_EVENT_ERROR, ERR_STREAMING_NOT_SUPPORTED, 0, false);
            return;
        }
        this->opened = true;
        this->source->openInit();
    }
}

NativeAVStreamReader::~NativeAVStreamReader() 
{
    delete this->stream;
}

NativeAVSource::NativeAVSource()
    : eventCbk(NULL), eventCbkCustom(NULL),
      opened(false), eof(false), container(NULL), coro(NULL), mainCoro(NULL),
      seeking(false), doSeek(false),  error(0)
{
}

void NativeAVSource::eventCallback(NativeAVSourceEventCallback cbk, void *custom) 
{
    this->eventCbk = cbk;
    this->eventCbkCustom = custom;
}

void NativeAVSource::sendEvent(int ev, int value1, int value2, bool fromThread) 
{
    if (this->eventCbk != NULL) {
        NativeAVSourceEvent *event = new NativeAVSourceEvent(this, ev, value1, value2, this->eventCbkCustom, fromThread);
        this->eventCbk(event);
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
        this->sendEvent(SOURCE_EVENT_ERROR, ERR_READING, 0, true);
        return -1;
    }

    return 0;
}

NativeAVSource::~NativeAVSource() 
{
}

