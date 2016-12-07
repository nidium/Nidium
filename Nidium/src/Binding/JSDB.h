/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsdb_h__
#define binding_jsdb_h__

#include "Core/DB.h"

#include <jsapi.h>

#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

class JSDB : public ClassMapper<JSDB>, public Nidium::Core::DB
{
public:
    JSDB(const char *name);
    bool set(JSContext *cx, const char *key, JS::HandleValue val);
    bool get(JSContext *cx, const char *key, JS::MutableHandleValue val);
    static void RegisterObject(JSContext *cx);

    static JSDB *Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj);

    static JSFunctionSpec *ListMethods();
protected:

    NIDIUM_DECL_JSCALL(get);
    NIDIUM_DECL_JSCALL(set);
    NIDIUM_DECL_JSCALL(delete);
    NIDIUM_DECL_JSCALL(close);
    NIDIUM_DECL_JSCALL(drop);

};

} // namespace Binding
} // namespace Nidium

#endif
