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

#include "NativeStreamInterface.h"
#include "NativeMessages.h"

#include "NativeFileStream.h"
#include "NativeHTTPStream.h"
#include "NativePath.h"

#include <string.h>


NativeBaseStream::NativeBaseStream(const char *location) :
    m_PacketsSize(0), m_NeedToSendUpdate(false), m_PendingSeek(false),
    m_Listener(NULL)
{
    m_Location = strdup(location);
    
    m_DataBuffer.back =         NULL;
    m_DataBuffer.front =        NULL;
    m_DataBuffer.alreadyRead =  true;
    m_DataBuffer.fresh =        true;
    m_DataBuffer.ended =        false;
}

NativeBaseStream::~NativeBaseStream()
{
    free(m_Location);
}

NativeBaseStream *NativeBaseStream::create(const NativePath &path)
{
    return path.createStream();
}

NativeBaseStream *NativeBaseStream::create(const char *location)
{
    const char *pLocation;
    NativePath::schemeInfo *info;

    if (location == NULL ||
        (info = NativePath::getScheme(location, &pLocation)) == NULL) {
        return NULL;
    }

    return info->base(pLocation);
}

void NativeBaseStream::start(size_t packets, size_t seek)
{
    if (m_Location == NULL || packets < 1) {
        this->error(NATIVESTREAM_ERROR_OPEN, -1);
        return;
    }

    m_PacketsSize = packets;
    m_NeedToSendUpdate = true;

    this->onStart(packets, seek);
}

const unsigned char *NativeBaseStream::getNextPacket(size_t *len, int *err)
{
    *len = 0;
    *err = 0;
    return this->onGetNextPacket(len, err);
}

void NativeBaseStream::notify(NativeSharedMessages::Message *msg)
{
    if (m_Listener) {
        m_Listener->postMessage(msg);
    } else {
        delete msg;
    }
}

void NativeBaseStream::error(StreamErrors err, unsigned int code)
{
    CREATE_MESSAGE(message, NATIVESTREAM_ERROR);
    message->args[0].set(err);
    message->args[1].set(code);

    this->notify(message);    
}

void NativeBaseStream::swapBuffer()
{
    buffer *tmp =           m_DataBuffer.back;
    m_DataBuffer.back =     m_DataBuffer.front;
    m_DataBuffer.front =    tmp;
    m_DataBuffer.fresh =    true;

    /*  If we have remaining data in our old backbuffer
        we place it in our new backbuffer.

        If after that, our new backbuffer contains enough data to be read,
        the user can read a next packet (alreadyRead == false)

        Further reading are queued to the new backbuffer
    */
    if (m_DataBuffer.front->used > m_PacketsSize) {
        m_DataBuffer.back->data = &m_DataBuffer.front->data[m_PacketsSize];
        m_DataBuffer.back->used = m_DataBuffer.front->used - m_PacketsSize;
        m_DataBuffer.back->size = m_DataBuffer.back->used;
        m_DataBuffer.fresh = false;

        if (m_DataBuffer.back->used >= m_PacketsSize) {
            m_DataBuffer.alreadyRead = false;
        }
    } else if (m_DataBuffer.ended) {
        m_DataBuffer.back->used = 0;
    } 
}
