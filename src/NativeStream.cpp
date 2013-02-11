#include "NativeStream.h"
#include "NativeHTTP.h"
#include "NativeFileIO.h"
#include <stdlib.h>
#include <string.h>

static struct _native_stream_interfaces {
    const char *str;
    NativeStream::StreamInterfaces interface_type;
} native_stream_interfaces[] = {
    {"http://",      NativeStream::INTERFACE_HTTP},
    {"file://",      NativeStream::INTERFACE_FILE},
    {NULL,           NativeStream::INTERFACE_UNKNOWN}
};

NativeStream::NativeStream(ape_global *net, const char *location)
{
    this->interface = NULL;
    this->location  = strdup(location);
    this->net = net;
    this->IInterface = INTERFACE_UNKNOWN;
    this->delegate  = NULL;

    this->getInterface();
}

NativeIStreamer *NativeStream::getInterface()
{
    if (interface) {
        return interface;
    }

    for (int i = 0; native_stream_interfaces[i].str != NULL; i++) {
        if (strncasecmp(native_stream_interfaces[i].str, (const char *)location,
            strlen(native_stream_interfaces[i].str)) == 0) {
            
            setInterface(native_stream_interfaces[i].interface_type);

            return interface;
        }
    }

    /* Default : use file */
    setInterface(INTERFACE_FILE);

    return interface;
}

void NativeStream::setInterface(StreamInterfaces interface)
{
    this->IInterface = interface;

    switch(interface) {
        case INTERFACE_HTTP:
            this->interface = new NativeHTTP(this->location, this->net);
            break;
        case INTERFACE_FILE:
            this->interface = new NativeFileIO(this->location, this, this->net);
            break;
        default:
            break;
    }
}

void NativeStream::getContent()
{
    switch(IInterface) {
        case INTERFACE_HTTP:
        {
            NativeHTTP *http = static_cast<NativeHTTP *>(this->interface);
            http->request(this);
            break;
        }
        case INTERFACE_FILE:
        {
            NativeFileIO *file = static_cast<NativeFileIO *>(this->interface);
            file->open("r");
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
        NFIO->read(NFIO->filesize);
    }
}

void NativeStream::onNFIOError(NativeFileIO *NFIO, int errno)
{

}

void NativeStream::onNFIORead(NativeFileIO *NFIO, unsigned char *data, size_t len)
{
    NFIO->close();
    if (this->delegate) {
        this->delegate->onGetContent((const char *)data, len);
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
}
/**********/

NativeStream::~NativeStream()
{
    free(location);
    if (this->interface) {
        delete this->interface;
    }
}
