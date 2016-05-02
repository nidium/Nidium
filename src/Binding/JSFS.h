/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsfs_h__
#define binding_jsfs_h__

#include "Core/Messages.h"
#include "Core/TaskManager.h"
#include "Binding/JSExposer.h"

namespace Nidium {
namespace Binding {

class JSFS : public JSExposer<JSFS>,
             public Nidium::Core::Managed
{
public:
    static void RegisterObject(JSContext *cx);
};

} // namespace Binding
} // namespace Nidium

#endif

