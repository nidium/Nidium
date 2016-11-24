/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsvm_h__
#define binding_jsvm_h__


#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

class JSVM : public ClassMapper<JSVM>
{
public:
    JSVM();

    static void RegisterObject(JSContext *cx);
    static JSObject *RegisterModule(JSContext *cx);

    static JSFunctionSpec *ListStaticMethods();
protected:

    NIDIUM_DECL_JSCALL_STATIC(runInScope);
};

} // namespace Binding
} // namespace Nidium

#endif
