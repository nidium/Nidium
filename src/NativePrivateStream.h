/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativeprivatestream_h__
#define nativeprivatestream_h__

#include <string>

#include <NativeFileStream.h>
#include <NativeNFSStream.h>

#include "NativeSystemInterface.h"

#ifndef NATIVE_EMBED_PRIVATE

class NativePrivateStream : public NativeFileStream
{
public:
    explicit NativePrivateStream(const char *location) :
        NativeFileStream(location)
    {
    }

    static NativeBaseStream *createStream(const char *location) {
        return new NativePrivateStream(location);
    }

    static bool allowLocalFileStream() {
        return true;
    }

    static bool allowSyncStream() {
        return true;
    }

    static const char *getBaseDir() {
        return NativeSystemInterface::getInstance()->getPrivateDirectory();
    }
};

#else

class NativePrivateStream : public NativeNFSStream
{
public:
    explicit NativePrivateStream(const char *location) :
#if 0
        NativeNFSStream((std::string("/private") + location).c_str())
#else
        NativeNFSStream(location)
#endif
    {
    }

    static NativeBaseStream *createStream(const char *location) {
        return new NativePrivateStream(location);
    }

    static bool allowLocalFileStream() {
        return true;
    }

    static bool allowSyncStream() {
        return true;
    }

    static const char *getBaseDir() {
        return "/";
    }
};

#endif

#endif

