#include <stdlib.h>
#include <string.h>
#include "NativeStream.h"
#include "NativeHTTP.h"
#include "NativeFileIO.h"
#include <ape_base64.h>

static struct _native_stream_interfaces {
    const char *str;
    NativeStream::StreamInterfaces interface_type;
} native_stream_interfaces[] = {
    {"http://",      NativeStream::INTERFACE_HTTP},
    {"file://",      NativeStream::INTERFACE_FILE},
    {"framework://", NativeStream::INTERFACE_FRAMEWORK},
    {"data:",        NativeStream::INTERFACE_DATA},
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
            this->interface = new NativeHTTP(this->location, this->net);
            break;
        case INTERFACE_FRAMEWORK:
        {
#define FRAMEWORK_LOCATION "./framework/"
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
        case INTERFACE_FRAMEWORK:
        {
            NativeFileIO *file = static_cast<NativeFileIO *>(this->interface);
            file->open("r");
            break;
        }
        case INTERFACE_DATA:
        {
            const char *data = strchr(this->location, ',');
            if (data != NULL && data[1] != '\0') {
                int len = strlen(&data[1]);
                unsigned char *out = (unsigned char *)malloc(sizeof(char) * (len + 1));
                int res = base64_decode(out, &data[1], len + 1);
                if (res > 0 && this->delegate) {
                    printf("Decoded : %d %d\n", res, len);
                    this->delegate->onGetContent((const char *)out, res);
                }
                free(out);
            }
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
