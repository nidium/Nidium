//#include "common.h"
#include "ape_array.h"
#include <string.h>

#ifdef _MSC_VER
#define strncasecmp _strnicmp
#endif

static void ape_array_clean_cb(ape_pool_t *item);

ape_array_t *ape_array_new(size_t n)
{
    ape_array_t *array;
    array = (ape_array_t *)ape_new_pool_list(sizeof(ape_array_item_t), n);
    
    return array;
}

ape_array_item_t *ape_array_lookup_item(ape_array_t *array,
        const char *key, int klen)
{
    buffer *k, *v;
    APE_A_FOREACH(array, k, v) {
        if (k->used == klen && memcmp(key, k->data, klen) == 0) {
            return __array_item;
        }
    }

    return NULL;
}

buffer *ape_array_lookup(ape_array_t *array, const char *key, int klen)
{
    buffer *k, *v;
    APE_A_FOREACH(array, k, v) {
        if (k->used == klen && strncasecmp(key, (const char *)k->data, klen) == 0) {
            return v;
        }
    }

    return NULL;
}

void *ape_array_lookup_data(ape_array_t *array, const char *key, int klen)
{
    buffer *k, *v;
    APE_A_FOREACH(array, k, v) {
        if (k->used == klen && strncasecmp(key, (const char *)k->data, klen) == 0) {
            return __array_item->pool.ptr.data;
        }
    }

    return NULL;
}

void ape_array_delete(ape_array_t *array, const char *key, int klen)
{
    ape_array_item_t *item = ape_array_lookup_item(array, key, klen);

    if (item != NULL) {
        item->pool.flags &= ~APE_ARRAY_USED_SLOT;
        buffer_destroy(item->key);

        switch(item->pool.flags & ~APE_POOL_ALL_FLAGS) {
            case APE_ARRAY_VAL_BUF:
                buffer_destroy(item->pool.ptr.buf);
                item->pool.flags &= ~APE_ARRAY_VAL_BUF;
                break;
            case APE_ARRAY_VAL_INT:
                break;
            default:
                break;
        }

        array->current = (ape_pool_t *)item;
    }
}

static ape_array_item_t *ape_array_add_s(ape_array_t *array, buffer *key)
{
    ape_array_item_t *slot;

    ape_array_delete(array, (const char *)key->data, key->used);

    slot = (ape_array_item_t *)array->current;

    if (slot == NULL || slot->pool.flags & APE_ARRAY_USED_SLOT) {
        slot = (ape_array_item_t *)ape_grow_pool(array,
                sizeof(ape_array_item_t), 4);
    }

    slot->pool.flags |= APE_ARRAY_USED_SLOT;

    slot->key = key;

    array->current = slot->pool.next;

    if (array->current == NULL ||
        ((ape_array_item_t *)array->current)->pool.flags &
        APE_ARRAY_USED_SLOT) {

        array->current = array->head;

        while (array->current != NULL &&
                ((ape_array_item_t *)array->current)->pool.flags &
                APE_ARRAY_USED_SLOT) {
            array->current = ((ape_array_item_t *)array->current)->pool.next;
        }
    }

    return slot;
}

void ape_array_add_b(ape_array_t *array, buffer *key, buffer *value)
{
    ape_array_item_t *slot = ape_array_add_s(array, key);

    slot->pool.flags |= APE_ARRAY_VAL_BUF;
    slot->pool.ptr.buf = value;
}

ape_array_item_t *ape_array_add_ptr(ape_array_t *array, buffer *key, void *ptr)
{
    ape_array_item_t *slot = ape_array_add_s(array, key);

    slot->pool.ptr.data = ptr;
    
    return slot;
}

ape_array_item_t *ape_array_add_ptrn(ape_array_t *array, const char *key,
        int klen, void *ptr)
{
    buffer *k;
    k = buffer_new(klen+1);

    buffer_append_string_n(k, key, klen);

    return ape_array_add_ptr(array, k, ptr);
}

void ape_array_add_n(ape_array_t *array, const char *key,
        int klen, const char *value, int vlen)
{
    buffer *k, *v;

    k = buffer_new(klen+1);
    v = buffer_new(vlen+1);

    buffer_append_string_n(k, key, klen);
    buffer_append_string_n(v, value, vlen);

    ape_array_add_b(array, k, v);
}

void ape_array_add(ape_array_t *array, const char *key, const char *value)
{
    ape_array_add_n(array, key, strlen(key), value, strlen(value));
}

void ape_array_destroy(ape_array_t *array)
{
    ape_destroy_pool_list_ordered((ape_pool_list_t *)array, ape_array_clean_cb);
}

static void ape_array_clean_cb(ape_pool_t *item)
{
    ape_array_item_t *array = (ape_array_item_t *)item;

    if (!(array->pool.flags & APE_ARRAY_USED_SLOT)) {
        return;
    }
    array->pool.flags &= ~APE_ARRAY_USED_SLOT;

    buffer_destroy(array->key);
    
    switch(array->pool.flags & ~APE_POOL_ALL_FLAGS) {
        case APE_ARRAY_VAL_BUF:
            buffer_destroy(array->pool.ptr.buf);
            break;
        case APE_ARRAY_VAL_INT:
        default:
            break;
    }
}

// vim: ts=4 sts=4 sw=4 et
