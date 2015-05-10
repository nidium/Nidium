#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeMessages.h>

static int dummyState = 0;

class SimpleMessages: NativeMessages{
public:
	SimpleMessages(){
		dummyState = 0;
	}
	void onMessage(const NativeSharedMessages::Message &msg){
		dummyState = 1;
	};
	void onMessageLost(const NativeSharedMessages::Message &msg){
		dummyState = 2;
	};
	void delMessages(int event = -1) {
		dummyState = event;
		
	}
};

#if 0
TEST(NativeMessages, Simple)
{
	SimpleMessages *m;
	
	m = new SimpleMessages();

	//@FIXME:  segfault
	m->onMessage(NULL);
	EXPECT_EQ(dummyState, 1);
	
	m->onMessageLost(NULL);
	EXPECT_EQ(dummyState, 2);

	m->delMessages();
	EXPECT_EQ(dummyState, -1);

//@TODO: void postMessage(void *dataptr, int event, bool forceAsync = false);
//@TODO: void postMessage(uint64_t dataint, int event, bool forceAsync = false);
//@TODO: void postMessage(NativeSharedMessages::Message *msg, bool forceAsync = false);
//@TODO: void delMessages(int event = -1);
//@TODO: static void initReader(ape_global *ape);
//@TODO: static void destroyReader();
//@TODO: NativeSharedMessages *getSharedMessages();

	delete m;
}

#endif
