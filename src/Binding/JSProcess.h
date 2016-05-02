/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsprocess_h__
#define binding_jsprocess_h__

#include "Binding/JSExposer.h"

namespace Nidium {
namespace Binding {

class JSProcess : public JSExposer<JSProcess>
{
  public:
    JSProcess(JS::HandleObject obj, JSContext *cx) : JSExposer<JSProcess>(obj, cx), m_SignalFunction(cx) {};
    virtual ~JSProcess() {};

    static void RegisterObject(JSContext *cx, char **argv, int argc, int workerId = 0);
    static const char *GetJSObjectName() {
        return "process";
    }

    static JSClass *jsclass;

    JS::PersistentRootedValue m_SignalFunction;
};

} // namespace Binding
} // namespace Nidium

#endif

