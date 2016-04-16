/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsstream_h__
#define binding_jsstream_h__

#include "Core/NativeMessages.h"
#include "JSExposer.h"
#include "IO/Stream.h"

namespace Nidium {
namespace Binding {

class JSStream :  public Nidium::Binding::JSExposer<JSStream>,
                        public NativeMessages
{
  public:
    static void registerObject(JSContext *cx);
    JSStream(JS::HandleObject obj, JSContext *cx, ape_global *net, const char *url);
    ~JSStream();
    Nidium::IO::Stream *getStream() const {
        return m_Stream;
    }

    void onMessage(const Nidium::Core::SharedMessages::Message &msg);
  private:
    Nidium::IO::Stream *m_Stream;
};

} // namespace Binding
} // namespace Nidium

#endif

