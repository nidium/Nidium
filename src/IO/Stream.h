/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef io_stream_h__
#define io_stream_h__

#include <stdint.h>
#include <stdlib.h>

#include <ape_buffer.h>

#include "Core/NativePath.h"
#include "Core/SharedMessages.h"
#include "Core/Messages.h"

namespace Nidium {
namespace IO {

#define STREAM_MESSAGE_BITS(id) ((1 << 21) | id)

enum {
    STREAM_ERROR =            STREAM_MESSAGE_BITS(1),
    STREAM_READ_BUFFER =      STREAM_MESSAGE_BITS(2),
    STREAM_AVAILABLE_DATA =   STREAM_MESSAGE_BITS(3),
    STREAM_PROGRESS =         STREAM_MESSAGE_BITS(4)
};

class Stream
{
public:
    enum StreamErrors {
        STREAM_ERROR_OPEN,
        STREAM_ERROR_SEEK,
        STREAM_ERROR_READ,
        STREAM_ERROR_UNKNOWN,
    };

    virtual ~Stream();

    static Stream *create(const NativePath &path);
    static Stream *create(const char *location);

    enum StreamDataStatus {
        STREAM_EAGAIN = -1,
        STREAM_ERROR = -2,
        STREAM_END = -3
    };

    void setListener(Nidium::Core::Messages *listener) {
        m_Listener = listener;
    }

    const char *getLocation() const {
        return m_Location;
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

    virtual bool getContentSync(char **data, size_t *len, bool mmap = false) {
        return false;
    }

    /*
        Return the base path for the stream
    */
    virtual const char *getPath() const {
        return NULL;
    }

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
        return !m_PendingSeek && m_DataBuffer.back->used && (!m_DataBuffer.alreadyRead ||
        (m_DataBuffer.ended && m_DataBuffer.back->used));
    }

protected:
    explicit Stream(const char *location);

    virtual const unsigned char *onGetNextPacket(size_t *len, int *err)=0;
    virtual void onStart(size_t packets, size_t seek)=0;

    /*
        Send a message to the listener
    */
    void notify(Nidium::Core::SharedMessages::Message *msg);

    /*
        Send an error message
    */
    void error(StreamErrors, unsigned int code);

    /*
        Swap back and front buffer.
        so that the requested data (getNextPacket()) remains valid
        while data are still incoming (double buffering).

        TODO: move to Nidium::IO::FileStream?
    */
    void swapBuffer();

    char*   m_Location;
    size_t  m_PacketsSize;
    bool    m_NeedToSendUpdate;
    bool    m_PendingSeek;

    Nidium::Core::Messages *m_Listener;

    struct {
        buffer *back, *front;
        bool alreadyRead;
        bool fresh;
        bool ended;
    } m_DataBuffer;
};

} // namespace IO
} // namespace Nidium

#endif

