/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#ifndef io_embedstream_h__
#define io_embedstream_h__

#include <string>

#include <IO/FileStream.h>
#include <IO/NFSStream.h>

#include <SystemInterface.h>

#ifndef NIDIUM_EMBED_PRIVATE

namespace Nidium {
namespace IO {

class EmbedStream : public IO::FileStream
{
public:
    explicit EmbedStream(const char *location) :
        IO::FileStream(location)
    {
    }

    static IO::Stream *CreateStream(const char *location) {
        return new EmbedStream(location);
    }

    static bool AllowLocalFileStream() {
        return true;
    }

    static bool AllowSyncStream() {
        return true;
    }

    static const char *GetBaseDir() {
        return Interface::SystemInterface::GetInstance()->getEmbedDirectory();
    }
};

#else

class EmbedStream : public NFSStream
{
public:
    explicit EmbedStream(const char *location) :
#if 0
        NFSStream((std::string("/embed") + location).c_str())
#else
        NFSStream(location)
#endif
    {
    }

    static IO::Stream *CreateStream(const char *location) {
        return new EmbedStream(location);
    }

    static bool AllowLocalFileStream() {
        return true;
    }

    static bool AllowSyncStream() {
        return true;
    }

    static const char *GetBaseDir() {
        return "/";
    }
};

#endif

#endif

} // namespace IO
} // namespace Nidium

