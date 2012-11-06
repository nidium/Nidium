CXX=~/.vim/bin/cc_args.py g++
PORTAUDIO=../../portaudio/
LIBFSRC=../../libfsrc/libfsrc/
CPPFLAGS=-I./ -I../src/ -I$(PORTAUDIO)/src/common/ -I$(LIBFSRC) -c -O2 -g -Wall -D__STDC_CONSTANT_MACROS

AR=ar
ARFLAGS=rcs
SOURCES=NativeAudioNode.cpp NativeAudio.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=libnativeaudio.a

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS)
		$(AR) $(ARFLAGS) $@ $(OBJECTS) 

.cpp.o:
		$(CXX) $(CPPFLAGS) $< -o $@
		
clean:
		rm -f $(OBJECTS) $(EXECUTABLE)
