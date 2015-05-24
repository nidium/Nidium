#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "unittest.h"

#include <NativeArgs.h>


TEST(NativeArgs, Simple)
{
    int i, max;
    NativeArgs args;

    i = max = 99;
    EXPECT_EQ(args[0].isSet(), false);
    args[0].set((void*)&i);
    EXPECT_EQ(args[0].isSet(), true);

    max = args.size() + 2;
    EXPECT_EQ(args.size(), 8);
    for (i = 1; i < max; i++) {
        EXPECT_EQ(args[i].isSet(), false);
#if 0
        EXPECT_TRUE(args[i].toPtr() == NULL);
        EXPECT_EQ(args[i].toInt64(), 0);
        EXPECT_EQ(args[i].toBool(), false);
#endif
        args[i].set(max);
        EXPECT_EQ(args[i].isSet(), true);
        EXPECT_EQ(args[i].toInt(), (uint32_t)max);
    }

    EXPECT_EQ(args.size(), 10);
}

