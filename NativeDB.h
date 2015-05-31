#ifndef nativedb_h__
#define nativedb_h__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <jsapi.h>

namespace leveldb {
    class DB;
};


class JSObject;
struct JSContext;

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

        /*
            Caller is responsible for knowing how to
            decode the data during a get()
        */
        bool insert(const char *key, JSContext *cx, JS::HandleValue val);

        bool get(const char *key, std::string &ret);
    private:
        leveldb::DB *m_Database;
        bool m_Status;
};

#endif
