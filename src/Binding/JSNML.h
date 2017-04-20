/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsnml_h__
#define binding_jsnml_h__

#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

class JSNML : public ClassMapper<JSNML>
{
public:
    JSNML();

    static void RegisterObject(JSContext *cx);

    static JSFunctionSpec *ListStaticMethods();
protected:

    NIDIUM_DECL_JSCALL_STATIC(parse);
    //NIDIUM_DECL_JSCALL_STATIC(stringify);
};

} // namespace Binding
} // namespace Nidium

#endif
