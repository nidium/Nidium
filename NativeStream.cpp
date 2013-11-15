/*
    NativeJS Core Library
    Copyright (C) 2013 Anthony Catel <paraboul@gmail.com>
    Copyright (C) 2013 Nicolas Trani <n.trani@weelya.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "NativeStream.h"
#include "NativeHTTP.h"
#include "NativeFileIO.h"
#include <ape_base64.h>
#include "NativeUtils.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

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

NativeStream::NativeStream(ape_global *net,
    const char *location, const char *prefix) :
    packets(0), needToSendUpdate(false), m_AutoClose(true),
    m_Buffered(0), m_BufferedPosition(0), m_FileSize(0), m_KnownSize(false)
{
    this->interface = NULL;
    int len = 0;
    if (prefix == NULL ||
        NativeStream::typeInterface(location, &len) != INTERFACE_UNKNOWN) {

        this->location  = strdup(location);
    } else {
        this->location = (char *)malloc(sizeof(char) *
            (strlen(location) + strlen(prefix) + 1));

        memcpy(this->location, prefix, strlen(prefix)+1);
        strcat(this->location, location);
    }

    this->net = net;
    this->IInterface = INTERFACE_UNKNOWN;
    this->delegate  = NULL;

    dataBuffer.back = NULL;
    dataBuffer.front = NULL;
    dataBuffer.alreadyRead = true;
    dataBuffer.fresh = true;
    dataBuffer.ended = false;

    mapped.addr = NULL;
    mapped.fd   = 0;
    mapped.idx = 0;
    mapped.size = 0;

    this->getInterface();

}

void NativeStream::clean()
{
    m_Buffered = 0;

    dataBuffer.alreadyRead = true;
    dataBuffer.fresh = true;
    dataBuffer.ended = false;

    if (mapped.addr) {
        munmap(mapped.addr, mapped.size);
        free(dataBuffer.back);
        free(dataBuffer.front);
    } else if (dataBuffer.back) {
        buffer_destroy(dataBuffer.back);
        buffer_destroy(dataBuffer.front);
    }

    if (mapped.fd) {
        close(mapped.fd);
    }

    mapped.addr = NULL;
    mapped.fd   = 0;
    mapped.idx = 0;
    mapped.size = 0;

    dataBuffer.back = NULL;
    dataBuffer.front = NULL;
}

void NativeStream::setAutoClose(bool close) 
{ 
    m_AutoClose = close;

    if (interface != NULL &&
            (IInterface == INTERFACE_FILE ||
            IInterface == INTERFACE_PRIVATE)) {
        static_cast<NativeFileIO *>(interface)->setAutoClose(close);
    }
}

char *NativeStream::resolvePath(const char *url, StreamResolveMode mode)
{
#define FRAMEWORK_LOCATION "./private/"
#define INDEX_FILE "index.nml"

    int len = 0;
    NativeStream::StreamInterfaces interface = NativeStream::typeInterface(url, &len);
    
    switch(interface) {
        case INTERFACE_HTTP:
        {
            const char *url_wo_http = &url[len];
            const char *end = strrchr(url_wo_http, '/');
            const char *start = strchr(url_wo_http, '/');
            size_t url_len = strlen(url);

            if (mode == STREAM_RESOLVE_PATH) {

                if (end == NULL) {
                    char *ret = (char *)malloc(sizeof(char) * (url_len + 2));
                    memcpy(ret, url, url_len+1);
                    strcat(ret, "/");

                    return ret;
                } else {
                    char *ret = (char *)malloc(sizeof(char) * (url_len + 1));

                    memcpy(ret, url, (end - url)+1);
                    ret[(end - url)+1] = '\0';

                    return ret;
                }
            } else if (mode == STREAM_RESOLVE_ROOT) {
                char *ret = (char *)malloc(sizeof(char) * (url_len + 2));
                if (start == NULL) {
                    memcpy(ret, url, url_len+1);
                    strcat(ret, "/");

                    return ret;
                } else {
                    char *ret = (char *)malloc(sizeof(char) * (url_len + 1));
                    memcpy(ret, url, (start - url)+1);
                    ret[(start - url)+1] = '\0';

                    return ret;
                }
            } else if (mode == STREAM_RESOLVE_FILE) {
                if (end == NULL) {
                    char *ret = (char *)malloc(sizeof(char) * (url_len + 2 + sizeof(INDEX_FILE)));
                    memcpy(ret, url, url_len+1);
                    strcat(ret, "/");
                    strcat(ret, INDEX_FILE);

                    return ret;
                } else if (url[url_len-1] == '/') {
                    char *ret = (char *)malloc(sizeof(char) * (url_len + 2 + sizeof(INDEX_FILE)));
                    memcpy(ret, url, url_len+1);
                    strcat(ret, INDEX_FILE);

                    return ret;
                } else {
                    return strdup(url);
                }
            }
        }
        case INTERFACE_FILE:
        case INTERFACE_UNKNOWN:
        {
            const char *url_wo_prefix = &url[len];
            const char *end = strrchr(url_wo_prefix, '/');
            //const char *start = strchr(url_wo_prefix, '/');
            size_t url_len = strlen(url);

            if (mode == STREAM_RESOLVE_ROOT) {
                if (url_wo_prefix[0] == '/') {
                    return strdup("/");
                } else {
                    return strdup("./");
                }
            } else if (mode == STREAM_RESOLVE_FILE) {
                return strdup(url);
            } else if (mode == STREAM_RESOLVE_PATH) {
                if (end == NULL) {
                    return strdup("./");
                } else {
                    char *ret = (char *)malloc(sizeof(char) * (url_len + 1));

                    memcpy(ret, url, (end - url)+1);
                    ret[(end - url)+1] = '\0';

                    return ret;                    
                }
            }
        }
        case INTERFACE_DATA:
            return strdup("./");
        case INTERFACE_PRIVATE:
        {
            char *flocation = (char *)malloc(sizeof(char) *
                    (strlen(&url[len]) + sizeof(FRAMEWORK_LOCATION) + 1));

            sprintf(flocation, FRAMEWORK_LOCATION "%s", &url[len]);

            /* TODO : force interface type to avoid looping (private://private://) */
            char *ret = NativeStream::resolvePath(flocation, mode);
            free(flocation);
            
            return ret;
        }
        default:
            return strdup("./");
    }

    return strdup("./");
