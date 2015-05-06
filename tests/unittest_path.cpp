#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeFileStream.h>
#include <NativeHTTPStream.h>
#include <NativePath.h>

TEST(NativePath, Nulling)
{
	NativePath path(NULL);
	EXPECT_TRUE(path.path() == NULL);
}

TEST(NativePath, Simple)
{
	NativePath::registerScheme(SCHEME_DEFINE("file://", NativeFileStream, false), true);
	NativePath::registerScheme(SCHEME_DEFINE("http://",    NativeHTTPStream,    true));

	const char *p ="file:///tmp/t.txt";
	NativePath path(p);
printf("\n\n%s\n\n\n", path.dir());
	EXPECT_TRUE(strlen(path.path()) > strlen(p));
//	EXPECT_TRUE(strcmp(path.dir(), "/tmp/") == 0);
	EXPECT_TRUE(strcmp(path.getScheme()->str, "file://") == 0);

	//delete path;
}

