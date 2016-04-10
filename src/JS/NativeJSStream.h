/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejsstream_h__
#define nativejsstream_h__

#include "Core/NativeMessages.h"
#include "NativeJSExposer.h"

class NativeBaseStream;

class NativeJSStream :  public NativeJSExposer<NativeJSStream>,
                        public NativeMessages
{
  public:
    static void registerObject(JSContext *cx);
    NativeJSStream(JS::HandleObject obj, JSContext *cx, ape_global *net, const char *url);
    ~NativeJSStream();
    NativeBaseStream *getStream() const {
        return m_Stream;
    }

    void onMessage(const NativeSharedMessages::Message &msg);
  private:
    NativeBaseStream *m_Stream;
};

#endif

