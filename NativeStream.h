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

#ifndef nativestream_h__
#define nativestream_h__

#include "NativeHTTP.h"
#include "NativeIStreamer.h"
#include "NativeTaskManager.h"

class NativeStreamDelegate;


class NativeStream : public NativeHTTPDelegate, public NativeMessages
{
    public:
        enum StreamInterfaces {
            INTERFACE_HTTP,
            INTERFACE_FILE,
            INTERFACE_PRIVATE,
            INTERFACE_DATA,
            INTERFACE_UNKNOWN
        } IInterface;

        enum StreamDataStatus {
            STREAM_EAGAIN = -1,
            STREAM_ERROR = -2,
            STREAM_END = -3
        };

        enum StreamResolveMode {
            STREAM_RESOLVE_FILE,
            STREAM_RESOLVE_PATH,
            STREAM_RESOLVE_ROOT
        };

        enum StreamError {
            STREAM_ERROR_OPEN
        };

        NativeStream(ape_global *net, const char *location,
            const char *prefix = NULL);
        virtual ~NativeStream();

        void setAutoClose(bool close); 
        const char *getLocation() const { return m_Location; }
        static char *resolvePath(const char *url,
            StreamResolveMode mode = STREAM_RESOLVE_PATH);

        /*
            One shot (get the entire file)
        */
        void getContent();

        /*
            TODO: Need to be implemented
        */
        off_t getFileSize() const {
            if (!m_KnownSize) return 0;

            return m_FileSize;
        }

        /*
            Move at pos on file
        */
        void seek(size_t pos);
        void seek2(size_t pos);

        /*
            Start streaming the file
            Each packet returned by getNextPacket is of size "packets". (could be smaller at EOF)
        */
        void start(size_t packets = 4096, size_t seek=0);

        /*
            Stop the current request (e.g. stop the background HTTP request)
        */
        void stop();

        /*
            Get the next packet available
        */
        const unsigned char *getNextPacket(size_t *len, int *err);

        /*
            Determine if getNextPacket is going to return some data
        */
        bool hasDataAvailable() const {
            return !m_DataBuffer.alreadyRead || (m_DataBuffer.ended && m_DataBuffer.back->used);
        }

        bool hasFileSize() const {
            return m_KnownSize;
        }

        /*****************************/
        NativeStreamDelegate *delegate;

        void setDelegate(NativeStreamDelegate *delegate) {
            this->delegate = delegate;
        }

        /* HTTP */
        void onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType);
        void onProgress(size_t offset, size_t len,
            NativeHTTP::HTTPData *h, NativeHTTP::DataType);
        void onError(NativeHTTP::HTTPError err);
        void onHeader();

        size_t getPacketSize() const {
            return this->m_PacketsSize;
        }

        static NativeStream::StreamInterfaces typeInterface(const char *url, int *len);

        void onMessage(const NativeSharedMessages::Message &msg);
    private:
        NativeIStreamer *interface;
        NativeIStreamer *getInterface(bool refresh = false);
        void setInterface(StreamInterfaces interface, int path_offset);
        void clean();
        ape_global *net;

        size_t m_PacketsSize;

        struct {
            buffer *back;
            buffer *front;
            bool alreadyRead;
            bool fresh;
            bool ended;
        } m_DataBuffer;

        struct {
            int fd;
            void *addr;
            size_t idx;
            size_t size;
        } m_Mapped;

        bool m_NeedToSendUpdate;
        bool m_AutoClose;
        void swapBuffer();

        size_t m_Buffered;
        size_t m_BufferedPosition;
        off_t m_FileSize;
        bool m_KnownSize;

        char *m_Location;

        bool m_Pending;
};

class NativeStreamDelegate
{
    public:
        virtual void onGetContent(const char *data, size_t len)=0;
        virtual void onError(NativeStream::StreamError err)=0;
        //virtual void onStreamRead()=0;
        //virtual void onStreamEnd()=0;
        // XXX : This need to be implemented in replacement of the old onProgress
        //virtual void onProgress(size_t buffered, size_t position, size_t len)=0;
        virtual void onProgress(size_t buffered, size_t len)=0;
        virtual void onAvailableData(size_t total)=0;
        virtual ~NativeStreamDelegate(){};
};

#endif
