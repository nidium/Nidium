/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsdebug_h__
#define binding_jsdebug_h__

#include "JSExposer.h"

namespace Nidium {
namespace Binding {

class JSDebug : public Nidium::Binding::JSExposer<JSDebug>
{
  public:
    JSDebug(JS::HandleObject obj, JSContext *cx) :
    Nidium::Binding::JSExposer<JSDebug>(obj, cx)
    {};
    virtual ~JSDebug() {};

    static void registerObject(JSContext *cx);
    static const char *getJSObjectName() {
        return "Debug";
    }

    static JSClass *jsclass;
};

} // namespace Binding
} // namespace Nidium

#endif

