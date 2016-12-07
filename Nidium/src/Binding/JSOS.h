/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jssystem_h__
#define binding_jssystem_h__

#include "Binding/ClassMapper.h"

using Nidium::Binding::ClassMapper;

namespace Nidium {
namespace Binding {

class JSOS : public ClassMapper<JSOS>
{
public:
    JSOS();
    static void RegisterObject(JSContext *cx);
    static JSObject *RegisterModule(JSContext *cx);
};

} // namespace Binding
} // namespce Nidium

#endif
