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
#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

class JSFile : public ClassMapper<JSFile>, public Nidium::Core::Messages
{
public:
    void onMessage(const Nidium::Core::SharedMessages::Message &msg);

    static Nidium::IO::File *GetFileFromJSObject(JSContext *cx,
                                                 JS::HandleObject jsobj);

    static JSFile *Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj);
    static JSFunctionSpec *ListMethods();
    static JSFunctionSpec *ListStaticMethods();
    static JSPropertySpec *ListProperties();

    static void RegisterObject(JSContext *cx);
    static JSObject *GenerateJSObject(JSContext *cx, const char *path);

    static bool HandleError(JSContext *cx,
                            const Nidium::Core::SharedMessages::Message &msg,
                            JS::MutableHandleValue vals);

    JSFile(const char *path,
           bool allowAll = false)
        : m_Encoding(NULL), m_File(NULL),
          m_Path(Core::Path(path, allowAll)){};

    virtual ~JSFile();

    bool allowSyncStream()
    {
        return m_Path.GetScheme()->AllowSyncStream();
    }

    const char *getPath()
    {
        return m_Path.path();
    }

    Nidium::IO::File *getFile() const
    {
        return m_File;
    }

    void setFile(Nidium::IO::File *file)
    {
        m_File = file;
    }

protected:
    NIDIUM_DECL_JSCALL(open);
    NIDIUM_DECL_JSCALL(openSync);
    NIDIUM_DECL_JSCALL(read);
    NIDIUM_DECL_JSCALL(readSync);
    NIDIUM_DECL_JSCALL(seek);
    NIDIUM_DECL_JSCALL(seekSync);
    NIDIUM_DECL_JSCALL(close);
    NIDIUM_DECL_JSCALL(closeSync);
    NIDIUM_DECL_JSCALL(write);
    NIDIUM_DECL_JSCALL(writeSync);
    NIDIUM_DECL_JSCALL(isDir);
    NIDIUM_DECL_JSCALL(listFiles);
    NIDIUM_DECL_JSCALL(mkDir);
    NIDIUM_DECL_JSCALL(rm);
    NIDIUM_DECL_JSCALL(rmrf);

    NIDIUM_DECL_JSCALL_STATIC(read);
    NIDIUM_DECL_JSCALL_STATIC(readSync);

    NIDIUM_DECL_JSGETTER(filesize);
    NIDIUM_DECL_JSGETTER(filename);

private:
    char *m_Encoding;
    Nidium::IO::File *m_File;
    Core::Path m_Path;
};

} // namespace Binding
} // namespace Nidium

#endif
