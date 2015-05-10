#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeEvents.h>

TEST(NativeEvents, Simple)
{
	NativeEvents *e;
	
	e = new NativeEvents();
	//@TODO: addListener
	//@TODO: removeListener
	//@TODO: fireEvent

	delete e;
}

