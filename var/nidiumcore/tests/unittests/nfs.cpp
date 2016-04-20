/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <IO/NFS.h>


TEST(NFS, Simple)
{
    class Nidium::IO::NFS * nfs = new Nidium::IO::NFS();
    EXPECT_TRUE(nfs != NULL);
    EXPECT_TRUE(nfs->validateArchive() == false);

    delete nfs;
}

