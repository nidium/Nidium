/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "AV.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

extern "C" {
#include <libavformat/avformat.h>
}

#include "AudioNode.h"

using Nidium::Core::Utils;
using Nidium::Core::Path;
using Nidium::Core::SharedMessages;
using Nidium::IO::Stream;

namespace Nidium {
namespace AV {

pthread_mutex_t AVSource::m_FfmpegLock = PTHREAD_MUTEX_INITIALIZER;

// {{{ AVBufferReader
AVBufferReader::AVBufferReader(uint8_t *buffer, unsigned long bufferSize)
    : m_Buffer(buffer), m_BufferSize(bufferSize), m_Pos(0)
{
}

int AVBufferReader::read(void *opaque, uint8_t *buffer, int size)
{
    AVBufferReader *reader = static_cast<AVBufferReader *>(opaque);

    if (reader->m_Pos + size > reader->m_BufferSize) {
        size = reader->m_BufferSize - reader->m_Pos;
    }

    if (size > 0) {
        memcpy(buffer, reader->m_Buffer + reader->m_Pos, size);
        reader->m_Pos += size;
    }

    return size;
}

int64_t AVBufferReader::seek(void *opaque, int64_t offset, int whence)
{
    AVBufferReader *reader = static_cast<AVBufferReader *>(opaque);
    int64_t pos = 0;
    switch (whence) {
        case AVSEEK_SIZE:
            return reader->m_BufferSize;
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos = reader->m_Pos + offset;
            break;
        case SEEK_END:
            pos = reader->m_BufferSize - offset;
            break;
        default:
            return -1;
    }

    if (pos < 0 || pos > reader->m_BufferSize) {
        return -1;
    }

    reader->m_Pos = pos;

    return pos;
}
// }}}

// {{{ AVSource
AVSource::AVSource()
    : m_Opened(false), m_Eof(false), m_Container(NULL),
      m_DoSemek(false), m_DoSeekTime(0.0f),
      m_SeekFlags(0), m_Error(0), m_SourceDoOpen(false)
{
}

AVDictionary *AVSource::getMetadata()
{
    if (!m_Opened) {
        return NULL;
    }

    return m_Container ? m_Container->metadata : NULL;
}

AVFormatContext *AVSource::getAVFormatContext()
{
    if (!m_Opened) {
        return NULL;
    }
    return m_Container;
}

int AVSource::getBitrate()
{
    return m_Container ? m_Container->bit_rate : 0;
}

double AVSource::getDuration()
{
    if (!m_Opened) {
        return 0;
    }

    if (m_Container) {
        return m_Container->duration / AV_TIME_BASE;
    }

    return 0;
}

int AVSource::readError(int err)
{
    SPAM(("readError Got error %d/%d\n", err, AVERROR_EOF));
    if (err == AVERROR_EOF
        || (m_Container->pb && m_Container->pb->eof_reached)) {
        m_Error = AVERROR_EOF;
        return AVERROR_EOF;
    } else if (err != AVERROR(EAGAIN)) {
        m_Error = AVERROR(err);
        this->sendEvent(SOURCE_EVENT_ERROR, ERR_READING);
        return -1;
    }

    return 0;
}


void AVSource::onMessage(const SharedMessages::Message &msg)
{
    switch (msg.event()) {
        case MSG_CLOSE:
            this->close();
            break;
    }
}
// }}}

} // namespace AV
} // namespace Nidium
