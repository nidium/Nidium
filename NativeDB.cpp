#include "NativeDB.h"
#include <leveldb/db.h>
#include <leveldb/filter_policy.h>
#include <jsapi.h>
#include <js/StructuredClone.h>

#ifndef NATIVE_NO_PRIVATE_DIR
#  include "../interface/NativeSystemInterface.h"
#endif

NativeDB::NativeDB(const char *name) :
    m_Database(NULL), m_Status(false)
{
    if (name == NULL) {
        m_Status = false;
        return;
    }
    leveldb::Options options;
#ifndef NATIVE_NO_PRIVATE_DIR
    const char *dir = NativeSystemInterface::getInstance()->getCacheDirectory();
#else
    const char *dir = "./";
#endif
    std::string sdir(dir);
    sdir += name;

    options.create_if_missing = true;
    options.filter_policy = leveldb::NewBloomFilterPolicy(8);

    leveldb::Status status = leveldb::DB::Open(options, sdir.c_str(), &m_Database);
    m_Status = status.ok();
}

bool NativeDB::insert(const char *key, const uint8_t *data, size_t data_len)
{
    leveldb::Slice input((const char *)data, data_len);
    leveldb::Status status = m_Database->Put(leveldb::WriteOptions(), key, input);

    return status.ok();
}

bool NativeDB::insert(const char *key, const char *string)
{
    leveldb::Slice input(string);
    leveldb::Status status = m_Database->Put(leveldb::WriteOptions(), key, input);
    
    return status.ok();
}

bool NativeDB::insert(const char *key, const std::string &string)
{
    leveldb::Status status = m_Database->Put(leveldb::WriteOptions(), key, string);
    
    return status.ok();
}

bool NativeDB::insert(const char *key, JSContext *cx, JS::HandleValue val)
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

bool NativeDB::get(const char *key, std::string &ret)
{
    leveldb::Status status = m_Database->Get(leveldb::ReadOptions(), key, &ret);

    return status.ok();
}

NativeDB::~NativeDB()
{
    delete m_Database;
}
