NATIVEJSCORE_SRC=../NativeStudio/nativejscore
NATIVE_LIBS_DIR=../NativeStudio/out/third-party-libs/release

CC=clang++
CFLAGS=-O2 -c -g -Wall -Wno-c++11-extensions -Wno-invalid-offsetof -DNATIVE_NO_PRIVATE_DIR=1
INC=-I$(NATIVEJSCORE_SRC) -I$(NATIVEJSCORE_SRC)/network -I$(NATIVEJSCORE_SRC)/../third-party/mozilla-central/js/src/dist/include/ -I$(NATIVEJSCORE_SRC)/../interface/

SOURCES=\
	./src/native_main.cpp \
	./src/NativeServer.cpp

LDFLAGS= \
	$(NATIVEJSCORE_SRC)/gyp/build/Release/libnativejscore.a \
	$(NATIVEJSCORE_SRC)/network/gyp/build/Release/libnativenetwork.a \
	$(NATIVE_LIBS_DIR)/libjs_static.a \
	$(NATIVE_LIBS_DIR)/libleveldb.a \
	$(NATIVE_LIBS_DIR)/libnspr4.a \
	$(NATIVE_LIBS_DIR)/libcares.a \
	$(NATIVE_LIBS_DIR)/libhttp_parser.a \
	$(NATIVE_LIBS_DIR)/libjsoncpp.a \

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=native_server

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS)
		@$(CC) $(OBJECTS) -o $@ $(LDFLAGS) -lz -lobjc -framework Cocoa

.cpp.o:
		$(CC) $(INC) $(CFLAGS) $< -o $@
.mm.o:
		$(CC) $(INC) $(CFLAGS) $< -o $@
		
clean:
		rm -f $(OBJECTS) $(EXECUTABLE)