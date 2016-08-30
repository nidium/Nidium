/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsconsole_h__
#define binding_jsconsole_h__

#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

class JSConsole : public ClassMapper<JSConsole>
{
public:
    static JSFunctionSpec *ListMethods();

    static void RegisterObject(JSContext *cx);
protected:
    NIDIUM_DECL_JSCALL(log);
    NIDIUM_DECL_JSCALL(write);
    NIDIUM_DECL_JSCALL(hide);
    NIDIUM_DECL_JSCALL(clear);
    NIDIUM_DECL_JSCALL(show);
};

} // namespace Binding
} // namespace Nidium

#endif
