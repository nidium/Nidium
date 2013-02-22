NATIVEFLAGS=-fvisibility=hidden -fvisibility-inlines-hidden -Wno-invalid-offsetof

CXX=clang
CPPFLAGS=-O2 $(NATIVEFLAGS) -c -g -O2 -Wall -I../src/ -I./ -I../network/ -I../third-party/jsoncpp/include -I../third-party/libzip/lib

AR=ar
ARFLAGS=rcs

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
VPATH = ./linux/
OBJECTS=$(SOURCES:.cpp=.o)
SOURCES=NativeUIInterface.cpp 
CPPFLAGS += -I../third-party/SDL/include/
else
VPATH = ./osx/
OBJECTS=$(SOURCES:.mm=.o)
SOURCES=NativeUIInterface.mm NativeUIConsole.mm
endif

EXECUTABLE=libnativeinterface.a

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS)
		$(AR) $(ARFLAGS) $@ $(OBJECTS) 

%.o: %.cpp
		$(CXX) $(CPPFLAGS) $< -o $@
		
clean:
		rm -f $(OBJECTS) $(EXECUTABLE)
