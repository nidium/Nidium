/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsdb_h__
#define binding_jsdb_h__

#include "Core/DB.h"

#include "Binding/JSExposer.h"

#include <jsapi.h>

namespace Nidium {
namespace Binding {

class JSDB : public JSExposer<JSDB>, public Nidium::Core::DB
{
public:
    JSDB(JSContext *cx, JS::HandleObject obj, const char *name);
    bool set(JSContext *cx, const char *key, JS::HandleValue val);
    bool get(JSContext *cx, const char *key, JS::MutableHandleValue val);
    static void RegisterObject(JSContext *cx);
};

} // namespace Binding
} // namespace Nidium

#endif
