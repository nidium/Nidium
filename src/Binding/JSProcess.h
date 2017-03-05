/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsprocess_h__
#define binding_jsprocess_h__

#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

class JSProcess : public ClassMapper<JSProcess>
{
public:
    virtual ~JSProcess(){};

    static void RegisterObject(JSContext *cx, char **argv,
      int argc, int workerId = 0);

    static JSFunctionSpec *ListMethods();
protected:
#ifndef _MSC_VER
    NIDIUM_DECL_JSCALL(getOwner);
    NIDIUM_DECL_JSCALL(setOwner);
#endif
    NIDIUM_DECL_JSCALL(setSignalHandler);
    NIDIUM_DECL_JSCALL(exit);
    NIDIUM_DECL_JSCALL(shutdown);
    NIDIUM_DECL_JSCALL(cwd);
};

} // namespace Binding
} // namespace Nidium

#endif
