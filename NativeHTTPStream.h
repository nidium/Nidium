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

#ifndef nativehttpstream_h__
#define nativehttpstream_h__

#include "NativeStreamInterface.h"
#include "NativeMessages.h"
#include "NativeHTTP.h"

class NativeHTTPStream : public NativeBaseStream,
                         public NativeHTTPDelegate
{
public:
    explicit NativeHTTPStream(const char *location);
    virtual ~NativeHTTPStream();
    
    static NativeBaseStream *createStream(const char *location) {
        return new NativeHTTPStream(location);
    }
    static const char *getBaseDir() {
        return NULL;
    }

    static bool allowLocalFileStream() {
        return false;
    }

    static bool allowSyncStream() {
        return false;
    }
    
    virtual void stop();
    virtual void getContent();
    virtual void seek(size_t pos) override;

    virtual size_t getFileSize() const;
    virtual bool hasDataAvailable() const;

    void notifyAvailable();
protected:
    virtual const unsigned char *onGetNextPacket(size_t *len, int *err);
    virtual void onStart(size_t packets, size_t seek);

    bool readComplete() const {
        return m_BytesBuffered == m_Mapped.size;
    }
private:
    NativeHTTP *m_Http;

    void onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType);
    void onProgress(size_t offset, size_t len, NativeHTTP::HTTPData *h,
        NativeHTTP::DataType);
    void onError(NativeHTTP::HTTPError err);
    void onHeader();
    void cleanCacheFile();

    struct {
        int fd;
        void *addr;
        size_t size;
    } m_Mapped;

    size_t m_StartPosition;
    size_t m_BytesBuffered;
    size_t m_LastReadUntil;
};

#endif
