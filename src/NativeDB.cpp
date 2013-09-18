#include "NativeDB.h"
#include "leveldb/db.h"
#include "leveldb/filter_policy.h"
#include "NativeSystemInterface.h"

NativeDB::NativeDB(const char *name) :
    m_Database(NULL), m_Status(false)
{
    leveldb::Options options;

    const char *dir = NativeSystemInterface::getInstance()->getCacheDirectory();
    std::string sdir(dir);
    sdir += name;

    options.create_if_missing = true;
    options.filter_policy = leveldb::NewBloomFilterPolicy(8);

    leveldb::Status status = leveldb::DB::Open(options, sdir.c_str(), &m_Database);
    m_Status = status.ok();

    printf("Creating DB at : %s\n", sdir.c_str());
}

NativeDB::~NativeDB()
{
    delete m_Database;
}
