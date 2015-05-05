#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeDB.h>

static const char * secret = "tetetetetetetetet";

TEST(NativeDB, Simple)
{
	bool inserted, success;
	std::string ret;

	class NativeDB *db = new NativeDB("testrun");
	EXPECT_TRUE(db != NULL);
	EXPECT_TRUE(db->ok() == true);

	inserted = db->insert("picard", (const uint8_t *) secret, strlen(secret));
	EXPECT_EQ(inserted, true);
	success = 	db->get("picard", ret);
	EXPECT_TRUE(success == true);
	EXPECT_TRUE(strcmp(ret.c_str(), secret) == 0);
	
	inserted = db->insert("ricard", secret);
	EXPECT_EQ(inserted, true);
	success = 	db->get("ricard", ret);
	EXPECT_TRUE(success == true);
	EXPECT_TRUE(strcmp(ret.c_str(), secret) == 0);

	inserted = db->insert("pernod", ret);
	EXPECT_EQ(inserted, true);
	success = 	db->get("ricard", ret);
	EXPECT_TRUE(success == true);
	EXPECT_TRUE(strcmp(ret.c_str(), secret) == 0);

	delete db;
}

TEST(NativeDb, StillThere)
{
	bool success;
	std::string ret;

	class NativeDB *db = new NativeDB("testrun");
	EXPECT_TRUE(db != NULL);
	EXPECT_TRUE(db->ok() == true);

	success = 	db->get("ricard", ret);
	EXPECT_TRUE(success == true);
	EXPECT_TRUE(strcmp(ret.c_str(), secret) == 0);

	delete db;
}

