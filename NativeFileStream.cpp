/*
    NativeJS Core Library
    Copyright (C) 2014 Anthony Catel <paraboul@gmail.com>

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

#include "NativeFileStream.h"
#include "NativeFile.h"
#include "NativeUtils.h"

NativeFileStream::NativeFileStream(const char *location) : 
    NativeBaseStream(location), m_File(location)
{
    m_File.setListener(this);
}

void NativeFileStream::onStart(size_t packets, size_t seek)
{
    if (m_DataBuffer.back == NULL) {
        m_DataBuffer.back = buffer_new(packets);
        m_DataBuffer.front = buffer_new(packets);
    }

    m_File.open("r");

    if (seek) {
        m_File.seek(seek);
    }

    m_File.read(packets);
}

const unsigned char *NativeFileStream::onGetNextPacket(size_t *len, int *err)
{
    unsigned char *data;

    if (m_DataBuffer.back == NULL) {
        *err = STREAM_ERROR;
        return NULL;
    }

    if (!this->hasDataAvailable()) {
        /*
            Notify the listener whenever data become available
        */
        m_NeedToSendUpdate = !m_DataBuffer.ended;
        *err = (m_DataBuffer.ended && m_DataBuffer.alreadyRead ?
            STREAM_END : STREAM_EAGAIN);
        return NULL;        
    }

    data = m_DataBuffer.back->data;
    *len = native_min(m_DataBuffer.back->used, m_PacketsSize);
    m_DataBuffer.alreadyRead = true;

    this->swapBuffer();

    m_File.read(m_PacketsSize);

    return data;
}

void NativeFileStream::stop()
{
    /*
        Do nothing
    */
}

void NativeFileStream::getContent()
{
    if (m_File.isOpen()) {
        return;
    }

    m_File.open("r");
    m_File.read(UINT64_MAX);
    m_File.close();
}

bool NativeFileStream::getContentSync(char **data, size_t *len)
{
    int err;
    ssize_t slen;

    if (!m_File.openSync("r", &err)) {
        return false;
    }

    if ((slen = m_File.readSync(m_File.getFileSize(), data, &err)) < 0) {
        *len = 0;
        return false;
    }

    *len = slen;

    return true;
}

size_t NativeFileStream::getFileSize() const
{
    return m_File.getFileSize();
}

void NativeFileStream::seek(size_t pos)
{
    m_File.seek(pos);
    m_File.read(m_PacketsSize);
    m_PendingSeek = true;
    m_DataBuffer.back->used = 0;
    m_NeedToSendUpdate = true;
}

void NativeFileStream::onMessage(const NativeSharedMessages::Message &msg)
{
    switch (msg.event()) {
        case NATIVEFILE_OPEN_SUCCESS:
            /* do nothing */
            break;
        case NATIVEFILE_OPEN_ERROR:
            this->error(NATIVESTREAM_ERROR_OPEN, msg.dataUInt());
            break;
        case NATIVEFILE_SEEK_ERROR:
            this->error(NATIVESTREAM_ERROR_SEEK, -1);
            /* fall through */
        case NATIVEFILE_SEEK_SUCCESS:
            m_PendingSeek = false;
            break;
        case NATIVEFILE_READ_ERROR:
            this->error(NATIVESTREAM_ERROR_READ, msg.dataUInt());
            break;
        case NATIVEFILE_READ_SUCCESS:
        {
            if (m_PendingSeek) {
                break;
            }
            /*
                the buffer is automatically detroyed by NativeFile
                after the return of this function
            */
            buffer *buf = (buffer *)msg.dataPtr();

            if (m_File.eof()) {
                m_DataBuffer.ended = true;
            }
            CREATE_MESSAGE(message, NATIVESTREAM_READ_BUFFER);
            message->args[0].set(buf);

            /*
                The underlying object is notified in a sync way 
                since it's on the same thread.
            */
            this->notify(message);

            m_DataBuffer.alreadyRead = false;

            if (m_DataBuffer.back != NULL) {
                m_DataBuffer.back->used = 0;
                if (buf->data != NULL) {
                    /*
                        Feed the backbuffer with the new data (copy)
                    */
                    buffer_append_data(m_DataBuffer.back, buf->data, buf->used);

                    if (m_NeedToSendUpdate) {
                        m_NeedToSendUpdate = false;
                        CREATE_MESSAGE(message_available,
                            NATIVESTREAM_AVAILABLE_DATA);
                        message_available->args[0].set(buf->used);
                        this->notify(message_available);
                    }
                }
            }
            break;
        }
    }
}
