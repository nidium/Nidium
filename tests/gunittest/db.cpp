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

TEST(DB, Simple)
{
    bool inserted, success;
    std::string ret;

    class Nidium::Core::DB *db = new Nidium::Core::DB("testrun");
    EXPECT_TRUE(db != NULL);
    EXPECT_TRUE(db->ok() == true);

    inserted = db->insert("picard", (const uint8_t *) secret, strlen(secret));
    EXPECT_EQ(inserted, true);
    success =     db->get("picard", ret);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(strcmp(ret.c_str(), secret) == 0);

    inserted = db->insert("ricard", secret);
    EXPECT_EQ(inserted, true);
    success =     db->get("ricard", ret);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(strcmp(ret.c_str(), secret) == 0);

    inserted = db->insert("pernod", ret);
    EXPECT_EQ(inserted, true);
    success =     db->get("ricard", ret);
    EXPECT_TRUE(success == true);
    EXPECT_TRUE(strcmp(ret.c_str(), secret) == 0);

    delete db;
}

TEST(Db, StillThere)
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

