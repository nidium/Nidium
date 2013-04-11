#include <stdlib.h>
#include <string.h>
#include "NativeStream.h"
#include "NativeHTTP.h"
#include "NativeFileIO.h"
#include <ape_base64.h>
#include "NativeUtils.h"

#include <fcntl.h>
#include <sys/mman.h>

static struct _native_stream_interfaces {
    const char *str;
    NativeStream::StreamInterfaces interface_type;
} native_stream_interfaces[] = {
    {"http://",      NativeStream::INTERFACE_HTTP},
    {"file://",      NativeStream::INTERFACE_FILE},
    {"private://",   NativeStream::INTERFACE_PRIVATE},
    {"data:",        NativeStream::INTERFACE_DATA},
    {NULL,           NativeStream::INTERFACE_UNKNOWN}
};

NativeStream::NativeStream(ape_global *net, const char *location) :
    packets(0), needToSendUpdate(false)
{
    this->interface = NULL;
    this->location  = strdup(location);
    this->net = net;
    this->IInterface = INTERFACE_UNKNOWN;
    this->delegate  = NULL;

    dataBuffer.current = NULL;
    dataBuffer.next = NULL;
    dataBuffer.alreadyRead = false;

    mapped.addr = NULL;
    mapped.fd   = 0;

    this->getInterface();
}

NativeIStreamer *NativeStream::getInterface()
{
    if (interface) {
        return interface;
    }

    for (int i = 0; native_stream_interfaces[i].str != NULL; i++) {
        int len = strlen(native_stream_interfaces[i].str);
        if (strncasecmp(native_stream_interfaces[i].str, (const char *)location,
                        len) == 0) {
            
            setInterface(native_stream_interfaces[i].interface_type, len);

            return interface;
        }
    }

    /* Default : use file */
    setInterface(INTERFACE_FILE, 0);

    return interface;
}

void NativeStream::setInterface(StreamInterfaces interface, int path_offset)
{
    this->IInterface = interface;

    switch(interface) {
        case INTERFACE_HTTP:
            this->interface = new NativeHTTP(
                new NativeHTTPRequest(this->location), this->net);
            break;
        case INTERFACE_PRIVATE:
        {
#define FRAMEWORK_LOCATION "./falcon/"
            char *flocation = (char *)malloc(sizeof(char) *
                    (strlen(&this->location[path_offset]) + sizeof(FRAMEWORK_LOCATION) + 1));
            sprintf(flocation, FRAMEWORK_LOCATION "%s", &this->location[path_offset]);
            this->interface = new NativeFileIO(&flocation[path_offset],
                                this, this->net);
            break;
#undef FRAMEWORK_LOCATION
        }
        case INTERFACE_FILE:
            this->interface = new NativeFileIO(&this->location[path_offset],
                                this, this->net);
            break;
        default:
            break;
    }
}

static int NativeStream_data_getContent(void *arg)
{
    NativeStream *stream = (NativeStream *)arg;
    const char *data = strchr(stream->location, ',');
    if (data != NULL && data[1] != '\0') {
        int len = strlen(&data[1]);
        unsigned char *out = (unsigned char *)malloc(sizeof(char) * (len + 1));
        int res = base64_decode(out, &data[1], len + 1);
        if (res > 0 && stream->delegate) {
            stream->delegate->onGetContent((const char *)out, res);
        }
        free(out);
    }
    return 0;
}

