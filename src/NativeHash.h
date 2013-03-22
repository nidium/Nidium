#ifndef nativehash_h__
#define nativehash_h__

#include <ape_hash.h>
#include <stdlib.h>
#include <string.h>

/*
    C++ wrapper to ape_hash
*/

template <typename T>
class NativeHash
{
    public:
        NativeHash() {
            this->table = hashtbl_init(APE_HASH_STR);
        }
        ~NativeHash() {
            hashtbl_free(this->table);
        }

        void set(const char *key, T val) {
            hashtbl_append(this->table, key, strlen(key), val);
        }

        T get(const char *key) const {
            return hashtbl_seek(this->table, key, strlen(key));
        }

        void erase(const char *key) {
            hashtbl_erase(this->table, key, strlen(key));
        }
    private:
        struct _ape_htable *table;
};

#endif
