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

#ifndef nativejsfileio_h__
#define nativejsfileio_h__

#include "NativeJSExposer.h"
#include "NativeFileIO.h"

class NativeJSFileIO : public NativeJSExposer<NativeJSFileIO>, public NativeFileIODelegate
{
  public:
    static void registerObject(JSContext *cx);
    NativeJSFileIO() : m_Binary(true) {};
    ~NativeJSFileIO() {};

    void onNFIOOpen(NativeFileIO *);
    void onNFIOError(NativeFileIO *, int errno);
    void onNFIORead(NativeFileIO *, unsigned char *data, size_t len);
    void onNFIOWrite(NativeFileIO *, size_t written);

    NativeFileIO *getNFIO() const { return NFIO; }
    void setNFIO(NativeFileIO *nfio) { NFIO = nfio; }

    struct {
        jsval open;
        jsval getContents;
        jsval read;
        jsval write;
    } callbacks;

    JSObject *jsobj;

    bool m_Binary;

  private:

    NativeFileIO *NFIO;


};

#endif
