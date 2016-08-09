/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef io_filestream_h__
#define io_filestream_h__

#include "Core/Messages.h"

#include "IO/Stream.h"
#include "IO/File.h"

namespace Nidium {
namespace IO {

class FileStream : public Stream, public Nidium::Core::Messages
{
public:
    explicit FileStream(const char *location);

    static Stream *CreateStream(const char *location)
    {
        return new FileStream(location);
    }
    static const char *GetBaseDir()
    {
        return Nidium::Core::Path::GetRoot();
    }

    static bool AllowLocalFileStream()
    {
        return true;
    }
    static bool AllowSyncStream()
    {
        return true;
    }

    virtual ~FileStream(){};

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
    File m_File;
    bool m_OpenFailed = false;
};

} // namespace IO
} // namespace Nidium

#endif
