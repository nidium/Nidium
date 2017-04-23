/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSDB.h"
#include "Binding/JSModules.h"

#include <leveldb/db.h>
#include <js/StructuredClone.h>

#include "Core/Path.h"
#include "Binding/NidiumJS.h"

namespace Nidium {
namespace Binding {

// {{{ JSDB Bindings
JSDB *JSDB::Constructor(JSContext *cx, JS::CallArgs &args,
    JS::HandleObject obj)
{

    if (!args[0].isString()) {
        JS_ReportError(cx, "First argument must be a string");
        return nullptr;
    }

    JS::RootedString jsPath(cx, args[0].toString());
    JSAutoByteString dbPath(cx, jsPath);
    Core::Path path(dbPath.ptr());
    if (!path.path()) {
        JS_ReportError(cx, "Invalid DB path");
        return nullptr;
    }

    JSDB *jsdb = new JSDB(path.path());
    if (!jsdb->ok()) {
        JS_ReportError(cx, "Failed to create DB");

        delete jsdb;

        return nullptr;
    }

    return jsdb;
}

bool JSDB::JS_set(JSContext *cx, JS::CallArgs &args)
{
    if (!args[0].isString()) {
        JS_ReportError(cx, "set() : key must be a string");
        return false;
    }

    JSAutoByteString key(cx, args[0].toString());
    if (!this->set(cx, key.ptr(), args[1])) {
        JS_ReportError(cx, "Failed to set data");
        return false;
    }

    return true;
}

bool JSDB::JS_get(JSContext *cx, JS::CallArgs &args)
{
    if (!args[0].isString()) {
        JS_ReportError(cx, "get() : key must be a string");
        return false;
    }

    JSAutoByteString key(cx, args[0].toString());
    JS::RootedValue rval(cx);

    if (!this->get(cx, key.ptr(), &rval)) {
        JS_ReportError(cx, "Failed to retreive data");
        return false;
    }

    args.rval().set(rval);

    return true;
}

bool JSDB::JS_delete(JSContext *cx, JS::CallArgs &args)
{
    if (!args[0].isString()) {
        JS_ReportError(cx, "delete() : key must be a string");
        return false;
    }

    JSAutoByteString key(cx, args[0].toString());
    JS::RootedValue rval(cx);

    if (!this->del(key.ptr())) {
        JS_ReportError(cx, "Failed to delete data");
        return false;
    }

    return true;
}

bool JSDB::JS_drop(JSContext *cx, JS::CallArgs &args)
{
    if (!this->drop()) {
        JS_ReportError(cx, "Failed to drop DB");
        return false;
    }

    return true;
}

bool JSDB::JS_close(JSContext *cx, JS::CallArgs &args)
{
    if (!this->close()) {
        JS_ReportError(cx, "Failed to drop DB");
        return false;
    }

    return true;
}


// }}}

// {{{ JSDB Implementation
JSDB::JSDB(const char *path)
    : DB(path)
{
}

bool JSDB::set(JSContext *cx, const char *key, JS::HandleValue val)
{
    uint64_t *data;
    size_t data_len;
    bool success;

    if (!JS_WriteStructuredClone(cx, val, &data, &data_len, NidiumJS::m_JsScc, NULL,
                                 JS::NullHandleValue)) {
        return false;
    }

    success = DB::set(key, reinterpret_cast<uint8_t *>(data), data_len);

    JS_ClearStructuredClone(data, data_len, NidiumJS::m_JsScc, nullptr);

    return success;
}

bool JSDB::get(JSContext *cx, const char *key, JS::MutableHandleValue rval)
{
    std::string data;

    if (m_Database == nullptr) {
        rval.setUndefined();
        return false;
    }

    if (!DB::get(key, data)) {
        rval.setUndefined();
        return true;
    }

    uint64_t *aligned_data;

    /*
        ReadStructuredClone requires 8-bytes aligned memory
    */
    if (((uintptr_t)data.data() & 7) == 0) {
        aligned_data = (uint64_t *)data.data();
    } else {
        if (posix_memalign((void **)&aligned_data, 8, data.length()) != 0) {
            return false;
        }

        memcpy(aligned_data, data.data(), data.length());
    }

    if (!JS_ReadStructuredClone(cx, aligned_data, data.length(),
                                JS_STRUCTURED_CLONE_VERSION, rval, NidiumJS::m_JsScc,
                                NULL)) {

        JS_ReportError(cx, "Unable to read internal data");
        return false;
    }

    if ((void *)aligned_data != data.data()) {
        free(aligned_data);
    }

    return true;
}
// }}}

JSFunctionSpec *JSDB::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSDB, get, 1),
        CLASSMAPPER_FN(JSDB, set, 2),
        CLASSMAPPER_FN(JSDB, delete, 1),
        CLASSMAPPER_FN(JSDB, close, 0),
        CLASSMAPPER_FN(JSDB, drop, 0),
        JS_FS_END
    };

    return funcs;
}


static JSObject *registerCallback(JSContext *cx)
{
    JS::RootedObject obj(cx, JS_NewPlainObject(cx));

    JSDB::ExposeClass<1>(cx, "DB", 0, JSDB::kEmpty_ExposeFlag, obj);

    JS::RootedValue val(cx);
    if (!JS_GetProperty(cx, obj, "DB", &val)) {
        return nullptr;
    }

    JS::RootedObject ret(cx, val.toObjectOrNull());

    return ret;
}

void JSDB::RegisterObject(JSContext *cx)
{
    JSModules::RegisterEmbedded("DB", registerCallback);
}


} // namespace Binding
} // namespace Nidium
