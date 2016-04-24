/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Stream.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "Core/Path.h"

using Nidium::Core::Path;

namespace Nidium {
namespace IO {

Stream::Stream(const char *location) :
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

Stream::~Stream()
{
    free(m_Location);
}

Stream *Stream::Create(const Path &path)
{
    return path.CreateStream();
}

Stream *Stream::Create(const char *location)
{
    const char *pLocation;
    Path::schemeInfo *info;

    if (location == NULL ||
        (info = Path::GetScheme(location, &pLocation)) == NULL) {
        return NULL;
    }

    return info->base(pLocation);
}

void Stream::start(size_t packets, size_t seek)
{
    if (m_Location == NULL || packets < 1) {
        this->error(ERROR_OPEN, -1);
        return;
    }

    m_PacketsSize = packets;
    m_NeedToSendUpdate = true;
    this->onStart(packets, seek);
}

const unsigned char *Stream::getNextPacket(size_t *len, int *err)
{
    *len = 0;
    *err = 0;
    return this->onGetNextPacket(len, err);
}

void Stream::notify(Core::SharedMessages::Message *msg)
{
    if (m_Listener) {
        m_Listener->postMessage(msg);
    } else {
        delete msg;
    }
}

void Stream::error(Errors err, unsigned int code)
{
    CREATE_MESSAGE(message, EVENT_ERROR);
    message->args[0].set(err);
    message->args[1].set(code);

    this->notify(message);
}

void Stream::swapBuffer()
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

} // namespace IO
} // namespace Nidium

