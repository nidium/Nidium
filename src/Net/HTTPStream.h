/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef net_httpstream_h__
#define net_httpstream_h__

#include "Core/Messages.h"
#include "IO/Stream.h"
#include "Net/HTTP.h"

namespace Nidium {
namespace Net {

class HTTPStream : public Nidium::IO::Stream, public HTTPDelegate
{
public:
    explicit HTTPStream(const char *location);
    virtual ~HTTPStream();

    static Nidium::IO::Stream *createStream(const char *location) {
        return new HTTPStream(location);
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
    HTTP *m_Http;

    void onRequest(HTTP::HTTPData *h, HTTP::DataType);
    void onProgress(size_t offset, size_t len, HTTP::HTTPData *h, HTTP::DataType);
    void onError(HTTP::HTTPError err);
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

} // namespace Net
} // namespace Nidium

#endif

