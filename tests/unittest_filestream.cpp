#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeFileStream.h>

TEST(NativeNFSStream, Simple)
{
	NativeFileStream fs("file://127.0.0.1:/tmp/tst.txt");
	EXPECT_TRUE(fs.getBaseDir() == NULL);
	EXPECT_TRUE(fs.allowSyncStream() == true);
	EXPECT_TRUE(fs.allowLocalFileStream() == true);
//@TODO: void stop();
//@TODO: void getContent();
//@TODO: void _getContent();
//@TODO: bool getContentSync(char **data, size_t *len, bool mmap = false);
//@TODO: size_t getFileSize() const;
//@TODO: void seek(size_t pos);
}

