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
#include "NativeSharedMessages.h"

#define NATIVESTREAM_MESSAGE_BITS(id) ((1 << 21) | id)

enum {
    NATIVESTREAM_ERROR =            NATIVESTREAM_MESSAGE_BITS(1),
    NATIVESTREAM_READ_BUFFER =      NATIVESTREAM_MESSAGE_BITS(2),
    NATIVESTREAM_AVAILABLE_DATA =   NATIVESTREAM_MESSAGE_BITS(3),
    NATIVESTREAM_PROGRESS =         NATIVESTREAM_MESSAGE_BITS(4)
};

class NativeMessages;

class NativeBaseStream
{
public:
    enum StreamErrors {
        NATIVESTREAM_ERROR_OPEN,
        NATIVESTREAM_ERROR_SEEK,
        NATIVESTREAM_ERROR_READ,
        NATIVESTREAM_ERROR_UNKNOWN,
    };

    virtual ~NativeBaseStream();

    static NativeBaseStream *create(const char *location);

    enum StreamDataStatus {
        STREAM_EAGAIN = -1,
        STREAM_ERROR = -2,
        STREAM_END = -3
    };

    void setListener(NativeMessages *listener) {
        m_Listener = listener;
    }

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
        return !m_DataBuffer.alreadyRead ||
        (m_DataBuffer.ended && m_DataBuffer.back->used);
    }

protected:
    explicit NativeBaseStream(const char *location);
    
    virtual const unsigned char *onGetNextPacket(size_t *len, int *err)=0;
    virtual void onStart(size_t packets, size_t seek)=0;

    /*
        Send a message to the listener
    */
    void notify(NativeSharedMessages::Message *msg);

    /*
        Send an error message
    */
    void error(StreamErrors, unsigned int code);

    /*
        Swap back and front buffer.
        so that the requested data (getNextPacket()) remains valid
        while data are still incoming (double buffering).
    */
    void swapBuffer();

    char*   m_Location;
    size_t  m_PacketsSize;
    bool    m_NeedToSendUpdate;
    bool    m_PendingSeek;

    NativeMessages *m_Listener;

    struct {
        buffer *back, *front;
        bool alreadyRead;
        bool fresh;
        bool ended;
    } m_DataBuffer;
};

#endif