#undef FRAMEWORK_LOCATION
#undef INDEX_FILE
}

NativeStream::StreamInterfaces NativeStream::typeInterface(const char *url, int *len)
{
    NativeStream::StreamInterfaces interface = NativeStream::INTERFACE_UNKNOWN;
    *len = 0;

    for (int i = 0; native_stream_interfaces[i].str != NULL; i++) {
        *len = strlen(native_stream_interfaces[i].str);
        if (strncasecmp(native_stream_interfaces[i].str, url,
                        *len) == 0) {
            interface = native_stream_interfaces[i].interface_type;

            break;
        }
    }

    return interface;

}

NativeIStreamer *NativeStream::getInterface(bool refresh)
{
    if (interface && !refresh) {
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
            char *flocation = NativeStream::resolvePath(this->location, STREAM_RESOLVE_FILE);
            NativeFileIO *nfio = new NativeFileIO(flocation, this, this->net);
            this->interface = nfio;
            nfio->setAutoClose(m_AutoClose);
            free(flocation);
            break;

        }
        case INTERFACE_FILE:
        {
            NativeFileIO *nfio = new NativeFileIO(&this->location[path_offset],
                                this, this->net);

            this->interface = nfio;
            nfio->setAutoClose(m_AutoClose);

            break;
        }
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

void NativeStream::start(size_t packets, size_t seek)
{
    this->packets = packets;

    needToSendUpdate = true;

    switch(IInterface) {
        case INTERFACE_FILE:
        {
            if (dataBuffer.back == NULL) {
                dataBuffer.back = buffer_new(packets);
                dataBuffer.front    = buffer_new(packets);
            }
            NativeFileIO *file = static_cast<NativeFileIO *>(this->interface);
            file->open("r");
            if (seek != 0) {
                file->seek(seek);
            }
            break;
        }
        case INTERFACE_HTTP:
        {
            if (dataBuffer.back == NULL) {
                dataBuffer.back     = buffer_new(0);
                dataBuffer.front    = buffer_new(0);
            }
            /* TODO: Windows */
            char tmpfname[] = "/tmp/nidiumtmp.XXXXXXXX";
            mapped.fd = mkstemp(tmpfname);
            if (mapped.fd == -1) {
                return;
            }
            printf("Created temp file for streaming : %s\n", tmpfname);
            if (unlink(tmpfname) == -1) {
            }

            NativeHTTP *http = static_cast<NativeHTTP *>(this->interface);

            if (seek != 0) {
                NativeHTTPRequest *req = http->getRequest();
                char seekstr[64];
                sprintf(seekstr, "bytes=%ld-", seek);
                req->setHeader("Range", seekstr);
            }
            http->request(this);
            break;
        }
        default:
            break;
    }
}

void NativeStream::stop()
{
    switch(IInterface) {
        case INTERFACE_HTTP:
        {
            NativeHTTP *http = static_cast<NativeHTTP *>(this->interface);
            http->stopRequest();

            break;
        }
        case INTERFACE_FILE:
        {
#if 0
            NativeFileIO *file = static_cast<NativeFileIO *>(this->interface);
#endif
            break;
        }
        default:
            break;
    }
}

void NativeStream::seek(size_t pos)
{
    mapped.idx = pos;

    needToSendUpdate = true;
    dataBuffer.ended = false;

    if (pos > m_BufferedPosition + m_Buffered) {
        // Seeking forward to whatever was buffered
        m_BufferedPosition = pos;
        m_Buffered = 0;
    } else if (pos < m_BufferedPosition) {
        // Seeking backward after a seek forward
        m_BufferedPosition = pos;
        m_Buffered = 0;
    } else if (IInterface == INTERFACE_HTTP) {
        // In memory seeking (only supported for INTERFACE_HTTP)
        dataBuffer.alreadyRead = false;
        dataBuffer.fresh = false;

        dataBuffer.back->data = &((unsigned char *)mapped.addr)[mapped.idx];
        dataBuffer.back->size = m_FileSize - (m_BufferedPosition + m_Buffered);
        dataBuffer.back->used = dataBuffer.back->size;

        dataBuffer.front->used = 0;
        dataBuffer.front->size = 0;

        // If we have enought data buffered
        if (m_BufferedPosition + m_Buffered >= pos + this->getPacketSize() ) {
            needToSendUpdate = false;
            if (this->delegate) {
                this->delegate->onAvailableData(this->getPacketSize());
            }
        }

        return;
    }

    dataBuffer.alreadyRead = false;
    dataBuffer.fresh = true;

    dataBuffer.back->used = 0;
    dataBuffer.front->used = 0;

    switch (IInterface) {
        case INTERFACE_HTTP: {
            lseek(mapped.fd, pos, SEEK_SET);

            NativeHTTP *http = static_cast<NativeHTTP *>(this->interface);
            NativeHTTPRequest *req = http->getRequest();

            http->stopRequest();

            char seekstr[64];
            sprintf(seekstr, "bytes=%ld-", pos);
            req->setHeader("Range", seekstr);
            http->request(this);
            break;
        }
        case INTERFACE_FILE:
        case INTERFACE_PRIVATE: {
            m_BufferedPosition = pos;
            m_Buffered = 0;

            NativeFileIO *file = static_cast<NativeFileIO *>(this->interface);
            file->seek(pos);
            file->read(this->getPacketSize());
            break;
        }
        case INTERFACE_DATA:
            // TODO
            break;
        default:
            break;
    }
}

/* Sync read in memory the current packet and start grabbing the next one
   data remains readable until the next getNextPacket().
*/
const unsigned char *NativeStream::getNextPacket(size_t *len, int *err)
{
    unsigned char *data;

    if (dataBuffer.back == NULL) {
        *len = 0;
        *err = STREAM_ERROR;
        return NULL;
    }

    if (!this->hasDataAvailable()) {
        needToSendUpdate = !dataBuffer.ended;
        *len = 0;
        *err = (dataBuffer.ended && dataBuffer.alreadyRead ?
            STREAM_END : STREAM_EAGAIN);
        return NULL;
    }

    data = dataBuffer.back->data;

    *err = 0;
    *len = native_min(dataBuffer.back->used, this->getPacketSize());

    dataBuffer.alreadyRead = true;    
    this->swapBuffer();

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

void NativeStream::swapBuffer()
{
    buffer *tmp = dataBuffer.back;
    dataBuffer.back = dataBuffer.front;
    dataBuffer.front = tmp;
    dataBuffer.fresh = true;

    /*  If we have remaining data in our old backbuffer
        we place it in our new backbuffer.

        If after that, our new backbuffer contains enough data to be read,
        the user can read a next packet (alreadyRead == false)

        Further reading are queued to the new backbuffer
    */
    if (dataBuffer.front->used > this->getPacketSize()) {
        dataBuffer.back->data = &dataBuffer.front->data[this->getPacketSize()];
        dataBuffer.back->used = dataBuffer.front->used - this->getPacketSize();
        dataBuffer.back->size = dataBuffer.back->used;
        dataBuffer.fresh = false;

        if (dataBuffer.back->used >= this->getPacketSize()) {
            dataBuffer.alreadyRead = false;
        }
    } else if (dataBuffer.ended) {
        dataBuffer.back->used = 0;
    } 
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
        this->m_FileSize = NFIO->filesize;
        this->m_KnownSize = true;

        NFIO->read(packetSize == 0 ? NFIO->filesize : packetSize);
    }
}

void NativeStream::onNFIOError(NativeFileIO *NFIO, int err)
{
    if (err == 0) {
        this->dataBuffer.ended = true;
        if (this->dataBuffer.alreadyRead) {
            // Trigger onAvailableData so caller can call getNextPacket()
            // and be aware that EOF occured
            this->delegate->onAvailableData(0);
        }
    } else {
        // XXX : Error are not always STREAM_ERROR_OPEN
        // we should forward the error to the callback
        this->delegate->onError(NativeStream::STREAM_ERROR_OPEN);
    }
}

void NativeStream::onNFIORead(NativeFileIO *NFIO, unsigned char *data, size_t len)
{
    m_Buffered += len;

    if (this->delegate) {
        this->delegate->onGetContent((const char *)data, len);

        dataBuffer.alreadyRead = false;
        if (dataBuffer.back != NULL) {
            dataBuffer.back->used = 0;
            buffer_append_data(dataBuffer.back, data, len);
        }
        if (needToSendUpdate) {
            needToSendUpdate = false;
            this->delegate->onAvailableData(len);
        }
    }
}

void NativeStream::onNFIOWrite(NativeFileIO *NFIO, size_t written)
{

}
/****************/

/* HTTP methods */

/* On data end */
void NativeStream::onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType)
{
    this->dataBuffer.ended = true;

    if (this->delegate) {
        this->delegate->onGetContent((const char *)h->data->data,
            h->data->used);

        if (mapped.addr) {
            
        }
        //printf("HTTP ended... with %d %ld\n", needToSendUpdate, dataBuffer.back->used);
        if (needToSendUpdate && dataBuffer.back->used) {
            needToSendUpdate = false;
            dataBuffer.alreadyRead = false;
            this->delegate->onAvailableData(dataBuffer.back->used);            
        }
    }
}

