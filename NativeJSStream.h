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

#ifndef nativejsstream_h__
#define nativejsstream_h__

#include "NativeJSExposer.h"
#include "NativeStream.h"


class NativeJSStream :  public NativeJSExposer<NativeJSStream>,
                        public NativeStreamDelegate
{
  public:
    static void registerObject(JSContext *cx);
    NativeJSStream(JSContext *cx, ape_global *net, const char *url);
    ~NativeJSStream();
    NativeStream *getStream() const {
        return m_stream;
    }

    void onGetContent(const char *data, size_t len){};
    void onAvailableData(size_t len);
    void onProgress(size_t buffered, size_t total);
    void onError(NativeStream::StreamError err);
  private:
    NativeStream *m_stream;
};

#endif
