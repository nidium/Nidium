NATIVEFLAGS=-fvisibility=hidden -fvisibility-inlines-hidden -Wno-invalid-offsetof

CXX = clang
CPPFLAGS = -O2 $(NATIVEFLAGS) -c -g -O2 -Wall -I../src/ -I./ -I../network/

VPATH = ./osx/

AR=ar
ARFLAGS=rcs
SOURCES=NativeUIInterface.mm

OBJECTS=$(SOURCES:.mm=.o)
EXECUTABLE=libnativeinterface.a

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS)
		$(AR) $(ARFLAGS) $@ $(OBJECTS) 

%.o: %.mm
		$(CXX) $(CPPFLAGS) $< -o $@
		
clean:
		rm -f $(OBJECTS) $(EXECUTABLE)
