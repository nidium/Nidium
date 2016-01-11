#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeStreamInterface.h>


class DummyStream: public NativeBaseStream
{
public:
    int counter;
    DummyStream(const char * location): NativeBaseStream(location) {
        counter = 0;
        NativeBaseStream::create(location);
    }
    DummyStream(const NativePath &path): NativeBaseStream(path) {
        counter = 0;
    }
    void stop() {
        counter = 10;
    }
    void getContent() {
    }
    bool getContentSync(char ** data, size_t *len, bool mmap = false) {
        return true;
    }
    size_t getFileSize() const {
        return 0;
    }
    void seek(size_t pos) {
    }
    bool hasDataAvailable() const {
        return false;
    }
protected:
    const unsigned char * onGetNextPacket(size_t *len, int *err) {
        counter++;
        if (counter == 2) {
            this->error(NATIVESTREAM_ERROR_UNKNOWN, 666);
        }
        return NULL;

    }
    void onStart(size_t packets, size_t seek) {
        counter = 1;
    }
};

TEST(NativeStreamInterface, Base)
{
    const char * fn = "/dev/dummy";
    int error;
    size_t len;

    DummyStream nbs(fn);
    EXPECT_TRUE(strcmp(nbs.getLocation(), fn) == 0);
    EXPECT_EQ(nbs.counter, 0);
    nbs.start(1, 0);
    EXPECT_EQ(nbs.counter, 1);

    len = 10;
    error = 10;
    nbs.getNextPacket(&len, &error);
    EXPECT_EQ(nbs.counter, 2);
    EXPECT_EQ(error, 0);
    EXPECT_EQ(len, 0);

    class NativeMessages listerer;
    setListener(listener);
}

