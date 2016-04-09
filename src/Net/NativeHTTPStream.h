/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativehttpstream_h__
#define nativehttpstream_h__

#include "IO/NativeStreamInterface.h"
#include "Core/NativeMessages.h"
#include "Net/NativeHTTP.h"

class NativeHTTPStream : public NativeBaseStream,
                         public Native::Core::HTTPDelegate
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

    virtual const char *getPath() const override;

    void notifyAvailable();
protected:
    virtual const unsigned char *onGetNextPacket(size_t *len, int *err);
    virtual void onStart(size_t packets, size_t seek);

    bool readComplete() const {
        return m_BytesBuffered == m_Mapped.size;
    }
private:
    Native::Core::HTTP *m_Http;

    void onRequest(Native::Core::HTTP::HTTPData *h, Native::Core::HTTP::DataType);
    void onProgress(size_t offset, size_t len, Native::Core::HTTP::HTTPData *h,
        Native::Core::HTTP::DataType);
    void onError(Native::Core::HTTP::HTTPError err);
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

