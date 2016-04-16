/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativefilestream_h__
#define nativefilestream_h__

#include "Core/NativeMessages.h"

#include "Stream.h"
#include "NativeFile.h"

class NativeFileStream : public Nidium::IO::Stream,
                         public NativeMessages
{
public:
    explicit NativeFileStream(const char *location);

    static Nidium::IO::Stream *createStream(const char *location) {
        return new NativeFileStream(location);
    }
    static const char *getBaseDir() {
        return NativePath::getRoot();
    }

    static bool allowLocalFileStream() {
        return true;
    }
    static bool allowSyncStream() {
        return true;
    }

    virtual ~NativeFileStream() {};

    virtual void stop();
    virtual void getContent();

    /*
        In case mmap is false, caller is responsible for freeing the data.
        In case of a mmap, the file is unmmaped in dtor.
    */
    virtual bool getContentSync(char **data, size_t *len, bool mmap = false);
    virtual size_t getFileSize() const;
    virtual void seek(size_t pos);

    virtual void onMessage(const Nidium::Core::SharedMessages::Message &msg);
protected:
    virtual const unsigned char *onGetNextPacket(size_t *len, int *err);
    virtual void onStart(size_t packets, size_t seek);
private:
    NativeFile m_File;
};

#endif

