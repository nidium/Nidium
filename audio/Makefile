CXX=~/.vim/bin/cc_args.py g++
PORTAUDIO=../../portaudio/
CPPFLAGS=-I./ -I$(PORTAUDIO)/src/common/ -c -g -Wall -D__STDC_CONSTANT_MACROS

AR=ar
ARFLAGS=rcs
SOURCES=NativeAudio.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=libnativeaudio.a

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS)
		$(AR) $(ARFLAGS) $@ $(OBJECTS) 

.cpp.o:
		$(CXX) $(CPPFLAGS) $< -o $@
		
clean:
		rm -f $(OBJECTS) $(EXECUTABLE)
