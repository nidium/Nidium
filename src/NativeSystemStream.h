/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativesystemstream_h__
#define nativesystemstream_h__

#include <string>

#include <NativeFileStream.h>
#include <NativeSystemInterface.h>

class NativeSystemStream : public NativeFileStream
{
public:
    explicit NativeSystemStream(const char *location) :
        NativeFileStream(location)
    {
    }

    static NativeBaseStream *createStream(const char *location) {
        return new NativeSystemStream(location);
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

class NativeUserStream : public NativeFileStream
{
public:
    explicit NativeUserStream(const char *location) :
        NativeFileStream(location)
    {
    }

    static NativeBaseStream *createStream(const char *location) {
        return new NativeUserStream(location);
    }

    static bool allowLocalFileStream() {
        return true;
    }

    static bool allowSyncStream() {
        return true;
    }

    static const char *getBaseDir() {
        return NativeSystemInterface::getInstance()->getUserDirectory();
    }
};

#endif

