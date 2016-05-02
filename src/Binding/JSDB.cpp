/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSDB.h"

#include <leveldb/db.h>
#include <js/StructuredClone.h>

namespace Nidium {
namespace Binding {

bool JSDB::insert(const char *key, JSContext *cx, JS::HandleValue val)
{
    uint64_t *data;
    size_t data_len;

    if (!JS_WriteStructuredClone(cx, val, &data, &data_len, NULL, NULL, JS::NullHandleValue)) {
        return false;
    }
    leveldb::Slice input(reinterpret_cast<char *>(data), data_len);
    leveldb::Status status = m_Database->Put(leveldb::WriteOptions(), key, input);

    JS_ClearStructuredClone(data, data_len, nullptr, nullptr);

    return status.ok();
}

} // namespace Binding
} // namespace Nidium

