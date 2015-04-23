CC=clang
VALGRIND=valgrind

CC_FLAGS=-Wall -Wextra
LD_FLAGS=-lstdc++ -lpthread -lz

GTEST_DIR=../../third-party/.default/angle/third_party/googletest
GTEST_LIB=$(GTEST_DIR)/libgtest.a $(GTEST_DIR)/libgtest_main.a

INCLUDES= \
        -isystem../../third-party/.default/c-ares \
        -isystem../../third-party/mozilla-central/js/src/dist/include \
        -isystem../../third-party/mozilla-central/js/src \
        -isystem../../third-party/mozilla-central/nsprpub/dist/include/nspr \
        -isystem../../third-party/leveldb/include \
        -isystem../../third-party/http-parser \
        -isystem../../nativejscore \
        -isystem../../nativejscore/network \
        -isystem../../third-party/c-ares \
        -isystem../../third-party/openssl/include \
        -isystem../../nativejscore/external/json \
        -isystem../../nativejscore/network \
        -I../ \
        -isystem $(GTEST_DIR)/include/
LIBS=../../build/out/Release/obj.target/nativejscore/network/gyp/libnativenetwork.a \
        ../../build/out/Release/obj.target/nativejscore/gyp/libnativejscore.a

TEST_OBJECTS=unittest_utils.o 
	
TEST_EXEC=run_unittests

all: $(TEST_EXEC)

$(TEST_EXEC): $(TEST_OBJECTS)
	@echo "LD\t" $@
	@$(CC) $(LD_FLAGS) -o $@ $(LIBS) $^ $(GTEST_LIB)
	@echo "Run\t" $@
	$(VALGRIND) ./$@

unittest_%.o: unittest_%.cpp
	@echo "CC\t" $@
	@$(CC) $(CC_FLAGS) -o $@ -c $< $(INCLUDES)

.PHONY: clean

clean:
	@echo "rm\t" $@
	@rm -rf $(TEST_EXEC) $(TEST_OBJECTS)

