/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejsdb_h__
#define nativejsdb_h__

#include "Core/NativeDB.h"

#include <jsapi.h>

class NativeJSDB: public NativeDB
{
    public:

        using NativeDB::NativeDB;
        /*
            Caller is responsible for knowing how to
            decode the data during a get()
        */
        bool insert(const char *key, JSContext *cx, JS::HandleValue val);
};

#endif

