/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeNFSStream.h>

TEST(NativeNFSStream, Simple)
{
    NativeNFSStream nfs("nfs://127.0.0.1:/tmp/tst.txt");
    EXPECT_TRUE(nfs.getBaseDir() == NULL);
    EXPECT_TRUE(nfs.allowSyncStream() == true);
    EXPECT_TRUE(nfs.allowLocalFileStream() == true);
}

