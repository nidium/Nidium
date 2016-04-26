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

#ifndef io_systemstream_h__
#define io_systemstream_h__

#include <string>

#include <IO/FileStream.h>

#include <SystemInterface.h>

namespace Nidium {
namespace IO {

// {{{ SystemStream
class SystemStream : public Nidium::IO::FileStream
{
public:
    explicit SystemStream(const char *location) :
        Nidium::IO::FileStream(location)
    {
    }

    static Nidium::IO::Stream *CreateStream(const char *location) {
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
class UserStream : public Nidium::IO::FileStream
{
public:
    explicit UserStream(const char *location) :
        Nidium::IO::FileStream(location)
    {
    }

    static Nidium::IO::Stream *CreateStream(const char *location) {
        return new UserStream(location);
    }

    static bool AllowLocalFileStream() {
        return true;
    }

    static bool AllowSyncStream() {
        return true;
    }

    static const char *GetBaseDir() {
        return Nidium::Interface::NativeSystemInterface::GetInstance()->getUserDirectory();
    }
};
// }}}

} // namespace IO
} // namespace Nidium

#endif

