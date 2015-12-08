#ifndef nativedb_h__
#define nativedb_h__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>

namespace leveldb {
    class DB;
};

class NativeDB
{
    public:
        explicit NativeDB(const char *name);
        ~NativeDB();

        /*
            Check status after the constructor is caller
        */
        bool ok() const {
            return m_Status;
        }

        bool insert(const char *key, const uint8_t *data, size_t data_len);
        bool insert(const char *key, const char *string);
        bool insert(const char *key, const std::string &string);
        bool get(const char *key, std::string &ret);
    protected:
        leveldb::DB *m_Database;
    private:
        bool m_Status;
};

#endif

