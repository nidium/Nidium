CC=clang++
CFLAGS= -O2 -fvisibility=hidden -c -g -Wall -Wno-c++11-extensions -Wno-invalid-offsetof
INC=-I./ -I../network/ -I../third-party/c-ares/ -I../third-party/http-parser/ -I../third-party/mozilla-central/js/src/dist/include/
AR=ar
ARFLAGS=rcs
SOURCES=NativeFileIO.cpp NativeHTTP.cpp NativeJS.cpp NativeJSExposer.cpp NativeJSFileIO.cpp NativeJSHttp.cpp NativeJSModules.cpp NativeJSSocket.cpp NativeJSThread.cpp NativeSharedMessages.cpp NativeStream.cpp NativeUtils.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=libnativejscore.a

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS)
		$(AR) $(ARFLAGS) $@ $(OBJECTS) 

.cpp.o:
		$(CC) $(INC) $(CFLAGS) $< -o $@
		
clean:
		rm -f $(OBJECTS) $(EXECUTABLE)
