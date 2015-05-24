#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeSharedMessages.h>

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
    //@FIXME: EXPECT_EQ(message.priv, 0);
    //@TODO args
}

TEST(NativeSharedMessages, constuctorMessage)
{
    NativeSharedMessages::Message message(15);

    //@FIXME: EXPECT_TRUE(message.dataPtr() == NULL);
    EXPECT_EQ(message.event(), 15);
    EXPECT_TRUE(message.dest() == NULL);
    EXPECT_TRUE(message.prev == NULL);
    //@FIXME: EXPECT_EQ(message.priv, 0);
    //@TODO args
}

TEST(NativeSharedMessages, constuctorIntMessage)
{
    struct dummy d;
    NativeSharedMessages::Message message(12, 15, &d);

    EXPECT_EQ(message.dataUInt(), 12);
    EXPECT_EQ(message.event(), 15);
    EXPECT_TRUE(message.dest() == &d);
    EXPECT_TRUE(message.prev == NULL);
    //@FIXME: EXPECT_EQ(message.priv, 12);
    //@TODO args

}
//@TODO: void postMessage(Message *msg);
//@TODO: void postMessage(void *dataptr, int event);
//@TODO: void postMessage(uint64_t dataint, int event);
//@TODO: Message *readMessage();
//@TODO: Message *readMessage(int ev);
//@TODO: void delMessagesForDest(void *dest, int event = -1);
//@TODO: void setCleaner(native_shared_message_cleaner cleaner)
//@TODO: int hasPendingMessages()

