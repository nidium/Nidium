CC=clang++
VALGRIND=valgrind

CC_FLAGS=-g -O2 -Wall -Wextra
LD_FLAGS=-g -O2

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

LIBS = \
	-Wl,--start-group \
		-Wl,-Bdynamic \
		-L../../build/out/Release/obj.target/gyp/ -lnativeinterface \
		-L../../build/out/Release/obj.target/gyp/ -ltranslator_glsl \
		-L../../build/out/Release/obj.target/gyp/ -lnativeav \
		-L../../build/out/Release/obj.target/gyp/ -lnativestudio \
		-L../../build/out/Release/obj.target/gyp/ -ltranslator_common \
		-L../../build/out/Release/obj.target/gyp/ -lpreprocessor \
		-L../../build/out/Release/obj.target/nativejscore/gyp/ -lnativejscore \
		-L../../build/out/Release/obj.target/nativejscore/gyp/ -ljsoncpp \
		-L../../build/out/Release/obj.target/nativejscore/network/gyp/ -lnativenetwork \
	-Wl,--end-group \
	-Wl,--start-group \
		-Wl,-Bdynamic \
		-L../../build/third-party/.default \
		-lavutil \
		-lbasekit \
		-lcares \
		-lcoroutine \
		-lhttp_parser \
		-lleveldb \
		-lportaudio \
		-lSDL2 \
		-lssl -lcrypto \
		-ljs_static -lnspr4 \
		-lavcodec -lavformat \
		-lswresample -lswscale \
		-lskia_core -lskia_effects -lskia_images -lskia_opts -lskia_opts_ssse3 -lskia_pdf -lskia_ports -lskia_sfnt -lskia_skgpu -lskia_utils \
		-lzip \
		-lzita-resampler \
	-Wl,--end-group \
	-Wl,-Bdynamic \
		-lstdc++ -lpthread -lz -lrt -ldl

TEST_OBJECTS=unittest_utils.o 
	
TEST_EXEC=run_unittests

all: $(TEST_EXEC)

$(TEST_EXEC): $(TEST_OBJECTS)
	@echo "LD\t" $@
	@$(CC) $(LD_FLAGS) -o $@ $^ $(LIBS) $(GTEST_LIB)
	@echo "Run\t" $@
	$(VALGRIND) ./$@

unittest_%.o: unittest_%.cpp
	@echo "CC\t" $@
	@$(CC) $(CC_FLAGS) -o $@ -c $< $(INCLUDES)

.PHONY: clean

clean:
	@echo "rm\t" $@
	@rm -rf $(TEST_EXEC) $(TEST_OBJECTS)

