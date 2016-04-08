/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#ifndef nativejsconsole_h__
#define nativejsconsole_h__

#include <NativeJSExposer.h>

class NativeJSconsole : public NativeJSExposer<NativeJSconsole>
{
  public:
    static void registerObject(JSContext *cx);
};

#endif

