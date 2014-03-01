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

#ifndef nativestreaminterface_h__
#define nativestreaminterface_h__

#include <stdint.h>
#include <stdlib.h>

#include <ape_buffer.h>

class NativeBaseStream
{
public:
    NativeBaseStream(const char *location);
    virtual ~NativeBaseStream();

    enum StreamDataStatus {
        STREAM_EAGAIN = -1,
        STREAM_ERROR = -2,
        STREAM_END = -3
    };
    /*
        Start streaming the file
        @param packets  How much bytes should be filled in
                        the buffer before getting readable (unless eof reached)
    */
    void start(size_t packets = 4096, size_t seek=0);

    /*
        Get a packet from the stream
    */
    const unsigned char *getNextPacket(size_t *len, int *err);

    /*
        Stop streaming
    */
    virtual void stop()=0;

    /*
        Get file as a whole
    */
    virtual void getContent()=0;

    /*
        Get file size
    */
    virtual size_t getFileSize() const=0;

    /*
        Set file position
    */
    virtual void seek(size_t pos)=0;

    /*
        Check whether there is data pending
    */
    virtual bool hasDataAvailable() const {
        return !m_DataBuffer.alreadyRead || (m_DataBuffer.ended && m_DataBuffer.back->used);
    }

protected:

    virtual const unsigned char *onGetNextPacket(size_t *len, int *err)=0;
    virtual void onStart(size_t packets, size_t seek)=0;

    char*   m_Location;
    size_t  m_PacketsSize;
    bool    m_NeedToSendUpdate;

    struct {
        buffer *back, *front;
        bool alreadyRead;
        bool fresh;
        bool ended;
    } m_DataBuffer;
};

#endif
