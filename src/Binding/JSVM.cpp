/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "JSVM.h"
#include "JSModules.h"


namespace Nidium {
namespace Binding {


void JSVM::RegisterObject(JSContext *cx)
{
    JSModules::RegisterEmbedded("vm", JSVM::RegisterModule);
}

JSObject *JSVM::RegisterModule(JSContext *cx)
{
    return JSVM::ExposeObject(cx, "vm");
}

} // namespace Binding
} // namespace Nidium

