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

NativeBaseStream::NativeBaseStream(const char *location) :
    m_PacketsSize(0), m_NeedToSendUpdate(false)
{
    m_DataBuffer.back =         NULL;
    m_DataBuffer.front =        NULL;
    m_DataBuffer.alreadyRead =  true;
    m_DataBuffer.fresh =        true;
    m_DataBuffer.ended =        false;
}

NativeBaseStream::~NativeBaseStream()
{

}

void NativeBaseStream::start(size_t packets, size_t seek)
{
    m_PacketsSize = packets;
    m_NeedToSendUpdate = true;

    this->onStart(packets, seek);
}

const unsigned char *NativeBaseStream::getNextPacket(size_t *len, int *err)
{
    unsigned char *data;

    if (m_DataBuffer.back == NULL) {
        *len = 0;
        *err = STREAM_ERROR;
        return NULL;
    }

    if (!this->hasDataAvailable()) {
        /*
            We need to notify when data
            become available through "onAvailableData"
        */
        m_NeedToSendUpdate = !m_DataBuffer.ended;

        *len = 0;
        *err = (m_DataBuffer.ended && m_DataBuffer.alreadyRead ?
            STREAM_END : STREAM_EAGAIN);

        return NULL;
    }    

    return this->onGetNextPacket(len, err);
}
