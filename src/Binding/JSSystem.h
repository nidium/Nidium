/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jssystem_h__
#define binding_jssystem_h__

#include <Binding/ClassMapper.h>

using Nidium::Binding::ClassMapper;

namespace Nidium {
namespace Binding {

class JSSystem : public ClassMapper<JSSystem>
{
public:
    static void RegisterObject(JSContext *cx);
    static JSFunctionSpec *ListMethods();
#if 0
    static JSPropertySpec *ListProperties();
#endif
protected:
    NIDIUM_DECL_JSCALL(getOpenFileStats);
#if 0
#ifdef NIDIUM_PRODUCT_UI
    NIDIUM_DECL_JSGETTER(language);
#endif
#endif
};

} // namespace Binding
} // namespce Nidium

#endif
