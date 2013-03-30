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


NativeAVFileReader::NativeAVFileReader(const char *src, pthread_cond_t *bufferCond, NativeAVSource *source, ape_global *net) 
: source(source), totalRead(0), bufferCond(bufferCond), error(0)
{
    this->async = true;
    this->nfio = new NativeFileIO(src, this, net);
    this->nfio->open("r");
}

int NativeAVFileReader::read(void *opaque, uint8_t *buffer, int size) 
{
    NativeAVFileReader *thiz = static_cast<NativeAVFileReader *>(opaque);
    
    thiz->pending = true;
    thiz->buffer = buffer;

    thiz->nfio->read(size);

    int ret = thiz->checkCoro();

    thiz->needWakup = false;
    thiz->error = 0;
    thiz->pending = false;
 
    return ret;
}

int NativeAVFileReader::checkCoro()
{
    Coro_switchTo_(this->source->coro, this->source->mainCoro);
    return this->error ? this->error : this->dataSize;
}

int64_t NativeAVFileReader::seek(void *opaque, int64_t offset, int whence) 
{
    NativeAVFileReader *thiz = static_cast<NativeAVFileReader *>(opaque);
    int64_t pos = 0;

    switch(whence)
    {
        case AVSEEK_SIZE:
            return thiz->nfio->filesize;
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos = (thiz->totalRead) + offset;
        case SEEK_END:
            pos = thiz->nfio->filesize - offset;
        default:
            return -1;
    }

    thiz->totalRead = pos;

    if( pos < 0 || pos > thiz->nfio->filesize) {
        thiz->error = AVERROR_EOF;
        return AVERROR_EOF;
    }

    thiz->nfio->seek(pos);

    return pos;
}


void NativeAVFileReader::onNFIOError(NativeFileIO * io, int err)
{
    if (this->totalRead >= this->nfio->filesize) {
        this->error = AVERROR_EOF;
    } else {
        this->error = AVERROR(err);
    }

    printf("NFIOERROR %d\n", err);

    this->needWakup = true;

    pthread_cond_signal(this->bufferCond);
}

void NativeAVFileReader::onNFIOOpen(NativeFileIO *) 
{
    this->source->openInit();
}

void NativeAVFileReader::onNFIORead(NativeFileIO *, unsigned char *data, size_t len) 
{
    this->dataSize = len;
    this->totalRead += len;
    if (this->totalRead > this->nfio->filesize) {
          printf("Oh shit, read after EOF\n");
//        exit(1);
    }

    memcpy(this->buffer, data, len);

    this->error = 0;
    this->needWakup = true;

    pthread_cond_signal(this->bufferCond);
}

void NativeAVFileReader::onNFIOWrite(NativeFileIO *, size_t written) 
{
}

NativeAVFileReader::~NativeAVFileReader() 
{
    this->nfio->close();
    delete this->nfio;
}

NativeAVSource::NativeAVSource()
    : eventCbk(NULL), eventCbkCustom(NULL),
      opened(false), container(NULL), coro(NULL), mainCoro(NULL),
      doSeek(false), seeking(false)
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

