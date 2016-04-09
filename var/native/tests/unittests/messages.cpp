/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeMessages.h>

static int dummyState = 0;

class SimpleMessages: NativeMessages{
public:
    SimpleMessages() {
        dummyState = 0;
    }
    void onMessage(const NativeSharedMessages::Message &msg) {
        dummyState = 1;
    };
    void onMessageLost(const NativeSharedMessages::Message &msg) {
        dummyState = 2;
    };
    void delMessages(int event = -1) {
        dummyState = event;

    }
};

TEST(NativeMessages, Simple)
{
    SimpleMessages *m;

    m = new SimpleMessages();

    m->onMessage(0);
    EXPECT_EQ(dummyState, 1);

    m->onMessageLost(0);
    EXPECT_EQ(dummyState, 2);

    m->delMessages();
    EXPECT_EQ(dummyState, -1);

    delete m;
}

