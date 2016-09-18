/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsfs_h__
#define binding_jsfs_h__

#include "Core/Messages.h"
#include "Core/TaskManager.h"
#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

class JSFS : public ClassMapper<JSFS>, public Nidium::Core::Managed
{
public:
    static void RegisterObject(JSContext *cx);
    static JSFunctionSpec *ListMethods();
protected:
    NIDIUM_DECL_JSCALL(readDir);
};

} // namespace Binding
} // namespace Nidium

#endif
