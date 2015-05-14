#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "unittest.h"

#include <NativeFile.h>

#if 0
TEST(NativeFile, Simple)
{
	size_t fs;
	NativeFile nf(__FILE__); 
	NativeFile *dup = NULL;

	fs = nf.getFileSize();
	EXPECT_TRUE(fs == 0);
	EXPECT_TRUE(nf.getFd() == NULL);
	EXPECT_TRUE(nf.isDir() == false);
	EXPECT_TRUE(nf.isOpen() == false);

	nf.open("r", NULL);
	EXPECT_TRUE(nf.isOpen() == true);
	EXPECT_TRUE(nf.getFd() != NULL);
	EXPECT_TRUE(nf.eof() == false);
	fs = nf.getFileSize();
	EXPECT_TRUE(fs > 0);

	nf.read(fs, NULL);
	EXPECT_TRUE(nf.eof() == true);
	
	printf("%s", nf.getFullPath());

	dup = nf.dup();
	EXPECT_TRUE(&nf != dup);
	EXPECT_TRUE(dup != NULL);
	EXPECT_TRUE(dup->isOpen() == true);

	nf.close();
	EXPECT_TRUE(nf.isOpen() == false);
	EXPECT_TRUE(nf.getFd() == NULL);

	EXPECT_TRUE(nf.isOpen() == false);
	EXPECT_TRUE(nf.getFd() == NULL);
	EXPECT_TRUE(dup->isOpen() == true);

}

TEST(NativeFile, Sync)
{
	dirent *dir;

	NativeFile nf("/tmp");
	EXPECT_TRUE(nf.isDir() == false);
	printf("%s", nf.getFullPath());

	nf.open("r", NULL);
	EXPECT_TRUE(nf.isDir() == true);

	//dir = nf.lst;
	//EXPECT_TRUE(dir == NULL);
	//nf.listFiles();
	//dir = nf.lst;
	//EXPECT_TRUE(dir != NULL);

	nf.close();
	
	//closeSync
	//seekSync
	//writeSync
	//mmapSync
	//readSync
	//openSync
	
}

TEST(NativeFile, Dir)
{
	//getDir
	//getFullPath
	//listFiles
}
//@TODO: onMessage
//@TODO: onMessageLost
//@TODO: listFilesTask
//@TODO: setListener
//@TODO: setAutoClose
//@TODO: seekTask
//@TODO: writeTask
//@TODO: readTask
//@TODO: closeTask
//@TODO: openTask
//@TODO: rmrf
//@TODO: seek
//@TODO: write
#endif