void NativeStream::onProgress(size_t offset, size_t len,
    NativeHTTP::HTTPData *h, NativeHTTP::DataType)
{
    if (!this->delegate) return;

    m_Buffered += len;

    if (mapped.fd) {
        NativeHTTP *http = static_cast<NativeHTTP *>(this->interface);
        ssize_t written = 0;
        retry:
        if ((written = write(mapped.fd, &h->data->data[offset], len)) < 0) {
            if (errno == EINTR) {
                goto retry;
            }
            http->resetData();
            printf("NativeStream Error : Failed to write streamed content (%d)\n", errno);
            return;
        }

        /* Reset the data buffer, so that data doesnt grow in memory */
        http->resetData();

        if (dataBuffer.fresh) {
            printf("Current buffer updated (%ld)\n", len);
            dataBuffer.back->data = &((unsigned char *)mapped.addr)[mapped.idx];
            dataBuffer.back->size = 0;
            dataBuffer.fresh = false;
        }

        dataBuffer.back->size += written;
        dataBuffer.back->used = dataBuffer.back->size;

        mapped.idx += written;

        //this->delegate->onProgress(m_Buffered, m_BufferedPosition, m_FileSize);
        this->delegate->onProgress(m_Buffered, m_FileSize);

        /*
            If our backbuffer contains enough data,
            the user is ready for a next packet
        */
        if (dataBuffer.back->used >= this->getPacketSize()) {
            dataBuffer.alreadyRead = false;
 
            if (needToSendUpdate) {
                needToSendUpdate = false;
                this->delegate->onAvailableData(this->getPacketSize());
            }
        }
        //printf("char : %c\n", ((char *)mapped.addr)[32]);
    }
}

void NativeStream::onError(NativeHTTP::HTTPError err)
{
    if (!this->delegate) return;

    printf("Got a stream error %d?\n", err);
    this->delegate->onError(NativeStream::STREAM_ERROR_OPEN);
    this->delegate->onGetContent(NULL, 0);
}

void NativeStream::onHeader()
{
    // If we already have a mapped file, the onHeader callback
    // was called after a seek. Don't do anything
    if (this->mapped.addr) return;

    NativeHTTP *http = static_cast<NativeHTTP *>(this->interface);
    this->m_KnownSize = true;
    this->m_FileSize = http->getFileSize();

    if (this->mapped.fd && http->http.contentlength) {
        this->mapped.size = http->http.contentlength;
        mapped.addr = mmap(NULL, this->mapped.size,
            PROT_READ, MAP_SHARED, mapped.fd, 0);
        printf("Got a header of size : %lld\n", http->http.contentlength);
        printf("File mapped at address : %p\n", mapped.addr);
    }
}

/**********/

NativeStream::~NativeStream()
{
    free(location);

    this->clean();

    if (this->interface) {
        delete this->interface;
    }
}
