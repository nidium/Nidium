/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsfs_h__
#define binding_jsfs_h__

#include "Core/Messages.h"
#include "Core/NativeTaskManager.h"
#include "JSExposer.h"

namespace Nidium {
namespace Binding {

class JSFS :     public Nidium::Binding::JSExposer<JSFS>, public NativeManaged
{
public:
    static void registerObject(JSContext *cx);
};

} // namespace Binding
} // namespace Nidium

#endif
