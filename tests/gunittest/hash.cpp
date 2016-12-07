/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <Core/Hash.h>

TEST(Hash, Simple32)
{
    Nidium::Core::Hash<uint32_t> ht;
    uint32_t value;

    value = ht.get("none");
    EXPECT_EQ(value, 0);
    ht.set("none", 12);
    value = ht.get("none");
    EXPECT_EQ(value, 12);
    ht.erase("none");
    value = ht.get("none");
    EXPECT_EQ(value, 0);
}

struct dummy{
    int i;
    };

TEST(Hash, Simple64)
{
    uint64_t key;
    struct dummy d, *dd;
    Nidium::Core::Hash64<struct dummy*> ht;

    ht.setAutoDelete(false);
    key = 123456;
    d.i = key;

    dd  = (struct dummy*)ht.get(key);
    EXPECT_TRUE(dd == NULL);
    ht.set(key, &d);
    dd  = (struct dummy*) ht.get(key);
    EXPECT_EQ(dd->i, d.i);
    ht.erase(key);
    dd  = (struct dummy*)ht.get(key);
    EXPECT_TRUE(dd == NULL);
}

TEST(Hash, Simple64Clean)
{
    uint64_t key;
    struct dummy *d, *dd;
    Nidium::Core::Hash64<struct dummy*> ht;

    ht.setAutoDelete(true);
    key = 123456;
    d = new struct dummy;
    d->i = key;

    dd  = (struct dummy*)ht.get(key);
    EXPECT_TRUE(dd == NULL);
    ht.set(key, d);
    dd  = (struct dummy*) ht.get(key);
    EXPECT_EQ(dd->i, d->i);
    ht.erase(key);
    dd  = (struct dummy*)ht.get(key);
    EXPECT_TRUE(dd == NULL);

}

TEST(Hash, Normal)
{
    const char *  key;
    struct dummy *d, *dd;
    Nidium::Core::Hash<struct dummy*> ht;

    ht.setAutoDelete(true);
    key = strdup("123456");
    d = new struct dummy;
    d->i = 123;

    dd  = (struct dummy*)ht.get(key);
    EXPECT_TRUE(dd == NULL);
    ht.set(key, d);
    dd  = (struct dummy*) ht.get(key);
    EXPECT_EQ(dd->i, d->i);
    ht.erase(key);
    dd  = (struct dummy*)ht.get(key);
    EXPECT_TRUE(dd == NULL);

    free((char*)key);
}

TEST(Hash, Iterable)
{
    struct dummy *d;
    Nidium::Core::Hash<struct dummy*> ht;

    ht.setAutoDelete(true);

    d = new struct dummy;
    d->i = 1;
    ht.set("one", d);

    d = new struct dummy;
    d->i = 2;
    ht.set("two", d);
}

