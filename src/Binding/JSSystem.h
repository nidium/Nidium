/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jssystem_h__
#define binding_jssystem_h__

#include <Binding/JSExposer.h>

using Nidium::Binding::JSExposer;

namespace Nidium {
namespace Binding {

class JSSystem : public JSExposer<JSSystem>
{
  public:
    static void RegisterObject(JSContext *cx);
};

} // namespace Binding
} // namespce Nidium

#endif

