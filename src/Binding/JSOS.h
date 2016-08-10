/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsos_h__
#define binding_jsos_h__

#include "Binding/JSExposer.h"

namespace Nidium {
namespace Binding {

class JSOS : public JSExposer<JSOS>
{
public:
    JSOS(JSContext *cx, JS::HandleObject obj, const char *name);
    static void RegisterObject(JSContext *cx);
};

} // namespace Binding
} // namespace Nidium

#endif
