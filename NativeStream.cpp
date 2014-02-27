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
#include "NativeFile.h"
#include <ape_base64.h>
#include "NativeUtils.h"

#ifndef NATIVE_NO_PRIVATE_DIR
#include "../interface/NativeSystemInterface.h"
#endif

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
    m_PacketsSize(0), m_NeedToSendUpdate(false), m_AutoClose(true),
    m_Buffered(0), m_BufferedPosition(0), m_FileSize(0), m_KnownSize(false), m_Pending(false)
{
    this->interface = NULL;

    int len = 0;
    StreamInterfaces streamInterface = NativeStream::typeInterface(location, &len);

    /*
        Already prefixed path given
    */
    if (prefix == NULL || streamInterface != INTERFACE_UNKNOWN) {
        m_Location  = strdup(location);
    } else {
    /*
        relative path given
    */
        char *tmp = NativeStream::resolvePath(location, STREAM_RESOLVE_FILE);

        m_Location = (char *)malloc(sizeof(char) *
            (strlen(tmp) + strlen(prefix) + 1));

        memcpy(m_Location, prefix, strlen(prefix)+1);
        strcat(m_Location, tmp);

        free(tmp);
    }

    this->net = net;
    this->IInterface = INTERFACE_UNKNOWN;
    this->delegate  = NULL;

    m_DataBuffer.back = NULL;
    m_DataBuffer.front = NULL;
    m_DataBuffer.alreadyRead = true;
    m_DataBuffer.fresh = true;
    m_DataBuffer.ended = false;

    m_Mapped.addr = NULL;
    m_Mapped.fd   = 0;
    m_Mapped.idx = 0;
    m_Mapped.size = 0;

    this->getInterface();

}

void NativeStream::clean()
{
    m_Buffered = 0;

    m_DataBuffer.alreadyRead = true;
    m_DataBuffer.fresh = true;
    m_DataBuffer.ended = false;

    if (m_Mapped.addr) {
        munmap(m_Mapped.addr, m_Mapped.size);
        free(m_DataBuffer.back);
        free(m_DataBuffer.front);
    } else if (m_DataBuffer.back) {
        buffer_destroy(m_DataBuffer.back);
        buffer_destroy(m_DataBuffer.front);
    }

    if (m_Mapped.fd) {
        close(m_Mapped.fd);
    }

    m_Mapped.addr = NULL;
    m_Mapped.fd   = 0;
    m_Mapped.idx = 0;
    m_Mapped.size = 0;

    m_DataBuffer.back = NULL;
    m_DataBuffer.front = NULL;
}

void NativeStream::setAutoClose(bool close) 
{ 
    m_AutoClose = close;

    if (interface != NULL &&
            (IInterface == INTERFACE_FILE ||
            IInterface == INTERFACE_PRIVATE)) {
        static_cast<NativeFile *>(interface)->setAutoClose(close);
    }
}

char *NativeStream::resolvePath(const char *url, StreamResolveMode mode)
{
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
                return strdup(&url[len]);
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
#ifndef NATIVE_NO_PRIVATE_DIR
        case INTERFACE_PRIVATE:
        {
            const char *privateLocation = 
                NativeSystemInterface::getInstance()->getPrivateDirectory();

            char *flocation = (char *)malloc(sizeof(char) *
                    (strlen(&url[len]) + strlen(privateLocation) + 1));

            sprintf(flocation, "%s%s", privateLocation, &url[len]);

            /* TODO : force interface type to avoid looping (private://private://) */
            char *ret = NativeStream::resolvePath(flocation, mode);
            free(flocation);
            
            return ret;
        }
#endif
        default:
            return strdup("./");
    }

    return strdup("./");
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

    if (interface == NativeStream::INTERFACE_UNKNOWN) {
        *len = 0;
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
        if (strncasecmp(native_stream_interfaces[i].str, (const char *)m_Location,
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
                new NativeHTTPRequest(m_Location), this->net);
            break;
        case INTERFACE_PRIVATE:
        {
            char *flocation = NativeStream::resolvePath(m_Location, STREAM_RESOLVE_FILE);

            NativeFile *file = new NativeFile(flocation);
            file->setListener(this);

            this->interface = file;
            file->setAutoClose(m_AutoClose);
            free(flocation);
            break;
        }
        case INTERFACE_FILE:
        {
            NativeFile *file = new NativeFile(&m_Location[path_offset]);
            file->setListener(this);

            this->interface = file;
            file->setAutoClose(m_AutoClose);

            break;
        }
        default:
            break;
    }
}

