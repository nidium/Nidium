/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsdebug_h__
#define binding_jsdebug_h__

#include "Binding/JSExposer.h"

namespace Nidium {
namespace Binding {

class JSDebug : public JSExposer<JSDebug>
{
public:
    JSDebug(JS::HandleObject obj, JSContext *cx)
        : JSExposer<JSDebug>(obj, cx){};
    virtual ~JSDebug(){};

    static void RegisterObject(JSContext *cx);
    static const char *GetJSObjectName()
    {
        return "Debug";
    }
};

} // namespace Binding
} // namespace Nidium

#endif
