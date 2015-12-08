#include "NativeJSDB.h"

#include <leveldb/db.h>
#include <jsapi.h>
#include <js/StructuredClone.h>

bool NativeJSDB::insert(const char *key, JSContext *cx, JS::HandleValue val)
{
    uint64_t *data;
    size_t data_len;

    if (!JS_WriteStructuredClone(cx, val, &data, &data_len, NULL, NULL, JS::NullHandleValue)) {
        return false;
    }
    leveldb::Slice input((char *)data, data_len);
    leveldb::Status status = m_Database->Put(leveldb::WriteOptions(), key, input);

    JS_ClearStructuredClone(data, data_len, nullptr, nullptr);

    return status.ok();
}