static int NativeStream_data_getContent(void *arg)
{
    NativeStream *stream = (NativeStream *)arg;
    const char *data = strchr(stream->getLocation(), ',');
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
    this->m_PacketsSize = packets;

    m_NeedToSendUpdate = true;

    switch(IInterface) {
        case INTERFACE_FILE:
        {
            if (m_DataBuffer.back == NULL) {
                m_DataBuffer.back = buffer_new(packets);
                m_DataBuffer.front    = buffer_new(packets);
            }
            NativeFile *file = static_cast<NativeFile *>(this->interface);
            file->open("r");
            if (seek != 0) {
                file->seek(seek);
            }
            break;
        }
        case INTERFACE_HTTP:
        {
            if (m_DataBuffer.back == NULL) {
                m_DataBuffer.back     = buffer_new(0);
                m_DataBuffer.front    = buffer_new(0);
            }
            /* TODO: Windows */
            char tmpfname[] = "/tmp/nidiumtmp.XXXXXXXX";
            m_Mapped.fd = mkstemp(tmpfname);
            if (m_Mapped.fd == -1) {
                return;
            }
            printf("Created temp file for streaming : %s\n", tmpfname);
            if (unlink(tmpfname) == -1) {
            }

            NativeHTTP *http = static_cast<NativeHTTP *>(this->interface);

            if (seek != 0) {
                NativeHTTPRequest *req = http->getRequest();
                char seekstr[64];
                sprintf(seekstr, "bytes=%zu-", seek);
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
    m_Mapped.idx = pos;

    m_NeedToSendUpdate = true;
    m_DataBuffer.ended = false;

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
        m_DataBuffer.alreadyRead = false;
        m_DataBuffer.fresh = false;

        m_DataBuffer.back->data = &((unsigned char *)m_Mapped.addr)[m_Mapped.idx];
        m_DataBuffer.back->size = m_FileSize - (m_BufferedPosition + m_Buffered);
        m_DataBuffer.back->used = 0;

        m_DataBuffer.front->used = 0;
        m_DataBuffer.front->size = 0;

        // If we have enought data buffered
        if (m_BufferedPosition + m_Buffered >= pos + this->getPacketSize() ) {
            m_NeedToSendUpdate = false;
            if (this->delegate) {
                this->delegate->onAvailableData(this->getPacketSize());
            }
        }

        return;
    }

    m_DataBuffer.alreadyRead = false;
    m_DataBuffer.fresh = true;

    m_DataBuffer.back->used = 0;
    m_DataBuffer.front->used = 0;

    switch (IInterface) {
        case INTERFACE_HTTP: {
            lseek(m_Mapped.fd, pos, SEEK_SET);

            NativeHTTP *http = static_cast<NativeHTTP *>(this->interface);
            NativeHTTPRequest *req = http->getRequest();

            http->stopRequest();

            char seekstr[64];
            sprintf(seekstr, "bytes=%zu-", pos);
            req->setHeader("Range", seekstr);
            http->request(this);
            break;
        }
        case INTERFACE_FILE:
        case INTERFACE_PRIVATE: {
            m_BufferedPosition = pos;
            m_Buffered = 0;

            NativeFile *file = static_cast<NativeFile *>(this->interface);
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

    if (m_DataBuffer.back == NULL) {
        *len = 0;
        *err = STREAM_ERROR;
        return NULL;
    }

    if (!this->hasDataAvailable()) {
        m_NeedToSendUpdate = !m_DataBuffer.ended;
        *len = 0;
        *err = (m_DataBuffer.ended && m_DataBuffer.alreadyRead ?
            STREAM_END : STREAM_EAGAIN);
        return NULL;
    }

    data = m_DataBuffer.back->data;

    *err = 0;
    *len = native_min(m_DataBuffer.back->used, this->getPacketSize());

    m_DataBuffer.alreadyRead = true;    
    this->swapBuffer();

    switch(IInterface) {
        case INTERFACE_FILE:
        {
            NativeFile *file = static_cast<NativeFile *>(this->interface);
            file->read(this->getPacketSize());
            break;
        }
        default:
            break;
    }
    m_Pending = false;

    return data;
}

void NativeStream::swapBuffer()
{
    buffer *tmp = m_DataBuffer.back;
    m_DataBuffer.back = m_DataBuffer.front;
    m_DataBuffer.front = tmp;
    m_DataBuffer.fresh = true;

    /*  If we have remaining data in our old backbuffer
        we place it in our new backbuffer.

        If after that, our new backbuffer contains enough data to be read,
        the user can read a next packet (alreadyRead == false)

        Further reading are queued to the new backbuffer
    */
    if (m_DataBuffer.front->used > this->getPacketSize()) {
        m_DataBuffer.back->data = &m_DataBuffer.front->data[this->getPacketSize()];
        m_DataBuffer.back->used = m_DataBuffer.front->used - this->getPacketSize();
        m_DataBuffer.back->size = m_DataBuffer.back->used;
        m_DataBuffer.fresh = false;

        if (m_DataBuffer.back->used >= this->getPacketSize()) {
            m_DataBuffer.alreadyRead = false;
        }
    } else if (m_DataBuffer.ended) {
        m_DataBuffer.back->used = 0;
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
            NativeFile *file = static_cast<NativeFile *>(this->interface);
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

void NativeStream::onMessage(const NativeSharedMessages::Message &msg)
{
    switch (msg.event()) {
        case NATIVEFILE_OPEN_SUCCESS:
            if (this->delegate) {
                NativeFile *file = static_cast<NativeFile *>(this->interface);
                size_t packetSize = this->getPacketSize();
                this->m_FileSize = file->getFileSize();
                this->m_KnownSize = true;

                file->read(packetSize == 0 ? file->getFileSize(): packetSize);
            }
            break;
        case NATIVEFILE_READ_SUCCESS:
        {
            NativeFile *file = static_cast<NativeFile *>(this->interface);
            buffer *buf = (buffer *)msg.dataPtr();
            m_Buffered += buf->used;

            /*if (NFIO->eof()) {
                m_DataBuffer.ended = true;
            }*/

            if (this->delegate) {
                this->delegate->onGetContent((const char *)buf->data, buf->used);

                m_DataBuffer.alreadyRead = false;
                if (m_DataBuffer.back != NULL) {
                    m_DataBuffer.back->used = 0;
                    if (buf->data != NULL) {
                        buffer_append_data(m_DataBuffer.back, buf->data, buf->used);
                    }
                }
                if (m_NeedToSendUpdate) {
                    m_NeedToSendUpdate = false;
                    this->delegate->onAvailableData(buf->used);
                }
            }
        }
        default:
            break;
    }
}

#if 0
/* File methods */

void NativeStream::onNFIOError(NativeFileIO *NFIO, int err)
{
    if (err == 0) {
        this->m_DataBuffer.ended = true;
        if (this->m_DataBuffer.alreadyRead) {
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


#endif
/****************/

/* HTTP methods */

/* On data end */
void NativeStream::onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType)
{
    this->m_DataBuffer.ended = true;

    if (this->delegate) {
        this->delegate->onGetContent((const char *)h->data->data,
            h->data->used);

        if (m_Mapped.addr) {
            
        }
        //printf("HTTP ended... with %d %ld\n", m_NeedToSendUpdate, m_DataBuffer.back->used);
        if (m_Pending && m_NeedToSendUpdate && m_DataBuffer.back->used) {
            m_NeedToSendUpdate = false;
            m_DataBuffer.alreadyRead = false;
            this->delegate->onAvailableData(m_DataBuffer.back->used);            
        } else if (!m_Pending && m_NeedToSendUpdate) {
            m_DataBuffer.alreadyRead = true;

            /*
                Set to 0 to satisfy !hasDataAvailable()
            */
            m_DataBuffer.back->used = 0;
            /*
                Notify delegate so that it has a chance to catch an "ended" stream
            */
            this->delegate->onAvailableData(0);

        }
    }
}

void NativeStream::onProgress(size_t offset, size_t len,
    NativeHTTP::HTTPData *h, NativeHTTP::DataType)
{
    if (!this->delegate) return;

    m_Buffered += len;
    m_Pending = true;

    if (m_Mapped.fd) {
        NativeHTTP *http = static_cast<NativeHTTP *>(this->interface);
        ssize_t written = 0;
retry:
        if ((written = write(m_Mapped.fd, &h->data->data[offset], len)) < 0) {
            if (errno == EINTR) {
                goto retry;
            }
            http->resetData();
            printf("NativeStream Error : Failed to write streamed content (%d)\n", errno);
            return;
        }

        /* Reset the data buffer, so that data doesnt grow in memory */
        http->resetData();

        if (m_DataBuffer.fresh) {
            m_DataBuffer.back->data = &((unsigned char *)m_Mapped.addr)[m_Mapped.idx];
            m_DataBuffer.back->size = 0;
            m_DataBuffer.fresh = false;
        }

        m_DataBuffer.back->size += written;
        m_DataBuffer.back->used = m_DataBuffer.back->size;

        m_Mapped.idx += written;

        //this->delegate->onProgress(m_Buffered, m_BufferedPosition, m_FileSize);
        this->delegate->onProgress(m_Buffered, m_FileSize);

        /*
            If our backbuffer contains enough data,
            the user is ready for a next packet
        */
        if (m_DataBuffer.back->used >= this->getPacketSize()) {
            m_DataBuffer.alreadyRead = false;
 
            if (m_NeedToSendUpdate) {
                m_NeedToSendUpdate = false;
                this->delegate->onAvailableData(this->getPacketSize());
            }
        }
        //printf("char : %c\n", ((char *)m_Mapped.addr)[32]);
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
    // If we already have a m_Mapped file, the onHeader callback
    // was called after a seek. Don't do anything
    if (this->m_Mapped.addr) return;

    NativeHTTP *http = static_cast<NativeHTTP *>(this->interface);
    this->m_KnownSize = true;
    this->m_FileSize = http->getFileSize();

    if (this->m_Mapped.fd && http->http.contentlength) {
        this->m_Mapped.size = http->http.contentlength;
        m_Mapped.addr = mmap(NULL, this->m_Mapped.size,
            PROT_READ, MAP_SHARED, m_Mapped.fd, 0);
    }
}

/**********/

NativeStream::~NativeStream()
{
    free(m_Location);

    this->clean();

    if (this->interface) {
        delete this->interface;
    }
}
