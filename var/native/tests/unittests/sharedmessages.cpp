/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <Core/NativeSharedMessages.h>

TEST(NativeSharedMessages, Simple)
{
    NativeSharedMessages *m;

    m = new NativeSharedMessages();
    delete m;
}

struct dummy{
    int x;
};

TEST(NativeSharedMessages, Message)
{
    struct dummy d;
    NativeSharedMessages::Message message(&d, 15);

    EXPECT_TRUE(message.dataPtr() == &d);
    EXPECT_EQ(message.event(), 15);

    message.setDest(&ape_running);
    EXPECT_EQ(message.dest(), &ape_running);
    EXPECT_TRUE(message.prev == NULL);
}

TEST(NativeSharedMessages, constuctorMessage)
{
    NativeSharedMessages::Message message(15);

    EXPECT_EQ(message.event(), 15);
    EXPECT_TRUE(message.dest() == NULL);
    EXPECT_TRUE(message.prev == NULL);
}

TEST(NativeSharedMessages, constuctorIntMessage)
{
    struct dummy d;
    NativeSharedMessages::Message message(12, 15, &d);

    EXPECT_EQ(message.dataUInt(), 12);
    EXPECT_EQ(message.event(), 15);
    EXPECT_TRUE(message.dest() == &d);
    EXPECT_TRUE(message.prev == NULL);

}

