/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Core/DB.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <leveldb/db.h>
#include <leveldb/filter_policy.h>

#include "Core/Path.h"

namespace Nidium {
namespace Core {

DB::DB(const char *name) :
    m_Database(NULL), m_Status(false), m_Name(strdup(name))
{

    if (name == NULL) {
        m_Status = false;
        return;
    }

    leveldb::Options options;
    options.create_if_missing = true;
    options.filter_policy = leveldb::NewBloomFilterPolicy(8);

    leveldb::Status status = leveldb::DB::Open(options, name, &m_Database);

    m_Status = status.ok();
}

bool DB::set(const char *key, const uint8_t *data, size_t data_len)
{
    if (m_Database == nullptr) {
        return false;
    }

    leveldb::Slice input(reinterpret_cast<const char *>(data), data_len);
    leveldb::Status status = m_Database->Put(leveldb::WriteOptions(), key, input);

    return status.ok();
}

bool DB::set(const char *key, const char *string)
{
    if (m_Database == nullptr) {
        return false;
    }

    leveldb::Slice input(string);
    leveldb::Status status = m_Database->Put(leveldb::WriteOptions(), key, input);

    return status.ok();
}

bool DB::set(const char *key, const std::string &string)
{
    if (m_Database == nullptr) {
        return false;
    }

    leveldb::Status status = m_Database->Put(leveldb::WriteOptions(), key, string);

    return status.ok();
}

bool DB::get(const char *key, std::string &ret)
{
    if (m_Database == nullptr) {
        return false;
    }

    leveldb::Status status = m_Database->Get(leveldb::ReadOptions(), key, &ret);

    return status.ok();
}

bool DB::del(const char *key)
{
    if (m_Database == nullptr) {
        return false;
    }

    leveldb::Status status = m_Database->Delete(leveldb::WriteOptions(), key);

    return status.ok();
}

bool DB::drop()
{
    this->close();

    leveldb::DestroyDB(m_Name, leveldb::Options());

    return true;
}

bool DB::close()
{
    delete m_Database;
    m_Database = nullptr;

    return true;
}

DB::~DB()
{
    delete m_Database;
    free(m_Name);
}

} // namespace Core
} // namespace Nidium

