/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef core_hash_h__
#define core_hash_h__

#include <stdlib.h>
#include <string.h>
#include <iterator>

#include <ape_hash.h>

#include "Core/Utils.h"

/*
    C++ wrapper to ape_hash
*/

namespace Nidium {
namespace Core {

// {{{ Hash64
template <typename T>
class Hash64 : public NonCopyable
{
    public:
        explicit Hash64(int size = 0)
        {
            if (!size) {
                this->m_table = hashtbl_init(APE_HASH_INT);
            } else {
                this->m_table = hashtbl_init_with_size(APE_HASH_INT, size);
            }
        }
        ~Hash64() {
            hashtbl_free(this->m_table);
        }

        void set(uint64_t key, T val) {
            hashtbl_append64(this->m_table, key, val);
        }

        T get(uint64_t key) const {
            return static_cast<T>(hashtbl_seek64(this->m_table, key));
        }

        void erase(uint64_t key) {
            hashtbl_erase64(this->m_table, key);
        }

        void setAutoDelete(bool val) {
            hashtbl_set_cleaner(this->m_table, (val ? Hash64<T>::Cleaner : NULL));
        }
        static void Cleaner(ape_htable_item_t *item) {
            delete static_cast<T>(item->content.addrs);
        }

        struct _ape_htable *accessCStruct() const {
            return m_table;
        }

    private:

        struct _ape_htable *m_table;
};
// }}}

// {{{ Hash
template <typename T>
class Hash : public NonCopyable
{
    public:
        Hash(int size = 0)
        {
           if (!size) {
                this->m_table = hashtbl_init(APE_HASH_STR);
            } else {
                this->m_table = hashtbl_init_with_size(APE_HASH_STR, size);
            }
        }
        ~Hash() {
            hashtbl_free(this->m_table);
        }

        void set(const char *key, T val) {
            hashtbl_append(this->m_table, key, strlen(key), val);
        }

        T get(const char *key) const {
            return static_cast<T>(hashtbl_seek(this->m_table, key, strlen(key)));
        }

        void erase(const char *key) {
            hashtbl_erase(this->m_table, key, strlen(key));
        }

        void setAutoDelete(bool val) {
            hashtbl_set_cleaner(this->m_table, (val ? Hash<T>::Cleaner : NULL));
        }
        static void Cleaner(ape_htable_item_t *item) {
            delete static_cast<T>(item->content.addrs);
        }
        struct _ape_htable *accessCStruct() const {
            return this->m_table;
        }

        class iterator: public std::iterator<std::input_iterator_tag, T>
        {
            public:
                iterator(ape_htable_item_t *ptr): m_ptr(ptr) {}
                iterator& operator++() {
                    m_ptr = m_ptr->lnext;
                    return *this;
                }
                bool operator != (iterator& other) {
                    return  m_ptr != other.m_ptr;
                }
                T operator->() {
                    return static_cast<T>(m_ptr->content.addrs);
                }
            private:
                ape_htable_item_t *m_ptr;
        };

        iterator begin()
        {
            if (!m_table->first) {
                return iterator(NULL);
            } else {
                return iterator(m_table->first);
            }
        }

        iterator end()
        {
            return iterator(NULL);
        }
    private:

        struct _ape_htable *m_table;
};
// }}}

// {{{ Hash<uint32_t
template <>
class Hash<uint32_t> : public NonCopyable
{
    public:
        Hash() {
            this->m_table = hashtbl_init(APE_HASH_STR);
        }
        ~Hash() {
            hashtbl_free(this->m_table);
        }

        void set(const char *key, uint32_t val) {
            hashtbl_append_val32(this->m_table, key, strlen(key), val);
        }

        uint32_t get(const char *key) const {
            return hashtbl_seek_val32(this->m_table, key, strlen(key));
        }

        void erase(const char *key) {
            hashtbl_erase(this->m_table, key, strlen(key));
        }
    private:
        struct _ape_htable *m_table;
};
// }}}

} // namespace Core
} // namespace Nidium

#endif

