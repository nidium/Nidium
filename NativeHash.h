/*
    NativeJS Core Library
    Copyright (C) 2013 Anthony Catel <paraboul@gmail.com>
    Copyright (C) 2013 Nicolas Trani <n.trani@weelya.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef nativehash_h__
#define nativehash_h__

#include <ape_hash.h>
#include <stdlib.h>
#include <string.h>
#include "NativeUtils.h"


/*
    C++ wrapper to ape_hash
*/

template <typename T>
class NativeHash64 : public NativeNoncopyable
{
    public:
        explicit NativeHash64(int size = 0) :
            m_AutoDelete(false)
        {
            if (!size) {
                this->table = hashtbl_init(APE_HASH_INT);
            } else {
                this->table = hashtbl_init_with_size(APE_HASH_INT, size);
            }
        }
        ~NativeHash64() {
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
            hashtbl_set_cleaner(this->table, (val ? NativeHash64<T>::cleaner : NULL));
        }
        static void cleaner(ape_htable_item_t *item) {
            delete (T)item->content.addrs;
        }

        struct _ape_htable *accessCStruct() const {
            return table;
        }

    private:

        struct _ape_htable *table;
        bool m_AutoDelete;
};
   

template <typename T>
class NativeHash : public NativeNoncopyable
{
    public:
        NativeHash() :
            m_AutoDelete(false)
        {
            this->table = hashtbl_init(APE_HASH_STR);
        }
        ~NativeHash() {
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
            hashtbl_set_cleaner(this->table, (val ? NativeHash<T>::cleaner : NULL));
        }
        static void cleaner(ape_htable_item_t *item) {
            delete (T)item->content.addrs;
        }
    private:

        struct _ape_htable *table;
        bool m_AutoDelete;
};

template <>
class NativeHash<uint32_t> : public NativeNoncopyable
{
    public:
        NativeHash() {
            this->table = hashtbl_init(APE_HASH_STR);
        }
        ~NativeHash() {
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

#endif
