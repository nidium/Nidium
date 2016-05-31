/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSDB.h"
#include "Binding/JSModules.h"

#include <leveldb/db.h>
#include <js/StructuredClone.h>
#include <Core/Path.h>

namespace Nidium {
namespace Binding {

// {{{ Preamble
static bool jsdb_get(JSContext *cx, unsigned argc, JS::Value *vp);
static bool jsdb_set(JSContext *cx, unsigned argc, JS::Value *vp);
static bool jsdb_del(JSContext *cx, unsigned argc, JS::Value *vp);
static bool jsdb_close(JSContext *cx, unsigned argc, JS::Value *vp);
static bool jsdb_drop(JSContext *cx, unsigned argc, JS::Value *vp);
static void JSDB_Finalizer(JSFreeOp *fop, JSObject *obj);

static JSClass JSDB_class = {
    "DB", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JSDB_Finalizer,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<> JSClass *JSExposer<JSDB>::jsclass = &JSDB_class;

static JSFunctionSpec JSDB_funcs[] = {
    JS_FN("get", jsdb_get, 1, NIDIUM_JS_FNPROPS),
    JS_FN("set", jsdb_set, 2, NIDIUM_JS_FNPROPS),
    JS_FN("del", jsdb_del, 1, NIDIUM_JS_FNPROPS),
    JS_FN("close", jsdb_close, 0, NIDIUM_JS_FNPROPS),
    JS_FN("drop", jsdb_drop, 0, NIDIUM_JS_FNPROPS),
    JS_FS_END
};
// }}}

// {{{ JSDB Bindings
static bool jsdb_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    NIDIUM_JS_CONSTRUCTOR_PROLOGUE();

    NIDIUM_JS_CHECK_ARGS("constructor", 1);

    if (!args[0].isString()) {
        JS_ReportError(cx, "First argument must be a string");
        return false;
    }

    JS::RootedString jsPath(cx, args[0].toString());
    JSAutoByteString dbPath(cx, jsPath);
    Core::Path path(dbPath.ptr());
    if (!path.path()) {
        JS_ReportError(cx, "Invalid DB path");
        return false;
    }

    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx,
                            &JSDB_class, args));

    JSDB *jsdb = new JSDB(cx, ret, path.path());
    if (!jsdb->ok()) {
        JS_ReportError(cx, "Failed to create DB");

        delete jsdb;

        return false;
    }

    JS_SetPrivate(ret, jsdb);

    args.rval().setObject(*ret);

    return true;
}

static bool jsdb_set(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSDB, &JSDB_class);

    NIDIUM_JS_CHECK_ARGS("set", 2);

    if (!args[0].isString()) {
        JS_ReportError(cx, "set() : key must be a string");
        return false;
    }

    JSAutoByteString key(cx, args[0].toString());
    if (!CppObj->set(cx, key.ptr(), args[1])) { 
        JS_ReportError(cx, "Failed to set data");
        return false;
    }

    return true;
}

static bool jsdb_get(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(JSDB, &JSDB_class);

    NIDIUM_JS_CHECK_ARGS("get", 1);

    if (!args[0].isString()) {
        JS_ReportError(cx, "get() : key must be a string");
        return false;
    }

    JSAutoByteString key(cx, args[0].toString());
    JS::RootedValue rval(cx);

    if (!CppObj->get(cx, key.ptr(), &rval)) {
        JS_ReportError(cx, "Failed to retreive data");
        return false;
    }

    args.rval().set(rval);

    return true;
}

static bool jsdb_del(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSDB, &JSDB_class);

    NIDIUM_JS_CHECK_ARGS("del", 1);

    if (!args[0].isString()) {
        JS_ReportError(cx, "del() : key must be a string");
        return false;
    }

    JSAutoByteString key(cx, args[0].toString());
    JS::RootedValue rval(cx);

    if (!CppObj->del(key.ptr())) {
        JS_ReportError(cx, "Failed to delete data");
        return false;
    }

    return true;
}

static bool jsdb_drop(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSDB, &JSDB_class);

    if (!CppObj->drop()) {
        JS_ReportError(cx, "Failed to drop DB");
        return false;
    }

    return true;
}

static bool jsdb_close(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS(JSDB, &JSDB_class);

    if (!CppObj->close()) {
        JS_ReportError(cx, "Failed to drop DB");
        return false;
    }

    return true;
}

static void JSDB_Finalizer(JSFreeOp *fop, JSObject *obj)
{
    JSDB *db = static_cast<JSDB *>(JS_GetPrivate(obj));

    if (db != nullptr) {
        delete db;
    }
}
// }}}

// {{{ JSDB Implementation
JSDB::JSDB(JSContext *cx, JS::HandleObject obj, const char *path) :
    JSExposer<JSDB>(obj, cx), DB(path)
{
}

bool JSDB::set(JSContext *cx, const char *key, JS::HandleValue val)
{
    uint64_t *data;
    size_t data_len;
    bool success;

    if (!JS_WriteStructuredClone(cx, val, &data, &data_len, NULL, NULL, JS::NullHandleValue)) {
        return false;
    }

    success = DB::set(key, reinterpret_cast<uint8_t *>(data), data_len);

    JS_ClearStructuredClone(data, data_len, nullptr, nullptr);

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
        JS_STRUCTURED_CLONE_VERSION, rval, nullptr, NULL)) {

        JS_ReportError(cx, "Unable to read internal data");
        return false;
    }

    if ((void *)aligned_data != data.data()) {
        free(aligned_data);
    }

    return true;
}
// }}}

// {{{ Registration
static JSObject *registerCallback(JSContext *cx) {
    JS::RootedObject obj(cx, JS_NewObject(cx, NULL, JS::NullPtr(), JS::NullPtr()));
    JS_InitClass(cx, obj, JS::NullPtr(), &JSDB_class, jsdb_constructor,
                1, NULL, JSDB_funcs, NULL, NULL);

    JS::RootedValue val(cx);
    if (!JS_GetProperty(cx, obj, JSDB_class.name, &val)) {
        return nullptr;
    }

    JS::RootedObject ret(cx, val.toObjectOrNull());

    return ret;
}

void JSDB::RegisterObject(JSContext *cx) 
{
    JSModules::RegisterEmbedded(JSDB_class.name, registerCallback);
}
// }}}

} // namespace Binding
} // namespace Nidium

