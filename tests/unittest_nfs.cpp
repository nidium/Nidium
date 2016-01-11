#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeNFS.h>


TEST(NativeNFS, Simple)
{
    class NativeNFS * nfs = new NativeNFS();
    EXPECT_TRUE(nfs != NULL);
    EXPECT_TRUE(nfs->validateArchive() == false);

    delete nfs;
}

