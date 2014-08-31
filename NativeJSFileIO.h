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
#include "NativeFile.h"
#include "NativeMessages.h"
#include <ape_buffer.h>

class NativeJSFileIO : public NativeJSExposer<NativeJSFileIO>,
                       public NativeMessages
{
  public:
    void onMessage(const NativeSharedMessages::Message &msg);

    static NativeFile *GetFileFromJSObject(JSContext *cx, JSObject *jsobj);
    static void registerObject(JSContext *cx);
    static JSObject *generateJSObject(JSContext *cx, const char *path);

    static bool handleError(JSContext *cx, const NativeSharedMessages::Message &msg, jsval &vals);
    static bool callbackForMessage(JSContext *cx,
        const NativeSharedMessages::Message &msg,
        JSObject *thisobj, const char *encoding = NULL);

    NativeJSFileIO()  : m_Encoding(NULL) {
    };

    ~NativeJSFileIO() {
        if (m_Encoding) {
            free(m_Encoding);
        }
    };

    NativeFile *getFile() const { return m_File; }
    void setFile(NativeFile *file) { m_File = file; }

    JSObject *jsobj;

    char *m_Encoding;
  private:
    NativeFile *m_File;
};

#endif
