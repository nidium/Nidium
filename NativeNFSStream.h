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

#ifndef nativenfsstream_h__
#define nativenfsstream_h__

#include "NativeStreamInterface.h"
#include "NativeMessages.h"

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

    virtual ~NativeNFSStream(){};

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