/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <Core/DB.h>

static const char * secret = "tetetetetetetetet";

TEST(DB, SetAndGet)
{
    bool inserted, success;
    std::string ret;

    class Nidium::Core::DB *db = new Nidium::Core::DB("testrun");
    EXPECT_TRUE(db != NULL);
    EXPECT_TRUE(db->ok() == true);

    inserted = db->set("picard", (const uint8_t *) secret, strlen(secret));
    EXPECT_EQ(inserted, true);

    success =     db->get("picard", ret);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(strcmp(ret.c_str(), secret) == 0);

    inserted = db->set("ricard", secret);
    EXPECT_EQ(inserted, true);

    success =     db->get("ricard", ret);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(strcmp(ret.c_str(), secret) == 0);

    inserted = db->set("pernod", ret);
    EXPECT_EQ(inserted, true);

    success =     db->get("ricard", ret);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(strcmp(ret.c_str(), secret) == 0);

    delete db;
}

TEST(DB, StillThere)
{
    bool success;
    std::string ret;

    class Nidium::Core::DB *db = new Nidium::Core::DB("testrun");
    EXPECT_TRUE(db != NULL);
    EXPECT_TRUE(db->ok() == true);

    success =     db->get("ricard", ret);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(strcmp(ret.c_str(), secret) == 0);

    delete db;
}

TEST(DB, Del)
{
    class Nidium::Core::DB *db = new Nidium::Core::DB("testrun");

    db->del("ricard");

    std::string ret;

    db->get("ricard", ret);
    EXPECT_TRUE(ret.length() == 0);

    printf("del=%s\n", ret.c_str());

    delete db;
}

TEST(DB, Cleanup)
{
    std::string ret;
    class Nidium::Core::DB *db = new Nidium::Core::DB("testrun");

    db->drop();

    EXPECT_FALSE(db->set("foo", "bar"));
    EXPECT_FALSE(db->get("foo", ret));

    delete db;
}
