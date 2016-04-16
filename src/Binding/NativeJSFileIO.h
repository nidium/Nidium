/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejsfileio_h__
#define nativejsfileio_h__

#include <ape_buffer.h>

#include "Core/NativeMessages.h"
#include "IO/File.h"
#include "JSExposer.h"

class NativeJSFileIO : public Nidium::Binding::JSExposer<NativeJSFileIO>,
                       public NativeMessages
{
  public:
    void onMessage(const Nidium::Core::SharedMessages::Message &msg);

    static Nidium::IO::File *GetFileFromJSObject(JSContext *cx, JS::HandleObject jsobj);
    static void registerObject(JSContext *cx);
    static JSObject *generateJSObject(JSContext *cx, const char *path);

    static bool handleError(JSContext *cx, const Nidium::Core::SharedMessages::Message &msg, JS::MutableHandleValue vals);
    bool callbackForMessage(JSContext *cx,
        const Nidium::Core::SharedMessages::Message &msg,
        JSObject *thisobj, const char *encoding = NULL);

    NativeJSFileIO(JS::HandleObject obj, JSContext *cx) :
        Nidium::Binding::JSExposer<NativeJSFileIO>(obj, cx), m_Encoding(NULL) {
    };

    ~NativeJSFileIO() {
        if (m_Encoding) {
            free(m_Encoding);
        }
    };

    Nidium::IO::File *getFile() const { return m_File; }
    void setFile(Nidium::IO::File *file) { m_File = file; }

    char *m_Encoding;
  private:
    Nidium::IO::File *m_File;
};

#endif

