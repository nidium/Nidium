/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef io_systemstream_h__
#define io_systemstream_h__

#include <string>

#include <IO/FileStream.h>

#include <SystemInterface.h>

using Nidium::Interface::SystemInterface;
using Nidium::IO::Stream;
using Nidium::IO::FileStream;

namespace Nidium {
namespace IO {

// {{{ SystemStream
class SystemStream : public FileStream
{
public:
    explicit SystemStream(const char *location) :
        FileStream(location)
    {
    }

    static Stream *CreateStream(const char *location) {
        return new SystemStream(location);
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
// }}}

// {{{ UserStream
class UserStream : public FileStream
{
public:
    explicit UserStream(const char *location) :
        FileStream(location)
    {
    }

    static Stream *CreateStream(const char *location) {
        return new UserStream(location);
    }

    static bool AllowLocalFileStream() {
        return true;
    }

    static bool AllowSyncStream() {
        return true;
    }

    static const char *GetBaseDir() {
        return SystemInterface::GetInstance()->getUserDirectory();
    }
};
// }}}

} // namespace IO
} // namespace Nidium

#endif

