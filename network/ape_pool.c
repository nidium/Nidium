#include "ape_pool.h"
#include <stdlib.h>

ape_pool_t *ape_new_pool(size_t size, size_t n)
{
    unsigned int i;
    ape_pool_t *pool = malloc(size * n), *current = NULL;

    for (i = 0; i < n; i++) {
        current            = ((void *)&pool[0])+(i*size);
        /* contiguous blocks */
        current->next      = (i == n-1 ? NULL : ((void *)&pool[0])+((i+1)*size));
        current->ptr.data  = NULL;
        current->flags     = (i == 0 ? APE_POOL_ALLOC : 0);
    }

    return pool;
}

ape_pool_list_t *ape_new_pool_list(size_t size, size_t n)
{
    ape_pool_list_t *list = malloc(sizeof(ape_pool_list_t));

    ape_init_pool_list(list, size, n);

    return list;
}

void ape_init_pool_list(ape_pool_list_t *list, size_t size, size_t n)
{
    ape_pool_t *pool = ape_new_pool(size, n);

    list->head  = pool;
    list->current   = pool;
    list->queue     = ((void *)&pool[0])+((n-1)*size);
}

ape_pool_t *ape_pool_head_to_queue(ape_pool_list_t *list)
{
    ape_pool_t *head = list->head;

    list->head = head->next;
    list->queue->next = head;
    list->queue = head;
    head->next = NULL;

    return list->head;
}

ape_pool_t *ape_pool_head_to_current(ape_pool_list_t *list)
{
    ape_pool_t *head = list->head;

    if (head == list->current) {
        return head;
    }

    list->head = head->next;
    head->next = list->current->next;
    list->current->next = head;
    list->current = head;

    if (head->next == NULL) {
        list->queue = head;
    }

    return list->head;

}

ape_pool_t *ape_grow_pool(ape_pool_list_t *list, size_t size, size_t n)
{
    ape_pool_t *pool;

    pool = ape_new_pool(size, n);
    list->queue->next = pool;
    list->queue = ((void *)&pool[0])+((n-1)*size);

    return pool;
}

void ape_destroy_pool_ordered(ape_pool_t *pool, ape_pool_clean_callback cleaner)
{
    ape_pool_t *tPool = NULL;

    while (pool != NULL) {
        /* TODO : callback ? (cleaner) */
        if (pool->flags & APE_POOL_ALLOC) {

            if (tPool != NULL) {
                if (cleaner != NULL) {
                    cleaner(tPool);
                }
                free(tPool);
            }
            tPool = pool;
        }
        pool = pool->next;
    }
    if (tPool != NULL) {
        if (cleaner != NULL) {
            cleaner(tPool);
        }
        free(tPool);
    }
}

void ape_destroy_pool(ape_pool_t *pool)
{
    ape_pool_t *tPool = NULL, *fPool = NULL;

    while (pool != NULL) {
        /* TODO : callback ? (cleaner) */
        if (pool->flags & APE_POOL_ALLOC) {
            if (fPool == NULL) {
                fPool = pool;
            }
            if (tPool != NULL) {
                fPool->next = pool->next;
                pool->next = tPool;
                tPool = pool;
                pool = fPool->next;
                continue;
            }
            tPool = pool;
        }
        pool = pool->next;
    }

    fPool->next = NULL;
    pool = tPool;

    while (pool != NULL && pool->flags & APE_POOL_ALLOC) {
        tPool = pool->next;
        free(pool);
        pool = tPool;
    }
}

void ape_destroy_pool_list(ape_pool_list_t *list)
{
    ape_destroy_pool(list->head);
    free(list);
}

void ape_destroy_pool_list_ordered(ape_pool_list_t *list,
            ape_pool_clean_callback cleaner)
{
    ape_destroy_pool_ordered(list->head, cleaner);
    free(list);
}

// vim: ts=4 sts=4 sw=4 et

