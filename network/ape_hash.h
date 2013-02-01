/*
  Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012  Anthony Catel <a.catel@weelya.com>

  This file is part of APE Server.
  APE is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  APE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with APE ; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef _APE_HASH_H
#define _APE_HASH_H

#include <stdint.h>

#define HACH_TABLE_MAX 8192

typedef enum {
    APE_HASH_STR,
    APE_HASH_INT
} ape_hash_type;

typedef struct _ape_htable
{
    struct _ape_htable_item *first;
    struct _ape_htable_item **table;
    
    ape_hash_type type;
    
} ape_htable_t;


typedef struct _ape_htable_item
{
    union {
        char *str;
        uint64_t integer;
    } key;
    
    void *addrs;
    struct _ape_htable_item *next;
    
    struct _ape_htable_item *lnext;
    struct _ape_htable_item *lprev;
    
} ape_htable_item_t;

#ifdef __cplusplus
extern "C" {
#endif

ape_htable_t *hashtbl_init(ape_hash_type type);

void hashtbl_free(ape_htable_t *htbl);
void *hashtbl_seek64(ape_htable_t *htbl, uint64_t key);
void hashtbl_erase64(ape_htable_t *htbl, uint64_t key);
void hashtbl_append64(ape_htable_t *htbl, uint64_t key, void *structaddr);

#ifdef __cplusplus
}
#endif

#endif

// vim: ts=4 sts=4 sw=4 et

