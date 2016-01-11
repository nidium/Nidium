#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeFileStream.h>

TEST(NativeFileStream, Simple)
{
    NativeFileStream fs("file:///tmp/tst.txt");
    EXPECT_TRUE(fs.getBaseDir() == NULL);
    EXPECT_TRUE(fs.allowSyncStream() == true);
    EXPECT_TRUE(fs.allowLocalFileStream() == true);
}

