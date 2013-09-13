#include "NativeAV.h"
#include "NativeAudioNode.h"
#include "pa_ringbuffer.h"
#include "Coro.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

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
        case SEEK_END:
            pos = reader->bufferSize - offset;
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
NativeAVStreamReader::NativeAVStreamReader(const char *src, bool *readFlag, pthread_cond_t *bufferCond, NativeAVSource *source, ape_global *net) 
: source(source), readFlag(readFlag), opened(false), bufferCond(bufferCond), totalRead(0), streamRead(STREAM_BUFFER_SIZE), streamBuffer(NULL), error(0)
{
    this->async = true;
    this->stream = new NativeStream(net, src);
    this->stream->start(STREAM_BUFFER_SIZE);
    this->stream->setDelegate(this);
}

int NativeAVStreamReader::read(void *opaque, uint8_t *buffer, int size) 
{
    NativeAVStreamReader *thiz = static_cast<NativeAVStreamReader *>(opaque);

    int err;
    size_t streamSize;
    size_t copied = 0;
    int avail = (STREAM_BUFFER_SIZE - thiz->streamRead);

    // Have data inside buffer
    if (avail > 0) {
        int copy = avail > size ? size : avail;
        memcpy(buffer, thiz->streamBuffer + thiz->streamRead, copy);
        thiz->totalRead += copy;
        thiz->streamRead += copy;

        if (copy >= size) {
            return size;
        }

        copied += copy;
    }

    // No more data inside buffer, need to get more
    for(;;) {
        bool loopCond = true;
        thiz->streamBuffer = thiz->stream->getNextPacket(&streamSize, &err);
        if (!thiz->streamBuffer) {
            switch (err) {
                case NativeStream::STREAM_EOF:
                case NativeStream::STREAM_ERROR:
                    thiz->error = AVERROR_EOF;
                    if (copied >  0) {
                        return copied;
                    } else {
                        return thiz->error;
                    }
                break;
                case NativeStream::STREAM_EAGAIN:
                    // Got EAGAIN, switch back to main coro
                    // and wait for onDataAvailable callback
                    thiz->pending = true;
                    Coro_switchTo_(thiz->source->coro, thiz->source->mainCoro);
                break;
            }
        } else {
            size_t copy = size;
            if (streamSize < size) {
                copy = streamSize;
            }

            memcpy(buffer + copied, thiz->streamBuffer, copy);

            thiz->streamRead = copy;
            thiz->totalRead += copy;
            copied += copy;

            // Got enought data, return
            if (streamSize >= size) {
                thiz->error = 0;
                thiz->pending = false;
                thiz->needWakup = false;
                *thiz->readFlag = false;
             
                return size;
            } 
        }
    }

    /*
    // FIXME : Uncomment this code once getFileSize() is implemented in NativeStream
    if (this->totalRead > this->stream->getFileSize()) {
          printf("Oh shit, read after EOF\n");
          //exit(1);
    }
    */

    return 0; 
}

int64_t NativeAVStreamReader::seek(void *opaque, int64_t offset, int whence) 
{
#if 0
    NativeAVStreamReader *thiz = static_cast<NativeAVStreamReader *>(opaque);
    int64_t pos = 0;

    switch(whence)
    {
        case AVSEEK_SIZE:
            return thiz->stream->filesize;
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos = (thiz->totalRead) + offset;
        case SEEK_END:
            pos = thiz->stream->filesize - offset;
        default:
            return -1;
    }

    thiz->totalRead = pos;

    if( pos < 0 || pos > thiz->stream->filesize) {
        thiz->error = AVERROR_EOF;
        return AVERROR_EOF;
    }

    thiz->stream->seek(pos);

    return pos;
#endif
    return 0;
}

void NativeAVStreamReader::onGetContent(const char *data, size_t len) {}

void NativeAVStreamReader::onAvailableData(size_t len) 
{
    this->error = 0;

    if (!this->opened) {
        this->opened = true;
        this->source->openInit();
        return;
    }

    if (this->pending) {
        this->needWakup = true;
        *this->readFlag = true;
        pthread_cond_signal(this->bufferCond);
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

void NativeAVSource::sendEvent(int ev, int value, bool fromThread) 
{
    if (this->eventCbk != NULL) {
        NativeAVSourceEvent *event = new NativeAVSourceEvent(this, ev, value, this->eventCbkCustom, fromThread);
        this->eventCbk(event);
    }
}

AVDictionary *NativeAVSource::getMetadata() 
{
    if (!this->opened) {
        return NULL;
    }

    return this->container->metadata;
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
    printf("readError Got error %d/%d\n", err, AVERROR_EOF);
    if (err == AVERROR_EOF || (this->container->pb && this->container->pb->eof_reached)) {
        this->eof = true;
        this->error = AVERROR_EOF;
        //if (this->track != NULL) {
            //this->track->eof = true; 
            // FIXME : Need to find out why when setting EOF, 
            // track sometimes fail to play when seeking backward
        //}
        return AVERROR_EOF;
    } else if (err != AVERROR(EAGAIN)) {
        this->error = AVERROR(err);
        this->sendEvent(SOURCE_EVENT_ERROR, ERR_READING, true);
        return -1;
    }

    return 0;
}

