/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsconsole_h__
#define binding_jsconsole_h__

#include "JSExposer.h"

namespace Nidium {
namespace Binding {

class JSConsole : public JSExposer<JSConsole>
{
  public:
    static void registerObject(JSContext *cx);
};

} // namespace Binding
} // namespace Nidium

#endif

