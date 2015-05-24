#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeIStreamer.h>

TEST(NativeIStream, Simple)
{
    NativeIStreamer *i;

    i = new NativeIStreamer();

    delete i;
}

