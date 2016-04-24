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

#ifndef nml_privatestream_h__
#define nml_privatestream_h__

#include <string>

#include <IO/FileStream.h>
#include <IO/NFSStream.h>

#include <SystemInterface.h>

#ifndef NATIVE_EMBED_PRIVATE

namespace Nidium {
namespace NML {

class PrivateStream : public Nidium::IO::FileStream
{
public:
    explicit PrivateStream(const char *location) :
        Nidium::IO::FileStream(location)
    {
    }

    static Nidium::IO::Stream *CreateStream(const char *location) {
        return new PrivateStream(location);
    }

    static bool AllowLocalFileStream() {
        return true;
    }

    static bool AllowSyncStream() {
        return true;
    }

    static const char *GetBaseDir() {
        return Nidium::Interface::NativeSystemInterface::GetInstance()->getPrivateDirectory();
    }
};

#else

class PrivateStream : public NativeNFSStream
{
public:
    explicit PrivateStream(const char *location) :
#if 0
        NativeNFSStream((std::string("/private") + location).c_str())
#else
        NativeNFSStream(location)
#endif
    {
    }

    static Nidium::IO::Stream *CreateStream(const char *location) {
        return new PrivateStream(location);
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

} // namespace NML
} // namespace Nidium

