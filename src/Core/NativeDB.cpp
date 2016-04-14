/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "NativeDB.h"

#include <stdbool.h>

#include <leveldb/db.h>
#include <leveldb/filter_policy.h>

#include "NativePath.h"

#include "NativePath.h"

#ifndef NATIVE_NO_PRIVATE_DIR
#  include "../interface/NativeSystemInterface.h"
#endif


NativeDB::NativeDB(const char *name) :
    m_Database(NULL), m_Status(false)
{
    char * chdir;
    bool found;
    size_t i;

    if (name == NULL) {
        m_Status = false;
        return;
    }
    leveldb::Options options;
#ifndef NATIVE_NO_PRIVATE_DIR
    std::string sdir(NativeSystemInterface::getInstance()->getCacheDirectory());
#else
    std::string sdir("./");
#endif
    /*
     change dots to slashes, to avoid a cluttered directory structure
    */
    sdir += name;
    chdir = strdup(sdir.c_str());
    found = false;
    for (i = 0; i < strlen(chdir); i++) {
        if (chdir[i] == '.' && found) {
            chdir[i] = '/';
        } else {
            found = true;
        }
    }
    NativePath::makedirs(chdir);
    options.create_if_missing = true;
    options.filter_policy = leveldb::NewBloomFilterPolicy(8);

    leveldb::Status status = leveldb::DB::Open(options, chdir, &m_Database);
    m_Status = status.ok();
    free(chdir);
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

bool NativeDB::get(const char *key, std::string &ret)
{
    leveldb::Status status = m_Database->Get(leveldb::ReadOptions(), key, &ret);

    return status.ok();
}

NativeDB::~NativeDB()
{
    delete m_Database;
}

