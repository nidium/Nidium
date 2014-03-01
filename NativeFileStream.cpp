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
}

const unsigned char *NativeFileStream::onGetNextPacket(size_t *len, int *err)
{


}

void NativeFileStream::stop()
{

}

void NativeFileStream::getContent()
{

}

size_t NativeFileStream::getFileSize()
{

}

void NativeFileStream::seek(size_t pos)
{
    
}

void NativeFileStream::onMessage(const NativeSharedMessages::Message &msg)
{
    switch (msg.event()) {
        case NATIVEFILE_OPEN_SUCCESS:
            m_File.read(m_PacketsSize == 0 ? m_File.getFileSize() : m_PacketsSize);
            break;
        case NATIVEFILE_READ_SUCCESS:
        {
            buffer *buf = (buffer *)msg.dataPtr();

            break;
        }
    }
}
