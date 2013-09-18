#ifndef nativedb_h__
#define nativedb_h__

#include <stdint.h>
#include <stdlib.h>

namespace leveldb {
    class DB;
};

class NativeDB
{
    public:
        NativeDB(const char *name);
        ~NativeDB();

        bool ok() const {
            return m_Status;
        }
    private:
        leveldb::DB *m_Database;
        bool m_Status;
};

#endif