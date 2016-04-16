/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejsstream_h__
#define nativejsstream_h__

#include "Core/NativeMessages.h"
#include "JSExposer.h"
#include "IO/Stream.h"

class NativeJSStream :  public Nidium::Binding::JSExposer<NativeJSStream>,
                        public NativeMessages
{
  public:
    static void registerObject(JSContext *cx);
    NativeJSStream(JS::HandleObject obj, JSContext *cx, ape_global *net, const char *url);
    ~NativeJSStream();
    Nidium::IO::Stream *getStream() const {
        return m_Stream;
    }

    void onMessage(const Nidium::Core::SharedMessages::Message &msg);
  private:
    Nidium::IO::Stream *m_Stream;
};

#endif

