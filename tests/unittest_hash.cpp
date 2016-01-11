#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeHash.h>

TEST(NativeHash, Simple32)
{
    NativeHash<uint32_t> ht;
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

TEST(NativeHash, Simple64)
{
    uint64_t key;
    struct dummy d, *dd;
    NativeHash64<struct dummy*> ht;

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

TEST(NativeHash, Simple64Clean)
{
    uint64_t key;
    struct dummy *d, *dd;
    NativeHash64<struct dummy*> ht;

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

TEST(NativeHash, Normal)
{
    const char *  key;
    struct dummy *d, *dd;
    NativeHash<struct dummy*> ht;

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

TEST(NativeHash, Iterable)
{
    struct dummy *d;
    NativeHash<struct dummy*> ht;

    ht.setAutoDelete(true);

    d = new struct dummy;
    d->i = 1;
    ht.set("one", d);

    d = new struct dummy;
    d->i = 2;
    ht.set("two", d);
}

