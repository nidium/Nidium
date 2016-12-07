/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef io_nfsstream_h__
#define io_nfsstream_h__

#include "Core/Messages.h"

#include "IO/Stream.h"

namespace Nidium {
namespace IO {

class NFS;

class NFSStream : public Stream
{
public:
    explicit NFSStream(const char *location);

    static Stream *CreateStream(const char *location)
    {
        return new NFSStream(location);
    }
    static const char *GetBaseDir()
    {
        return nullptr;
    }

    static bool AllowLocalFileStream()
    {
        return true;
    }
    static bool AllowSyncStream()
    {
        return true;
    }

    virtual ~NFSStream(){};

    virtual void stop();
    virtual void getContent();
    void _getContent();

    virtual bool exists();
    virtual bool isDir();

    virtual bool getContentSync(char **data, size_t *len, bool mmap = false);
    virtual size_t getFileSize() const;
    virtual void seek(size_t pos);

protected:
    virtual const unsigned char *onGetNextPacket(size_t *len, int *err);
    virtual void onStart(size_t packets, size_t seek);

private:
    NFS *m_NFS;

    struct
    {
        const unsigned char *data;
        size_t len;
        off_t pos;
    } m_File;
};

} // namespace IO
} // namespace Nidium

#endif
