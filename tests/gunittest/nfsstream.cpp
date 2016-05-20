/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <IO/NFSStream.h>

TEST(NFSStream, Simple)
{
    Nidium::IO::NFSStream nfs("nfs://127.0.0.1:/tmp/tst.txt");

    ASSERT_STREQ(nfs.GetBaseDir(), "/");
    EXPECT_TRUE(nfs.AllowSyncStream() == true);
    EXPECT_TRUE(nfs.AllowLocalFileStream() == true);
}

