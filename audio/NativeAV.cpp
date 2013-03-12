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


NativeAVFileReader::NativeAVFileReader(const char *src, NativeAVSource *source, ape_global *net) 
: source(source), totalRead(0), error(0) 
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

    thiz->pending = false;
    thiz->error = 0;
 
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
            pos = (thiz->nfio->filesize - thiz->totalRead) + offset;
        case SEEK_END:
            pos = thiz->nfio->filesize - offset;
        default:
            return -1;
    }

    if( pos < 0 || pos > thiz->nfio->filesize) {
        return -1;
    }

    thiz->nfio->seek(pos);
    thiz->totalRead = pos;

    return pos;
}


void NativeAVFileReader::onNFIOError(NativeFileIO * io, int err)
{
    if (this->totalRead >= this->nfio->filesize) {
        this->error = AVERROR_EOF;
    } else {
        this->error = AVERROR(err);
    }

    Coro_switchTo_(this->source->mainCoro, this->source->coro);
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
        exit(1);
    }

    memcpy(this->buffer, data, len);
    Coro_switchTo_(this->source->mainCoro, this->source->coro);
};

void NativeAVFileReader::onNFIOWrite(NativeFileIO *, size_t written) 
{
}

NativeAVFileReader::~NativeAVFileReader() 
{
}

NativeAVSource::NativeAVSource()
    : eventCbk(NULL), eventCbkCustom(NULL),
      opened(false), container(NULL), doSeek(false)
{
    this->mainCoro = Coro_new();
    Coro_initializeMainCoro(this->mainCoro);

    /* XXX : Not sure if i should free main coro inside destructor
     *  libcoroutine docs : 
     *       "Note that you can't free the currently 
     *        running coroutine as this would free 
     *        the current C stack."
     */
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

