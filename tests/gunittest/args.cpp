/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "unittest.h"

#include <Core/Args.h>

#define MAX_ARGS_SIZE 10

TEST(Args, Simple)
{
    Nidium::Core::Args args;

    EXPECT_EQ(args.size(), 0);

    for (int i = 0; i < MAX_ARGS_SIZE; i++) {
        // Test for default values
        EXPECT_EQ(args[i].isSet(), false);
        EXPECT_EQ(args[i].toPtr(), nullptr);
        EXPECT_EQ(args[i].toInt64(), 0);
        EXPECT_EQ(args[i].toInt(), 0);
        EXPECT_EQ(args[i].toBool(), false);

        // Test assignation
        args[i].set(i);
        EXPECT_EQ(args[i].isSet(), true);
        EXPECT_EQ(args[i].toInt(), i);

        EXPECT_EQ(args.size(), i + 1);
    }

}

TEST(Args, OverflowDeath)
{
    
    Nidium::Core::Args args;

    EXPECT_DEATH_IF_SUPPORTED(args[MAX_ARGS_SIZE + 1].isSet(), "");
}
