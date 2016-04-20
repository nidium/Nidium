/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <IO/FileStream.h>

TEST(FileStream, Simple)
{
    Nidium::IO::FileStream fs("file:///tmp/tst.txt");
    EXPECT_TRUE(fs.getBaseDir() == NULL);
    EXPECT_TRUE(fs.allowSyncStream() == true);
    EXPECT_TRUE(fs.allowLocalFileStream() == true);
}

