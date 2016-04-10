/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativenfsstream_h__
#define nativenfsstream_h__


#include "Core/NativeMessages.h"

#include "NativeStreamInterface.h"

class NativeNFS;

class NativeNFSStream :  public NativeBaseStream
{
public:
    explicit NativeNFSStream(const char *location);

    static NativeBaseStream *createStream(const char *location) {
        return new NativeNFSStream(location);
    }
    static const char *getBaseDir() {
        return NULL;
    }

    static bool allowLocalFileStream() {
        return true;
    }
    static bool allowSyncStream() {
        return true;
    }

    virtual ~NativeNFSStream() {};

    virtual void stop();
    virtual void getContent();
    void _getContent();

    virtual bool getContentSync(char **data, size_t *len, bool mmap = false);
    virtual size_t getFileSize() const;
    virtual void seek(size_t pos);

protected:
    virtual const unsigned char *onGetNextPacket(size_t *len, int *err);
    virtual void onStart(size_t packets, size_t seek);
private:
    NativeNFS *m_NFS;

    struct {
        const unsigned char *data;
        size_t len;
        off_t pos;
    } m_File;
};

#endif

