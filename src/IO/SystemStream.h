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

namespace Nidium {
namespace IO {

// {{{ SystemStream
class SystemStream : public IO::FileStream
{
public:
    explicit SystemStream(const char *location) :
        IO::FileStream(location)
    {
    }

    static IO::Stream *CreateStream(const char *location) {
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
class UserStream : public IO::FileStream
{
public:
    explicit UserStream(const char *location) :
        IO::FileStream(location)
    {
    }

    static IO::Stream *CreateStream(const char *location) {
        return new UserStream(location);
    }

    static bool AllowLocalFileStream() {
        return true;
    }

    static bool AllowSyncStream() {
        return true;
    }

    static const char *GetBaseDir() {
        return Interface::SystemInterface::GetInstance()->getUserDirectory();
    }
};
// }}}

} // namespace IO
} // namespace Nidium

#endif

