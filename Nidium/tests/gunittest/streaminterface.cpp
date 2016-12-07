/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <IO/Stream.h>

class DummyStream: public Nidium::IO::Stream
{
public:
    int counter;
    DummyStream(const char * location): Nidium::IO::Stream(location) {
        counter = 0;
        Nidium::IO::Stream::Create(location);
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
            this->error(Stream::kErrors_Unknown, 666);
        }
        return NULL;

    }
    void onStart(size_t packets, size_t seek) {
        counter = 1;
    }
};

TEST(StreamInterface, Base)
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

    //class Nidium::Core::Messages listerer;
    //setListener(listener);
}

