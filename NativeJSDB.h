#ifndef nativejsdb_h__
#define nativejsdb_h__

#include "NativeDB.h"

#include <jsapi.h>

class NativeJSDB: public NativeDB
{
    public:
        /*
            Caller is responsible for knowing how to
            decode the data during a get()
        */
        bool insert(const char *key, JSContext *cx, JS::HandleValue val);
};

#endif

