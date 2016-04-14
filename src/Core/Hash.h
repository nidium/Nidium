/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativehash_h__
#define nativehash_h__

#include <stdlib.h>
#include <string.h>
#include <iterator>

#include <ape_hash.h>

#include "NativeUtils.h"

/*
    C++ wrapper to ape_hash
*/

namespace Nidium {
namespace Core {

template <typename T>
class Hash64 : public NativeNoncopyable
{
    public:
        explicit Hash64(int size = 0)
        {
            if (!size) {
                this->table = hashtbl_init(APE_HASH_INT);
            } else {
                this->table = hashtbl_init_with_size(APE_HASH_INT, size);
            }
        }
        ~Hash64() {
            hashtbl_free(this->table);
        }

        void set(uint64_t key, T val) {
            hashtbl_append64(this->table, key, val);
        }

        T get(uint64_t key) const {
            return (T)hashtbl_seek64(this->table, key);
        }

        void erase(uint64_t key) {
            hashtbl_erase64(this->table, key);
        }

        void setAutoDelete(bool val) {
            hashtbl_set_cleaner(this->table, (val ? Hash64<T>::cleaner : NULL));
        }
        static void cleaner(ape_htable_item_t *item) {
            delete (T)item->content.addrs;
        }

        struct _ape_htable *accessCStruct() const {
            return table;
        }

    private:

        struct _ape_htable *table;
};

template <typename T>
class Hash : public NativeNoncopyable
{
    public:
        Hash(int size = 0)
        {
           if (!size) {
                this->table = hashtbl_init(APE_HASH_STR);
            } else {
                this->table = hashtbl_init_with_size(APE_HASH_STR, size);
            }
        }
        ~Hash() {
            hashtbl_free(this->table);
        }

        void set(const char *key, T val) {
            hashtbl_append(this->table, key, strlen(key), val);
        }

        T get(const char *key) const {
            return (T)hashtbl_seek(this->table, key, strlen(key));
        }

        void erase(const char *key) {
            hashtbl_erase(this->table, key, strlen(key));
        }

        void setAutoDelete(bool val) {
            hashtbl_set_cleaner(this->table, (val ? Hash<T>::cleaner : NULL));
        }
        static void cleaner(ape_htable_item_t *item) {
            delete (T)item->content.addrs;
        }
        struct _ape_htable *accessCStruct() const {
            return table;
        }

        class iterator: public std::iterator<std::input_iterator_tag, T>
        {
            public:
                iterator(ape_htable_item_t *ptr): ptr(ptr) {}
                iterator& operator++() {
                    ptr = ptr->lnext;
                    return *this;
                }
                bool operator != (iterator& other) {
                    return  ptr != other.ptr;
                }
                T operator->() {
                    return (T)ptr->content.addrs;
                }
            private:
                ape_htable_item_t *ptr;
        };

        iterator begin()
        {
            if (!table->first) {
                return iterator(NULL);
            } else {
                return iterator(table->first);
            }
        }

        iterator end()
        {
            return iterator(NULL);
        }
    private:

        struct _ape_htable *table;
};

template <>
class Hash<uint32_t> : public NativeNoncopyable
{
    public:
        Hash() {
            this->table = hashtbl_init(APE_HASH_STR);
        }
        ~Hash() {
            hashtbl_free(this->table);
        }

        void set(const char *key, uint32_t val) {
            hashtbl_append_val32(this->table, key, strlen(key), val);
        }

        uint32_t get(const char *key) const {
            return hashtbl_seek_val32(this->table, key, strlen(key));
        }

        void erase(const char *key) {
            hashtbl_erase(this->table, key, strlen(key));
        }
    private:
        struct _ape_htable *table;
};

} // namespace Binding
} // namespace Nidium

#endif

