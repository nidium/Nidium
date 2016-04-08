/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#ifndef nativejssystem_h__
#define nativejssystem_h__

#include <NativeJSExposer.h>

class NativeJSSystem : public NativeJSExposer<NativeJSSystem>
{
  public:
    static void registerObject(JSContext *cx);
};

#endif

