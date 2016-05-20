/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsdb_h__
#define binding_jsdb_h__

#include "Core/DB.h"

#include <jsapi.h>

namespace Nidium {
namespace Binding {

class JSDB: public Nidium::Core::DB
{
    public:
        using DB::DB;
        /*
            Caller is responsible for knowing how to
            decode the data during a get()
        */
        bool insert(const char *key, JSContext *cx, JS::HandleValue val);
};

} // namespace Binding
} // namespace Nidium

#endif

