/*
    NativeJS Core Library
    Copyright (C) 2013 Anthony Catel <paraboul@gmail.com>
    Copyright (C) 2013 Nicolas Trani <n.trani@weelya.com>

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

#ifndef nativefilestream_h__
#define nativefilestream_h__

#include "NativeStreamInterface.h"
#include "NativeMessages.h"
#include "NativeFile.h"

class NativeFileStream : public NativeBaseStream,
                         public NativeMessages
{
public:
    explicit NativeFileStream(const char *location);
    static NativeBaseStream *createStream(const char *location);

    virtual ~NativeFileStream(){};

    virtual void stop();
    virtual void getContent();
    virtual size_t getFileSize() const;
    virtual void seek(size_t pos);
    
    virtual void onMessage(const NativeSharedMessages::Message &msg);
protected:
    virtual const unsigned char *onGetNextPacket(size_t *len, int *err);
    virtual void onStart(size_t packets, size_t seek);
private:
    NativeFile m_File;
    bool m_PendingSeek;
};

#endif