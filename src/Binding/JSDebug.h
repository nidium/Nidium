/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsdebug_h__
#define binding_jsdebug_h__

#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

class JSDebug : public ClassMapper<JSDebug>
{
public:
    virtual ~JSDebug(){};

    static JSFunctionSpec *ListMethods();
    static void RegisterObject(JSContext *cx);
protected:
    NIDIUM_DECL_JSCALL(serialize);
    NIDIUM_DECL_JSCALL(unserialize);

};

} // namespace Binding
} // namespace Nidium

#endif
