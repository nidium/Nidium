/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsstream_h__
#define binding_jsstream_h__

#include "Core/Messages.h"
#include "IO/Stream.h"
#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

class JSStream : public ClassMapper<JSStream>, public Nidium::Core::Messages
{
public:
    static void RegisterObject(JSContext *cx);
    JSStream(ape_global *net,
             const char *url);
    ~JSStream();
    Nidium::IO::Stream *getStream() const
    {
        return m_Stream;
    }

    void onMessage(const Nidium::Core::SharedMessages::Message &msg);
    static JSStream *Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj);
    static JSFunctionSpec *ListMethods();
    static JSPropertySpec *ListProperties();
protected:
    NIDIUM_DECL_JSCALL(seek);
    NIDIUM_DECL_JSCALL(start);
    NIDIUM_DECL_JSCALL(stop);
    NIDIUM_DECL_JSCALL(getNextPacket);

    NIDIUM_DECL_JSGETTER(filesize);
private:
    Nidium::IO::Stream *m_Stream;
};

} // namespace Binding
} // namespace Nidium

#endif
