/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsconsole_h__
#define binding_jsconsole_h__

#include "Binding/JSExposer.h"

namespace Nidium {
namespace Binding {

class JSConsole : public JSExposer<JSConsole>
{
  public:
    static void RegisterObject(JSContext *cx);
};

} // namespace Binding
} // namespace Nidium

#endif

