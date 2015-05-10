CC=clang++
VALGRIND=valgrind

LD_FLAGS=-g -O2
CC_FLAGS=-g -O2 -Wall -Wextra 
#-DNATIVE_NO_PRIVATE_DIR

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

TEST_OBJECTS=unittest_0.o \
	unittest_utils.o \
	unittest_nfs.o \
	unittest_path.o \
	unittest_websocket.o \
	unittest_httpstream.o \
	unittest_args.o \
	unittest_hash.o \

TODO =\
	unittest_db.o \
	unittest_httplistener.o \
	unittest_http.o \
	unittest_filestream.o \
	unittest_file.o \
	unittest_taskmanager.o \
	unittest_streaminterface.o \
	unittest_sharedmessages.o \
	unittest_nfsstream.o \
	unittest_messages.o \
	unittest_jsexposer.o \
	unittest_jsmodules.o \
	unittest_jswebsocket.o \
	unittest_jsutils.o \
	unittest_Jsthread.o \
	unittest_jsstream.o \
	unittest_jssocket.o \
	unittest_jsprofiler.o \
	unittest_jsprocess.o \
	unittest_jshttplistener.o \
	unittest_jshttp.o \
	unittest_jsfs.o \
	unittest_jsfileio.o \
	unittest_jsdebug.o \
	unittest_jsconsole.o \
	unittest_js.o \

TEST_EXEC=run_unittests

unittest_db.o:			EXCEPTION = 

all: $(TEST_EXEC)

$(TEST_EXEC): $(TEST_OBJECTS)
	@echo "LD\t" $@
	@$(CC) $(LD_FLAGS) -o $@ $^ $(LIBS) $(GTEST_LIB)
	@echo "Run\t" $@
	$(VALGRIND) ./$@


unittest_%.o: unittest_%.cpp
	@echo "CC\t" $@
	@$(CC) $(CC_FLAGS) -o $@ -c $< $(EXCEPTION) $(INCLUDES)

.PHONY: clean

clean:
	@echo "rm\t" $@
	@rm -rf $(TEST_EXEC) $(TEST_OBJECTS)


