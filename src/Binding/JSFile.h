/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsfileio_h__
#define binding_jsfileio_h__

#include <ape_buffer.h>

#include "Core/Messages.h"
#include "Core/Path.h"
#include "IO/File.h"
#include "Binding/JSExposer.h"

namespace Nidium {
namespace Binding {

class JSFile : public JSExposer<JSFile>, public Nidium::Core::Messages
{
public:
    void onMessage(const Nidium::Core::SharedMessages::Message &msg);

    static Nidium::IO::File *GetFileFromJSObject(JSContext *cx,
                                                 JS::HandleObject jsobj);
    static void RegisterObject(JSContext *cx);
    static JSObject *GenerateJSObject(JSContext *cx, const char *path);

    static bool HandleError(JSContext *cx,
                            const Nidium::Core::SharedMessages::Message &msg,
                            JS::MutableHandleValue vals);
    bool callbackForMessage(JSContext *cx,
                            const Nidium::Core::SharedMessages::Message &msg,
                            JSObject *thisobj,
                            const char *encoding = NULL);

    JSFile(JS::HandleObject obj,
           JSContext *cx,
           const char *path,
           bool allowAll = false)
        : JSExposer<JSFile>(obj, cx), m_Encoding(NULL), m_File(NULL),
          m_Path(Core::Path(path, allowAll)){};

    bool allowSyncStream()
    {
        return m_Path.GetScheme()->AllowSyncStream();
    }

    const char *getPath()
    {
        return m_Path.path();
    }

    ~JSFile()
    {
        if (m_Encoding) {
            free(m_Encoding);
        }
    };

    Nidium::IO::File *getFile() const
    {
        return m_File;
    }
    void setFile(Nidium::IO::File *file)
    {
        m_File = file;
    }

    char *m_Encoding;

private:
    Nidium::IO::File *m_File;
    Core::Path m_Path;
};

} // namespace Binding
} // namespace Nidium

#endif