void NativeStream::start(size_t packets)
{
    this->packets = packets;
    if (dataBuffer.current == NULL) {
        dataBuffer.current = buffer_new(packets);
        dataBuffer.next    = buffer_new(packets);
        printf("Creating buffers...\n");
    }

    needToSendUpdate = true;

    switch(IInterface) {
        case INTERFACE_FILE:
        {
            printf("Opening file...\n");
            NativeFileIO *file = static_cast<NativeFileIO *>(this->interface);
            file->open("r");
            break;
        }
        case INTERFACE_HTTP:
        {
            mapped.fd = open("/tmp/nativesstream.data",
                O_RDWR | O_CREAT | O_TRUNC, S_IRUSR| S_IWUSR);
            if (mapped.fd == 0) {
                return;
            }
            mapped.addr = mmap(NULL, packets,
                PROT_READ|PROT_WRITE, MAP_SHARED, mapped.fd, 0);
            
            NativeHTTP *http = static_cast<NativeHTTP *>(this->interface);
            http->request(this);            
            break;
        }
        default:
            break;
    }
}

/* Sync read in memory the current packet and start grabbing the next one
   data remains readable until the next getNextPacket().
*/
const unsigned char *NativeStream::getNextPacket(size_t *len, int *err)
{
    needToSendUpdate = true;

    if (dataBuffer.alreadyRead) {
        *len = 0;
        *err = STREAM_EAGAIN;
        return NULL;
    }
    *err = 0;
    *len = native_min(dataBuffer.current->used, this->getPacketSize());
    unsigned char *data = dataBuffer.current->data;
    
    buffer *tmp = dataBuffer.current;
    dataBuffer.current = dataBuffer.next;
    dataBuffer.next = tmp;
    dataBuffer.alreadyRead = true;

    switch(IInterface) {
        case INTERFACE_FILE:
        {
            NativeFileIO *file = static_cast<NativeFileIO *>(this->interface);
            file->read(this->getPacketSize());
            break;
        }
        default:
            break;
    }

    return data;
}

void NativeStream::getContent()
{
    ape_global *ape = this->net;

    switch(IInterface) {
        case INTERFACE_HTTP:
        {
            NativeHTTP *http = static_cast<NativeHTTP *>(this->interface);
            http->request(this);
            break;
        }
        case INTERFACE_FILE:
        case INTERFACE_PRIVATE:
        {
            NativeFileIO *file = static_cast<NativeFileIO *>(this->interface);
            file->open("r");
            break;
        }
        case INTERFACE_DATA:
        {
            /* async dispatch so that onload callback
            can be triggered even if declared after reading */
            timer_dispatch_async(NativeStream_data_getContent, this);
            break;
        }
        default:
            break;
    }
}

/* File methods */
void NativeStream::onNFIOOpen(NativeFileIO *NFIO)
{
    if (this->delegate) {
        size_t packetSize = this->getPacketSize();
        NFIO->read(packetSize == 0 ? NFIO->filesize : packetSize);
    }
}

void NativeStream::onNFIOError(NativeFileIO *NFIO, int errno)
{

}

void NativeStream::onNFIORead(NativeFileIO *NFIO, unsigned char *data, size_t len)
{
    if (this->delegate) {
        this->delegate->onGetContent((const char *)data, len);
        if (needToSendUpdate) {
            dataBuffer.alreadyRead = false;
            needToSendUpdate = false;
            buffer_append_data(dataBuffer.current, data, len);
            this->delegate->onAvailableData(len);
        }
    }
}

void NativeStream::onNFIOWrite(NativeFileIO *NFIO, size_t written)
{

}
/****************/

/* HTTP methods */
void NativeStream::onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType)
{
    if (this->delegate) {
        this->delegate->onGetContent((const char *)h->data->data,
            h->data->used);
    }
}

void NativeStream::onProgress(size_t offset, size_t len,
    NativeHTTP::HTTPData *h, NativeHTTP::DataType)
{
    if (!this->delegate) return;
}

void NativeStream::onError(NativeHTTP::HTTPError err)
{
    if (!this->delegate) return;

    this->delegate->onGetContent(NULL, 0);
}
/**********/

NativeStream::~NativeStream()
{
    free(location);

    buffer_destroy(dataBuffer.current);
    buffer_destroy(dataBuffer.next);

    if (this->interface) {
        delete this->interface;
    }
}
